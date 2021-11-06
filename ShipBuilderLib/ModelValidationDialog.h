/***************************************************************************************
 * Original Author:     Gabriele Giuseppini
 * Created:             2021-11-06
 * Copyright:           Gabriele Giuseppini  (https://github.com/GabrieleGiuseppini)
 ***************************************************************************************/
#pragma once

#include "Controller.h"

#include <Game/ResourceLocator.h>

#include <wx/dialog.h>
#include <wx/gauge.h>
#include <wx/panel.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/timer.h>

#include <memory>
#include <optional>
#include <thread>

namespace ShipBuilder {

class ModelValidationDialog : public wxDialog
{
public:

    ModelValidationDialog(
        wxWindow * parent,
        ResourceLocator const & resourceLocator);

    void ShowModal(Controller & controller);

private:

    void OnOkButton(wxCommandEvent & event);
    void OnCancelButton(wxCommandEvent & event);


    void StartValidation();
    void ValidationThreadLoop();
    void OnValidationTimer(wxTimerEvent & event);
    void ShowResults(ModelValidationResults const & results);

private:

    ResourceLocator const & mResourceLocator;
    Controller * mController;

    wxSizer * mMainVSizer;

    // Validation panel
    wxPanel * mValidationPanel;
    wxGauge * mValidationWaitGauge;

    // Results panel
    wxScrolledWindow * mResultsPanel;
    wxSizer * mResultsPanelVSizer;

    // Buttons panel
    wxPanel * mButtonsPanel;

    // Validation thread
    std::unique_ptr<wxTimer> mValidationTimer;
    std::thread mValidationThread;
    std::optional<ModelValidationResults> mValidationResults;
};

}