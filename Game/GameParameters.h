
/***************************************************************************************
* Original Author:      Gabriele Giuseppini
* Created:              2018-01-19
* Copyright:            Gabriele Giuseppini  (https://github.com/GabrieleGiuseppini)
***************************************************************************************/
#pragma once

#include <GameCore/GameTypes.h>
#include <GameCore/Vectors.h>

#include <chrono>

/*
 * Parameters that affect the game (physics, world).
 */
struct GameParameters
{
    GameParameters();

    //
    // The dt of each step
    //

    template <typename T>
    static constexpr T SimulationStepTimeDuration = 0.02f;

    template <typename T>
    inline T MechanicalSimulationStepTimeDuration() const
    {
        return MechanicalSimulationStepTimeDuration(NumMechanicalDynamicsIterations<T>());
    }

    template <typename T>
    static inline T MechanicalSimulationStepTimeDuration(T numMechanicalDynamicsIterations)
    {
        return SimulationStepTimeDuration<T> / numMechanicalDynamicsIterations;
    }


    //
    // The low-frequency update dt
    //

    template <typename T>
    static constexpr T LowFrequencySimulationStepTimeDuration = 1.0f;


    //
    // Physical Constants
    //

    // Gravity
    static constexpr vec2f Gravity = vec2f(0.0f, -9.80f);
    static constexpr vec2f GravityNormalized = vec2f(0.0f, -1.0f);
    static float constexpr GravityMagnitude = 9.80f; // m/s

    // Air
    static float constexpr AirMass = 1.2754f; // Kg

    // Water
    static float constexpr WaterMass = 1000.0f; // Kg



    //
    // Tunable parameters
    //

    // Dynamics

    // Fraction of a spring displacement that is removed during a spring relaxation
    // iteration. The remaining spring displacement is (1.0 - this fraction).
    static float constexpr SpringReductionFraction = 0.4f;

    // The empirically-determined constant for the spring damping.
    // The simulation is quite sensitive to this value:
    // - 0.03 is almost fine (though bodies are sometimes soft)
    // - 0.8 makes everything explode
    static float constexpr SpringDampingCoefficient = 0.03f;

    //
    // The number of mechanical iterations dictates how stiff bodies are:
    // - Less iterations => softer (jelly) body
    // - More iterations => hard body (never breaks though)
    //

    float NumMechanicalDynamicsIterationsAdjustment;
    static float constexpr MinNumMechanicalDynamicsIterationsAdjustment = 0.5f;
    static float constexpr MaxNumMechanicalDynamicsIterationsAdjustment = 20.0f;

    template <typename T>
    inline T NumMechanicalDynamicsIterations() const
    {
        return static_cast<T>(
            static_cast<float>(BasisNumMechanicalDynamicsIterations)
            * NumMechanicalDynamicsIterationsAdjustment);
    }

    float SpringStiffnessAdjustment;
    static float constexpr MinSpringStiffnessAdjustment = 0.001f;
    static float constexpr MaxSpringStiffnessAdjustment = 2.4f;

    float SpringDampingAdjustment;
    static float constexpr MinSpringDampingAdjustment = 0.001f;
    static float constexpr MaxSpringDampingAdjustment = 4.0f;

    float SpringStrengthAdjustment;
    static float constexpr MinSpringStrengthAdjustment = 0.01f;
    static float constexpr MaxSpringStrengthAdjustment = 50.0f;

    static float constexpr GlobalDamp = 0.9996f; // // We've shipped 1.7.5 with 0.9997, but splinter springs used to dance for too long

    float RotAcceler8r;
    static float constexpr MinRotAcceler8r = 0.0f;
    static float constexpr MaxRotAcceler8r = 1000.0f;

    // Water

    float WaterDensityAdjustment;
    static float constexpr MinWaterDensityAdjustment = 0.0f;
    static float constexpr MaxWaterDensityAdjustment = 4.0f;

    static float constexpr WaterDragLinearCoefficient =
        0.020f  // ~= 1.0f - powf(0.6f, 0.02f)
        * 5.0f;  // Once we were comfortable with square law at |v|=5, now we use linear law and want to maintain the same force there

