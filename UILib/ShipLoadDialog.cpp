/***************************************************************************************
* Original Author:		Gabriele Giuseppini
* Created:				2019-01-26
* Copyright:			Gabriele Giuseppini  (https://github.com/GabrieleGiuseppini)
***************************************************************************************/
#include "ShipLoadDialog.h"

#include "StandardSystemPaths.h"

#include <GameCore/Log.h>

#include <UILib/ShipDescriptionDialog.h>
#include <UILib/WxHelpers.h>

#include <wx/sizer.h>

constexpr int MinDirCtrlWidth = 260;
constexpr int MaxDirComboWidth = 650;

ShipLoadDialog::ShipLoadDialog(
    wxWindow * parent,
    ResourceLocator const & resourceLocator)
	: mParent(parent)
    , mResourceLocator(resourceLocator)
    , mStandardInstalledShipFolderPath(resourceLocator.GetInstalledShipFolderPath())
    , mUserShipFolderPath(StandardSystemPaths::GetInstance().GetUserShipFolderPath())
{
	Create(
		mParent,
		wxID_ANY,
        _("Load Ship"),
		wxDefaultPosition,
        wxDefaultSize,
        wxCAPTION | wxRESIZE_BORDER | wxCLOSE_BOX | wxFRAME_SHAPED
#if !defined(_DEBUG) || !defined(_WIN32)
            | wxSTAY_ON_TOP
#endif
            ,
        wxS("Load Ship Dialog"));

	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));

    Bind(wxEVT_CLOSE_WINDOW, &ShipLoadDialog::OnCloseWindow, this);

    //
    // Bitmaps
    //

    mSortByNameIcon = WxHelpers::LoadBitmap("sort_by_name_button", resourceLocator);
    mSortByLastModifiedIcon = WxHelpers::LoadBitmap("sort_by_time_button", resourceLocator);
    mSortByYearBuiltIcon = WxHelpers::LoadBitmap("sort_by_year_button", resourceLocator);
    mSortByFeaturesIcon = WxHelpers::LoadBitmap("sort_by_features_button", resourceLocator);
    mSortAscendingIcon = WxHelpers::LoadBitmap("arrow_down_medium", resourceLocator);
    mSortDescendingIcon = WxHelpers::LoadBitmap("arrow_up_medium", resourceLocator);

    //
    // Layout controls
    //

    wxBoxSizer * vSizer = new wxBoxSizer(wxVERTICAL);

    //
    // [ Directory tree ] [ Toolbar ]
    // [                ] [ Preview ]
    //

    {
        wxBoxSizer * hSizer1 = new wxBoxSizer(wxHORIZONTAL);

        // Directory tree
        {
            LogMessage("ShipLoadDialog::cctor(): creating wxGenericDirCtrl...");

            auto const minSize = wxSize(MinDirCtrlWidth, 680);

            mDirCtrl = new wxGenericDirCtrl(
                this,
                wxID_ANY,
                "", // Start with empty dir
                wxDefaultPosition,
                minSize,
                wxDIRCTRL_DIR_ONLY);

            LogMessage("ShipLoadDialog::cctor(): ...wxGenericDirCtrl created.");

            mDirCtrl->ShowHidden(true); // When installing per-user, the Ships folder is under AppData, which is hidden
            mDirCtrl->SetMinSize(minSize);
            mDirCtrl->Bind(wxEVT_DIRCTRL_SELECTIONCHANGED, (wxObjectEventFunction)&ShipLoadDialog::OnDirCtrlDirSelected, this);

            hSizer1->Add(mDirCtrl, 0, wxEXPAND | wxALIGN_TOP);
        }

        {
            wxBoxSizer * vSizer1 = new wxBoxSizer(wxVERTICAL);

            vSizer1->AddSpacer(5);

            // Toolbar
            {                
                int constexpr LargeMargin = 30;

                wxBoxSizer * hToolbarSizer = new wxBoxSizer(wxHORIZONTAL);

                hToolbarSizer->AddStretchSpacer();

                // Info button
                {
                    mInfoButton = new wxBitmapButton(this, wxID_ANY, WxHelpers::LoadBitmap("info", resourceLocator), wxDefaultPosition, wxDefaultSize);
                    mInfoButton->SetToolTip(_("See ship information"));
                    mInfoButton->Bind(wxEVT_BUTTON, &ShipLoadDialog::OnInfoButtonClicked, this);

                    hToolbarSizer->Add(
                        mInfoButton,
                        0,
                        wxALIGN_BOTTOM,
                        0);
                }

                hToolbarSizer->AddSpacer(LargeMargin);

                // Load modifiers buttons
                // TODO

                // Sort method button
                {
                    mSortMethodButton = new wxBitmapButton(this, wxID_ANY, mSortByFeaturesIcon, wxDefaultPosition, wxDefaultSize);
                    mSortMethodButton->SetToolTip(_("Change order of ships"));
                    mSortMethodButton->Bind(wxEVT_BUTTON,
                        [this](wxCommandEvent &)
                        {
                            mSortMethodSelectionPopupWindow->SetPosition(this->ClientToScreen(mSortMethodButton->GetPosition() + mSortMethodButton->GetSize() / 2));
                            mSortMethodSelectionPopupWindow->Popup();
                        });

                    hToolbarSizer->Add(
                        mSortMethodButton,
                        0,
                        wxALIGN_BOTTOM,
                        0);
                }

                // Sort direction button
                {
                    mSortDirectionButton = new wxBitmapButton(this, wxID_ANY, mSortAscendingIcon, wxDefaultPosition, wxDefaultSize);
                    mSortDirectionButton->SetToolTip(_("Change direction of ships' order"));
                    mSortDirectionButton->Bind(wxEVT_BUTTON,
                        [this](wxCommandEvent &)
                        {
                            OnSortDirectionChanged(!mShipPreviewWindow->GetCurrentIsSortDescending());
                        });

                    hToolbarSizer->Add(
                        mSortDirectionButton,
                        0,
                        wxALIGN_BOTTOM,
                        0);
                }

                hToolbarSizer->AddSpacer(LargeMargin);

                // Search
                {
                    wxBoxSizer * vSearchSizer = new wxBoxSizer(wxVERTICAL);

                    // Label
                    {
                        wxStaticText * searchLabel = new wxStaticText(this, wxID_ANY, _("Search in this folder:"));
                        vSearchSizer->Add(searchLabel, 0, wxALIGN_LEFT | wxEXPAND);
                    }

                    // Search box + button
                    {
                        wxBoxSizer * hSearchSizer = new wxBoxSizer(wxHORIZONTAL);

                        // Search box

                        mShipSearchCtrl = new wxSearchCtrl(
                            this,
                            wxID_ANY,
                            wxEmptyString,
                            wxDefaultPosition,
                            wxSize(-1, 24),
                            0);

                        mShipSearchCtrl->ShowCancelButton(true);
                        mShipSearchCtrl->Bind(wxEVT_TEXT, &ShipLoadDialog::OnShipSearchCtrlText, this);
                        mShipSearchCtrl->Bind(wxEVT_SEARCHCTRL_SEARCH_BTN, &ShipLoadDialog::OnShipSearchCtrlSearchBtn, this);
                        mShipSearchCtrl->Bind(wxEVT_SEARCHCTRL_CANCEL_BTN, &ShipLoadDialog::OnShipSearchCtrlCancelBtn, this);

                        hSearchSizer->Add(mShipSearchCtrl, 1, wxALIGN_CENTRE_VERTICAL);

                        // Search button

                        wxBitmap searchNextBitmap(mResourceLocator.GetIconFilePath("right_arrow").string(), wxBITMAP_TYPE_PNG);
                        mSearchNextButton = new wxBitmapButton(this, wxID_ANY, searchNextBitmap, wxDefaultPosition, wxDefaultSize);
                        mSearchNextButton->SetToolTip(_("Go to the next search result"));
                        mSearchNextButton->Bind(wxEVT_BUTTON, &ShipLoadDialog::OnSearchNextButtonClicked, this);

                        hSearchSizer->Add(mSearchNextButton, 0, wxALIGN_CENTRE_VERTICAL);

                        vSearchSizer->Add(hSearchSizer, 1, wxALIGN_LEFT | wxEXPAND);
                    }

                    hToolbarSizer->Add(
                        vSearchSizer,
                        0,
                        wxALIGN_BOTTOM,
                        0);
                }

                hToolbarSizer->AddStretchSpacer();

                vSizer1->Add(
                    hToolbarSizer, 
                    0, 
                    wxEXPAND, // Expand to occupy all available H space
                    0);
            }

            vSizer1->AddSpacer(5);

            // Preview
            {
                mShipPreviewWindow = new ShipPreviewWindow(this, mResourceLocator);

                mShipPreviewWindow->SetMinSize(wxSize(ShipPreviewWindow::CalculateMinWidthForColumns(3) + 40, -1));
                mShipPreviewWindow->Bind(fsEVT_SHIP_FILE_SELECTED, &ShipLoadDialog::OnShipFileSelected, this);
                mShipPreviewWindow->Bind(fsEVT_SHIP_FILE_CHOSEN, &ShipLoadDialog::OnShipFileChosen, this);

                vSizer1->Add(
                    mShipPreviewWindow, 
                    1,  // Use all V space
                    wxEXPAND, // Expand to occupy all available H space
                    0);
            }

            hSizer1->Add(vSizer1, 1, wxALIGN_TOP | wxEXPAND);
        }

        vSizer->Add(hSizer1, 1, wxEXPAND);
    }

    vSizer->AddSpacer(10);

    //
    // Recent directories combo and home button
    //

    {
        // |  | Label       |       |              |  |
        // |  | Combo, Home |       | Load, Cancel |  |

        wxFlexGridSizer * gridSizer = new wxFlexGridSizer(2, 5, 0, 0);

        gridSizer->AddGrowableCol(1, 4);
        gridSizer->AddGrowableCol(3, 1);

        //
        // ROW 1
        //

        gridSizer->AddSpacer(10);

        {
            wxStaticText * recentDirsLabel = new wxStaticText(this, wxID_ANY, _("Recent directories:"));
            gridSizer->Add(recentDirsLabel, 4, wxALIGN_LEFT | wxEXPAND | wxALL);
        }

        gridSizer->AddSpacer(10);

        gridSizer->AddSpacer(0);

        gridSizer->AddSpacer(10);


        //
        // ROW 2
        //

        gridSizer->AddSpacer(10);

        {
            wxBoxSizer * hComboSizer = new wxBoxSizer(wxHORIZONTAL);

            // Combo

            wxArrayString emptyComboChoices;
            mRecentDirectoriesComboBox = new wxComboBox(
                this,
                wxID_ANY,
                wxEmptyString,
                wxDefaultPosition,
                wxDefaultSize,
                emptyComboChoices,
                wxCB_DROPDOWN | wxCB_READONLY);

			mRecentDirectoriesComboBox->SetMaxSize(wxSize(MaxDirComboWidth, -1));
            mRecentDirectoriesComboBox->Bind(wxEVT_COMBOBOX, &ShipLoadDialog::OnRecentDirectorySelected, this);

            hComboSizer->Add(mRecentDirectoriesComboBox, 1, wxALIGN_CENTRE_VERTICAL);

            hComboSizer->AddSpacer(4);

            // Standard HomeDir button
            {
                wxBitmapButton * homeDirButton = new wxBitmapButton(this, wxID_ANY,
                    WxHelpers::LoadBitmap("home", resourceLocator), wxDefaultPosition, wxDefaultSize);
                homeDirButton->SetToolTip(_("Go to the default Ships folder"));
                homeDirButton->Bind(wxEVT_BUTTON, &ShipLoadDialog::OnStandardHomeDirButtonClicked, this);

                hComboSizer->Add(homeDirButton, 0, wxALIGN_CENTRE_VERTICAL);
            }

            hComboSizer->AddSpacer(4);

            // User HomeDir button
            {
                wxBitmapButton * homeDirButton = new wxBitmapButton(this, wxID_ANY,
                    WxHelpers::LoadBitmap("home_user", resourceLocator), wxDefaultPosition, wxDefaultSize);
                homeDirButton->SetToolTip(_("Go to your Ships folder"));
                homeDirButton->Bind(wxEVT_BUTTON, &ShipLoadDialog::OnUserHomeDirButtonClicked, this);

                hComboSizer->Add(homeDirButton, 0, wxALIGN_CENTRE_VERTICAL);
            }

            gridSizer->Add(hComboSizer, 1, wxALIGN_LEFT | wxEXPAND | wxALL);
        }

        gridSizer->AddSpacer(10);

        {
            wxBoxSizer * buttonsSizer = new wxBoxSizer(wxHORIZONTAL);

            buttonsSizer->AddStretchSpacer(1);

            mLoadButton = new wxButton(this, wxID_ANY, _("Load"));
            mLoadButton->Bind(wxEVT_BUTTON, &ShipLoadDialog::OnLoadButton, this);
            buttonsSizer->Add(mLoadButton, 0);

            buttonsSizer->AddSpacer(20);

            wxButton * cancelButton = new wxButton(this, wxID_ANY, _("Cancel"));
            cancelButton->Bind(wxEVT_BUTTON, &ShipLoadDialog::OnCancelButton, this);
            buttonsSizer->Add(cancelButton, 0);

            buttonsSizer->AddSpacer(10);

            gridSizer->Add(buttonsSizer, 0, wxEXPAND);
        }

        gridSizer->AddSpacer(10);

        vSizer->Add(gridSizer, 0, wxEXPAND | wxALL);
    }

    vSizer->AddSpacer(15);

    //
    // Finalize layout
    //

    SetSizerAndFit(vSizer);

    int const TotalWidth = MinDirCtrlWidth + mShipPreviewWindow->GetMinWidth() + 10;
    SetSize(wxSize(TotalWidth, 600 * TotalWidth / 800));

    Centre();

    //
    // Create sort method popup window
    //

    {
        mSortMethodSelectionPopupWindow = new wxPopupTransientWindow(this, wxPU_CONTAINS_CONTROLS | wxBORDER_SIMPLE);

        auto * tmpVSizer = new wxBoxSizer(wxVERTICAL);

        tmpVSizer->AddSpacer(5);

        {
            auto * hSizer = new wxBoxSizer(wxHORIZONTAL);

            {
                int constexpr HHalfMargin = 10;

                hSizer->AddSpacer(HHalfMargin);

                // By name
                {
                    auto * button = new wxBitmapButton(mSortMethodSelectionPopupWindow, wxID_ANY, mSortByNameIcon, wxDefaultPosition, wxDefaultSize);
                    button->SetToolTip(_("Order ship by name"));
                    button->Bind(wxEVT_BUTTON,
                        [this](wxCommandEvent &)
                        {
                            mSortMethodSelectionPopupWindow->Dismiss();

                            OnSortMethodChanged(ShipPreviewWindow::SortMethod::ByName);
                        });

                    hSizer->Add(
                        button,
                        0,
                        wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT,
                        HHalfMargin);
                }

                // By last modified
                {
                    auto * button = new wxBitmapButton(mSortMethodSelectionPopupWindow, wxID_ANY, mSortByLastModifiedIcon, wxDefaultPosition, wxDefaultSize);
                    button->SetToolTip(_("Order ship by last modified time"));
                    button->Bind(wxEVT_BUTTON,
                        [this](wxCommandEvent &)
                        {
                            mSortMethodSelectionPopupWindow->Dismiss();

                            OnSortMethodChanged(ShipPreviewWindow::SortMethod::ByLastModified);
                        });

                    hSizer->Add(
                        button,
                        0,
                        wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT,
                        HHalfMargin);
                }

                // By year built
                {
                    auto * button = new wxBitmapButton(mSortMethodSelectionPopupWindow, wxID_ANY, mSortByYearBuiltIcon, wxDefaultPosition, wxDefaultSize);
                    button->SetToolTip(_("Order ship by year built"));
                    button->Bind(wxEVT_BUTTON,
                        [this](wxCommandEvent &)
                        {
                            mSortMethodSelectionPopupWindow->Dismiss();

                            OnSortMethodChanged(ShipPreviewWindow::SortMethod::ByYearBuilt);
                        });

                    hSizer->Add(
                        button,
                        0,
                        wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT,
                        HHalfMargin);
                }

                // By features
                {
                    auto * button = new wxBitmapButton(mSortMethodSelectionPopupWindow, wxID_ANY, mSortByFeaturesIcon, wxDefaultPosition, wxDefaultSize);
                    button->SetToolTip(_("Order ship by features"));
                    button->Bind(wxEVT_BUTTON,
                        [this](wxCommandEvent &)
                        {
                            mSortMethodSelectionPopupWindow->Dismiss();

                            OnSortMethodChanged(ShipPreviewWindow::SortMethod::ByFeatures);
                        });

                    hSizer->Add(
                        button,
                        0,
                        wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT,
                        HHalfMargin);
                }

                hSizer->AddSpacer(HHalfMargin);
            }

            tmpVSizer->Add(
                hSizer,
                0,
                wxEXPAND,
                0);
        }

        tmpVSizer->AddSpacer(5);

        mSortMethodSelectionPopupWindow->SetSizerAndFit(tmpVSizer);
    }

    //
    // Initialize UI
    //

    ReconciliateUIWithSortMethod();
}

