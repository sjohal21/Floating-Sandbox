/***************************************************************************************
 * Original Author:     Gabriele Giuseppini
 * Created:             2021-08-27
 * Copyright:           Gabriele Giuseppini  (https://github.com/GabrieleGiuseppini)
 ***************************************************************************************/
#pragma once

#include "Controller.h"
#include "IUserInterface.h"
#include "MaterialPalette.h"
#include "StatusBar.h"
#include "View.h"
#include "WorkbenchState.h"

#include <UILib/BitmapButton.h>
#include <UILib/BitmapToggleButton.h>
#include <UILib/LocalizationManager.h>
#include <UILib/LoggingDialog.h>
#include <UILib/ShipLoadDialog.h>
#include <UILib/ShipSaveDialog.h>

#include <Game/MaterialDatabase.h>
#include <Game/ResourceLocator.h>
#include <Game/ShipTexturizer.h>

#include <GameOpenGL/GameOpenGL.h>

#include <wx/app.h>
#include <wx/frame.h>
#include <wx/glcanvas.h> // Need to include this *after* our glad.h has been included, so that wxGLCanvas ends
                         // up *not* including the system's OpenGL header but glad's instead
#include <wx/menu.h>
#include <wx/panel.h>
#include <wx/scrolbar.h>
#include <wx/slider.h>
#include <wx/statbmp.h>

#include <array>
#include <filesystem>
#include <functional>
#include <memory>
#include <vector>

namespace ShipBuilder {

/*
 * The main window of the ship builder GUI.
 *
 * - Owns Controller and View
 * - Very thin, calls into Controller for each high-level interaction (e.g. new tool selected, tool setting changed) and for each mouse event
 * - Implements IUserInterface with interface needed by Controller, e.g. to make UI state changes, to capture the mouse, to update visualization of undo stack
 * - Owns WorkbenchState
 * - Implements ship load/save logic, giving/getting whole ShipDefinition to/from ModelController
 */
class MainFrame final : public wxFrame, public IUserInterface
{
public:

    MainFrame(
        wxApp * mainApp,
        ResourceLocator const & resourceLocator,
        LocalizationManager const & localizationManager,
        MaterialDatabase const & materialDatabase,
        ShipTexturizer const & shipTexturizer,
        std::function<void(std::optional<std::filesystem::path>)> returnToGameFunctor);

    void OpenForNewShip();

    void OpenForLoadShip(std::filesystem::path const & shipFilePath);

public:

    //
    // IUserInterface
    //

    void RefreshView() override;

    void OnViewModelChanged() override;

    void OnShipMetadataChanged(ShipMetadata const & shipMetadata) override;

    void OnShipSizeChanged(ShipSpaceSize const & shipSize) override;

    void OnLayerPresenceChanged() override;

    void OnPrimaryLayerChanged(LayerType primaryLayer) override;

    void OnModelDirtyChanged() override;

    void OnWorkbenchStateChanged() override;

    void OnCurrentToolChanged(std::optional<ToolType> tool) override;

    void OnUndoStackStateChanged() override;

    void OnToolCoordinatesChanged(std::optional<ShipSpaceCoordinates> coordinates) override;

    void SetToolCursor(wxImage const & cursorImage) override;

    void ResetToolCursor() override;

    void ScrollIntoViewIfNeeded(DisplayLogicalCoordinates const & workCanvasDisplayLogicalCoordinates) override;

private:

    wxPanel * CreateFilePanel(wxWindow * parent);
    wxPanel * CreateShipSettingsPanel(wxWindow * parent);
    wxPanel * CreateToolSettingsPanel(wxWindow * parent);
    wxPanel * CreateLayersPanel(wxWindow * parent);
    wxPanel * CreateToolbarPanel(wxWindow * parent);
    wxPanel * CreateWorkPanel(wxWindow * parent);

    void OnWorkCanvasPaint(wxPaintEvent & event);
    void OnWorkCanvasResize(wxSizeEvent & event);
    void OnWorkCanvasLeftDown(wxMouseEvent & event);
    void OnWorkCanvasLeftUp(wxMouseEvent & event);
    void OnWorkCanvasRightDown(wxMouseEvent & event);
    void OnWorkCanvasRightUp(wxMouseEvent & event);
    void OnWorkCanvasMouseMove(wxMouseEvent & event);
    void OnWorkCanvasMouseWheel(wxMouseEvent & event);
    void OnWorkCanvasCaptureMouseLost(wxMouseCaptureLostEvent & event);
    void OnWorkCanvasMouseLeftWindow(wxMouseEvent & event);

    void OnNewShip(wxCommandEvent & event);
    void OnLoadShip(wxCommandEvent & event);
    void OnSaveShip(wxCommandEvent & event);
    void OnSaveShipAs(wxCommandEvent & event);
    void OnSaveAndGoBack(wxCommandEvent & event);
    void OnQuitAndGoBack(wxCommandEvent & event);
    void OnQuit(wxCommandEvent & event);
    void OnUndo(wxCommandEvent & event);
    void OnClose(wxCloseEvent & event);
    void OnZoomIn(wxCommandEvent & event);
    void OnZoomOut(wxCommandEvent & event);
    void OnResetView(wxCommandEvent & event);
    void OnOpenLogWindowMenuItemSelected(wxCommandEvent & event);
    void OnStructuralMaterialSelected(fsStructuralMaterialSelectedEvent & event);
    void OnElectricalMaterialSelected(fsElectricalMaterialSelectedEvent & event);

private:

