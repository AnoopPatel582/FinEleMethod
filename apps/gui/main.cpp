#include <wx/button.h>
#include <wx/frame.h>
#include <wx/menu.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/wx.h>

namespace
{
class MainFrame final : public wxFrame
{
  public:
    MainFrame() : wxFrame(nullptr, wxID_ANY, "FinEleMethod", wxDefaultPosition, wxSize(900, 600))
    {
        SetMinSize(wxSize(720, 480));
        create_menu_bar();
        create_content();
        CreateStatusBar();
        SetStatusText("Ready");
        Centre();
    }

  private:
    void create_menu_bar()
    {
        auto *file_menu = new wxMenu;
        file_menu->Append(wxID_EXIT, "E&xit\tAlt+F4");

        auto *help_menu = new wxMenu;
        help_menu->Append(wxID_ABOUT, "&About FinEleMethod");

        auto *menu_bar = new wxMenuBar;
        menu_bar->Append(file_menu, "&File");
        menu_bar->Append(help_menu, "&Help");
        SetMenuBar(menu_bar);

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

    void create_content()
    {
        auto *panel = new wxPanel(this);
        auto *layout = new wxBoxSizer(wxVERTICAL);

        auto *title = new wxStaticText(panel, wxID_ANY, "FinEleMethod");
        wxFont title_font = title->GetFont();
        title_font.SetPointSize(20);
        title_font.SetWeight(wxFONTWEIGHT_BOLD);
        title->SetFont(title_font);

        auto *subtitle = new wxStaticText(
            panel, wxID_ANY,
            "Finite element analysis workbench for Q4 and H8 solid mechanics models");

        auto *solver_box = new wxStaticBoxSizer(wxVERTICAL, panel, "Solver status");
        solver_box->Add(new wxStaticText(panel, wxID_ANY, "Command-line solver engine: ready"), 0,
                        wxBOTTOM, 8);
        solver_box->Add(
            new wxStaticText(panel, wxID_ANY,
                             "Supported analyses: Q4 plane stress, Q4 plane strain, and H8 3D"),
            0, wxBOTTOM, 8);
        solver_box->Add(
            new wxStaticText(panel, wxID_ANY,
                             "The next GUI increment will add project and ABAQUS input selection."),
            0);

        auto *close_button = new wxButton(panel, wxID_CLOSE, "Close");
        close_button->Bind(wxEVT_BUTTON, [this](wxCommandEvent &) { Close(true); });

        layout->Add(title, 0, wxLEFT | wxRIGHT | wxTOP, 28);
        layout->Add(subtitle, 0, wxLEFT | wxRIGHT | wxTOP, 28);
        layout->Add(solver_box, 0, wxEXPAND | wxALL, 28);
        layout->AddStretchSpacer();
        layout->Add(close_button, 0, wxALIGN_RIGHT | wxLEFT | wxRIGHT | wxBOTTOM, 28);

        panel->SetSizer(layout);
    }
};

class FinEleMethodApp final : public wxApp
{
  public:
    bool OnInit() override
    {
        auto *frame = new MainFrame;
        frame->Show(true);
        return true;
    }
};
} // namespace

wxIMPLEMENT_APP(FinEleMethodApp);