ShipLoadDialog::~ShipLoadDialog()
{
}

int ShipLoadDialog::ShowModal(std::vector<std::filesystem::path> const & shipLoadDirectories)
{
    // Reset our current ship selection
    mSelectedShipMetadata.reset();
    mSelectedShipFilepath.reset();
    mChosenShipFilepath.reset();

    // Disable controls
    mInfoButton->Enable(false);
    mLoadButton->Enable(false);

    // Clear search
    mShipSearchCtrl->Clear();
    mSearchNextButton->Enable(false);

    // Populate recent directories
    RepopulateRecentDirectoriesComboBox(shipLoadDirectories);

    //
    // Initialize preview panel
    //

    mShipPreviewWindow->OnOpen();

    auto selectedPath = mDirCtrl->GetPath();
    if (!selectedPath.IsEmpty())
        mShipPreviewWindow->SetDirectory(std::filesystem::path(selectedPath.ToStdString()));

    // Run self as modal
    return wxDialog::ShowModal();
}

void ShipLoadDialog::OnDirCtrlDirSelected(wxCommandEvent & /*event*/)
{
    std::filesystem::path selectedDirPath(mDirCtrl->GetPath().ToStdString());

    OnDirectorySelected(selectedDirPath);
}

void ShipLoadDialog::OnShipFileSelected(fsShipFileSelectedEvent & event)
{
    // Store selection
    if (!!event.GetShipMetadata())
        mSelectedShipMetadata.emplace(*event.GetShipMetadata());
    else
        mSelectedShipMetadata.reset();
    mSelectedShipFilepath = event.GetShipFilepath();

    // Enable buttons
    mInfoButton->Enable(!!(event.GetShipMetadata()) && !!(event.GetShipMetadata()->Description));
    mLoadButton->Enable(true);
}

