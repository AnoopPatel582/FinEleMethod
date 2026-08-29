#include "finelemethod/input/abaqus_input_file.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

namespace
{
using finelemethod::input::read_abaqus_input_file;

TEST(AbaqusInputFile, ReadsCompleteFileContents)
{
    const auto path = std::filesystem::temp_directory_path() / "finelemethod_input_reader.inp";
    {
        std::ofstream file(path, std::ios::binary);
        file << "*Heading\r\n** model\r\n";
    }

    EXPECT_EQ(read_abaqus_input_file(path), "*Heading\r\n** model\r\n");
    std::filesystem::remove(path);
}

TEST(AbaqusInputFile, RejectsMissingAndEmptyFiles)
{
    const auto missing =
        std::filesystem::temp_directory_path() / "finelemethod_missing_input_reader.inp";
    std::filesystem::remove(missing);
    EXPECT_THROW(static_cast<void>(read_abaqus_input_file(missing)), std::runtime_error);

    const auto empty =
        std::filesystem::temp_directory_path() / "finelemethod_empty_input_reader.inp";
    {
        std::ofstream file(empty, std::ios::binary);
    }
    EXPECT_THROW(static_cast<void>(read_abaqus_input_file(empty)), std::runtime_error);
    std::filesystem::remove(empty);
}
} // namespace
