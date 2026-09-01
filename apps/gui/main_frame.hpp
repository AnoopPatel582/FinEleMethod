#pragma once

#include "finelemethod/output/analysis_summary.hpp"
#include "finelemethod/project/analysis_run.hpp"
#include "finelemethod/project/project_file.hpp"

#include <wx/frame.h>
#include <wx/timer.h>

#include <filesystem>
#include <optional>
#include <string>

class wxButton;
class wxCloseEvent;
class wxCommandEvent;
class wxProcess;
class wxProcessEvent;
class wxStaticText;
class wxTextCtrl;
class wxTimerEvent;

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
    void poll_analysis_progress(wxTimerEvent &event);
    void analysis_finished(wxProcessEvent &event);
    void open_result(wxCommandEvent &event);
    void close_window(wxCloseEvent &event);
    void read_process_output();
    void consume_progress_lines(bool include_incomplete_line);

    wxTextCtrl *input_path_{};
    wxTextCtrl *project_path_{};
    wxTextCtrl *run_path_{};
    wxStaticText *progress_text_{};
    wxStaticText *summary_text_{};
    wxButton *create_project_button_{};
    wxButton *run_button_{};
    wxButton *open_result_button_{};
    std::filesystem::path selected_input_file_;
    std::optional<project::ProjectFile> active_project_;
    std::optional<project::AnalysisRun> active_run_;
    std::optional<output::AnalysisSummary> completed_summary_;
    wxProcess *solver_process_{};
    wxTimer progress_timer_;
    std::string standard_output_buffer_;
    std::string standard_error_buffer_;
    std::string progress_protocol_error_;
};
} // namespace finelemethod::gui