void ShipLoadDialog::OnShipFileChosen(fsShipFileChosenEvent & event)
{
    // Store selection
    mSelectedShipFilepath = event.GetShipFilepath();

    // Process
    OnShipFileChosen(*mSelectedShipFilepath);

    // Do not continue processing, as OnShipFileChosen() will fire event again
}

void ShipLoadDialog::OnRecentDirectorySelected(wxCommandEvent & /*event*/)
{
    mDirCtrl->SetPath(mRecentDirectoriesComboBox->GetValue()); // Will send its own event
}

void ShipLoadDialog::OnShipSearchCtrlText(wxCommandEvent & /*event*/)
{
    StartShipSearch();
}

void ShipLoadDialog::OnShipSearchCtrlSearchBtn(wxCommandEvent & /*event*/)
{
    mShipPreviewWindow->ChooseSelectedIfAny();
}

void ShipLoadDialog::OnShipSearchCtrlCancelBtn(wxCommandEvent & /*event*/)
{
    mShipSearchCtrl->Clear();
    mSearchNextButton->Enable(false);
}

void ShipLoadDialog::OnSearchNextButtonClicked(wxCommandEvent & /*event*/)
{
    auto searchString = mShipSearchCtrl->GetValue();
    assert(!searchString.IsEmpty());

    mShipPreviewWindow->Search(searchString.ToStdString());
}

