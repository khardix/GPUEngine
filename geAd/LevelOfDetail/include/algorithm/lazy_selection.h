#ifndef LAZY_SELECTION_H_DX3JHVCY
#define LAZY_SELECTION_H_DX3JHVCY
/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Lazy selection algorithm for mesh decimation.
 */

#include <algorithm>
#include <functional>
#include <iterator>
#include <limits>
#include <memory>
#include <queue>
#include <set>
#include <unordered_set>
#include <vector>

#include <nonstd/variant.hpp>

#include <geSG/Mesh.h>

#include "../graph/Mesh.h"

namespace lod {
namespace algorithm {
/** @brief Maximum cost of operation to apply to the mesh. */
template <typename ErrorType = float>
struct MaxError {
    ErrorType threshold = static_cast<ErrorType>(0);
};

/** @brief Desired number of (decimated) elements in output mesh. */
struct ElementCount {
    std::size_t count = std::numeric_limits<std::size_t>::max();
};

/** @brief Lazy selection decimation algorithm.
 * @tparam Tag The tag type that determine the kind of operation to perform.
 * @tparam Metric The metric to use for ordering the operations.
 * @tparam Operator The operator to use by the algorithm.
 */
template <
    typename Tag,
    template <typename> class Metric,
    template <typename> class Operator>
class LazySelection {
public:
    using operation_type = typename Metric<Tag>::result_type;
    using element_type = typename operation_type::element_pointer::type;
    using element_pointer = typename operation_type::element_pointer;

    using pointer_set_type = std::set<
        typename element_pointer::pointer_type,
        typename element_pointer::pointer_cmp>;
    using queue_type = std::priority_queue<
        operation_type,
        std::vector<operation_type>,
        std::greater<operation_type>>;

    using cost_type = typename operation_type::cost_type;
    using size_type = typename queue_type::size_type;

    /// @brief Decimate the mesh until a specified condition is satisfied.
    template <typename ConditionType>
    graph::Mesh &operator()(
        graph::Mesh &mesh, const ConditionType &condition) const;
    template <typename ConditionType>
    ge::sg::Mesh operator()(
        const ge::sg::Mesh &mesh, const ConditionType &condition) const;
    template <typename ConditionType>
    std::shared_ptr<ge::sg::Mesh> operator()(
        const std::shared_ptr<const ge::sg::Mesh> &mesh,
        const ConditionType &                      condition) const;
    template <typename InputIt, typename OutputIt>
    OutputIt operator()(
        const ge::sg::Mesh &mesh,
        InputIt             condition_begin,
        InputIt             condition_end,
        OutputIt            destination_begin) const;
    template <typename InputIt, typename OutputIt>
    OutputIt operator()(
        const std::shared_ptr<const ge::sg::Mesh> &mesh,
        InputIt                                    condition_begin,
        InputIt                                    condition_end,
        OutputIt                                   destination_begin) const;

    /// @brief Create regular series of simplified variants of the input mesh.
    template <typename OutputIt>
    OutputIt operator()(
        const ge::sg::Mesh &mesh,
        size_type           num_variants,
        OutputIt            out_begin) const;
    template <typename OutputIt>
    OutputIt operator()(
        const std::shared_ptr<const ge::sg::Mesh> &mesh,
        size_type                                  num_variants,
        OutputIt                                   out_begin) const;

protected:
    /// @brief Initialize the internal state for new mesh processing.
    void initialize(graph::Mesh &mesh) const;

    /// @brief Convert stop condition to generic function/closure.
    std::function<bool()> convert_condition(
        const MaxError<cost_type> &cost) const;
    std::function<bool()> convert_condition(const ElementCount &target) const;

    /// @brief Perform one round of the decimation.
    void decimate(const std::function<bool()> &should_continue) const;

private:
    Metric<Tag>   m_metric;    ///< Metric to use for evaluation.
    Operator<Tag> m_operator;  ///< Operator used for decimation.

