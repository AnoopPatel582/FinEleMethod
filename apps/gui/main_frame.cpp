#include "main_frame.hpp"

#include <wx/button.h>
#include <wx/filedlg.h>
#include <wx/font.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

namespace finelemethod::gui
{
namespace
{
constexpr int open_input_id = wxID_HIGHEST + 1;
}

MainFrame::MainFrame()
    : wxFrame(nullptr, wxID_ANY, "FinEleMethod", wxDefaultPosition, wxSize(900, 600))
{
    SetMinSize(wxSize(720, 480));
    create_menu_bar();
    create_content();
    CreateStatusBar();
    SetStatusText("Ready");
    Centre();
}

void MainFrame::create_menu_bar()
{
    auto *file_menu = new wxMenu;
    file_menu->Append(open_input_id, "&Open ABAQUS Input...\tCtrl+O");
    file_menu->AppendSeparator();
    file_menu->Append(wxID_EXIT, "E&xit\tAlt+F4");

    auto *help_menu = new wxMenu;
    help_menu->Append(wxID_ABOUT, "&About FinEleMethod");

    auto *menu_bar = new wxMenuBar;
    menu_bar->Append(file_menu, "&File");
    menu_bar->Append(help_menu, "&Help");
    SetMenuBar(menu_bar);

    Bind(wxEVT_MENU, &MainFrame::choose_abaqus_input, this, open_input_id);
    Bind(wxEVT_MENU, [this](wxCommandEvent &) { Close(true); }, wxID_EXIT);
    Bind(
        wxEVT_MENU,
        [this](wxCommandEvent &) {
            wxMessageBox("FinEleMethod Windows engineering workbench\n"
                         "C++20 finite element solver",
                         "About FinEleMethod", wxOK | wxICON_INFORMATION, this);
        },
        wxID_ABOUT);
}

void MainFrame::create_content()
{
    auto *panel = new wxPanel(this);
    auto *layout = new wxBoxSizer(wxVERTICAL);

    auto *title = new wxStaticText(panel, wxID_ANY, "FinEleMethod");
    wxFont title_font = title->GetFont();
    title_font.SetPointSize(20);
    title_font.SetWeight(wxFONTWEIGHT_BOLD);
    title->SetFont(title_font);

    auto *subtitle = new wxStaticText(
        panel, wxID_ANY, "Finite element analysis workbench for Q4 and H8 solid mechanics models");

    auto *input_box = new wxStaticBoxSizer(wxVERTICAL, panel, "ABAQUS model input");
    auto *input_row = new wxBoxSizer(wxHORIZONTAL);
    input_path_ = new wxTextCtrl(panel, wxID_ANY, "No input file selected", wxDefaultPosition,
                                 wxDefaultSize, wxTE_READONLY);
    auto *browse_button = new wxButton(panel, open_input_id, "Browse...");
    browse_button->Bind(wxEVT_BUTTON, &MainFrame::choose_abaqus_input, this);
    input_row->Add(input_path_, 1, wxEXPAND | wxRIGHT, 10);
    input_row->Add(browse_button, 0);
    input_box->Add(input_row, 0, wxEXPAND | wxALL, 10);

    auto *solver_box = new wxStaticBoxSizer(wxVERTICAL, panel, "Solver status");
    solver_box->Add(new wxStaticText(panel, wxID_ANY, "Command-line solver engine: ready"), 0,
                    wxBOTTOM, 8);
    solver_box->Add(
        new wxStaticText(panel, wxID_ANY,
                         "Supported analyses: Q4 plane stress, Q4 plane strain, and H8 3D"),
        0, wxBOTTOM, 8);
    solver_box->Add(
        new wxStaticText(panel, wxID_ANY,
                         "Select an ABAQUS input file to prepare a future analysis run."),
        0);

    auto *close_button = new wxButton(panel, wxID_CLOSE, "Close");
    close_button->Bind(wxEVT_BUTTON, [this](wxCommandEvent &) { Close(true); });

    layout->Add(title, 0, wxLEFT | wxRIGHT | wxTOP, 28);
    layout->Add(subtitle, 0, wxLEFT | wxRIGHT | wxTOP, 28);
    layout->Add(input_box, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 28);
    layout->Add(solver_box, 0, wxEXPAND | wxALL, 28);
    layout->AddStretchSpacer();
    layout->Add(close_button, 0, wxALIGN_RIGHT | wxLEFT | wxRIGHT | wxBOTTOM, 28);

    panel->SetSizer(layout);
}

void MainFrame::choose_abaqus_input(wxCommandEvent &)
{
    wxFileDialog dialog(this, "Select ABAQUS input file", wxEmptyString, wxEmptyString,
                        "ABAQUS input files (*.inp)|*.inp|All files (*.*)|*.*",
                        wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dialog.ShowModal() != wxID_OK)
    {
        return;
    }

    input_path_->SetValue(dialog.GetPath());
    SetStatusText("ABAQUS input selected");
}
} // namespace finelemethod::gui
