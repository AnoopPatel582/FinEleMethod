#pragma once

#include <wx/frame.h>

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

    wxTextCtrl *input_path_{};
};
} // namespace finelemethod::gui