    /// @brief Currently decimated mesh.
    mutable graph::Mesh *m_mesh = nullptr;
    /// @brief Elements currently scheduled for decimation.
    mutable queue_type m_queue = {};
    /// @brief Elements that need to be re-evaluated before decimation.
    mutable pointer_set_type m_dirty = {};
};

/** @brief Convenience wrapper around the full LazySelection functor. */
template <
    typename T,
    template <class> class M,
    template <class> class O,
    typename... Args>
inline decltype(auto) lazy_selection(Args &&... args)
{
    return LazySelection<T, M, O>{}(std::forward<Args>(args)...);
}


// Inline and template members
/** Initialize the internal state with a new mesh.
 * @param[in] mesh The new mesh to decimate.
 */
template <typename T, template <class> class M, template <class> class O>
inline void LazySelection<T, M, O>::initialize(graph::Mesh &mesh) const
{
    m_mesh = std::addressof(mesh);
    m_queue = queue_type{};
    m_dirty = pointer_set_type{};

    // Fill queue with the initial costs.
    for (const auto &element : m_mesh->container<element_type>()) {
        m_queue.push(m_metric(element));
    }
}

/** Encapsulate stop condition to one function/closure.
 * @param[in] cost Maximum cost of operation to be applied.
 * @return Function that indicates if the decimation should apply next
 * operation.
 */
template <typename T, template <class> class M, template <class> class O>
inline std::function<bool()> LazySelection<T, M, O>::convert_condition(
    const MaxError<cost_type> &cost) const
{
    return [this, threshold = cost.threshold]() {
        return !m_queue.empty() && m_queue.top().cost() <= threshold;
    };
}

/** @overload
 * @param[in] target The number of (decimated) elements in output mesh.
 * @return Function that indicates if the decimation should apply next
 * operation.
 */
template <typename T, template <class> class M, template <class> class O>
inline std::function<bool()> LazySelection<T, M, O>::convert_condition(
    const ElementCount &target) const
{
    return [this, target = target.count]() {
        return m_mesh->container<element_type>().size() > target;
    };
}

/** Runs the decimation until the stop condition is fulfilled. */
template <typename T, template <class> class M, template <class> class O>
void LazySelection<T, M, O>::decimate(
    const std::function<bool()> &should_continue) const
{
    while (!m_queue.empty() && should_continue()) {
        auto operation = m_queue.top();
        m_queue.pop();

        // skip operations with invalid elements
        if (!operation) {
            continue;
        }

        // lazy re-evaluation
        auto dirty_it = m_dirty.find(operation.element());
        if (dirty_it != m_dirty.end()) {
            m_queue.push(m_metric(operation.element().get()));
            m_dirty.erase(dirty_it);
        }
        // operator application
        else {
            auto modified = m_operator(*m_mesh, operation);
            for (auto &&element : modified) {
                m_dirty.insert(std::move(element));
            }
        }
    }
}

/** Decimates all elements that have metric-dependent cost
 * lesser than specified maximum cost.
 * @param[in,out] mesh The mesh to decimate. It is decimated in-place.
 * @param[in] condition The condition to satisfy by the decimation.
 * @return Reference to the decimated mesh.
 */
template <typename T, template <class> class M, template <class> class O>
template <typename ConditionType>
inline graph::Mesh &LazySelection<T, M, O>::operator()(
    graph::Mesh &mesh, const ConditionType &condition) const
{
    initialize(mesh);
    decimate(convert_condition(condition));
    return mesh;
}

/** @overload
 * @param[in] mesh The original mesh to decimate.
 * @param[in] condition The condition to satisfy by the decimation.
 * @return New decimated mesh.
 */
template <typename T, template <class> class M, template <class> class O>
template <typename ConditionType>
inline ge::sg::Mesh LazySelection<T, M, O>::operator()(
    const ge::sg::Mesh &mesh, const ConditionType &condition) const
{
    auto graph = graph::Mesh(mesh);

    operator()(graph, condition);

    return static_cast<ge::sg::Mesh>(graph);
}

/** @overload
 * @param[in] mesh Pointer to the original mesh to decimate.
 * @param[in] condition The condition to satisfy by the decimation.
 * @return Pointer to new decimated mesh.
 */
template <typename T, template <class> class M, template <class> class O>
template <typename ConditionType>
inline std::shared_ptr<ge::sg::Mesh> LazySelection<T, M, O>::operator()(
    const std::shared_ptr<const ge::sg::Mesh> &mesh,
    const ConditionType &                      condition) const
{
    auto graph = graph::Mesh(*mesh);

    operator()(graph, condition);

    return std::make_shared<ge::sg::Mesh>(static_cast<ge::sg::Mesh>(graph));
}

/** @overload
 * This overload efficiently creates several variant of the original mesh
 * and stores them in container starting at destination_begin.
 * The destination_begin should conform to the same requirements
 * as i.e. d_first parameter of std::transform.
 * @see std::transform algorithm.
 * @note The condition container SHOULD be logically sorted
 * (ascending order for costs, descending for number of elements, etc.).
 * Unsorted container will result in duplicate meshes in output.
 * @param[in] mesh The original mesh to decimate.
 * @param[in] condition_begin The start of condition container.
 * @param[in] condition_end The end of condition container.
 * @param[in] destination_begin The beginning of the output container.
 * @return Iterator after the last generated variant,
 * created from destination_begin.
 */
template <typename T, template <class> class M, template <class> class O>
template <typename InputIt, typename OutputIt>
OutputIt LazySelection<T, M, O>::operator()(
    const ge::sg::Mesh &mesh,
    InputIt             condition_begin,
    InputIt             condition_end,
    OutputIt            destination_begin) const
{
    auto graph = graph::Mesh(mesh);
    initialize(graph);

    // Prepare the conditions
    auto steps = std::vector<std::function<bool()>>{};
    steps.reserve(std::distance(condition_begin, condition_end));
    std::transform(
        condition_begin,
        condition_end,
        std::back_inserter(steps),
        [this](auto &cond) { return convert_condition(cond); });

    // Decimate
    for (const auto &step : steps) {
        decimate(step);
        *destination_begin++ = static_cast<ge::sg::Mesh>(*m_mesh);
    }

    return destination_begin;
}

/** @overload
 * @see Non-pointer variant of the same overload.
 * @param[in] mesh The original mesh to decimate.
 * @param[in] condition_begin The start of condition container.
 * @param[in] condition_end The end of condition container.
 * @param[in] destination_begin The beginning of the output container.
 * @return Iterator after the last generated variant,
 * created from destination_begin.
 */
template <typename T, template <class> class M, template <class> class O>
template <typename InputIt, typename OutputIt>
OutputIt LazySelection<T, M, O>::operator()(
    const std::shared_ptr<const ge::sg::Mesh> &mesh,
    InputIt                                    condition_begin,
    InputIt                                    condition_end,
    OutputIt                                   destination_begin) const
{
    auto graph = graph::Mesh(*mesh);
    initialize(graph);

    // Prepare the conditions
    auto steps = std::vector<std::function<bool()>>{};
    steps.reserve(std::distance(condition_begin, condition_end));
    std::transform(
        condition_begin,
        condition_end,
        std::back_inserter(steps),
        [this](auto &cond) { return convert_condition(cond); });

    // Decimate
    for (const auto &step : steps) {
        decimate(step);
        *destination_begin++ = std::make_shared<ge::sg::Mesh>(
            static_cast<ge::sg::Mesh>(*m_mesh));
    }

    return destination_begin;
}

/** Efficiently creates several variants from one mesh.
 * Each variant is created after decimating a fraction of original elements.
 * @param[in] mesh The mesh to decimate.
 * @param[in] num_variants The number of variants to create.
 * @param[in] out_begin The start of output container.
 * @return Iterator after the last generated variant,
 * created from out_begin.
 */
template <typename T, template <class> class M, template <class> class O>
template <typename OutputIt>
OutputIt LazySelection<T, M, O>::operator()(
    const ge::sg::Mesh &mesh, size_type num_variants, OutputIt out_begin) const
{
    auto graph = graph::Mesh(mesh);
    initialize(graph);
    const auto queue_size = m_queue.size();

    auto stops = std::vector<std::function<bool()>>(num_variants - 1);
    std::generate(
        std::begin(stops),
        std::end(stops),
        // generate regular fractions
        [this,
         n = num_variants,
         size_unit = queue_size / num_variants]() mutable {
            return convert_condition(ElementCount{(--n) * size_unit});
        });

    for (const auto &stop : stops) {
        decimate(stop);
        *out_begin++ = static_cast<ge::sg::Mesh>(*m_mesh);
    }
    return out_begin;
}

/** Efficiently creates several variants from one mesh.
 * Each variant is created after decimating a fraction of original elements.
 * @param[in] mesh The mesh to decimate.
 * @param[in] num_variants The number of variants to create.
 * @param[in] out_begin The start of output container.
 * @return Iterator after the last generated variant,
 * created from out_begin.
 */
template <typename T, template <class> class M, template <class> class O>
template <typename OutputIt>
OutputIt LazySelection<T, M, O>::operator()(
    const std::shared_ptr<const ge::sg::Mesh> &mesh,
    size_type                                  num_variants,
    OutputIt                                   out_begin) const
{
    auto graph = graph::Mesh(*mesh);
    initialize(graph);
    const auto queue_size = m_queue.size();

    auto stops = std::vector<std::function<bool()>>(num_variants - 1);
    std::generate(
        std::begin(stops),
        std::end(stops),
        // generate regular fractions
        [this,
         n = num_variants,
         size_unit = queue_size / num_variants]() mutable {
            return convert_condition(ElementCount{(--n) * size_unit});
        });

    for (const auto &stop : stops) {
        decimate(stop);
        *out_begin++ = std::make_shared<ge::sg::Mesh>(
            static_cast<ge::sg::Mesh>(*m_mesh));
    }
    return out_begin;
}
}  // namespace algorithm
}  // namespace lod

#endif /* end of include guard: LAZY_SELECTION_H_DX3JHVCY */
