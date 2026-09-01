#include "main_frame.hpp"

#include "finelemethod/output/analysis_progress.hpp"

#include <wx/button.h>
#include <wx/dirdlg.h>
#include <wx/filedlg.h>
#include <wx/font.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/panel.h>
#include <wx/process.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/stdpaths.h>
#include <wx/stream.h>
#include <wx/textctrl.h>
#include <wx/textdlg.h>

#include <exception>
#include <stdexcept>
#include <string>
#include <vector>

namespace finelemethod::gui
{
namespace
{
constexpr int open_input_id = wxID_HIGHEST + 1;
constexpr int create_project_id = wxID_HIGHEST + 2;
constexpr int run_analysis_id = wxID_HIGHEST + 3;
constexpr int solver_process_id = wxID_HIGHEST + 4;
constexpr int progress_timer_id = wxID_HIGHEST + 5;
} // namespace

MainFrame::MainFrame()
    : wxFrame(nullptr, wxID_ANY, "FinEleMethod", wxDefaultPosition, wxSize(900, 680)),
      progress_timer_(this, progress_timer_id)
{
    SetMinSize(wxSize(720, 560));
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
    file_menu->Append(create_project_id, "&Create Project...\tCtrl+Shift+N");
    file_menu->AppendSeparator();
    file_menu->Append(wxID_EXIT, "E&xit\tAlt+F4");

    auto *help_menu = new wxMenu;
    help_menu->Append(wxID_ABOUT, "&About FinEleMethod");

    auto *menu_bar = new wxMenuBar;
    menu_bar->Append(file_menu, "&File");
    auto *analysis_menu = new wxMenu;
    analysis_menu->Append(run_analysis_id, "&Run Analysis\tF5");
    menu_bar->Append(analysis_menu, "&Analysis");
    menu_bar->Append(help_menu, "&Help");
    SetMenuBar(menu_bar);
    menu_bar->Enable(create_project_id, false);
    menu_bar->Enable(run_analysis_id, false);

    Bind(wxEVT_MENU, &MainFrame::choose_abaqus_input, this, open_input_id);
    Bind(wxEVT_MENU, &MainFrame::create_project, this, create_project_id);
    Bind(wxEVT_MENU, &MainFrame::run_analysis, this, run_analysis_id);
    Bind(wxEVT_END_PROCESS, &MainFrame::analysis_finished, this, solver_process_id);
    Bind(wxEVT_TIMER, &MainFrame::poll_analysis_progress, this, progress_timer_id);
    Bind(wxEVT_CLOSE_WINDOW, &MainFrame::close_window, this);
    Bind(wxEVT_MENU, [this](wxCommandEvent &) { Close(); }, wxID_EXIT);
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

    auto *project_box = new wxStaticBoxSizer(wxVERTICAL, panel, "FinEleMethod project");
    auto *project_row = new wxBoxSizer(wxHORIZONTAL);
    project_path_ = new wxTextCtrl(panel, wxID_ANY, "No project created", wxDefaultPosition,
                                   wxDefaultSize, wxTE_READONLY);
    create_project_button_ = new wxButton(panel, create_project_id, "Create Project...");
    create_project_button_->Disable();
    create_project_button_->Bind(wxEVT_BUTTON, &MainFrame::create_project, this);
    project_row->Add(project_path_, 1, wxEXPAND | wxRIGHT, 10);
    project_row->Add(create_project_button_, 0);
    project_box->Add(project_row, 0, wxEXPAND | wxALL, 10);

    auto *solver_box = new wxStaticBoxSizer(wxVERTICAL, panel, "Solver status");
    solver_box->Add(new wxStaticText(panel, wxID_ANY, "Command-line solver engine: ready"), 0,
                    wxBOTTOM, 8);
    solver_box->Add(
        new wxStaticText(panel, wxID_ANY,
                         "Supported analyses: Q4 plane stress, Q4 plane strain, and H8 3D"),
        0, wxBOTTOM, 8);
    solver_box->Add(new wxStaticText(panel, wxID_ANY,
                                     "Create a project, then run its authoritative ABAQUS model."),
                    0, wxBOTTOM, 10);
    auto *run_row = new wxBoxSizer(wxHORIZONTAL);
    run_path_ = new wxTextCtrl(panel, wxID_ANY, "No analysis run prepared", wxDefaultPosition,
                               wxDefaultSize, wxTE_READONLY);
    run_button_ = new wxButton(panel, run_analysis_id, "Run Analysis");
    run_button_->Disable();
    run_button_->Bind(wxEVT_BUTTON, &MainFrame::run_analysis, this);
    run_row->Add(run_path_, 1, wxEXPAND | wxRIGHT, 10);
    run_row->Add(run_button_, 0);
    solver_box->Add(run_row, 0, wxEXPAND);
    progress_text_ = new wxStaticText(panel, wxID_ANY, "Progress: idle");
    solver_box->Add(progress_text_, 0, wxTOP, 10);

    auto *close_button = new wxButton(panel, wxID_CLOSE, "Close");
    close_button->Bind(wxEVT_BUTTON, [this](wxCommandEvent &) { Close(); });

    layout->Add(title, 0, wxLEFT | wxRIGHT | wxTOP, 28);
    layout->Add(subtitle, 0, wxLEFT | wxRIGHT | wxTOP, 28);
    layout->Add(input_box, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 28);
    layout->Add(project_box, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 28);
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

    selected_input_file_ = std::filesystem::path{dialog.GetPath().ToStdWstring()};
    active_project_.reset();
    active_run_.reset();
    input_path_->SetValue(dialog.GetPath());
    project_path_->SetValue("No project created");
    run_path_->SetValue("No analysis run prepared");
    progress_text_->SetLabel("Progress: idle");
    create_project_button_->Enable();
    run_button_->Disable();
    GetMenuBar()->Enable(create_project_id, true);
    GetMenuBar()->Enable(run_analysis_id, false);
    SetStatusText("ABAQUS input selected");
}

void MainFrame::create_project(wxCommandEvent &)
{
    if (selected_input_file_.empty())
    {
        wxMessageBox("Select an ABAQUS .inp file before creating a project.", "Input required",
                     wxOK | wxICON_WARNING, this);
        return;
    }

    wxTextEntryDialog name_dialog(this,
                                  "Enter a name using letters, numbers, hyphens, or underscores.",
                                  "Create FinEleMethod Project", "NewProject");
    if (name_dialog.ShowModal() != wxID_OK)
    {
        return;
    }

    wxDirDialog directory_dialog(this, "Select the folder that will contain the new project",
                                 wxEmptyString, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    if (directory_dialog.ShowModal() != wxID_OK)
    {
        return;
    }

    try
    {
        const auto project = finelemethod::project::create_project(
            std::filesystem::path{directory_dialog.GetPath().ToStdWstring()},
            name_dialog.GetValue().ToStdString(), selected_input_file_);

        project_path_->SetValue(wxString{project.project_file.wstring()});
        selected_input_file_ = project.input_file;
        active_project_ = project;
        active_run_.reset();
        input_path_->SetValue(wxString{project.input_file.wstring()});
        run_path_->SetValue("No analysis run prepared");
        progress_text_->SetLabel("Progress: ready");
        create_project_button_->Disable();
        run_button_->Enable();
        GetMenuBar()->Enable(create_project_id, false);
        GetMenuBar()->Enable(run_analysis_id, true);
        SetStatusText("Project created");
        wxMessageBox("Project created successfully.\n\n" +
                         wxString{project.project_directory.wstring()},
                     "FinEleMethod Project", wxOK | wxICON_INFORMATION, this);
    }
    catch (const std::exception &exception)
    {
        SetStatusText("Project creation failed");
        wxMessageBox(wxString::FromUTF8(exception.what()), "Could not create project",
                     wxOK | wxICON_ERROR, this);
    }
}

void MainFrame::run_analysis(wxCommandEvent &)
{
    if (!active_project_ || solver_process_ != nullptr)
    {
        return;
    }

    const std::filesystem::path gui_executable{
        wxStandardPaths::Get().GetExecutablePath().ToStdWstring()};
    const std::filesystem::path solver_executable =
        gui_executable.parent_path() / "FinEleMethod.exe";
    if (!std::filesystem::is_regular_file(solver_executable))
    {
        wxMessageBox("The command-line solver was not found beside the workbench.\n\n" +
                         wxString{solver_executable.wstring()},
                     "Solver not found", wxOK | wxICON_ERROR, this);
        return;
    }

    try
    {
        active_run_ = project::prepare_analysis_run(*active_project_);

        std::vector<std::wstring> arguments{solver_executable.wstring(), L"--request",
                                            active_run_->request_file.wstring()};
        std::vector<const wchar_t *> argument_pointers;
        argument_pointers.reserve(arguments.size() + 1);
        for (const auto &argument : arguments)
        {
            argument_pointers.push_back(argument.c_str());
        }
        argument_pointers.push_back(nullptr);

        auto *process = new wxProcess(this, solver_process_id);
        process->Redirect();
        const long process_id =
            wxExecute(argument_pointers.data(), wxEXEC_ASYNC | wxEXEC_HIDE_CONSOLE, process);
        if (process_id == 0)
        {
            delete process;
            throw std::runtime_error("Windows could not start the command-line solver.");
        }

        solver_process_ = process;
        standard_output_buffer_.clear();
        standard_error_buffer_.clear();
        progress_protocol_error_.clear();
        run_path_->SetValue(wxString{active_run_->run_directory.wstring()});
        progress_text_->SetLabel("Progress: starting solver");
        run_button_->Disable();
        GetMenuBar()->Enable(run_analysis_id, false);
        progress_timer_.Start(100);
        SetStatusText("Analysis running");
    }
    catch (const std::exception &exception)
    {
        active_run_.reset();
        SetStatusText("Analysis could not start");
        wxMessageBox(wxString::FromUTF8(exception.what()), "Could not run analysis",
                     wxOK | wxICON_ERROR, this);
    }
}

void MainFrame::poll_analysis_progress(wxTimerEvent &)
{
    read_process_output();
}

void MainFrame::read_process_output()
{
    if (solver_process_ == nullptr)
    {
        return;
    }

    const auto read_available = [](wxInputStream *stream, const auto is_available,
                                   std::string &buffer) {
        char bytes[4096];
        while (is_available())
        {
            stream->Read(bytes, sizeof(bytes));
            const std::size_t count = stream->LastRead();
            if (count == 0)
            {
                break;
            }
            buffer.append(bytes, count);
        }
    };

    read_available(
        solver_process_->GetInputStream(), [this] { return solver_process_->IsInputAvailable(); },
        standard_output_buffer_);
    read_available(
        solver_process_->GetErrorStream(), [this] { return solver_process_->IsErrorAvailable(); },
        standard_error_buffer_);
    consume_progress_lines(false);
}

void MainFrame::consume_progress_lines(const bool include_incomplete_line)
{
    while (true)
    {
        const std::size_t newline = standard_output_buffer_.find('\n');
        if (newline == std::string::npos && !include_incomplete_line)
        {
            return;
        }
        if (standard_output_buffer_.empty())
        {
            return;
        }

        const std::size_t length =
            newline == std::string::npos ? standard_output_buffer_.size() : newline;
        std::string line = standard_output_buffer_.substr(0, length);
        standard_output_buffer_.erase(0, length + (newline == std::string::npos ? 0 : 1));
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        if (line.empty())
        {
            continue;
        }

        try
        {
            const output::AnalysisProgressEvent event =
                output::parse_analysis_progress_json_line(line);
            progress_text_->SetLabel("Progress: " + wxString::FromUTF8(event.message));
            SetStatusText(wxString::FromUTF8(event.message));
        }
        catch (const std::exception &exception)
        {
            progress_protocol_error_ = exception.what();
            progress_text_->SetLabel("Progress: incompatible solver output");
        }
    }
}

void MainFrame::analysis_finished(wxProcessEvent &event)
{
    progress_timer_.Stop();
    read_process_output();
    consume_progress_lines(true);
    delete solver_process_;
    solver_process_ = nullptr;
    run_button_->Enable(active_project_.has_value());
    GetMenuBar()->Enable(run_analysis_id, active_project_.has_value());

    if (event.GetExitCode() == 0 && progress_protocol_error_.empty() && active_run_ &&
        std::filesystem::is_regular_file(active_run_->result_file))
    {
        SetStatusText("Analysis completed");
        wxMessageBox("Analysis completed successfully.\n\nResult:\n" +
                         wxString{active_run_->result_file.wstring()},
                     "FinEleMethod Analysis", wxOK | wxICON_INFORMATION, this);
        return;
    }

    SetStatusText("Analysis failed");
    progress_text_->SetLabel("Progress: failed");
    wxString details;
    if (!progress_protocol_error_.empty())
    {
        details =
            "Progress protocol error: " + wxString::FromUTF8(progress_protocol_error_) + "\n\n";
    }
    if (!standard_error_buffer_.empty())
    {
        details += "Solver message: " + wxString::FromUTF8(standard_error_buffer_) + "\n\n";
    }
    wxMessageBox(wxString::Format("The solver exited with code %d.\n\n", event.GetExitCode()) +
                     details + "Run directory:\n" +
                     (active_run_ ? wxString{active_run_->run_directory.wstring()}
                                  : wxString{"Unavailable"}),
                 "Analysis failed", wxOK | wxICON_ERROR, this);
}

void MainFrame::close_window(wxCloseEvent &event)
{
    if (solver_process_ != nullptr && event.CanVeto())
    {
        wxMessageBox("Wait for the active analysis to finish before closing FinEleMethod.",
                     "Analysis running", wxOK | wxICON_WARNING, this);
        event.Veto();
        return;
    }
    if (solver_process_ != nullptr)
    {
        progress_timer_.Stop();
        solver_process_->Detach();
        solver_process_ = nullptr;
    }
    event.Skip();
}
} // namespace finelemethod::gui
