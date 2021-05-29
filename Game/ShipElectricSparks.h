/***************************************************************************************
* Original Author:		Gabriele Giuseppini
* Created:				2021-05-29
* Copyright:			Gabriele Giuseppini  (https://github.com/GabrieleGiuseppini)
***************************************************************************************/
#pragma once

#include "RenderContext.h"

#include <GameCore/Buffer.h>
#include <GameCore/BufferAllocator.h>
#include <GameCore/Vectors.h>

#include <vector>

namespace Physics
{

class ShipElectricSparks
{
public:

    ShipElectricSparks(
        Points const & points,
        Springs const & springs);

    bool ApplySparkAt(
        vec2f const & targetPos,
        std::uint64_t counter,
        float progress,
        // TODO: see if both needed
        Points const & points,
        Springs const & springs,
        GameParameters const & gameParameters);

    void Update();

    void Upload(
        Points const & points,
        ShipId shipId,
        Render::RenderContext & renderContext) const;

private:

    void PropagateSparks(
        ElementIndex startingPointIndex,
        std::uint64_t counter,
        float progress,
        // TODO: see if both needed
        Points const & points,
        Springs const & springs);

private:

    // Flag remembering whether a spring is electrified or not;
    // cardinality=springs
    Buffer<bool> mIsSpringElectrified;
    Buffer<bool> mIsSpringElectrifiedBackup;

    // Flag remembering whether electric sparks have been populated prior to the next Update() step
    bool mAreSparksPopulatedBeforeNextUpdate;

    //
    // Rendering
    //

    struct ElectricSpark
    {
        ElementIndex StartPointIndex;
        float StartSize;
        ElementIndex EndPointIndex;
        float EndSize;

        ElectricSpark(
            ElementIndex startPointIndex,
            float startSize,
            ElementIndex endPointIndex,
            float endSize)
            : StartPointIndex(startPointIndex)
            , StartSize(startSize)
            , EndPointIndex(endPointIndex)
            , EndSize(endSize)
        {}
    };

    // The electric sparks
    std::vector<ElectricSpark> mSparksToRender;
};

}