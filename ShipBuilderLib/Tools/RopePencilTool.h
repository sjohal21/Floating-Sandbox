/***************************************************************************************
* Original Author:		Gabriele Giuseppini
* Created:				2021-11-13
* Copyright:			Gabriele Giuseppini  (https://github.com/GabrieleGiuseppini)
***************************************************************************************/
#pragma once

#include "Tool.h"

#include <Game/Layers.h>
#include <Game/Materials.h>
#include <Game/ResourceLocator.h>

#include <GameCore/GameTypes.h>

#include <memory>
#include <optional>

namespace ShipBuilder {

class RopePencilTool : public Tool
{
public:

    RopePencilTool(
        ModelController & modelController,
        UndoStack & undoStack,
        WorkbenchState const & workbenchState,
        IUserInterface & userInterface,
        View & view,
        ResourceLocator const & resourceLocator);

    ~RopePencilTool();

    void OnMouseMove(DisplayLogicalCoordinates const & mouseCoordinates) override;
    void OnLeftMouseDown() override;
    void OnLeftMouseUp() override;
    void OnRightMouseDown() override;
    void OnRightMouseUp() override;
    void OnShiftKeyDown() override {}
    void OnShiftKeyUp() override {}

private:

    void CheckEngagement(
        ShipSpaceCoordinates const & coords,
        MaterialPlaneType materialPlane);

    void DoTempVisualization(ShipSpaceCoordinates const & coords);

    void MendTempVisualization();

    void CommmitAndStopEngagement(ShipSpaceCoordinates const & coords);

    void DrawOverlay(ShipSpaceCoordinates const & coords);

    void HideOverlay();

    inline StructuralMaterial const * GetMaterial(MaterialPlaneType plane) const;

private:

    // True when we have temp visualization
    bool mHasTempVisualization;

    // True when we have uploaded an overlay
    bool mHasOverlay;

    struct EngagementData
    {
        // Original layer
        RopesLayerData OriginalLayerClone;

        // Original dirty state
        Model::DirtyState OriginalDirtyState;

        // Rope start position
        ShipSpaceCoordinates StartCoords;

        // Rope element index (if moving a rope endpoint), nullopt otherwise (if creating a rope)
        std::optional<size_t> ExistingRopeElementIndex;

        // Plane of the engagement
        MaterialPlaneType Plane;

        EngagementData(
            RopesLayerData && originalLayerClone,
            Model::DirtyState const & dirtyState,
            ShipSpaceCoordinates const & startCoords,
            std::optional<size_t> const & existingRopeElementIndex,
            MaterialPlaneType plane)
            : OriginalLayerClone(std::move(originalLayerClone))
            , OriginalDirtyState(dirtyState)
            , StartCoords(startCoords)
            , ExistingRopeElementIndex(existingRopeElementIndex)
            , Plane(plane)
        {}
    };

    // Engagement data - when set, it means we're engaged
    std::optional<EngagementData> mEngagementData;
};

}