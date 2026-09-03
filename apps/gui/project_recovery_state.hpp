#pragma once

namespace finelemethod::gui
{
// Tracks only recovered metadata not yet written to the main project JSON.
// Snapshot creation and failed saves must leave this state unchanged.
class ProjectRecoveryState
{
  public:
    void loaded(const bool recovered)
    {
        unsaved_ = recovered;
    }
    void saved()
    {
        unsaved_ = false;
    }
    [[nodiscard]] bool needs_confirmation() const
    {
        return unsaved_;
    }

  private:
    bool unsaved_{};
};
} // namespace finelemethod::gui