    bool IsStandAlone() const
    {
        return !mReturnToGameFunctor;
    }

    void Open();

    void NewShip();

    void LoadShip();

    bool SaveShip();

    bool SaveShipAs();

    void SaveAndSwitchBackToGame();

    void QuitAndSwitchBackToGame();

    void SwitchBackToGame(std::optional<std::filesystem::path> shipFilePath);

    void OpenMaterialPalette(
        wxMouseEvent const & event,
        MaterialLayerType layer,
        MaterialPlaneType plane);

    bool AskUserIfSure(wxString caption);

    void ShowError(std::string const & message);

    void DoNewShip(std::string const & shipName);

    void DoLoadShip(std::filesystem::path const & shipFilePath);

    void DoSaveShip(std::filesystem::path const & shipFilePath);

    void RecalculateWorkCanvasPanning();

    void SetFrameTitle(std::string const & shipName, bool isDirty);

    void DeviateFocus();

    //
    // UI Consistency
    //

    void ReconciliateUI();

    void ReconciliateUIWithViewModel();

    void ReconciliateUIWithShipMetadata(ShipMetadata const & shipMetadata);

    void ReconciliateUIWithShipSize(ShipSpaceSize const & shipSize);

    void ReconciliateUIWithLayerPresence();

    void ReconciliateUIWithPrimaryLayerSelection(LayerType primaryLayer);

    void ReconciliateUIWithModelDirtiness();

    void ReconciliateUIWithWorkbenchState();

    void ReconciliateUIWithSelectedTool(std::optional<ToolType> tool);

    void ReconciliateUIWithUndoStackState();

private:

    wxApp * const mMainApp;

    std::function<void(std::optional<std::filesystem::path>)> const mReturnToGameFunctor;

    //
    // Owned members
    //

    std::unique_ptr<Controller> mController;
    std::unique_ptr<View> mView;

    //
    // Helpers
    //

    ResourceLocator const & mResourceLocator;
    LocalizationManager const & mLocalizationManager;
    MaterialDatabase const & mMaterialDatabase;
    ShipTexturizer const & mShipTexturizer;

    //
    // UI
    //

    wxPanel * mMainPanel;

    // Menu
    wxMenuItem * mSaveShipMenuItem;
    wxMenuItem * mSaveShipAsMenuItem;
    wxMenuItem * mSaveAndGoBackMenuItem;
    wxMenuItem * mUndoMenuItem;

    // File panel
    BitmapButton * mSaveShipButton;
    BitmapButton * mSaveShipAsButton;

    // Layers panel
    std::array<BitmapToggleButton *, LayerCount> mLayerSelectButtons;
    std::array<BitmapButton *, LayerCount> mLayerSaveButtons;
    std::array<BitmapButton *, LayerCount> mLayerDeleteButtons;
    wxSlider * mOtherLayersOpacitySlider;

    // Toolbar panel
    wxSizer * mToolbarPanelsSizer;
    std::array<wxPanel *, LayerCount> mToolbarPanels;
    std::array<BitmapToggleButton *, static_cast<size_t>(ToolType::_Last) + 1> mToolButtons;
    wxStaticBitmap * mStructuralForegroundMaterialSelector;
    wxStaticBitmap * mStructuralBackgroundMaterialSelector;
    wxStaticBitmap * mElectricalForegroundMaterialSelector;
    wxStaticBitmap * mElectricalBackgroundMaterialSelector;
    wxBitmap mNullMaterialBitmap;

    // Work panel
    std::unique_ptr<wxGLCanvas> mWorkCanvas;
    std::unique_ptr<wxGLContext> mGLContext;
    wxScrollBar * mWorkCanvasHScrollBar;
    wxScrollBar * mWorkCanvasVScrollBar;

    // Misc UI elements
    std::unique_ptr<MaterialPalette<StructuralMaterial>> mStructuralMaterialPalette;
    std::unique_ptr<MaterialPalette<ElectricalMaterial>> mElectricalMaterialPalette;
    StatusBar * mStatusBar;

    //
    // Dialogs
    //

    std::unique_ptr<ShipLoadDialog> mShipLoadDialog;
    std::unique_ptr<ShipSaveDialog> mShipSaveDialog;
    std::unique_ptr<LoggingDialog> mLoggingDialog;

    //
    // UI state
    //

    bool mIsMouseCapturedByWorkCanvas;

    //
    // State
    //

    WorkbenchState mWorkbenchState;
    std::optional<std::filesystem::path> mCurrentShipFilePath;
    std::vector<std::filesystem::path> mShipLoadDirectories;
};

}