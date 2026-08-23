#include "finelemethod/core/application.hpp"

#include <gtest/gtest.h>

TEST(Application, HasExpectedName)
{
    EXPECT_EQ(finelemethod::application_name(), "FinEleMethod command-line solver");
}