    float WaterDragAdjustment;
    static float constexpr MinWaterDragAdjustment = 0.0f;
    static float constexpr MaxWaterDragAdjustment = 1000.0f; // Safe to avoid drag instability (2 * m / (dt * C) at minimal mass, 1Kg)

    float WaterIntakeAdjustment;
    static float constexpr MinWaterIntakeAdjustment = 0.001f;
    static float constexpr MaxWaterIntakeAdjustment = 10.0f;

    float WaterDiffusionSpeedAdjustment;
    static float constexpr MinWaterDiffusionSpeedAdjustment = 0.001f;
    static float constexpr MaxWaterDiffusionSpeedAdjustment = 2.0f;

    float WaterCrazyness;
    static float constexpr MinWaterCrazyness = 0.0f;
    static float constexpr MaxWaterCrazyness = 2.0f;

    // Ephemeral particles

    static constexpr ElementCount MaxEphemeralParticles = 4096;

    bool DoGenerateDebris;
    static constexpr unsigned int MinDebrisParticlesPerEvent = 4;
    static constexpr unsigned int MaxDebrisParticlesPerEvent = 9;
    static float constexpr MinDebrisParticlesVelocity = 12.5f;
    static float constexpr MaxDebrisParticlesVelocity = 20.0f;
    static constexpr std::chrono::milliseconds MinDebrisParticlesLifetime = std::chrono::milliseconds(400);
    static constexpr std::chrono::milliseconds MaxDebrisParticlesLifetime = std::chrono::milliseconds(900);

    bool DoGenerateSparkles;
    static constexpr unsigned int MinSparkleParticlesPerEvent = 4;
    static constexpr unsigned int MaxSparkleParticlesPerEvent = 10;
    static float constexpr MinSparkleParticlesVelocity = 75.0f;
    static float constexpr MaxSparkleParticlesVelocity = 150.0f;
    static constexpr std::chrono::milliseconds MinSparkleParticlesLifetime = std::chrono::milliseconds(200);
    static constexpr std::chrono::milliseconds MaxSparkleParticlesLifetime = std::chrono::milliseconds(500);

    bool DoGenerateAirBubbles;
    float CumulatedIntakenWaterThresholdForAirBubbles;
    static float constexpr MinCumulatedIntakenWaterThresholdForAirBubbles = 2.0f;
    static float constexpr MaxCumulatedIntakenWaterThresholdForAirBubbles = 128.0f;
    static float constexpr MinAirBubblesVortexAmplitude = 0.05f;
    static float constexpr MaxAirBubblesVortexAmplitude = 2.0f;
    static float constexpr MinAirBubblesVortexPeriod = 3.0f; // seconds
    static float constexpr MaxAirBubblesVortexPeriod = 7.5f; // seconds

    // Wind

    static constexpr vec2f WindDirection = vec2f(1.0f, 0.0f);

    bool DoModulateWind;

    float WindSpeedBase; // Beaufort scale, km/h
    static float constexpr MinWindSpeedBase = -100.f;
    static float constexpr MaxWindSpeedBase = 100.0f;

    float WindSpeedMaxFactor; // Multiplier on base
    static float constexpr MinWindSpeedMaxFactor = 1.f;
    static float constexpr MaxWindSpeedMaxFactor = 10.0f;

    float WindGustFrequencyAdjustment;
    static float constexpr MinWindGustFrequencyAdjustment = 0.1f;
    static float constexpr MaxWindGustFrequencyAdjustment = 10.0f;

    // Waves

    float BasalWaveHeightAdjustment;
    static float constexpr MinBasalWaveHeightAdjustment = 0.0f;
    static float constexpr MaxBasalWaveHeightAdjustment = 100.0f;

    float BasalWaveLengthAdjustment;
    static float constexpr MinBasalWaveLengthAdjustment = 0.3f;
    static float constexpr MaxBasalWaveLengthAdjustment = 20.0f;

    float BasalWaveSpeedAdjustment;
    static float constexpr MinBasalWaveSpeedAdjustment = 0.75f;
    static float constexpr MaxBasalWaveSpeedAdjustment = 20.0f;

