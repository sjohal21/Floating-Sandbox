﻿/***************************************************************************************
* Original Author:		Gabriele Giuseppini
* Created:				2020-10-21
* Copyright:			Gabriele Giuseppini  (https://github.com/GabrieleGiuseppini)
***************************************************************************************/
#include "Physics.h"

#include <GameCore/GameRandomEngine.h>
#include <GameCore/Utils.h>

#include <picojson.h>

#include <chrono>

namespace Physics {

float constexpr TurningThreshold = 7.0f;
float constexpr TurningTimeSeconds = 2.5f; // TODOTEST

Fishes::Fishes(FishSpeciesDatabase const & fishSpeciesDatabase)
    : mFishSpeciesDatabase(fishSpeciesDatabase)
    , mFishes()
{
}

void Fishes::Update(
    float currentSimulationTime,
    GameParameters const & gameParameters,
    VisibleWorld const & visibleWorld)
{
    //
    // 1) Update number of fish
    //

    if (mFishes.size() > gameParameters.NumberOfFishes)
    {
        // Remove extra fish
        mFishes.erase(
            mFishes.begin() + gameParameters.NumberOfFishes,
            mFishes.end());
    }
    else
    {
        // Add missing fish
        for (size_t f = mFishes.size(); f < gameParameters.NumberOfFishes; ++f)
        {
            // Choose species
            size_t const speciesIndex = GameRandomEngine::GetInstance().Choose(mFishSpeciesDatabase.GetFishSpecies().size());
            FishSpecies const & species = mFishSpeciesDatabase.GetFishSpecies()[speciesIndex];

            //
            // Choose initial and target position
            //

            vec2f const initialPosition = ChooseTargetPosition(species, visibleWorld);
            vec2f const targetPosition = CalculateNewCruisingTargetPosition(initialPosition, species, visibleWorld);

            float const personalitySeed = GameRandomEngine::GetInstance().GenerateNormalizedUniformReal();

            mFishes.emplace_back(
                &species,
                static_cast<TextureFrameIndex>(speciesIndex),
                personalitySeed,
                StateType::Cruising,
                initialPosition,
                targetPosition,
                CalculateVelocity(initialPosition, targetPosition, species, 1.0f, personalitySeed),
                GameRandomEngine::GetInstance().GenerateUniformReal(0.0f, 2.0f * Pi<float>)); // initial progress phase
        }
    }

    //
    // 2) Update fish
    //

    float constexpr BasalSpeedToProgressPhaseSpeedFactor =
        40.0f // Magic, from observation
        * GameParameters::SimulationStepTimeDuration<float>;

    for (auto & fish : mFishes)
    {
        switch (fish.CurrentState)
        {
            case StateType::Cruising:
            {
                //
                // Check whether we should start turning
                //

                if (std::abs(fish.CurrentPosition.x - fish.TargetPosition.x) < TurningThreshold)
                {
                    //
                    // Transition to Turning
                    //

                    // Change state
                    fish.CurrentState = StateType::Turning;

                    // Choose new target position
                    fish.TargetPosition = CalculateNewCruisingTargetPosition(fish.CurrentPosition, *(fish.Species), visibleWorld);

                    // Calculate new target velocity and direction
                    fish.StartVelocity = fish.CurrentVelocity;
                    fish.TargetVelocity = CalculateVelocity(fish.CurrentPosition, fish.TargetPosition, *(fish.Species), 1.0f, fish.PersonalitySeed);
                    fish.StartDirection = fish.CurrentDirection;
                    fish.TargetDirection = fish.TargetVelocity.normalise();

                    // Remember turn starting time
                    fish.TurningSimulationTimeStart = currentSimulationTime;
                }
                else
                {
                    //
                    // Normal dynamics
                    //

                    // Update position: add velocity, with superimposed sin component
                    fish.CurrentPosition +=
                        fish.CurrentVelocity
                        + fish.CurrentVelocity.normalise() * (1.0f + std::sin(2.0f * fish.CurrentProgressPhase + Pi<float> / 2.0f)) / 100.0f;

                    // Update progress phase: add basal speed
                    fish.CurrentProgressPhase += fish.Species->BasalSpeed * BasalSpeedToProgressPhaseSpeedFactor;
                }

                break;
            }

            case StateType::Turning:
            {
                //
                // Check whether we should stop turning
                //

                if (currentSimulationTime - fish.TurningSimulationTimeStart >= TurningTimeSeconds)
                {
                    //
                    // Transition to Cruising
                    //

                    // Change state
                    fish.CurrentState = StateType::Cruising;

                    // Reach all target quantities
                    fish.CurrentVelocity = fish.TargetVelocity;
                    fish.CurrentDirection = fish.TargetDirection;
                }
                else
                {
                    //
                    // Turning dynamics
                    //
                    // |      Velocity -> 0        |      Velocity -> Target      |
                    // |  DirY -> 0  |                          |  DirY -> Target |
                    // |        |            DirX -> Target             |         |
                    //

                    float const elapsedFraction = (currentSimulationTime - fish.TurningSimulationTimeStart) / TurningTimeSeconds;

                    // Velocity:
                    // - smooth towards zero during first half
                    // - smooth towards target during second half
                    if (elapsedFraction <= 0.5f)
                    {
                        fish.CurrentVelocity = fish.StartVelocity * (1.0f - elapsedFraction / 0.5f);
                    }
                    else
                    {
                        fish.CurrentVelocity = fish.TargetVelocity * 1.0f/0.5f * (elapsedFraction - 0.5f);
                    }

                    // Direction Y:
                    // - smooth towards zero during an initial interval
                    // - smooth towards target during a second interval
                    if (elapsedFraction <= 0.30f)
                    {
                        fish.CurrentDirection.y = fish.StartDirection.y * (1.0f - elapsedFraction / 0.30f);
                    }
                    else if (elapsedFraction >= 0.70f)
                    {
                        fish.CurrentDirection.y = fish.TargetDirection.y * 1.0f / 0.30f * (elapsedFraction - 0.70f);
                    }

                    // Direction X:
                    // - smooth towards target during a central interval
                    // - smooth towards target during a second interval
                    if (elapsedFraction >= 0.15f && elapsedFraction <= 0.85f)
                    {
                        fish.CurrentDirection.x = fish.StartDirection.x + (fish.TargetDirection.x - fish.StartDirection.x) / 0.70f * (elapsedFraction - 0.15f);
                    }

                    //
                    // Normal dynamics
                    //

                    // Update position: add velocity
                    fish.CurrentPosition += fish.CurrentVelocity;

                    // Update progress phase: add basal speed
                    fish.CurrentProgressPhase += fish.Species->BasalSpeed * BasalSpeedToProgressPhaseSpeedFactor;
                }

                break;

            }
        }

        //
        // Update current progress
        //

        fish.CurrentProgress = std::sin(fish.CurrentProgressPhase);
    }
}

void Fishes::Upload(Render::RenderContext & renderContext) const
{
    renderContext.UploadFishesStart(mFishes.size());

    for (auto const & fish : mFishes)
    {
        float angleCw = fish.CurrentDirection.angleCw();
        float horizontalScale = fish.CurrentDirection.length();

        if (angleCw < -Pi<float> / 2.0f)
        {
            angleCw = Pi<float> + angleCw;
            //angleCw *= -1.0f;
            horizontalScale *= -1.0f;
        }
        else if (angleCw > Pi<float> / 2.0f)
        {
            angleCw = -Pi<float> + angleCw;
            //angleCw *= -1.0f;
            horizontalScale *= -1.0f;
        }

        renderContext.UploadFish(
            TextureFrameId<Render::FishTextureGroups>(Render::FishTextureGroups::Fish, fish.RenderFrameIndex),
            fish.CurrentPosition,
            angleCw,
            horizontalScale,
            fish.Species->TailX,
            fish.CurrentProgress);
    }

    renderContext.UploadFishesEnd();
}

////////////////////////////////////////////////////////////////////////////////////////////////

vec2f Fishes::ChooseTargetPosition(
    FishSpecies const & fishSpecies,
    VisibleWorld const & visibleWorld)
{
    //static vec2f todo = vec2f(0.0f, 0.0f);
    static vec2f todo = vec2f(-20.0f, -20.0f);

    if (todo == vec2f(0.0f, 0.0f))
    {
        todo = vec2f(-20.0f, -20.0f);
    }
    else if (todo == vec2f(-20.0f, -20.0f))
    {
        todo = vec2f(0.0f, -40.0f);
    }
    else if (todo == vec2f(0.0f, -40.0f))
    {
        todo = vec2f(20.0f, -20.0f);
    }
    else if (todo == vec2f(20.0f, -20.0f))
    {
        todo = vec2f(0.0f, 0.0f);
    }

    return todo;
    // TODOTEST
    /*
    float const x = GameRandomEngine::GetInstance().GenerateNormalReal(visibleWorld.Center.x, visibleWorld.Width);

    float const y =
        -5.0f // Min depth
        - std::fabs(GameRandomEngine::GetInstance().GenerateNormalReal(fishSpecies.BasalDepth, 15.0f));

    return vec2f(x, y);
    */
}

vec2f Fishes::CalculateNewCruisingTargetPosition(
    vec2f const & currentPosition,
    FishSpecies const & species,
    VisibleWorld const & visibleWorld)
{
    // TODOTEST
    return ChooseTargetPosition(species, visibleWorld);

    // TODO: opposite side of screen, at least at minimum distance

    float const x = GameRandomEngine::GetInstance().GenerateNormalReal(visibleWorld.Center.x, visibleWorld.Width);

    float const y =
        -5.0f // Min depth
        - std::fabs(GameRandomEngine::GetInstance().GenerateNormalReal(species.BasalDepth, 15.0f));

    return vec2f(x, y);
}

vec2f Fishes::CalculateVelocity(
    vec2f const & startPosition,
    vec2f const & endPosition,
    FishSpecies const & species,
    float velocityMultiplier,
    float personalitySeed)
{
    return (endPosition - startPosition).normalise()
        * (species.BasalSpeed * velocityMultiplier * (0.7f + personalitySeed * 0.3f));
}

}