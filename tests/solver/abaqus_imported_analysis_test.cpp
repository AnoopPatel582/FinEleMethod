#include "finelemethod/output/abaqus_imported_vtu.hpp"
#include "finelemethod/solver/abaqus_imported_analysis.hpp"

#include <gtest/gtest.h>

#include <string>
#include <string_view>

namespace
{
struct ImportedCase
{
    std::string_view type;
    std::string_view nodes;
    std::string_view connectivity;
    std::string_view boundaries;
    std::string_view vtk_type;
    std::size_t dimension;
};

class AbaqusImportedAnalysis : public testing::TestWithParam<ImportedCase>
{
};

std::string input_for(const ImportedCase &test_case)
{
    return "*Heading\n"
           "*Node\n" +
           std::string(test_case.nodes) + "*Element, Type=" + std::string(test_case.type) +
           ", Elset=AllElements\n" + std::string(test_case.connectivity) +
           "*Material, Name=Steel\n"
           "*Elastic\n"
           "200000, 0.25\n"
           "*Solid Section, Elset=AllElements, Material=Steel\n"
           "1.0,\n"
           "*Boundary\n" +
           std::string(test_case.boundaries);
}

TEST_P(AbaqusImportedAnalysis, SolvesRecoversAndWritesVtkCell)
{
    const ImportedCase &test_case = GetParam();
    const auto solution = finelemethod::solver::analyze_abaqus_imported_model(input_for(test_case));

    ASSERT_EQ(solution.model.elements.size(), 1);
    ASSERT_EQ(solution.element_results.size(), 1);
    EXPECT_NEAR(solution.element_results[0].strain[0], 0.01, 1.0e-14);
    EXPECT_NEAR(solution.element_results[0].strain[1], 0.02, 1.0e-14);
    if (test_case.dimension == 3)
    {
        EXPECT_NEAR(solution.element_results[0].strain[2], 0.03, 1.0e-14);
    }
    EXPECT_GT(solution.element_results[0].von_mises, 0.0);

    const std::string vtu = finelemethod::output::create_abaqus_imported_vtu(solution);
    EXPECT_NE(vtu.find("Name=\"Displacement\""), std::string::npos);
    EXPECT_NE(vtu.find("Name=\"ReactionForce\""), std::string::npos);
    EXPECT_NE(vtu.find("Name=\"VonMises\""), std::string::npos);
    EXPECT_NE(vtu.find("          " + std::string(test_case.vtk_type) + "\n        </DataArray>"),
              std::string::npos);
}

INSTANTIATE_TEST_SUITE_P(
    InstructorElements, AbaqusImportedAnalysis,
    testing::Values(ImportedCase{"CPS3", "1,0,0\n2,1,0\n3,0,1\n", "1,1,2,3\n",
                                 "1,1,1,0\n1,2,2,0\n2,1,1,0.01\n2,2,2,0\n"
                                 "3,1,1,0\n3,2,2,0.02\n",
                                 "5", 2},
                    ImportedCase{"CPS4R", "1,0,0\n2,1,0\n3,1,1\n4,0,1\n", "1,1,2,3,4\n",
                                 "1,1,1,0\n1,2,2,0\n2,1,1,0.01\n2,2,2,0\n"
                                 "3,1,1,0.01\n3,2,2,0.02\n4,1,1,0\n4,2,2,0.02\n",
                                 "9", 2},
                    ImportedCase{"C3D4", "1,0,0,0\n2,1,0,0\n3,0,1,0\n4,0,0,1\n", "1,1,2,3,4\n",
                                 "1,1,1,0\n1,2,2,0\n1,3,3,0\n"
                                 "2,1,1,0.01\n2,2,2,0\n2,3,3,0\n"
                                 "3,1,1,0\n3,2,2,0.02\n3,3,3,0\n"
                                 "4,1,1,0\n4,2,2,0\n4,3,3,0.03\n",
                                 "10", 3},
                    ImportedCase{"C3D8R",
                                 "1,0,0,0\n2,1,0,0\n3,1,1,0\n4,0,1,0\n"
                                 "5,0,0,1\n6,1,0,1\n7,1,1,1\n8,0,1,1\n",
                                 "1,1,2,3,4,5,6,7,8\n",
                                 "1,1,1,0\n1,2,2,0\n1,3,3,0\n"
                                 "2,1,1,0.01\n2,2,2,0\n2,3,3,0\n"
                                 "3,1,1,0.01\n3,2,2,0.02\n3,3,3,0\n"
                                 "4,1,1,0\n4,2,2,0.02\n4,3,3,0\n"
                                 "5,1,1,0\n5,2,2,0\n5,3,3,0.03\n"
                                 "6,1,1,0.01\n6,2,2,0\n6,3,3,0.03\n"
                                 "7,1,1,0.01\n7,2,2,0.02\n7,3,3,0.03\n"
                                 "8,1,1,0\n8,2,2,0.02\n8,3,3,0.03\n",
                                 "12", 3}),
    [](const testing::TestParamInfo<ImportedCase> &information) {
        return std::string(information.param.type);
    });
} // namespace
