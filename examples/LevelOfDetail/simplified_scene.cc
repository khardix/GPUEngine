/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Structure holding mesh variants for a simplified scene
 * (implementation).
 */

#include <iterator>

#include <LoDGenerator.h>

#include "scene_util.hh"
#include "simplified_scene.hh"


/** Prepare the internal structure for further simplification.
 * @param[in] scene The original scene.
 */
SimplifiedScene::SimplifiedScene(scene_pointer_type scene)
    : m_scene(std::move(scene))
{
    using MC = util::SceneWalker::iterator::MeshContainer;

    for (const auto &model : util::SceneWalker(m_scene)) {
        for (auto &&mesh : std::get<MC &>(model)) {
            m_variants.insert({std::addressof(mesh), {mesh}});
        }
    }
}

/** Generate simplified variants of all meshes in the scene.
 * @param[in] num_variants How many levels of simplification should be
 * generated.
 * @return True if any generation has taken place, false otherwise (the scene is
 * empty).
 */
bool SimplifiedScene::generate(std::size_t num_variants)
{
    if (m_variants.empty()) {
        return false;
    }

    for (auto &&item : m_variants) {
        item.second.resize(1);  // only keep the original
        lod::simplify(
            item.second[0], num_variants, std::back_inserter(item.second));
    }

    return true;
}

/** Selects the appropriate level to display in scene.
 * @param[in] level_index Index of the level to select; 0 is original.
 */
void SimplifiedScene::select_level(std::size_t level_index)
{
    for (const auto &item : m_variants) {
        *item.first = item.second.at(level_index);
    }
}
