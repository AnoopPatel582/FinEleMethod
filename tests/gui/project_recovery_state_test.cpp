#include "../../apps/gui/project_recovery_state.hpp"

#include <gtest/gtest.h>

namespace finelemethod::gui
{
TEST(ProjectRecoveryState, OrdinaryProjectsDoNotRequireConfirmation)
{
    ProjectRecoveryState state;
    EXPECT_FALSE(state.needs_confirmation());
    state.loaded(false);
    EXPECT_FALSE(state.needs_confirmation());
}

TEST(ProjectRecoveryState, RecoveredMetadataRequiresConfirmationUntilSaved)
{
    ProjectRecoveryState state;
    state.loaded(true);
    EXPECT_TRUE(state.needs_confirmation());
    // Checking or declining a transition does not clear the unsaved state.
    EXPECT_TRUE(state.needs_confirmation());
    state.saved();
    EXPECT_FALSE(state.needs_confirmation());
    state.loaded(true);
    EXPECT_TRUE(state.needs_confirmation());
}

TEST(ProjectRecoveryState, LoadingAnotherProjectClearsPreviousRecoveryState)
{
    ProjectRecoveryState state;
    state.loaded(true);
    state.loaded(false);
    EXPECT_FALSE(state.needs_confirmation());
}
} // namespace finelemethod::gui
