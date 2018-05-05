#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <tuple>

#include <glm/gtx/io.hpp>

#include "catch.hpp"
#include <graph/Mesh.h>
#include <graph/Triangle.h>
#include <operator/edge_collapse.h>
#include <util/set_operations.h>

lod::graph::Mesh make_mesh()
{
    using namespace lod::graph;

    auto  nodes = Mesh::NodeSet{};
    auto &C = *(nodes.insert(Node{{0.f, 0.f, 0.f}}).first);
    auto &X = *(nodes.insert(Node{{1.f, 0.f, 0.f}}).first);
    auto &Y = *(nodes.insert(Node{{0.f, 1.f, 0.f}}).first);
    auto &Z = *(nodes.insert(Node{{0.f, 0.f, 1.f}}).first);

    auto CXY = make_triangle({&C, &X, &Y});
    auto CYZ = make_triangle({&C, &Y, &Z});
    auto CZX = make_triangle({&C, &Z, &X});

    // neighbours
    CXY[0]->neighbour = CYZ[1].get();
    CYZ[1]->neighbour = CXY[0].get();

    CYZ[0]->neighbour = CZX[1].get();
    CZX[1]->neighbour = CYZ[0].get();

    CZX[0]->neighbour = CXY[1].get();
    CXY[1]->neighbour = CZX[0].get();

    // emanating edges
    C.edge = CYZ[1].get();
    X.edge = CZX[0].get();
    Y.edge = CXY[0].get();
    Z.edge = CYZ[0].get();

    auto edges = Mesh::EdgeSet{};
    std::move(CXY.begin(), CXY.end(), std::inserter(edges, edges.end()));
    std::move(CYZ.begin(), CYZ.end(), std::inserter(edges, edges.end()));
    std::move(CZX.begin(), CZX.end(), std::inserter(edges, edges.end()));

    return Mesh(std::move(nodes), std::move(edges));
}

SCENARIO(
    "Application of half-edge collapse operator"
    "[operator]")
{
    using namespace lod;

    using Tag = operation::HalfEdgeTag;
    using HalfEdgeCollapse = oper::EdgeCollapse<Tag>;
    using Operation = operation::Simple<Tag::element_type>;
    using Labeled = std::tuple<std::string, Operation>;

    GIVEN("Two opposite triangles")
    {
        using lod::oper::common::EdgeCollapse;

        auto O = graph::Node{{0.f, 0.f, 0.f}};
        auto T = graph::Node{{1.f, 0.f, 0.f}};
        auto C = graph::Node{{0.f, 1.f, 0.f}};

        auto A = graph::Node{{0.f, -1.f, 0.f}};
        auto B = graph::Node{{0.f, 1.f, 1.f}};

        auto triangle = graph::make_triangle({&O, &T, &A});

        WHEN("Measuring transformation for folds")
        {
            THEN("Potential fold is detected")
            {
                REQUIRE(EdgeCollapse::would_fold(*triangle[1], C, A));
            }
            THEN("Small change passes")
            {
                REQUIRE(!EdgeCollapse::would_fold(*triangle[1], C, B));
            }
        }
    }

    GIVEN("Mesh and Half-Edge collapse operator")
    {
        auto mesh = make_mesh();
        auto collapse = HalfEdgeCollapse{};

        // prepare the operations
        const auto center = graph::Node{{0.f, 0.f, 0.f}};
        auto       center_it = mesh.nodes().find(center);
        REQUIRE(center_it != mesh.nodes().end());

        auto edge = mesh.edges().find(util::elevate(center_it->edge));
        REQUIRE(edge != mesh.edges().end());

        auto neigh_ptr = nonstd::get<graph::DirectedEdge *>((*edge)->neighbour);
        auto neigh = mesh.edges().find(util::elevate(neigh_ptr));
        REQUIRE(neigh != mesh.edges().end());

        auto to_boundary = Labeled{"towards boundary", Operation(*edge, 0.f)};
        auto from_boundary = Labeled{"from boundary", Operation(*neigh, 0.f)};

        for (const auto &op : {to_boundary, from_boundary}) {
            WHEN("An edge is collapsed " + std::get<std::string>(op))
            {
                auto &operation = std::get<Operation>(op);
                auto  origin = *(operation.element().get()->previous->target);
                collapse(mesh, operation);

                THEN("The mesh contains expected number of elements")
                {
                    REQUIRE(mesh.nodes().size() == 3);
                    REQUIRE(mesh.edges().size() == 3);
                }
                THEN("All edges are boundary edges")
                {
                    auto is_boundary
                        = [](const auto &edge) { return edge->boundary(); };

                    REQUIRE(std::all_of(
                        mesh.edges().cbegin(),
                        mesh.edges().cend(),
                        is_boundary));
                }
                THEN("All edges are connected")
                {
                    auto connected = [&mesh](const auto &edge) {
                        auto cnt
                            = mesh.edges().count(util::elevate(edge->previous));
                        return edge->previous != nullptr && cnt > 0;
                    };

                    REQUIRE(std::all_of(
                        mesh.edges().cbegin(), mesh.edges().cend(), connected));
                }
                THEN("Mesh does not contain the origin node")
                {
                    REQUIRE(mesh.nodes().count(origin) == 0);
                }
                THEN("All nodes have valid edge reference")
                {
                    auto valid_ref = [&mesh](const auto &node) {
                        auto elevated = util::elevate(node.edge);
                        return mesh.edges().count(elevated) > 0;
                    };

                    for (const auto &node : mesh.nodes()) {
                        CAPTURE(node.position);
                        REQUIRE(valid_ref(node));
                    }
                }
            }
        }
    }
}
