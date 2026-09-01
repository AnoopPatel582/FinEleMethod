#include "main_frame.hpp"

#include <wx/app.h>

namespace
{
class FinEleMethodApp final : public wxApp
{
  public:
    bool OnInit() override
    {
        auto *frame = new finelemethod::gui::MainFrame;
        frame->Show(true);
        return true;
    }
};
} // namespace

wxIMPLEMENT_APP(FinEleMethodApp);
