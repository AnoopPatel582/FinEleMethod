#include "finelemethod/output/h8_analysis_vtu.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

namespace
{
using finelemethod::elements::H8PointResult;
using finelemethod::math::DenseVector;
using finelemethod::model::DisplacementComponent;
using finelemethod::model::DofMap;
using finelemethod::model::H8Element;
using finelemethod::model::H8ElementCollection;
using finelemethod::model::H8NodeIds;
using finelemethod::model::Node;
using finelemethod::model::NodeCollection;
using finelemethod::model::SpatialDimension;
using finelemethod::output::create_h8_analysis_vtu;
using finelemethod::output::write_h8_analysis_vtu;
using finelemethod::postprocessing::H8ElementResults;
using finelemethod::solver::H8AnalysisResult;

struct H8VtuFixture
{
    NodeCollection nodes;
    H8ElementCollection elements;

    H8VtuFixture()
    {
        nodes.add(Node(7, 1.0, 1.0, 1.0));
        nodes.add(Node(1, 0.0, 0.0, 0.0));
        nodes.add(Node(5, 0.0, 0.0, 1.0));
        nodes.add(Node(3, 1.0, 1.0, 0.0));
        nodes.add(Node(8, 0.0, 1.0, 1.0));
        nodes.add(Node(2, 1.0, 0.0, 0.0));
        nodes.add(Node(6, 1.0, 0.0, 1.0));
        nodes.add(Node(4, 0.0, 1.0, 0.0));
        elements.add(H8Element(9, H8NodeIds{{1, 2, 3, 4, 5, 6, 7, 8}}, 1));
    }
};

H8AnalysisResult make_analysis_result(const DofMap &dof_map, const std::size_t element_id = 9)
{
    const H8PointResult point{
        0.0,
        0.0,
        0.0,
        {1.0, 2.0, 3.0, 4.0, 5.0, 6.0},
        {10.0, 20.0, 30.0, 40.0, 50.0, 60.0},
        25.0,
        {60.0, 30.0, 10.0},
    };
    H8AnalysisResult result{
        DenseVector(dof_map.size()),
        DenseVector(dof_map.size()),
        {H8ElementResults{element_id, {{point, point, point, point, point, point, point, point}}}},
    };
    result.displacements[dof_map.global_index(7, DisplacementComponent::z)] = -0.1;
    result.reactions[dof_map.global_index(1, DisplacementComponent::z)] = 2.5;
    return result;
}

TEST(H8AnalysisVtu, WritesCompleteMeshPointAndCellData)
{
    const H8VtuFixture fixture;
    const DofMap dof_map(fixture.nodes, SpatialDimension::three_dimensional);
    const H8AnalysisResult result = make_analysis_result(dof_map);

    const std::string vtu =
        create_h8_analysis_vtu(fixture.nodes, fixture.elements, dof_map, result);

    EXPECT_NE(vtu.find("<Piece NumberOfPoints=\"8\" NumberOfCells=\"1\">"), std::string::npos);
    EXPECT_NE(vtu.find("Name=\"Displacement\" NumberOfComponents=\"3\""), std::string::npos);
    EXPECT_NE(vtu.find("Name=\"ReactionForce\" NumberOfComponents=\"3\""), std::string::npos);
    EXPECT_NE(vtu.find("Name=\"Strain\" NumberOfComponents=\"6\""), std::string::npos);
    EXPECT_NE(vtu.find("Name=\"Stress\" NumberOfComponents=\"6\""), std::string::npos);
    EXPECT_NE(vtu.find("Name=\"VonMises\" format=\"ascii\">\n"
                       "          2.50000000000000000e+01"),
              std::string::npos);
    EXPECT_NE(vtu.find("Name=\"PrincipalStress\" NumberOfComponents=\"3\""), std::string::npos);
    EXPECT_NE(vtu.find("\n          1 5 3 7 2 6 0 4\n"), std::string::npos);
    EXPECT_NE(vtu.find("Name=\"offsets\" format=\"ascii\">\n          8\n"), std::string::npos);
    EXPECT_NE(vtu.find("Name=\"types\" format=\"ascii\">\n          12\n"), std::string::npos);
}

TEST(H8AnalysisVtu, RejectsElementResultsInDifferentOrder)
{
    const H8VtuFixture fixture;
    const DofMap dof_map(fixture.nodes, SpatialDimension::three_dimensional);

    EXPECT_THROW(static_cast<void>(create_h8_analysis_vtu(fixture.nodes, fixture.elements, dof_map,
                                                          make_analysis_result(dof_map, 99))),
                 std::invalid_argument);
}

TEST(H8AnalysisVtu, RejectsTwoDimensionalDofMap)
{
    const H8VtuFixture fixture;
    const DofMap dof_map(fixture.nodes, SpatialDimension::two_dimensional);
    H8AnalysisResult result{DenseVector(dof_map.size()), DenseVector(dof_map.size()), {}};

    EXPECT_THROW(
        static_cast<void>(create_h8_analysis_vtu(fixture.nodes, fixture.elements, dof_map, result)),
        std::invalid_argument);
}

TEST(H8AnalysisVtu, WritesGeneratedContentToFile)
{
    const H8VtuFixture fixture;
    const DofMap dof_map(fixture.nodes, SpatialDimension::three_dimensional);
    const H8AnalysisResult result = make_analysis_result(dof_map);
    const auto path =
        std::filesystem::temp_directory_path() / "finelemethod_h8_analysis_vtu_test.vtu";
    std::filesystem::remove(path);

    write_h8_analysis_vtu(path, fixture.nodes, fixture.elements, dof_map, result);

    std::ifstream file(path, std::ios::binary);
    const std::string written{std::istreambuf_iterator<char>(file),
                              std::istreambuf_iterator<char>()};
    EXPECT_EQ(written, create_h8_analysis_vtu(fixture.nodes, fixture.elements, dof_map, result));
    file.close();
    std::filesystem::remove(path);
}
} // namespace
