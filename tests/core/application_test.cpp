#include "finelemethod/core/application.hpp"

#include <gtest/gtest.h>

TEST(Application, HasExpectedName)
{
    EXPECT_EQ(finelemethod::application_name(), "FinEleMethod command-line solver");
}

TEST(Application, BuildInfoContainsConfiguredDiagnostics)
{
    const auto info = finelemethod::application_build_info();
    EXPECT_NE(info.find("FinEleMethod development build\n"), std::string::npos);
    EXPECT_NE(info.find("Configuration: "), std::string::npos);
    EXPECT_EQ(info.find("unspecified"), std::string::npos);
    EXPECT_NE(info.find("Architecture: 64-bit\n"), std::string::npos);
    EXPECT_NE(info.find("Compiler: MSVC "), std::string::npos);
    EXPECT_EQ(info, finelemethod::application_build_info());
}
