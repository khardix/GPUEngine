#include <algorithm>
#include <array>
#include <iosfwd>
#include <iterator>

#include "catch.hpp"
#include <glm/gtx/io.hpp>
#include <graph/Mesh.h>
#include <metric/qem.h>

SCENARIO(
    "QEM correctly calculates error and new position"
    "[metric]"
    "[qem]")
{
    using namespace lod::graph;
    using namespace lod::operation;
    using lod::metric::QEM;

    GIVEN("Three nodes defining a plane")
    {
        auto nodes = std::array<Node, 3>{
            Node{{0.f, 0.f, 0.f}},
            Node{{0.f, 1.f, 0.f}},
            Node{{0.f, 0.f, 1.f}},
        };

        auto edges = std::array<DirectedEdge, 3>{
            DirectedEdge{&nodes[0]},
            DirectedEdge{&nodes[1]},
            DirectedEdge{&nodes[2]},
        };

        auto triangle = Triangle{
            edges.data(),
            edges.data() + 1,
            edges.data() + 2,
        };

        WHEN("A plane vector is calculated")
        {
            const auto plane = QEM<FullEdgeTag>::plane(triangle);

            THEN("It computes the expected values")
            {
                const auto expected = glm::vec4{1.f, 0.f, 0.f, 0.f};
                REQUIRE(plane == expected);
            }
        }
    }

    GIVEN("A node with surrounding triangles")
    {
        auto C = Node{{0.f, 0.f, 0.f}};
        auto X = Node{{1.f, 0.f, 0.f}};
        auto Y = Node{{0.f, 1.f, 0.f}};
        auto Z = Node{{0.f, 0.f, 1.f}};

        auto CX = DirectedEdge{&X, nullptr};
        auto XY = DirectedEdge{&Y, &CX};
        auto YC = DirectedEdge{&C, &XY};
        CX.previous = &YC;

        auto CY = DirectedEdge{&Y, nullptr};
        auto YZ = DirectedEdge{&Z, &CY};
        auto ZC = DirectedEdge{&C, &YZ};
        CY.previous = &ZC;

        auto CZ = DirectedEdge{&Z, nullptr};
        auto ZX = DirectedEdge{&X, &CZ};
        auto XC = DirectedEdge{&C, &ZX};
        CZ.previous = &XC;

        // clang-format off
        CX.neighbour = &XC; XC.neighbour = &CX;
        CY.neighbour = &YC; YC.neighbour = &CY;
        CZ.neighbour = &ZC; ZC.neighbour = &CZ;
        // clang-format on

        C.edge = &CX;

        WHEN("A quadric is calculated")
        {
            const auto qem = QEM<FullEdgeTag>();
            const auto quadric = qem.quadric(C);

            THEN("It provides expected results")
            {
                // clang-format off
                const auto expected = glm::mat4{
                    1.f, 0.f, 0.f, 0.f,
                    0.f, 1.f, 0.f, 0.f,
                    0.f, 0.f, 1.f, 0.f,
                    0.f, 0.f, 0.f, 0.f,
                };
                // clang-format on

                REQUIRE(quadric == expected);
            }
        }
    }

    GIVEN("A quadric")
    {
        // clang-format off
        const auto quadric = glm::mat4{
            1.f, 0.f, 0.f, 0.f,
            0.f, 1.f, 0.f, 0.f,
            0.f, 0.f, 1.f, 0.f,
            0.f, 0.f, 0.f, 0.f,
        };
        // clang-format on

        WHEN("An optimal position is calculated")
        {
            const auto position = QEM<FullEdgeTag>::position(quadric);

            THEN("It contains the expected value")
            {
                const auto expected = glm::vec3{0.f, 0.f, 0.f};
                REQUIRE(position == expected);
            }
        }

        WHEN("An error is measured")
        {
            const auto error
                = QEM<FullEdgeTag>::error(quadric, glm::vec3{1.f, 1.f, 1.f});

            THEN("It has the expected size")
            {
                REQUIRE(error == 3.f);
            }
        }
    }
}
