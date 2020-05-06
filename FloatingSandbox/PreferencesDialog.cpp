/***************************************************************************************
 * Original Author:     Gabriele Giuseppini
 * Created:             2019-03-20
 * Copyright:           Gabriele Giuseppini  (https://github.com/GabrieleGiuseppini)
 ***************************************************************************************/
#include "PreferencesDialog.h"

#include <GameCore/LinearSliderCore.h>

#include <wx/gbsizer.h>
#include <wx/notebook.h>
#include <wx/stattext.h>

static constexpr int Border = 10;

static int constexpr StaticBoxTopMargin = 7;
static int constexpr StaticBoxInsetMargin = 10;

static int constexpr SliderWidth = 40;
static int constexpr SliderHeight = 140;

static constexpr int MaxZoomIncrementPosition = 200;
static constexpr int MaxPanIncrementPosition = 200;

PreferencesDialog::PreferencesDialog(
    wxWindow* parent,
    std::shared_ptr<UIPreferencesManager> uiPreferencesManager,
    std::function<void()> onChangeCallback)
    : mParent(parent)
    , mUIPreferencesManager(std::move(uiPreferencesManager))
    , mOnChangeCallback(std::move(onChangeCallback))
{
    Create(
        mParent,
        wxID_ANY,
        _("Preferences"),
        wxDefaultPosition,
        wxSize(400, -1),
        wxCAPTION | wxCLOSE_BOX | wxMINIMIZE_BOX | wxFRAME_SHAPED,
        _T("Preferences Window"));

    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));


    //
    // Lay the dialog out
    //

    wxBoxSizer * dialogVSizer = new wxBoxSizer(wxVERTICAL);

    wxNotebook * notebook = new wxNotebook(
        this,
        wxID_ANY,
        wxPoint(-1, -1),
        wxSize(-1, -1),
        wxNB_TOP);

    //
    // Game Preferences
    //

    wxPanel * gamePanel = new wxPanel(notebook);

    PopulateGamePanel(gamePanel);

    notebook->AddPage(gamePanel, "Game Preferences");


    //
    // Ships Preferences
    //

    wxPanel * shipsPanel = new wxPanel(notebook);

    PopulateShipsPanel(shipsPanel);

    notebook->AddPage(shipsPanel, "Ships Preferences");


    //
    // Global Sound and Music
    //

    wxPanel * musicPanel = new wxPanel(notebook);

    PopulateMusicPanel(musicPanel);

    notebook->AddPage(musicPanel, "Global Sound and Music");


    dialogVSizer->Add(notebook, 1, wxEXPAND);

    dialogVSizer->AddSpacer(20);


    // Buttons

    wxBoxSizer * buttonsSizer = new wxBoxSizer(wxHORIZONTAL);

    buttonsSizer->AddSpacer(20);

    mOkButton = new wxButton(this, wxID_ANY, _("Done"));
    mOkButton->Bind(wxEVT_BUTTON, &PreferencesDialog::OnOkButton, this);
    buttonsSizer->Add(mOkButton, 0);

    buttonsSizer->AddSpacer(20);

    dialogVSizer->Add(buttonsSizer, 0, wxALIGN_CENTER_HORIZONTAL);

    dialogVSizer->AddSpacer(20);



    //
    // Finalize dialog
    //

    SetSizerAndFit(dialogVSizer);

    Centre(wxCENTER_ON_SCREEN | wxBOTH);
}

PreferencesDialog::~PreferencesDialog()
{
}

void PreferencesDialog::Open()
{
    ReadSettings();

    this->Show();
}

void PreferencesDialog::OnScreenshotDirPickerChanged(wxCommandEvent & /*event*/)
{
    assert(!!mUIPreferencesManager);
    mUIPreferencesManager->SetScreenshotsFolderPath(mScreenshotDirPickerCtrl->GetPath().ToStdString());

    mOnChangeCallback();
}

void PreferencesDialog::OnShowTipOnStartupCheckBoxClicked(wxCommandEvent & /*event*/)
{
    assert(!!mUIPreferencesManager);
    mUIPreferencesManager->SetShowStartupTip(mShowTipOnStartupCheckBox->GetValue());

    mOnChangeCallback();
}

void PreferencesDialog::OnCheckForUpdatesAtStartupCheckBoxClicked(wxCommandEvent & /*event*/)
{
    assert(!!mUIPreferencesManager);
    mUIPreferencesManager->SetCheckUpdatesAtStartup(mCheckForUpdatesAtStartupCheckBox->GetValue());

    if (mCheckForUpdatesAtStartupCheckBox->GetValue())
    {
        mUIPreferencesManager->ResetUpdateBlacklist();
    }

    mOnChangeCallback();
}

