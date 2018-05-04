#ifndef SET_OPERATIONS_H_FQJ75PRD
#define SET_OPERATIONS_H_FQJ75PRD
/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Set operations over std::unordered_set.
 */

#include <algorithm>
#include <iterator>
#include <unordered_set>

namespace lod {
namespace util {
/** @brief Type converter of raw pointer to non-owning shared_ptr.
 * Useful for searching in sets of smart pointers with a raw one.
 * @param ptr The pointer to elevate.
 * @return Non-owning shared_ptr equivalent to ptr.
 */
template <typename T>
inline std::shared_ptr<T> elevate(T *ptr)
{
    return {ptr, [](T *) {}};
}


/** @brief Symmetrical difference.
 * @param[in] lhs One set to be processed.
 * @param[in] rhs The other set to be processed.
 * @returns Elements that are in one set or the other, but not both.
 */
template <typename... S>
inline std::unordered_set<S...> symmetrical_difference(
    const std::unordered_set<S...> &lhs, const std::unordered_set<S...> &rhs)
{
    // create empty set with parameters indentical to lhs
    auto result = std::unordered_set<S...>(
        std::min(lhs.bucket_count(), rhs.bucket_count()),
        lhs.hash_function(),
        lhs.key_eq(),
        lhs.get_allocator());
    auto inserter = std::inserter(result, std::end(result));

    // copy elements from each set that are not in the other one
    std::copy_if(
        std::cbegin(lhs), std::cend(lhs), inserter, [&rhs](const auto &item) {
            return rhs.count(item) == 0;
        });
    std::copy_if(
        std::cbegin(rhs), std::cend(rhs), inserter, [&lhs](const auto &item) {
            return lhs.count(item) == 0;
        });

    return result;
}

/** @brief Intersection.
 * @param[in] lhs One set to be processed.
 * @param[in] rhs The other set to be processed.
 * @returns Elements that are in both sets.
 */
template <typename... S>
inline std::unordered_set<S...> intersection(
    const std::unordered_set<S...> &lhs, const std::unordered_set<S...> &rhs)
{
    // create empty set with parameters indentical to lhs
    auto result = std::unordered_set<S...>(
        std::min(lhs.bucket_count(), rhs.bucket_count()),
        lhs.hash_function(),
        lhs.key_eq(),
        lhs.get_allocator());
    auto inserter = std::inserter(result, std::end(result));

    // copy elements from lhs that are in rhs
    std::copy_if(
        std::cbegin(lhs), std::cend(lhs), inserter, [&rhs](const auto &item) {
            return rhs.count(item) > 0;
        });

    return result;
}
}  // namespace util
}  // namespace lod

#endif /* end of include guard: SET_OPERATIONS_H_FQJ75PRD */
