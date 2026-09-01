#pragma once

#include "finelemethod/project/analysis_run.hpp"
#include "finelemethod/project/project_file.hpp"

#include <wx/frame.h>

#include <filesystem>
#include <optional>

class wxButton;
class wxCloseEvent;
class wxCommandEvent;
class wxProcess;
class wxProcessEvent;
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
    void run_analysis(wxCommandEvent &event);
    void analysis_finished(wxProcessEvent &event);
    void close_window(wxCloseEvent &event);

    wxTextCtrl *input_path_{};
    wxTextCtrl *project_path_{};
    wxTextCtrl *run_path_{};
    wxButton *create_project_button_{};
    wxButton *run_button_{};
    std::filesystem::path selected_input_file_;
    std::optional<project::ProjectFile> active_project_;
    std::optional<project::AnalysisRun> active_run_;
    wxProcess *solver_process_{};
};
} // namespace finelemethod::gui