void PreferencesDialog::OnSaveSettingsOnExitCheckBoxClicked(wxCommandEvent & /*event*/)
{
    assert(!!mUIPreferencesManager);
    mUIPreferencesManager->SetSaveSettingsOnExit(mSaveSettingsOnExitCheckBox->GetValue());

    mOnChangeCallback();
}

void PreferencesDialog::OnShowTsunamiNotificationsCheckBoxClicked(wxCommandEvent & /*event*/)
{
    assert(!!mUIPreferencesManager);
    mUIPreferencesManager->SetDoShowTsunamiNotifications(mShowTsunamiNotificationsCheckBox->GetValue());

    mOnChangeCallback();
}

void PreferencesDialog::OnZoomIncrementSpinCtrl(wxSpinEvent & event)
{
    assert(!!mUIPreferencesManager);
    mUIPreferencesManager->SetZoomIncrement(ZoomIncrementSpinToZoomIncrement(event.GetPosition()));

    mOnChangeCallback();
}

void PreferencesDialog::OnPanIncrementSpinCtrl(wxSpinEvent & event)
{
    assert(!!mUIPreferencesManager);
    mUIPreferencesManager->SetPanIncrement(PanIncrementSpinToPanIncrement(event.GetPosition()));

    mOnChangeCallback();
}

void PreferencesDialog::OnShowStatusTextCheckBoxClicked(wxCommandEvent & /*event*/)
{
    assert(!!mUIPreferencesManager);
    mUIPreferencesManager->SetShowStatusText(mShowStatusTextCheckBox->GetValue());

    mOnChangeCallback();
}

void PreferencesDialog::OnShowExtendedStatusTextCheckBoxClicked(wxCommandEvent & /*event*/)
{
    assert(!!mUIPreferencesManager);
    mUIPreferencesManager->SetShowExtendedStatusText(mShowExtendedStatusTextCheckBox->GetValue());

    mOnChangeCallback();
}

void PreferencesDialog::OnShowShipDescriptionAtShipLoadCheckBoxClicked(wxCommandEvent & /*event*/)
{
    assert(!!mUIPreferencesManager);
    mUIPreferencesManager->SetShowShipDescriptionsAtShipLoad(mShowShipDescriptionAtShipLoadCheckBox->GetValue());

    mOnChangeCallback();
}

void PreferencesDialog::OnAutoZoomAtShipLoadCheckBoxClicked(wxCommandEvent & /*event*/)
{
    assert(!!mUIPreferencesManager);
    mUIPreferencesManager->SetDoAutoZoomAtShipLoad(mAutoZoomAtShipLoadCheckBox->GetValue());

    mOnChangeCallback();
}

void PreferencesDialog::OnAutoShowSwitchboardCheckBoxClicked(wxCommandEvent & /*event*/)
{
    assert(!!mUIPreferencesManager);
    mUIPreferencesManager->SetAutoShowSwitchboard(mAutoShowSwitchboardCheckBox->GetValue());

    mOnChangeCallback();
}

void PreferencesDialog::OnShowElectricalNotificationsCheckBoxClicked(wxCommandEvent & /*event*/)
{
    assert(!!mUIPreferencesManager);
    mUIPreferencesManager->SetDoShowElectricalNotifications(mShowElectricalNotificationsCheckBox->GetValue());

    mOnChangeCallback();
}

void PreferencesDialog::OnAutoTexturizationModeRadioButtonClick(wxCommandEvent & /*event*/)
{
    if (mFlatStructureAutoTexturizationModeRadioButton->GetValue())
    {
        mUIPreferencesManager->SetShipAutoTexturizationMode(ShipAutoTexturizationMode::FlatStructure);
    }
    else
    {
        assert(mMaterialTexturesAutoTexturizationModeRadioButton->GetValue());
        mUIPreferencesManager->SetShipAutoTexturizationMode(ShipAutoTexturizationMode::MaterialTextures);
    }

    ReconciliateShipAutoTexturizationModeSettings();

    mOnChangeCallback();
}

void PreferencesDialog::OnMaterialTextureMagnificationSpinCtrl(wxSpinEvent & event)
{
    assert(!!mUIPreferencesManager);
    mUIPreferencesManager->SetShipAutoTexturizationMaterialTextureMagnification(MaterialTextureMagnificationSpinToMaterialTextureMagnification(event.GetPosition()));

    mOnChangeCallback();
}