    float TsunamiRate; // Minutes
    static float constexpr MinTsunamiRate = 0.0f;
    static float constexpr MaxTsunamiRate = 60.0f;

    float RogueWaveRate; // Minutes
    static float constexpr MinRogueWaveRate = 0.0f;
    static float constexpr MaxRogueWaveRate = 15.0f;

    // Storm

    std::chrono::seconds StormDuration;

	float StormMaxWindSpeed;
	static float constexpr MinStormMaxWindSpeed = 35.0f;
	static float constexpr MaxStormMaxWindSpeed = 80.0f;

	float LightningDuration; // Seconds
	static float constexpr MinLightningDuration = 0.1f;
	static float constexpr MaxLightningDuration = 5.0f;

	bool DoRainWithStorm;

    // Heat and combustion

    static float constexpr InitialTemperature = 298.15f; // 25C

    float AirTemperature; // Kelvin
    static float constexpr MinAirTemperature = 273.15f; // 0C
    static float constexpr MaxAirTemperature = 2073.15f; // 1800C

    static float constexpr AirConvectiveHeatTransferCoefficient = 100.45f; // J/(s*m2*K) - arbitrary, higher than real

    float WaterTemperature; // Kelvin
    static float constexpr MinWaterTemperature = 273.15f; // 0C
    static float constexpr MaxWaterTemperature = 2073.15f; // 1800C

    static float constexpr WaterConvectiveHeatTransferCoefficient = 2500.0f; // J/(s*m2*K) - arbitrary, higher than real

    static float constexpr IgnitionTemperatureHighWatermark = 0.0f;
    static float constexpr IgnitionTemperatureLowWatermark = -30.0f;

    static float constexpr SmotheringWaterLowWatermark = 0.05f;
    static float constexpr SmotheringWaterHighWatermark = 0.1f;

    static float constexpr SmotheringDecayLowWatermark = 0.0005f;
    static float constexpr SmotheringDecayHighWatermark = 0.05f;

    unsigned int MaxBurningParticles;
    static unsigned int constexpr MaxMaxBurningParticles = 1000;
    static unsigned int constexpr MinMaxBurningParticles = 10;

    float ThermalConductivityAdjustment;
    static float constexpr MinThermalConductivityAdjustment = 0.1f;
    static float constexpr MaxThermalConductivityAdjustment = 100.0f;

    float HeatDissipationAdjustment;
    static float constexpr MinHeatDissipationAdjustment = 0.01f;
    static float constexpr MaxHeatDissipationAdjustment = 20.0f;

    float IgnitionTemperatureAdjustment;
    static float constexpr MinIgnitionTemperatureAdjustment = 0.1f;
    static float constexpr MaxIgnitionTemperatureAdjustment = 1000.0f;

    float MeltingTemperatureAdjustment;
    static float constexpr MinMeltingTemperatureAdjustment = 0.1f;
    static float constexpr MaxMeltingTemperatureAdjustment = 1000.0f;

    float CombustionSpeedAdjustment;
    static float constexpr MinCombustionSpeedAdjustment = 0.1f;
    static float constexpr MaxCombustionSpeedAdjustment = 100.0f;

    float CombustionHeatAdjustment;
    static float constexpr MinCombustionHeatAdjustment = 0.1f;
    static float constexpr MaxCombustionHeatAdjustment = 100.0f;

    float HeatBlasterHeatFlow; // KJoules/sec
    static float constexpr MinHeatBlasterHeatFlow = 200.0f;
    static float constexpr MaxHeatBlasterHeatFlow = 100000.0f;

    float HeatBlasterRadius;
    static float constexpr MinHeatBlasterRadius = 1.0f;
    static float constexpr MaxHeatBlasterRadius = 100.0f;

    float ElectricalElementHeatProducedAdjustment;
    static float constexpr MinElectricalElementHeatProducedAdjustment = 0.0f;
    static float constexpr MaxElectricalElementHeatProducedAdjustment = 1000.0f;

    // Misc

    float SeaDepth;
    static float constexpr MinSeaDepth = -50.0f;
    static float constexpr MaxSeaDepth = 10000.0f;

