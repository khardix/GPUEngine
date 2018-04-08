#include <MeshGraph.h>

#include "catch.hpp"

SCENARIO(
    "undirected edges are comparable and hashable"
    "[graph]")
{
    using namespace lod::graph;

    auto v_a = Node{}, v_b = Node{}, v_c = Node{};

    GIVEN("two identical edges")
    {
        auto lhs = UndirectedEdge(&v_a, &v_b);
        auto rhs = lhs;

        WHEN("compared")
        {
            THEN("they compare equal")
            {
                REQUIRE(lhs == rhs);
                REQUIRE(rhs == lhs);
            }
        }

        WHEN("hashed")
        {
            auto lhash = std::hash<UndirectedEdge>{}(lhs);
            auto rhash = std::hash<UndirectedEdge>{}(rhs);

            THEN("hashes are equal")
            {
                REQUIRE(lhash == rhash);
            }
        }
    }

    GIVEN("two 'opposite' edges")
    {
        auto lhs = UndirectedEdge(&v_a, &v_b);
        auto rhs = UndirectedEdge(&v_b, &v_a);

        WHEN("compared")
        {
            THEN("they compare equal")
            {
                REQUIRE(lhs == rhs);
                REQUIRE(rhs == lhs);
            }
        }

        WHEN("hashed")
        {
            auto lhash = std::hash<UndirectedEdge>{}(lhs);
            auto rhash = std::hash<UndirectedEdge>{}(rhs);

            THEN("hashes are equal")
            {
                REQUIRE(lhash == rhash);
            }
        }
    }

    GIVEN("two different edges")
    {
        auto lhs = UndirectedEdge(&v_a, &v_b);
        auto rhs = UndirectedEdge(&v_a, &v_c);

        WHEN("compared")
        {
            THEN("they do not compare equal")
            {
                REQUIRE(lhs != rhs);
                REQUIRE(rhs != lhs);
            }
        }

        WHEN("hashed")
        {
            auto lhash = std::hash<UndirectedEdge>{}(lhs);
            auto rhash = std::hash<UndirectedEdge>{}(rhs);

            THEN("hashes are different")
            {
                REQUIRE(lhash != rhash);
            }
        }
    }
}