void PreferencesDialog::OnGlobalMuteCheckBoxClicked(wxCommandEvent & /*event*/)
{
    assert(!!mUIPreferencesManager);
    mUIPreferencesManager->SetGlobalMute(mGlobalMuteCheckBox->GetValue());

    ReconcileSoundSettings();

    mOnChangeCallback();
}

void PreferencesDialog::OnPlayBackgroundMusicCheckBoxClicked(wxCommandEvent & /*event*/)
{
    assert(!!mUIPreferencesManager);
    mUIPreferencesManager->SetPlayBackgroundMusic(mPlayBackgroundMusicCheckBox->GetValue());

    ReconcileSoundSettings();

    mOnChangeCallback();
}

void PreferencesDialog::OnPlaySinkingMusicCheckBoxClicked(wxCommandEvent & /*event*/)
{
    assert(!!mUIPreferencesManager);
    mUIPreferencesManager->SetPlaySinkingMusic(mPlaySinkingMusicCheckBox->GetValue());

    ReconcileSoundSettings();

    mOnChangeCallback();
}

void PreferencesDialog::OnOkButton(wxCommandEvent & /*event*/)
{
    // Close ourselves
    this->Close();
}

void PreferencesDialog::PopulateGamePanel(wxPanel * panel)
{
    wxGridBagSizer* gridSizer = new wxGridBagSizer(0, 0);

    gridSizer->SetFlexibleDirection(wxHORIZONTAL);
    gridSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_NONE);


    //
    // Row 1
    //

    {
        wxStaticText * screenshotDirStaticText = new wxStaticText(panel, wxID_ANY, "Screenshot directory:", wxDefaultPosition, wxDefaultSize, 0);

        gridSizer->Add(
            screenshotDirStaticText,
            wxGBPosition(0, 0),
            wxGBSpan(1, 4), // Take entire row
            wxRIGHT | wxLEFT | wxEXPAND | wxALIGN_BOTTOM,
            Border);
    }

    //
    // Row 2
    //

    {
        mScreenshotDirPickerCtrl = new wxDirPickerCtrl(
            panel,
            wxID_ANY,
            _T(""),
            _("Select directory that screenshots will be saved to:"),
            wxDefaultPosition,
            wxSize(-1, -1),
            wxDIRP_DIR_MUST_EXIST | wxDIRP_USE_TEXTCTRL);
        mScreenshotDirPickerCtrl->SetToolTip("Sets the directory into which in-game screenshots are automatically saved.");
        mScreenshotDirPickerCtrl->SetMinSize(wxSize(540, -1));

        mScreenshotDirPickerCtrl->Bind(wxEVT_DIRPICKER_CHANGED, &PreferencesDialog::OnScreenshotDirPickerChanged, this);

        gridSizer->Add(
            mScreenshotDirPickerCtrl,
            wxGBPosition(1, 0),
            wxGBSpan(1, 4), // Take entire row
            wxRIGHT | wxLEFT | wxEXPAND,
            Border);
    }

    //
    // Row 3
    //

    {
        mShowTipOnStartupCheckBox = new wxCheckBox(panel, wxID_ANY, _("Show Tips on Startup"), wxDefaultPosition, wxDefaultSize, 0);

        mShowTipOnStartupCheckBox->SetToolTip("Enables or disables the tips shown when the game starts.");

        mShowTipOnStartupCheckBox->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &PreferencesDialog::OnShowTipOnStartupCheckBoxClicked, this);

        gridSizer->Add(
            mShowTipOnStartupCheckBox,
            wxGBPosition(2, 0),
            wxGBSpan(1, 1),
            wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM,
            Border);
    }

    {
        mZoomIncrementSpinCtrl = new wxSpinCtrl(panel, wxID_ANY, _T("Zoom Increment"), wxDefaultPosition, wxSize(75, -1),
            wxSP_ARROW_KEYS | wxALIGN_CENTRE_HORIZONTAL);

        mZoomIncrementSpinCtrl->SetRange(1, MaxZoomIncrementPosition);

        mZoomIncrementSpinCtrl->SetToolTip("Changes the amount by which zoom changes when using the zoom controls.");

        mZoomIncrementSpinCtrl->Bind(wxEVT_SPINCTRL, &PreferencesDialog::OnZoomIncrementSpinCtrl, this);

        gridSizer->Add(
            mZoomIncrementSpinCtrl,
            wxGBPosition(2, 2),
            wxGBSpan(1, 1),
            wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT,
            Border);
    }

    {
        auto label = new wxStaticText(panel, wxID_ANY, "Zoom Increment", wxDefaultPosition, wxDefaultSize,
            wxALIGN_LEFT);

        gridSizer->Add(
            label,
            wxGBPosition(2, 3),
            wxGBSpan(1, 1),
            wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxBOTTOM,
            Border);
    }

    //
    // Row 4
    //

    {
        mCheckForUpdatesAtStartupCheckBox = new wxCheckBox(panel, wxID_ANY, _("Check for Updates on Startup"), wxDefaultPosition, wxDefaultSize, 0);

        mCheckForUpdatesAtStartupCheckBox->SetToolTip("Enables or disables checking for new versions when the game starts.");

        mCheckForUpdatesAtStartupCheckBox->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &PreferencesDialog::OnCheckForUpdatesAtStartupCheckBoxClicked, this);

        gridSizer->Add(
            mCheckForUpdatesAtStartupCheckBox,
            wxGBPosition(3, 0),
            wxGBSpan(1, 1),
            wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM,
            Border);
    }

    {
        mPanIncrementSpinCtrl = new wxSpinCtrl(panel, wxID_ANY, _T("Pan Increment"), wxDefaultPosition, wxSize(75, -1),
            wxSP_ARROW_KEYS | wxALIGN_CENTRE_HORIZONTAL);

        mPanIncrementSpinCtrl->SetRange(1, MaxPanIncrementPosition);

        mPanIncrementSpinCtrl->SetToolTip("Changes the amount by which the camera position changes when using the pan controls.");

        mPanIncrementSpinCtrl->Bind(wxEVT_SPINCTRL, &PreferencesDialog::OnPanIncrementSpinCtrl, this);

        gridSizer->Add(
            mPanIncrementSpinCtrl,
            wxGBPosition(3, 2),
            wxGBSpan(1, 1),
            wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT,
            Border);
    }

    {
        auto label = new wxStaticText(panel, wxID_ANY, "Pan Increment", wxDefaultPosition, wxDefaultSize,
            wxALIGN_LEFT);

        gridSizer->Add(
            label,
            wxGBPosition(3, 3),
            wxGBSpan(1, 1),
            wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxBOTTOM,
            Border);
    }

    //
    // Row 5
    //

    {
        mSaveSettingsOnExitCheckBox = new wxCheckBox(panel, wxID_ANY, _("Save Settings on Exit"), wxDefaultPosition, wxDefaultSize, 0);

        mSaveSettingsOnExitCheckBox->SetToolTip("Enables or disables saving the last-modified settings when exiting the game.");

        mSaveSettingsOnExitCheckBox->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &PreferencesDialog::OnSaveSettingsOnExitCheckBoxClicked, this);

        gridSizer->Add(
            mSaveSettingsOnExitCheckBox,
            wxGBPosition(4, 0),
            wxGBSpan(1, 1),
            wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM,
            Border);
    }

    {
        mShowStatusTextCheckBox = new wxCheckBox(panel, wxID_ANY, _("Show Status Text"), wxDefaultPosition, wxDefaultSize, 0);

        mShowStatusTextCheckBox->SetToolTip("Enables or disables the display of game performance information, such as frame rate and time elapsed.");

        mShowStatusTextCheckBox->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &PreferencesDialog::OnShowStatusTextCheckBoxClicked, this);

        gridSizer->Add(
            mShowStatusTextCheckBox,
            wxGBPosition(4, 2),
            wxGBSpan(1, 2),
            wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM,
            Border);
    }

    //
    // Row 6
    //

    {
        mShowTsunamiNotificationsCheckBox = new wxCheckBox(panel, wxID_ANY, _("Show Tsunami Notifications"), wxDefaultPosition, wxDefaultSize, 0);

        mShowTsunamiNotificationsCheckBox->SetToolTip("Enables or disables notifications when a tsunami is being spawned.");

        mShowTsunamiNotificationsCheckBox->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &PreferencesDialog::OnShowTsunamiNotificationsCheckBoxClicked, this);

        gridSizer->Add(
            mShowTsunamiNotificationsCheckBox,
            wxGBPosition(5, 0),
            wxGBSpan(1, 1),
            wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM,
            Border);
    }

    {
        mShowExtendedStatusTextCheckBox = new wxCheckBox(panel, wxID_ANY, _("Show Extended Status Text"), wxDefaultPosition, wxDefaultSize, 0);

        mShowExtendedStatusTextCheckBox->SetToolTip("Enables or disables the display of extended game performance information, such as update/render ratio and counts of primitives being rendered.");

        mShowExtendedStatusTextCheckBox->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &PreferencesDialog::OnShowExtendedStatusTextCheckBoxClicked, this);

        gridSizer->Add(
            mShowExtendedStatusTextCheckBox,
            wxGBPosition(5, 2),
            wxGBSpan(1, 2),
            wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM,
            Border);
    }

    //
    // Add spacers
    //

    // Col 1
    gridSizer->Add(
        40,
        0,
        wxGBPosition(0, 1),
        wxGBSpan(6, 1));


    // Finalize panel

    panel->SetSizerAndFit(gridSizer);
}