    // The number of ocean floor terrain samples for the entire world width;
    // a higher value means more resolution, at the expense of cache misses
    template <typename T>
    static T constexpr OceanFloorTerrainSamples = 2048;

    float OceanFloorBumpiness;
    static float constexpr MinOceanFloorBumpiness = 0.0f;
    static float constexpr MaxOceanFloorBumpiness = 6.0f;

    float OceanFloorDetailAmplification;
    static float constexpr MinOceanFloorDetailAmplification = 0.0f;
    static float constexpr MaxOceanFloorDetailAmplification = 200.0f;

    float LuminiscenceAdjustment;
    static float constexpr MinLuminiscenceAdjustment = 0.0f;
    static float constexpr MaxLuminiscenceAdjustment = 4.0f;

    float LightSpreadAdjustment;
    static float constexpr MinLightSpreadAdjustment = 0.0f;
    static float constexpr MaxLightSpreadAdjustment = 10.0f;

    unsigned int NumberOfStars;
    static constexpr unsigned int MinNumberOfStars = 0;
    static constexpr unsigned int MaxNumberOfStars = 10000;

    unsigned int NumberOfClouds;
    static constexpr unsigned int MinNumberOfClouds = 0;
    static constexpr unsigned int MaxNumberOfClouds = 500;

    // Interactions

    float ToolSearchRadius;

    float DestroyRadius;
    static float constexpr MinDestroyRadius = 5.0f;
    static float constexpr MaxDestroyRadius = 100.0f;

    float RepairRadius;
    static float constexpr MinRepairRadius = 0.1f;
    static float constexpr MaxRepairRadius = 10.0f;

    float RepairSpeedAdjustment;
    static float constexpr MinRepairSpeedAdjustment = 0.25f;
    static float constexpr MaxRepairSpeedAdjustment = 10.0f;

    static float constexpr DrawForce = 40000.0f;

    static float constexpr SwirlForce = 600.0f;

    float BombBlastRadius;
    static float constexpr MinBombBlastRadius = 0.1f;
    static float constexpr MaxBombBlastRadius = 20.0f;

    float BombBlastHeat; // KJoules/sec
    static float constexpr MinBombBlastHeat = 0.0f;
    static float constexpr MaxBombBlastHeat = 10000000.0f;

    float AntiMatterBombImplosionStrength;
    static float constexpr MinAntiMatterBombImplosionStrength = 0.1f;
    static float constexpr MaxAntiMatterBombImplosionStrength = 10.0f;

    static float constexpr BombNeighborhoodRadius = 3.5f;

    static float constexpr BombsTemperatureTrigger = 373.15f; // 100C

    std::chrono::seconds TimerBombInterval;

    float BombMass;

    float FloodRadius;
    static float constexpr MinFloodRadius = 0.1f;
    static float constexpr MaxFloodRadius = 10.0f;

    float FloodQuantity;
    static float constexpr MinFloodQuantity = 0.1f;
    static float constexpr MaxFloodQuantity = 100.0f;

    float FireExtinguisherRadius;

    float ScrubRadius;

    bool IsUltraViolentMode;

    float MoveToolInertia;

    //
    // Limits
    //

    static float constexpr MaxWorldWidth = 5000.0f;
    static float constexpr HalfMaxWorldWidth = MaxWorldWidth / 2.0f;

    static float constexpr MaxWorldHeight = 40000.0f;
    static float constexpr HalfMaxWorldHeight = MaxWorldHeight / 2.0f;

    static_assert(HalfMaxWorldHeight >= MaxSeaDepth);


    static constexpr size_t MaxBombs = 64u;
    static constexpr size_t MaxPinnedPoints = 64u;
    static constexpr size_t MaxThanosSnaps = 8u;

    static constexpr size_t MaxSpringsPerPoint = 8u + 1u; // 8 neighbours and 1 rope spring, when this is a rope endpoint
    static constexpr size_t MaxTrianglesPerPoint = 8u;

private:

    //
    // The basis number of iterations we run in the mechanical dynamics update for
    // each simulation step.
    //
    // The actual number of iterations is the product of this value with
    // MechanicalIterationsAdjust.
    //

    static constexpr size_t BasisNumMechanicalDynamicsIterations = 24;
};
