/***************************************************************************************
* Original Author:		Gabriele Giuseppini
* Created:				2021-08-28
* Copyright:			Gabriele Giuseppini  (https://github.com/GabrieleGiuseppini)
***************************************************************************************/
#pragma once

#include "Model.h"
#include "ShipBuilderTypes.h"
#include "UndoStack.h"
#include "ViewModel.h"

#include <wx/image.h>
#include <wx/string.h>

#include <optional>

namespace ShipBuilder {

class Controller;

/*
 * Interface of MainFrame that is seen by Controller and underneath.
 */
struct IUserInterface
{
public:

    virtual void RefreshView() = 0;

    // Notifies of a change in the view model geometry
    virtual void OnViewModelChanged(ViewModel const & viewModel) = 0;

    // Notifies of a change in the size of the model
    virtual void OnShipSizeChanged(ShipSpaceSize const & shipSpaceSize) = 0;

    // Notifies of a change in the scale of the ship
    virtual void OnShipScaleChanged(ShipSpaceToWorldSpaceCoordsRatio const & scale) = 0;

    // Notifies of a change in the name of the ship
    virtual void OnShipNameChanged(Model const & model) = 0;

    // Notifies of a (possible) change in the presence of a layer
    virtual void OnLayerPresenceChanged(Model const & model) = 0;

    // Notifies of a (possible) change in the dirtiness of the model
    virtual void OnModelDirtyChanged(Model const & model) = 0;

    // Notifies of a (possible) change in the model's macro properties analysis
    virtual void OnModelMacroPropertiesUpdated(ModelMacroProperties const & properties) = 0;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    virtual void OnStructuralMaterialChanged(StructuralMaterial const * material, MaterialPlaneType plane) = 0;
    virtual void OnElectricalMaterialChanged(ElectricalMaterial const * material, MaterialPlaneType plane) = 0;
    virtual void OnRopesMaterialChanged(StructuralMaterial const * material, MaterialPlaneType plane) = 0;

    virtual void OnCurrentToolChanged(std::optional<ToolType> tool) = 0;

    virtual void OnPrimaryVisualizationChanged(VisualizationType primaryVisualization) = 0;

    virtual void OnGameVisualizationModeChanged(GameVisualizationModeType mode) = 0;
    virtual void OnStructuralLayerVisualizationModeChanged(StructuralLayerVisualizationModeType mode) = 0;
    virtual void OnElectricalLayerVisualizationModeChanged(ElectricalLayerVisualizationModeType mode) = 0;
    virtual void OnRopesLayerVisualizationModeChanged(RopesLayerVisualizationModeType mode) = 0;
    virtual void OnTextureLayerVisualizationModeChanged(TextureLayerVisualizationModeType mode) = 0;

    virtual void OnOtherVisualizationsOpacityChanged(float opacity) = 0;

    virtual void OnVisualGridEnablementChanged(bool isEnabled) = 0;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Notifies of a change in the state of the undo stack
    virtual void OnUndoStackStateChanged(UndoStack & undoStack) = 0;

    // Notifies of a change in the tool coordinates to display
    virtual void OnToolCoordinatesChanged(std::optional<ShipSpaceCoordinates> coordinates, ShipSpaceSize const & shipSize) = 0;

    // Notifies of a change in the currently-sampled material
    virtual void OnSampledMaterialChanged(std::optional<std::string> materialName) = 0;

    // Notifies of a change in the currently-measured length
    virtual void OnMeasuredWorldLengthChanged(std::optional<int> length) = 0;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    virtual void OnError(wxString const & errorMessage) const = 0;

    virtual DisplayLogicalSize GetDisplaySize() const = 0;

    virtual int GetLogicalToPhysicalPixelFactor() const = 0;

    virtual void SwapRenderBuffers() = 0;

    virtual DisplayLogicalCoordinates GetMouseCoordinates() const = 0;

    virtual bool IsMouseInWorkCanvas() const = 0;

    virtual std::optional<DisplayLogicalCoordinates> GetMouseCoordinatesIfInWorkCanvas() const = 0;

    virtual void SetToolCursor(wxImage const & cursorImage) = 0;

    virtual void ResetToolCursor() = 0;
};

}