void PreferencesDialog::PopulateShipsPanel(wxPanel * panel)
{
    wxGridBagSizer * gridSizer = new wxGridBagSizer(0, 0);

    gridSizer->SetFlexibleDirection(wxVERTICAL);
    gridSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_NONE);


    //
    // Row 1
    //

    {
        mShowShipDescriptionAtShipLoadCheckBox = new wxCheckBox(panel, wxID_ANY, _("Show Ship Descriptions at Load"), wxDefaultPosition, wxDefaultSize, 0);

        mShowShipDescriptionAtShipLoadCheckBox->SetToolTip("Enables or disables the window showing ship descriptions when ships are loaded.");

        mShowShipDescriptionAtShipLoadCheckBox->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &PreferencesDialog::OnShowShipDescriptionAtShipLoadCheckBoxClicked, this);

        gridSizer->Add(
            mShowShipDescriptionAtShipLoadCheckBox,
            wxGBPosition(0, 0),
            wxGBSpan(1, 1),
            wxALIGN_CENTER_VERTICAL | wxALL,
            Border);
    }

    //
    // Row 2
    //

    {
        mAutoZoomAtShipLoadCheckBox = new wxCheckBox(panel, wxID_ANY, _("Auto-Zoom at Ship Load"), wxDefaultPosition, wxDefaultSize, 0);

        mAutoZoomAtShipLoadCheckBox->SetToolTip("Enables or disables auto-zooming when loading a new ship.");

        mAutoZoomAtShipLoadCheckBox->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &PreferencesDialog::OnAutoZoomAtShipLoadCheckBoxClicked, this);

        gridSizer->Add(
            mAutoZoomAtShipLoadCheckBox,
            wxGBPosition(1, 0),
            wxGBSpan(1, 1),
            wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM,
            Border);
    }

    //
    // Row 3
    //

    {
        mAutoShowSwitchboardCheckBox = new wxCheckBox(panel, wxID_ANY, _("Open Electrical Panel at Load"), wxDefaultPosition, wxDefaultSize, 0);

        mAutoShowSwitchboardCheckBox->SetToolTip("Enables or disables automatic showing of the ship's electrical panel when a ship with interactive electrical elements is loaded.");

        mAutoShowSwitchboardCheckBox->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &PreferencesDialog::OnAutoShowSwitchboardCheckBoxClicked, this);

        gridSizer->Add(
            mAutoShowSwitchboardCheckBox,
            wxGBPosition(2, 0),
            wxGBSpan(1, 1),
            wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM,
            Border);
    }

    //
    // Row 4
    //

    {
        mShowElectricalNotificationsCheckBox = new wxCheckBox(panel, wxID_ANY, _("Show Electrical Notifications"), wxDefaultPosition, wxDefaultSize, 0);

        mShowElectricalNotificationsCheckBox->SetToolTip("Enables or disables visual notifications when an electrical element changes state.");

        mShowElectricalNotificationsCheckBox->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &PreferencesDialog::OnShowElectricalNotificationsCheckBoxClicked, this);

        gridSizer->Add(
            mShowElectricalNotificationsCheckBox,
            wxGBPosition(3, 0),
            wxGBSpan(1, 1),
            wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM,
            Border);
    }

    //
    // Row 5
    //

    {
        wxStaticBox * autoTexturizationBox = new wxStaticBox(panel, wxID_ANY, _("Auto-Texturization Mode"));

        wxBoxSizer * autoTexturizationBoxSizer1 = new wxBoxSizer(wxVERTICAL);
        autoTexturizationBoxSizer1->AddSpacer(StaticBoxTopMargin);

        {
            wxGridBagSizer * autoTexturizationModeBoxSizer = new wxGridBagSizer(5, 5);
            autoTexturizationModeBoxSizer->SetFlexibleDirection(wxHORIZONTAL);
            autoTexturizationModeBoxSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_NONE);

            mFlatStructureAutoTexturizationModeRadioButton = new wxRadioButton(autoTexturizationBox, wxID_ANY, _("Flat Structure"),
                wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
            mFlatStructureAutoTexturizationModeRadioButton->SetToolTip("Generates a ship's high-definition image using the materials' matte colors.");
            mFlatStructureAutoTexturizationModeRadioButton->Bind(wxEVT_RADIOBUTTON, &PreferencesDialog::OnAutoTexturizationModeRadioButtonClick, this);
            autoTexturizationModeBoxSizer->Add(mFlatStructureAutoTexturizationModeRadioButton, wxGBPosition(0, 0), wxGBSpan(1, 3), wxALL | wxALIGN_CENTER_VERTICAL, 0);

            //

            mMaterialTexturesAutoTexturizationModeRadioButton = new wxRadioButton(autoTexturizationBox, wxID_ANY, _("Material Textures"),
                wxDefaultPosition, wxDefaultSize);
            mMaterialTexturesAutoTexturizationModeRadioButton->SetToolTip("Generates a ship's high-definition image using material-specific textures.");
            mMaterialTexturesAutoTexturizationModeRadioButton->Bind(wxEVT_RADIOBUTTON, &PreferencesDialog::OnAutoTexturizationModeRadioButtonClick, this);
            autoTexturizationModeBoxSizer->Add(mMaterialTexturesAutoTexturizationModeRadioButton, wxGBPosition(1, 0), wxGBSpan(1, 1),
                wxBOTTOM | wxALIGN_CENTER_VERTICAL, 2); // Doesn't align otherwise

            auto label = new wxStaticText(autoTexturizationBox, wxID_ANY, "Texture Magnification:", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
            autoTexturizationModeBoxSizer->Add(label, wxGBPosition(1, 1), wxGBSpan(1, 1), wxALL | wxALIGN_CENTER_VERTICAL, 0);

            mMaterialTextureMagnificationSpinCtrl = new wxSpinCtrl(autoTexturizationBox, wxID_ANY, _T("Texture Magnification"), wxDefaultPosition, wxSize(75, -1),
                wxSP_ARROW_KEYS | wxALIGN_CENTRE_HORIZONTAL);
            mMaterialTextureMagnificationSpinCtrl->SetRange(1, 100);
            mMaterialTextureMagnificationSpinCtrl->SetToolTip("Changes the zoom level of materials' textures.");
            mMaterialTextureMagnificationSpinCtrl->Bind(wxEVT_SPINCTRL, &PreferencesDialog::OnMaterialTextureMagnificationSpinCtrl, this);
            autoTexturizationModeBoxSizer->Add(mMaterialTextureMagnificationSpinCtrl, wxGBPosition(1, 2), wxGBSpan(1, 1), wxALL | wxALIGN_CENTER_VERTICAL, 0);

            autoTexturizationBoxSizer1->Add(autoTexturizationModeBoxSizer, 0, wxALL, StaticBoxInsetMargin);
        }

        autoTexturizationBox->SetSizerAndFit(autoTexturizationBoxSizer1);

        gridSizer->Add(
            autoTexturizationBox,
            wxGBPosition(4, 0),
            wxGBSpan(1, 1),
            wxALL,
            Border);
    }


    // Finalize panel

    panel->SetSizerAndFit(gridSizer);
}

