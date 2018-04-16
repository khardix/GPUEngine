#include <MeshGraph.h>

#include "catch.hpp"

SCENARIO(
    "Nodes are hashable"
    "[graph]")
{
    using namespace lod::graph;

    GIVEN("Two identical nodes")
    {
        auto n1 = Node{glm::vec3{1.f, 1.f, 1.f}};
        auto n2 = n1;

        WHEN("compared")
        {
            THEN("they compare equal")
            {
                REQUIRE(n1 == n2);
                REQUIRE(n2 == n1);
            }
        }

        WHEN("hashed")
        {
            THEN("the hashes are equal")
            {
                auto hash = std::hash<Node>{};

                REQUIRE(hash(n1) == hash(n2));
            }
        }
    }
    GIVEN("Two different nodes")
    {
        auto n1 = Node{glm::vec3{1.f, 1.f, 1.f}};
        auto n2 = Node{glm::vec3{-1.f, -1.f, -1.f}};

        WHEN("compared")
        {
            THEN("they do not compare equal")
            {
                REQUIRE(n1 != n2);
                REQUIRE(n2 != n1);
            }
        }

        WHEN("hashed")
        {
            THEN("the hashes are different")
            {
                auto hash = std::hash<Node>{};

                REQUIRE(hash(n1) != hash(n2));
            }
        }
    }
}

SCENARIO(
    "undirected edges are comparable and hashable"
    "[graph]")
{
    using namespace lod::graph;

    auto v_a = Node{}, v_b = Node{}, v_c = Node{};
    auto prev_a = DirectedEdge{&v_a}, e_a = DirectedEdge{&v_b, &prev_a};  // A→B
    auto prev_b = DirectedEdge{&v_b}, e_b = DirectedEdge{&v_a, &prev_b};  // B→A
    auto prev_c = DirectedEdge{&v_c}, e_c = DirectedEdge{&v_a, &prev_c};  // C→A

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

            THEN("hashes are equal")
            {
                REQUIRE(lhash == rhash);
            }
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

            THEN("hashes are equal")
            {
                REQUIRE(lhash == rhash);
            }
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

            THEN("hashes are different")
            {
                REQUIRE(lhash != rhash);
            }
        }
    }
}