void ShipLoadDialog::OnStandardHomeDirButtonClicked(wxCommandEvent & /*event*/)
{
    assert(mRecentDirectoriesComboBox->GetCount() >= 1);

    // Change combo
    mRecentDirectoriesComboBox->Select(0);

    // Change dir tree
    mDirCtrl->SetPath(mRecentDirectoriesComboBox->GetValue()); // Will send its own event
}

void ShipLoadDialog::OnUserHomeDirButtonClicked(wxCommandEvent & /*event*/)
{
    assert(mRecentDirectoriesComboBox->GetCount() >= 2);

    // Change combo
    mRecentDirectoriesComboBox->Select(1);

    // Change dir tree
    mDirCtrl->SetPath(mRecentDirectoriesComboBox->GetValue()); // Will send its own event
}

void ShipLoadDialog::OnSortMethodChanged(ShipPreviewWindow::SortMethod sortMethod)
{
    mShipPreviewWindow->SetSortMethod(sortMethod);
    ReconciliateUIWithSortMethod();
}

void ShipLoadDialog::OnSortDirectionChanged(bool isSortDescending)
{
    mShipPreviewWindow->SetIsSortDescending(isSortDescending);
    ReconciliateUIWithSortMethod();
}

void ShipLoadDialog::OnInfoButtonClicked(wxCommandEvent & /*event*/)
{
    assert(!!mSelectedShipMetadata);

    if (!!(mSelectedShipMetadata->Description))
    {
        ShipDescriptionDialog shipDescriptionDialog(
            this,
            *mSelectedShipMetadata,
            false,
            mResourceLocator);

        shipDescriptionDialog.ShowModal();
    }
}

