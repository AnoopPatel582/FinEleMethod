#include "finelemethod/output/q4_vtu_writer.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

namespace
{
using finelemethod::math::DenseVector;
using finelemethod::model::DisplacementComponent;
using finelemethod::model::DofMap;
using finelemethod::model::Node;
using finelemethod::model::NodeCollection;
using finelemethod::model::Q4Element;
using finelemethod::model::Q4ElementCollection;
using finelemethod::model::Q4NodeIds;
using finelemethod::model::SpatialDimension;
using finelemethod::output::create_q4_displacement_vtu;
using finelemethod::output::write_q4_displacement_vtu;

struct Q4VtuFixture
{
    NodeCollection nodes;
    Q4ElementCollection elements;

    Q4VtuFixture()
    {
        nodes.add(Node(30, 1.0, 1.0));
        nodes.add(Node(10, 0.0, 0.0));
        nodes.add(Node(40, 0.0, 1.0));
        nodes.add(Node(20, 1.0, 0.0));
        elements.add(Q4Element(1, Q4NodeIds{{10, 20, 30, 40}}, 1, 1.0));
    }
};

TEST(Q4VtuWriter, CreatesMeshConnectivityAndNodalDisplacements)
{
    const Q4VtuFixture fixture;
    const DofMap dof_map(fixture.nodes, SpatialDimension::two_dimensional);
    DenseVector displacements(dof_map.size());
    displacements[dof_map.global_index(30, DisplacementComponent::x)] = 0.1;
    displacements[dof_map.global_index(30, DisplacementComponent::y)] = -0.2;

    const std::string vtu =
        create_q4_displacement_vtu(fixture.nodes, fixture.elements, dof_map, displacements);

    EXPECT_NE(vtu.find("<Piece NumberOfPoints=\"4\" NumberOfCells=\"1\">"), std::string::npos);
    EXPECT_NE(vtu.find("Name=\"Displacement\" NumberOfComponents=\"3\""), std::string::npos);
    EXPECT_NE(vtu.find("1.00000000000000006e-01 -2.00000000000000011e-01 "
                       "0.00000000000000000e+00"),
              std::string::npos);
    EXPECT_NE(vtu.find("\n          1 3 0 2\n"), std::string::npos);
    EXPECT_NE(vtu.find("Name=\"offsets\" format=\"ascii\">\n          4\n"), std::string::npos);
    EXPECT_NE(vtu.find("Name=\"types\" format=\"ascii\">\n          9\n"), std::string::npos);
}

TEST(Q4VtuWriter, WritesGeneratedContentToFile)
{
    const Q4VtuFixture fixture;
    const DofMap dof_map(fixture.nodes, SpatialDimension::two_dimensional);
    const DenseVector displacements(dof_map.size());
    const auto path =
        std::filesystem::temp_directory_path() / "finelemethod_q4_vtu_writer_test.vtu";
    std::filesystem::remove(path);

    write_q4_displacement_vtu(path, fixture.nodes, fixture.elements, dof_map, displacements);

    std::ifstream file(path, std::ios::binary);
    const std::string written{std::istreambuf_iterator<char>(file),
                              std::istreambuf_iterator<char>()};
    EXPECT_EQ(written,
              create_q4_displacement_vtu(fixture.nodes, fixture.elements, dof_map, displacements));
    file.close();
    std::filesystem::remove(path);
}

TEST(Q4VtuWriter, RejectsMismatchedDisplacementVector)
{
    const Q4VtuFixture fixture;
    const DofMap dof_map(fixture.nodes, SpatialDimension::two_dimensional);

    EXPECT_THROW(static_cast<void>(create_q4_displacement_vtu(
                     fixture.nodes, fixture.elements, dof_map, DenseVector(dof_map.size() - 1))),
                 std::invalid_argument);
}
} // namespace
