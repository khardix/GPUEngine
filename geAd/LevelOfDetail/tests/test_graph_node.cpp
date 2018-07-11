#include "catch.hpp"
#include <graph/Node.h>

SCENARIO(
    "Nodes are hashable"
    "[graph]")
{
    using namespace lod::graph;

    GIVEN("Two identical nodes")
    {
        auto n1 = Node::make(glm::vec3{1.f, 1.f, 1.f});
        auto n2 = std::make_shared<Node>(*n1);

        WHEN("compared")
        {
            THEN("they compare equal")
            {
                REQUIRE(*n1 == *n2);
                REQUIRE(*n2 == *n1);
                REQUIRE(std::equal_to<Node::pointer_type>{}(n1, n2));
                REQUIRE(std::equal_to<Node::pointer_type>{}(n2, n1));
            }
        }

        WHEN("hashed")
        {
            THEN("the hashes are equal")
            {
                auto hash = std::hash<Node::pointer_type>{};

                REQUIRE(hash(n1) == hash(n2));
            }
        }
    }
    GIVEN("Two different nodes")
    {
        auto n1 = Node::make(glm::vec3{1.f, 1.f, 1.f});
        auto n2 = Node::make(glm::vec3{-1.f, -1.f, -1.f});

        WHEN("compared")
        {
            THEN("they do not compare equal")
            {
                REQUIRE(*n1 != *n2);
                REQUIRE(*n2 != *n1);
                REQUIRE(!std::equal_to<>{}(n1, n2));
                REQUIRE(!std::equal_to<>{}(n2, n1));
            }
        }

        WHEN("hashed")
        {
            THEN("the hashes are different")
            {
                auto hash = std::hash<Node::pointer_type>{};

                REQUIRE(hash(n1) != hash(n2));
            }
        }
    }
}