void ShipLoadDialog::OnLoadButton(wxCommandEvent & /*event*/)
{
    assert(!!mSelectedShipFilepath);

    // Process
    OnShipFileChosen(*mSelectedShipFilepath);
}

void ShipLoadDialog::OnCancelButton(wxCommandEvent & /*event*/)
{
    EndModal(wxID_CANCEL);
}

void ShipLoadDialog::OnCloseWindow(wxCloseEvent & /*event*/)
{
    // Invoked when the user has tried to close a frame or dialog box
    // using the window manager (X) or system menu (Windows); it can also be invoked by the application itself
    EndModal(wxID_CANCEL);
}

void ShipLoadDialog::OnDirectorySelected(std::filesystem::path directoryPath)
{
    // Reset our current selection
    mSelectedShipMetadata.reset();
    mSelectedShipFilepath.reset();

    // Disable controls
    mInfoButton->Enable(false);
    mLoadButton->Enable(false);

    // Clear search
    mShipSearchCtrl->Clear();
    mSearchNextButton->Enable(false);

    // Propagate to preview panel
    mShipPreviewWindow->SetDirectory(directoryPath);
}

void ShipLoadDialog::OnShipFileChosen(std::filesystem::path shipFilepath)
{
    LogMessage("ShipLoadDialog::OnShipFileChosen: ", shipFilepath);

    // Store path
    mChosenShipFilepath = shipFilepath;

    // End modal dialog
    EndModal(wxID_OK);
}

