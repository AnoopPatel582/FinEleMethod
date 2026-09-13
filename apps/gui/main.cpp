#include "main_frame.hpp"

#include "finelemethod/core/application.hpp"

#include <wx/app.h>

#include <iostream>

namespace
{
class FinEleMethodApp final : public wxApp
{
  public:
    bool OnInit() override
    {
        if (argc == 2 && wxString(argv[1]) == "--build-info")
        {
            std::cout << finelemethod::application_build_info();
            build_info_requested_ = true;
            return true;
        }

        auto *frame = new finelemethod::gui::MainFrame;
        frame->Show(true);
        return true;
    }

    int OnRun() override
    {
        if (build_info_requested_)
        {
            return 0;
        }
        return wxApp::OnRun();
    }

  private:
    bool build_info_requested_{false};
};
} // namespace

wxIMPLEMENT_APP(FinEleMethodApp);
