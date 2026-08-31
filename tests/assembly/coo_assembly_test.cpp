#include "finelemethod/assembly/coo_assembly.hpp"

#include "finelemethod/math/coo_matrix.hpp"
#include "finelemethod/math/csr_matrix.hpp"
#include "finelemethod/math/dense_matrix.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <limits>
#include <stdexcept>
#include <vector>

namespace
{
using finelemethod::assembly::add_element_matrix_to_coo;
using finelemethod::math::convert_to_csr;
using finelemethod::math::CooMatrix;
using finelemethod::math::DenseMatrix;

TEST(CooAssembly, MapsElementEntriesToGlobalDofs)
{
    CooMatrix global(4, 4);
    DenseMatrix element(2, 2);
    element(0, 0) = 10.0;
    element(0, 1) = -3.0;
    element(1, 0) = -3.0;
    element(1, 1) = 8.0;
    constexpr std::array<std::size_t, 2> mapping{3, 1};

    add_element_matrix_to_coo(global, element, mapping);
    const auto assembled = convert_to_csr(global);

    EXPECT_TRUE(
        std::ranges::equal(assembled.row_offsets(), std::vector<std::size_t>{0, 0, 2, 2, 4}));
    EXPECT_TRUE(
        std::ranges::equal(assembled.column_indices(), std::vector<std::size_t>{1, 3, 1, 3}));
    EXPECT_TRUE(std::ranges::equal(assembled.values(), std::vector<double>{8.0, -3.0, -3.0, 10.0}));
}

TEST(CooAssembly, RetainsSharedDofContributionsForCsrSummation)
{
    CooMatrix global(3, 3);
    DenseMatrix first(2, 2, 1.0);
    DenseMatrix second(2, 2, 2.0);
    constexpr std::array<std::size_t, 2> first_mapping{0, 1};
    constexpr std::array<std::size_t, 2> second_mapping{1, 2};

    add_element_matrix_to_coo(global, first, first_mapping);
    add_element_matrix_to_coo(global, second, second_mapping);

    EXPECT_EQ(global.nonzero_count(), 8U);
    const auto assembled = convert_to_csr(global);
    ASSERT_EQ(assembled.row_offsets()[2] - assembled.row_offsets()[1], 3U);
    EXPECT_DOUBLE_EQ(assembled.values()[3], 3.0);
}

TEST(CooAssembly, DoesNotStoreZeroElementEntries)
{
    CooMatrix global(2, 2);
    DenseMatrix element(2, 2);
    element(0, 0) = 5.0;
    constexpr std::array<std::size_t, 2> mapping{0, 1};

    add_element_matrix_to_coo(global, element, mapping);

    ASSERT_EQ(global.nonzero_count(), 1U);
    EXPECT_DOUBLE_EQ(global.entries().front().value, 5.0);
}

TEST(CooAssembly, RejectsMismatchedElementMatrixWithoutChangingTarget)
{
    CooMatrix global(3, 3);
    const DenseMatrix rectangular_element(2, 3, 1.0);
    const DenseMatrix wrong_size_element(2, 2, 1.0);
    constexpr std::array<std::size_t, 2> two_dofs{0, 1};
    constexpr std::array<std::size_t, 3> three_dofs{0, 1, 2};

    EXPECT_THROW(add_element_matrix_to_coo(global, rectangular_element, two_dofs),
                 std::invalid_argument);
    EXPECT_THROW(add_element_matrix_to_coo(global, wrong_size_element, three_dofs),
                 std::invalid_argument);
    EXPECT_EQ(global.nonzero_count(), 0U);
}

TEST(CooAssembly, RejectsNonsquareGlobalMatrixWithoutChangingTarget)
{
    CooMatrix global(2, 3);
    const DenseMatrix element(2, 2, 1.0);
    constexpr std::array<std::size_t, 2> mapping{0, 1};

    EXPECT_THROW(add_element_matrix_to_coo(global, element, mapping), std::invalid_argument);
    EXPECT_EQ(global.nonzero_count(), 0U);
}

TEST(CooAssembly, RejectsOutOfRangeMappingWithoutChangingTarget)
{
    CooMatrix global(2, 2);
    const DenseMatrix element(2, 2, 1.0);
    constexpr std::array<std::size_t, 2> mapping{0, 2};

    EXPECT_THROW(add_element_matrix_to_coo(global, element, mapping), std::out_of_range);
    EXPECT_EQ(global.nonzero_count(), 0U);
}

TEST(CooAssembly, RejectsNonfiniteElementValueWithoutChangingTarget)
{
    CooMatrix global(2, 2);
    DenseMatrix element(2, 2, 1.0);
    element(1, 1) = std::numeric_limits<double>::quiet_NaN();
    constexpr std::array<std::size_t, 2> mapping{0, 1};

    EXPECT_THROW(add_element_matrix_to_coo(global, element, mapping), std::invalid_argument);
    EXPECT_EQ(global.nonzero_count(), 0U);
}
} // namespace
