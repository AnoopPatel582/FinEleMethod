#pragma once

#include <wx/frame.h>

#include <filesystem>

class wxButton;
class wxCommandEvent;
class wxTextCtrl;

namespace finelemethod::gui
{
class MainFrame final : public wxFrame
{
  public:
    MainFrame();

  private:
    void create_menu_bar();
    void create_content();
    void choose_abaqus_input(wxCommandEvent &event);
    void create_project(wxCommandEvent &event);

    wxTextCtrl *input_path_{};
    wxTextCtrl *project_path_{};
    wxButton *create_project_button_{};
    std::filesystem::path selected_input_file_;
};
} // namespace finelemethod::gui
