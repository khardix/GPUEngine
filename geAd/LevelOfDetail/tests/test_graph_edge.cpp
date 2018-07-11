#include "catch.hpp"
#include <graph/Edge.h>
#include <graph/Node.h>

SCENARIO(
    "undirected edges are comparable and hashable"
    "[graph]")
{
    using namespace lod::graph;

    // clang-format off
    auto v_a = Node::make(), v_b = Node::make(), v_c = Node::make();
    auto prev_a = DirectedEdge::make(v_a), e_a = DirectedEdge::make(v_b, prev_a);  // A→B
    auto prev_b = DirectedEdge::make(v_b), e_b = DirectedEdge::make(v_a, prev_b);  // B→A
    auto prev_c = DirectedEdge::make(v_c), e_c = DirectedEdge::make(v_a, prev_c);  // C→A
    // clang-format on

    GIVEN("two identical edges")
    {
        auto lhs = UndirectedEdge(e_a), rhs = lhs;

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

            THEN("hashes are equal") { REQUIRE(lhash == rhash); }
        }
    }

    GIVEN("two 'opposite' edges")
    {
        auto rhs = UndirectedEdge(e_a);
        auto lhs = UndirectedEdge(e_b);

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

            THEN("hashes are equal") { REQUIRE(lhash == rhash); }
        }
    }

    GIVEN("two different edges")
    {
        auto lhs = UndirectedEdge(e_b);
        auto rhs = UndirectedEdge(e_c);

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

            THEN("hashes are different") { REQUIRE(lhash != rhash); }
        }
    }
}
