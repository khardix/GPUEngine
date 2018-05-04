#include "catch.hpp"
#include <util/set_operations.h>

SCENARIO(
    "Set operation utilities"
    "[util]")
{
    using namespace lod::util;
    using set = std::unordered_set<int>;

    GIVEN("Two unordered sets of the same type")
    {
        const auto lhs = set{3, 15, 43, -8};
        const auto rhs = set{15, -8, 0, 42};

        WHEN("An intersection is calculated")
        {
            const auto intersect = intersection(lhs, rhs);

            THEN("The result contains elements present in both sets")
            {
                REQUIRE(intersect == set{-8, 15});
            }
        }

        WHEN("A symmetrical difference is calculated")
        {
            const auto difference = symmetrical_difference(lhs, rhs);

            THEN("The result contains elements not common to both sets")
            {
                REQUIRE(difference == set{0, 3, 42, 43});
            }
        }
    }

    GIVEN("One unordered set")
    {
        const auto test = set{42};

        WHEN("An intersection with empty set is calculated")
        {
            const auto as_left = intersection(test, set{});
            const auto as_right = intersection(set{}, test);

            THEN("The result is an empty set")
            {
                REQUIRE(as_left == as_right);
                REQUIRE(as_left == set{});
            }
        }

        WHEN("Symmetrical difference with empty set is calculated")
        {
            const auto as_left = symmetrical_difference(test, set{});
            const auto as_right = symmetrical_difference(set{}, test);

            THEN("The result is the original set")
            {
                REQUIRE(as_left == as_right);
                REQUIRE(as_left == test);
            }
        }
    }
}
