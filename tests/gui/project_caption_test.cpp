#include "../../apps/gui/project_caption.hpp"

#include <gtest/gtest.h>

namespace finelemethod::gui
{
TEST(ProjectCaption, EmptyProjectKeepsApplicationTitle)
{
    EXPECT_EQ(project_caption("", false), "FinEleMethod");
    EXPECT_EQ(project_caption("", true), "FinEleMethod");
}

TEST(ProjectCaption, NormalProjectShowsItsIdentity)
{
    EXPECT_EQ(project_caption("BracketStudy", false), "FinEleMethod - BracketStudy");
}

TEST(ProjectCaption, RecoveredProjectRemainsMarkedUntilSaved)
{
    EXPECT_EQ(project_caption("BracketStudy", true),
              "FinEleMethod - BracketStudy [recovered, not saved]");
    EXPECT_EQ(project_caption("BracketStudy", false), "FinEleMethod - BracketStudy");
}
} // namespace finelemethod::gui
