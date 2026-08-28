#include "finelemethod/output/q4_analysis_vtu.hpp"

#include "finelemethod/output/q4_vtu_writer.hpp"

#include <gtest/gtest.h>

namespace
{
using finelemethod::elements::Q4PlaneStressPointResult;
using finelemethod::math::DenseVector;
using finelemethod::model::DofMap;
using finelemethod::model::Node;
using finelemethod::model::NodeCollection;
using finelemethod::model::Q4Element;
using finelemethod::model::Q4ElementCollection;
using finelemethod::model::Q4NodeIds;
using finelemethod::model::SpatialDimension;
using finelemethod::output::create_q4_analysis_vtu;
using finelemethod::output::create_q4_results_vtu;
using finelemethod::postprocessing::Q4ElementPlaneStressResults;
using finelemethod::solver::Q4PlaneStressAnalysisResult;

TEST(Q4AnalysisVtu, ConvertsCompleteAnalysisResultUsingResultWriter)
{
    NodeCollection nodes;
    nodes.add(Node(1, 0.0, 0.0));
    nodes.add(Node(2, 1.0, 0.0));
    nodes.add(Node(3, 1.0, 1.0));
    nodes.add(Node(4, 0.0, 1.0));
    const DofMap dof_map(nodes, SpatialDimension::two_dimensional);

    Q4ElementCollection elements;
    elements.add(Q4Element(7, Q4NodeIds{{1, 2, 3, 4}}, 1, 1.0));

    const Q4PlaneStressPointResult point_result{
        0.0, 0.0, {0.01, -0.0025, 0.0}, {10.0, 0.0, 0.0}, 10.0, {10.0, 0.0, 0.0}};
    Q4PlaneStressAnalysisResult analysis_result{
        DenseVector(dof_map.size()),
        DenseVector(dof_map.size()),
        {Q4ElementPlaneStressResults{7,
                                     {{point_result, point_result, point_result, point_result}}}},
    };
    analysis_result.displacements[2] = 0.01;
    analysis_result.displacements[4] = 0.01;

    const std::string direct = create_q4_analysis_vtu(nodes, elements, dof_map, analysis_result);
    const std::string expected = create_q4_results_vtu(
        nodes, elements, dof_map, analysis_result.displacements, analysis_result.element_results);

    EXPECT_EQ(direct, expected);
    EXPECT_NE(direct.find("Name=\"Displacement\""), std::string::npos);
    EXPECT_NE(direct.find("Name=\"VonMises\""), std::string::npos);
}
} // namespace
