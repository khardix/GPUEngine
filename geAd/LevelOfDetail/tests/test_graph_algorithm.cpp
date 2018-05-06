#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "catch.hpp"
#include <graph/algorithm.h>

SCENARIO(
    "Adjacency nodes are listed as expected"
    "[graph]"
    "[algorithm]")
{
    using namespace lod::graph;

    GIVEN("Non-boundary patch")
    {
        const auto center = Node{};
        const auto x = Node{glm::vec3{1.f, 0.f, 0.f}};
        const auto y = Node{glm::vec3{0.f, 1.f, 0.f}};
        const auto z = Node{glm::vec3{0.f, 0.f, 1.f}};

        auto edges = std::vector<DirectedEdge::pointer_type>{};
        auto pairs = {std::make_pair(&x, &y),
                      std::make_pair(&y, &z),
                      std::make_pair(&z, &x)};
        for (auto &&p : pairs) {
            auto emanating = DirectedEdge::make(p.first);
            if (center.edge.expired()) {
                center.edge = emanating;
            }
            if (!edges.empty()) {
                emanating->neighbour() = edges.back();
                edges.back()->neighbour() = emanating;
            }

            auto opposite = DirectedEdge::make(p.second, emanating);
            auto incoming = DirectedEdge::make(&center, opposite);

            emanating->previous() = incoming;

            edges.push_back(std::move(emanating));
            edges.push_back(std::move(opposite));
            edges.push_back(std::move(incoming));
        }
        edges.back()->neighbour() = edges.front();
        edges.front()->neighbour() = edges.back();

        WHEN("Extracting adjacent nodes")
        {
            const auto adjacent = adjacent_nodes(center);

            THEN("All adjacent nodes are extracted")
            {
                REQUIRE(adjacent.size() == 3);

                const auto begin = std::cbegin(adjacent);
                const auto end = std::cend(adjacent);

                REQUIRE(std::find(begin, end, &x) != end);
                REQUIRE(std::find(begin, end, &y) != end);
                REQUIRE(std::find(begin, end, &z) != end);
            }
        }
    }

    GIVEN("A single triangle")
    {
        const auto x = Node{glm::vec3{1.f, 0.f, 0.f}};
        const auto y = Node{glm::vec3{0.f, 1.f, 0.f}};
        const auto z = Node{glm::vec3{0.f, 0.f, 1.f}};

        auto e_x = DirectedEdge::make(&x);
        auto e_y = DirectedEdge::make(&y, e_x);
        auto e_z = DirectedEdge::make(&z, e_y);
        e_x->previous() = e_z;

        x.edge = e_y;
        y.edge = e_z;
        z.edge = e_x;

        WHEN("Adjacent nodes are queried")
        {
            const auto adjacent = adjacent_nodes(x);

            THEN("Both other nodes are extracted")
            {
                REQUIRE(adjacent.size() == 2);

                const auto begin = std::cbegin(adjacent);
                const auto end = std::cend(adjacent);

                REQUIRE(std::find(begin, end, &y) != end);
                REQUIRE(std::find(begin, end, &z) != end);
            }
        }
    }
}