void PreferencesDialog::PopulateMusicPanel(wxPanel * panel)
{
    wxBoxSizer * vSizer = new wxBoxSizer(wxVERTICAL);

    //
    // Row 1
    //

    {
        // Global mute
        {
            mGlobalMuteCheckBox = new wxCheckBox(panel, wxID_ANY, _("Mute All Sounds"), wxDefaultPosition, wxDefaultSize, 0);

            mGlobalMuteCheckBox->SetToolTip("Mutes or allows all sounds.");

            mGlobalMuteCheckBox->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &PreferencesDialog::OnGlobalMuteCheckBoxClicked, this);

            vSizer->Add(
                mGlobalMuteCheckBox,
                0,
                wxALIGN_LEFT | wxALL,
                Border);
        }
    }

    //
    // Row 2
    //

    {

        wxGridBagSizer* gridSizer = new wxGridBagSizer(0, 0);

        {
            //
            // Row 1
            //

            {
                // Background music volume
                {
                    mBackgroundMusicVolumeSlider = new SliderControl<float>(
                        panel,
                        SliderWidth,
                        SliderHeight,
                        "Background Music Volume",
                        "Adjusts the volume of background music.",
                        [this](float value)
                        {
                            this->mUIPreferencesManager->SetBackgroundMusicVolume(value);
                            this->mOnChangeCallback();
                        },
                        std::make_unique<LinearSliderCore>(
                            0.0f,
                            100.0f));

                    gridSizer->Add(
                        mBackgroundMusicVolumeSlider,
                        wxGBPosition(0, 1),
                        wxGBSpan(1, 1),
                        wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxALL,
                        Border);
                }

                // Sinking Music Volume
                {
                    mSinkingMusicVolumeSlider = new SliderControl<float>(
                        panel,
                        SliderWidth,
                        SliderHeight,
                        "Farewell Music Volume",
                        "Adjusts the volume of the music played when a ship is sinking.",
                        [this](float value)
                        {
                            this->mUIPreferencesManager->SetGameMusicVolume(value);
                            this->mOnChangeCallback();
                        },
                        std::make_unique<LinearSliderCore>(
                            0.0f,
                            100.0f));

                    gridSizer->Add(
                        mSinkingMusicVolumeSlider,
                        wxGBPosition(0, 3),
                        wxGBSpan(1, 1),
                        wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxALL,
                        Border);
                }
            }

            //
            // Row 2
            //

            {
                // Play background music
                {
                    mPlayBackgroundMusicCheckBox = new wxCheckBox(panel, wxID_ANY, _("Play Background Music"), wxDefaultPosition, wxDefaultSize, 0);

                    mPlayBackgroundMusicCheckBox->SetToolTip("Enables or disables background music while playing the game.");

                    mPlayBackgroundMusicCheckBox->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &PreferencesDialog::OnPlayBackgroundMusicCheckBoxClicked, this);

                    gridSizer->Add(
                        mPlayBackgroundMusicCheckBox,
                        wxGBPosition(1, 1),
                        wxGBSpan(1, 1),
                        wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL,
                        Border);
                }

                // Play sinking music
                {
                    mPlaySinkingMusicCheckBox = new wxCheckBox(panel, wxID_ANY, _("Play Farewell Music"), wxDefaultPosition, wxDefaultSize, 0);

                    mPlaySinkingMusicCheckBox->SetToolTip("Enables or disables playing sorrow music when a ship starts sinking.");

                    mPlaySinkingMusicCheckBox->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &PreferencesDialog::OnPlaySinkingMusicCheckBoxClicked, this);

                    gridSizer->Add(
                        mPlaySinkingMusicCheckBox,
                        wxGBPosition(1, 3),
                        wxGBSpan(1, 1),
                        wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL,
                        Border);
                }
            }


            //
            // Add spacers
            //

            // Col 0
            gridSizer->Add(
                1,
                0,
                wxGBPosition(0, 0),
                wxGBSpan(2, 1),
                wxEXPAND);

            // Col 2
            gridSizer->Add(
                1,
                0,
                wxGBPosition(0, 2),
                wxGBSpan(2, 1),
                wxEXPAND);

            // Col 4
            gridSizer->Add(
                1,
                0,
                wxGBPosition(0, 4),
                wxGBSpan(2, 1),
                wxEXPAND);

            gridSizer->AddGrowableCol(0);
            gridSizer->AddGrowableCol(2);
            gridSizer->AddGrowableCol(4);
        }

        vSizer->Add(
            gridSizer,
            1,
            wxEXPAND,
            0);
    }

    // Finalize panel
    panel->SetSizerAndFit(vSizer);
}

