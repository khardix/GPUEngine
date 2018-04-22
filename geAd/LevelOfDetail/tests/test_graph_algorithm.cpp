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

        auto edges = std::vector<std::unique_ptr<DirectedEdge>>{};
        auto pairs = {std::make_pair(&x, &y),
                      std::make_pair(&y, &z),
                      std::make_pair(&z, &x)};
        for (auto &&p : pairs) {
            auto emanating = std::make_unique<DirectedEdge>();
            emanating->target = p.first;
            if (center.edge == nullptr) {
                center.edge = emanating.get();
            }
            if (!edges.empty()) {
                emanating->neighbour = edges.back().get();
                edges.back()->neighbour = emanating.get();
            }

            auto opposite = std::make_unique<DirectedEdge>();
            opposite->target = p.second;
            opposite->previous = emanating.get();

            auto incoming = std::make_unique<DirectedEdge>();
            incoming->target = &center;
            incoming->previous = opposite.get();

            emanating->previous = incoming.get();

            edges.push_back(std::move(emanating));
            edges.push_back(std::move(opposite));
            edges.push_back(std::move(incoming));
        }
        edges.back()->neighbour = edges.front().get();
        edges.front()->neighbour = edges.back().get();

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

        auto e_x = DirectedEdge{&x};
        auto e_y = DirectedEdge{&y, &e_x};
        auto e_z = DirectedEdge{&z, &e_y};
        e_x.previous = &e_z;

        x.edge = &e_y;
        y.edge = &e_z;
        z.edge = &e_x;

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
