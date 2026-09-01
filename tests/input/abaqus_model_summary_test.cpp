#include "finelemethod/input/abaqus_model_summary.hpp"

#include "finelemethod/input/abaqus_input_file.hpp"

#include <gtest/gtest.h>

#include <filesystem>

namespace finelemethod::input
{
namespace
{
std::filesystem::path example_path(const std::string_view filename)
{
    return std::filesystem::path{FINELEMETHOD_SOURCE_DIR} / "examples" / "abaqus" / filename;
}

TEST(AbaqusModelSummary, SummarizesQ4PlaneStressModelWithoutSolving)
{
    const AbaqusModelSummary summary =
        summarize_abaqus_model(read_abaqus_input_file(example_path("q4_tension.inp")));

    EXPECT_EQ(summary.analysis_type, AbaqusAnalysisType::q4_plane_stress);
    EXPECT_EQ(summary.node_count, 4);
    EXPECT_EQ(summary.element_count, 1);
    EXPECT_EQ(summary.material_count, 1);
    EXPECT_EQ(summary.prescribed_displacement_count, 3);
    EXPECT_EQ(summary.point_load_count, 2);
    EXPECT_EQ(summary.pressure_load_count, 1);
}

TEST(AbaqusModelSummary, SummarizesH8ModelWithoutSolving)
{
    const AbaqusModelSummary summary =
        summarize_abaqus_model(read_abaqus_input_file(example_path("h8_compression.inp")));

    EXPECT_EQ(summary.analysis_type, AbaqusAnalysisType::h8_three_dimensional);
    EXPECT_EQ(summary.node_count, 8);
    EXPECT_EQ(summary.element_count, 1);
    EXPECT_EQ(summary.material_count, 1);
    EXPECT_EQ(summary.prescribed_displacement_count, 12);
    EXPECT_EQ(summary.point_load_count, 0);
    EXPECT_EQ(summary.pressure_load_count, 1);
}

TEST(AbaqusModelSummary, DistinguishesQ4PlaneStrain)
{
    const AbaqusModelSummary summary =
        summarize_abaqus_model(read_abaqus_input_file(example_path("q4_plane_strain_tension.inp")));

    EXPECT_EQ(summary.analysis_type, AbaqusAnalysisType::q4_plane_strain);
}
} // namespace
} // namespace finelemethod::input