void PreferencesDialog::ReadSettings()
{
    assert(!!mUIPreferencesManager);

    mScreenshotDirPickerCtrl->SetPath(mUIPreferencesManager->GetScreenshotsFolderPath().string());

    mShowTipOnStartupCheckBox->SetValue(mUIPreferencesManager->GetShowStartupTip());
    mCheckForUpdatesAtStartupCheckBox->SetValue(mUIPreferencesManager->GetCheckUpdatesAtStartup());
    mSaveSettingsOnExitCheckBox->SetValue(mUIPreferencesManager->GetSaveSettingsOnExit());
    mShowTsunamiNotificationsCheckBox->SetValue(mUIPreferencesManager->GetDoShowTsunamiNotifications());
    mZoomIncrementSpinCtrl->SetValue(ZoomIncrementToZoomIncrementSpin(mUIPreferencesManager->GetZoomIncrement()));
    mPanIncrementSpinCtrl->SetValue(PanIncrementToPanIncrementSpin(mUIPreferencesManager->GetPanIncrement()));
    mShowStatusTextCheckBox->SetValue(mUIPreferencesManager->GetShowStatusText());
    mShowExtendedStatusTextCheckBox->SetValue(mUIPreferencesManager->GetShowExtendedStatusText());

    mShowShipDescriptionAtShipLoadCheckBox->SetValue(mUIPreferencesManager->GetShowShipDescriptionsAtShipLoad());
    mAutoZoomAtShipLoadCheckBox->SetValue(mUIPreferencesManager->GetDoAutoZoomAtShipLoad());
    mAutoShowSwitchboardCheckBox->SetValue(mUIPreferencesManager->GetAutoShowSwitchboard());
    mShowElectricalNotificationsCheckBox->SetValue(mUIPreferencesManager->GetDoShowElectricalNotifications());
    switch (mUIPreferencesManager->GetShipAutoTexturizationMode())
    {
        case ShipAutoTexturizationMode::FlatStructure:
        {
            mFlatStructureAutoTexturizationModeRadioButton->SetValue(true);
            break;
        }

        case ShipAutoTexturizationMode::MaterialTextures:
        {
            mMaterialTexturesAutoTexturizationModeRadioButton->SetValue(true);
            break;
        }
    }
    mMaterialTextureMagnificationSpinCtrl->SetValue(MaterialTextureMagnificationToMaterialTextureMagnificationSpin(mUIPreferencesManager->GetShipAutoTexturizationMaterialTextureMagnification()));

    mGlobalMuteCheckBox->SetValue(mUIPreferencesManager->GetGlobalMute());
    mBackgroundMusicVolumeSlider->SetValue(mUIPreferencesManager->GetBackgroundMusicVolume());
    mPlayBackgroundMusicCheckBox->SetValue(mUIPreferencesManager->GetPlayBackgroundMusic());
    mSinkingMusicVolumeSlider->SetValue(mUIPreferencesManager->GetGameMusicVolume());
    mPlaySinkingMusicCheckBox->SetValue(mUIPreferencesManager->GetPlaySinkingMusic());

    ReconcileSoundSettings();
}

