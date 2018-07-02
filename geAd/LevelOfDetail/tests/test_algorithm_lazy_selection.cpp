#include "catch.hpp"
#include <algorithm/lazy_selection.h>
#include <metric/qem.h>
#include <operator/edge_collapse.h>

void force_compilation()
{
    using lod::algorithm::LazySelection;
    using lod::algorithm::MaxError;
    using lod::metric::QEM;
    using lod::oper::EdgeCollapse;
    using lod::operation::HalfEdgeTag;

    auto simplify = LazySelection<HalfEdgeTag, QEM, EdgeCollapse>{};
    auto mesh = lod::graph::Mesh({}, {});

    simplify(mesh, MaxError<float>{10.f});
}
