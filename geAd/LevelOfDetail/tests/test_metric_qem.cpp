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

        auto triangle = std::array<DirectedEdge::const_pointer_type, 3>{
            DirectedEdge::make(&nodes[0]),
            DirectedEdge::make(&nodes[1]),
            DirectedEdge::make(&nodes[2]),
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

        auto CX = DirectedEdge::make(&X);
        auto XY = DirectedEdge::make(&Y, CX);
        auto YC = DirectedEdge::make(&C, XY);
        CX->previous() = YC;

        auto CY = DirectedEdge::make(&Y);
        auto YZ = DirectedEdge::make(&Z, CY);
        auto ZC = DirectedEdge::make(&C, YZ);
        CY->previous() = ZC;

        auto CZ = DirectedEdge::make(&Z);
        auto ZX = DirectedEdge::make(&X, CZ);
        auto XC = DirectedEdge::make(&C, ZX);
        CZ->previous() = XC;

        // clang-format off
        CX->neighbour() = XC; XC->neighbour() = CX;
        CY->neighbour() = YC; YC->neighbour() = CY;
        CZ->neighbour() = ZC; ZC->neighbour() = CZ;
        // clang-format on

        C.edge = CX;

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
            THEN("The node's error is 0")
            {
                auto error = qem.error(quadric, C.position);
                REQUIRE(error == 0.0);
            }
        }
    }

    GIVEN("A quadric")
    {
        // clang-format off
        // column-major storage!
        const auto quadric = glm::transpose(glm::mat4{
            1.f, 2.f, 0.f, 1.f,
            2.f, 1.f, 0.f, 0.f,
            0.f, 0.f, 1.f, 0.f,
            0.f, 0.f, 0.f, 0.f,
        });
        // clang-format on

        WHEN("An optimal position is calculated")
        {
            const auto position = QEM<FullEdgeTag>::position(quadric);

            THEN("It contains the expected value")
            {
                const auto expected = glm::vec3{1 / 3.f, -2 / 3.f, -0.f};
                REQUIRE(position == expected);
            }
        }

        WHEN("An error is measured")
        {
            const auto error
                = QEM<FullEdgeTag>::error(quadric, glm::vec3{1.f, 1.f, 1.f});

            THEN("It has the expected size") { REQUIRE(error == 8.f); }
        }
    }
}