////////////////////////////////////////////////////////////////////////////

void ShipLoadDialog::EndModal(int retCode)
{
    LogMessage("ShipLoadDialog::EndModal(", retCode, ")");

    mShipPreviewWindow->OnClose();

    wxDialog::EndModal(retCode);
}

void ShipLoadDialog::ReconciliateUIWithSortMethod()
{
    switch (mShipPreviewWindow->GetCurrentSortMethod())
    {
        case ShipPreviewWindow::SortMethod::ByFeatures:
        {
            mSortMethodButton->SetBitmap(mSortByFeaturesIcon);
            break;
        }

        case ShipPreviewWindow::SortMethod::ByLastModified:
        {
            mSortMethodButton->SetBitmap(mSortByLastModifiedIcon);
            break;
        }

        case ShipPreviewWindow::SortMethod::ByName:
        {
            mSortMethodButton->SetBitmap(mSortByNameIcon);
            break;
        }

        case ShipPreviewWindow::SortMethod::ByYearBuilt:
        {
            mSortMethodButton->SetBitmap(mSortByYearBuiltIcon);
            break;
        }
    }

    mSortDirectionButton->SetBitmap(mShipPreviewWindow->GetCurrentIsSortDescending() ? mSortDescendingIcon : mSortAscendingIcon);
}

