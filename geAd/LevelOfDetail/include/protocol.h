#ifndef PROTOCOL_H_DJOVTO94
#define PROTOCOL_H_DJOVTO94
/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Structures for passing data between parts of simplification algorithm.
 */

#include "graph/Node.h"

namespace lod {
/// @brief Encapsulations of operation cost measurement.
namespace cost {
using error_type = float;

/// @brief Only the margin of error for given operation.
struct Simple {
public:
    /// @brief Extract the error amount.
    explicit operator const error_type &() const noexcept;

    /// @brief Stored margin of error.
    error_type error = static_cast<error_type>(0);
};

/// @brief Margin of error with placement hint for new vertex.
struct VertexPlacement : public Simple {
public:
    using position_type = graph::Node::position_type;

    explicit VertexPlacement(
        error_type error, graph::Node::position_type position) noexcept;

    position_type position_hint = {0.f, 0.f, 0.f};
};
}  // namespace cost


// Inline and template members
inline cost::Simple::operator const error_type &() const noexcept
{
    return error;
}

inline cost::VertexPlacement::VertexPlacement(
    error_type error, position_type position) noexcept
    : Simple{std::move(error)}, position_hint(std::move(position))
{
}
}  // namespace lod

#endif /* end of include guard: PROTOCOL_H_DJOVTO94 */