float PreferencesDialog::ZoomIncrementSpinToZoomIncrement(int spinPosition)
{
    return 1.0f + static_cast<float>(spinPosition) * 2.0f / static_cast<float>(MaxZoomIncrementPosition);
}

int PreferencesDialog::ZoomIncrementToZoomIncrementSpin(float zoomIncrement)
{
    return static_cast<int>(std::round((zoomIncrement - 1.0f) * static_cast<float>(MaxZoomIncrementPosition) / 2.0f));
}

float PreferencesDialog::PanIncrementSpinToPanIncrement(int spinPosition)
{
    return static_cast<float>(spinPosition);
}

int PreferencesDialog::PanIncrementToPanIncrementSpin(float panIncrement)
{
    return static_cast<int>(panIncrement);
}

float PreferencesDialog::MaterialTextureMagnificationSpinToMaterialTextureMagnification(int spinPosition)
{
    if (spinPosition <= 50) // 0.02 -> 1.0
        return static_cast<float>(spinPosition) / 50.0f;
    else // 1.02 -> 2.0
        return 1.0f + 1.0f * static_cast<float>(spinPosition - 50) / 50.0f;
}

int PreferencesDialog::MaterialTextureMagnificationToMaterialTextureMagnificationSpin(float materialTextureMagnification)
{
    if (materialTextureMagnification <= 1.0f)
        return static_cast<int>(round(materialTextureMagnification * 50.0f));
    else
        return 50 + static_cast<int>(round((materialTextureMagnification - 1.0f) / 1.0f * 50.0f));
}

void PreferencesDialog::ReconciliateShipAutoTexturizationModeSettings()
{
    mMaterialTextureMagnificationSpinCtrl->Enable(mMaterialTexturesAutoTexturizationModeRadioButton->GetValue());
}

void PreferencesDialog::ReconcileSoundSettings()
{
    mBackgroundMusicVolumeSlider->Enable(!mGlobalMuteCheckBox->GetValue() && mPlayBackgroundMusicCheckBox->GetValue());
    mSinkingMusicVolumeSlider->Enable(!mGlobalMuteCheckBox->GetValue() && mPlaySinkingMusicCheckBox->GetValue());
}