void ShipLoadDialog::StartShipSearch()
{
    bool found = false;

    auto searchString = mShipSearchCtrl->GetValue();
    if (!searchString.IsEmpty())
    {
        found = mShipPreviewWindow->Search(searchString.ToStdString());
    }

    mSearchNextButton->Enable(found);
}

void ShipLoadDialog::RepopulateRecentDirectoriesComboBox(std::vector<std::filesystem::path> const & shipLoadDirectories)
{
    // Get currently-selected directory
    wxString const currentlySelectedDir = mDirCtrl->GetPath();

    // Clear recent directories combo box
    mRecentDirectoriesComboBox->Clear();

    // Add standard paths - always at first places
    mRecentDirectoriesComboBox->Append(mStandardInstalledShipFolderPath.string());
    mRecentDirectoriesComboBox->Append(mUserShipFolderPath.string());

    // Add all other paths
    for (auto const & dir : shipLoadDirectories)
    {
        if (std::filesystem::exists(dir)
            && dir != mStandardInstalledShipFolderPath
            && dir != mUserShipFolderPath)
        {
            assert(mRecentDirectoriesComboBox->FindString(dir.string()) == wxNOT_FOUND);
            mRecentDirectoriesComboBox->Append(dir.string());
        }
    }

    // Re-select currently-selected directory, as long as it's in the list of recent directories
    wxString dirToSelect;
    if (!currentlySelectedDir.empty()
        && mRecentDirectoriesComboBox->FindString(currentlySelectedDir) != wxNOT_FOUND)
    {
        dirToSelect = currentlySelectedDir;
    }
    else
    {
        dirToSelect = mRecentDirectoriesComboBox->GetString(0);
    }

    mDirCtrl->SetPath(dirToSelect);
    mRecentDirectoriesComboBox->SetValue(dirToSelect);
}