#pragma once
/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Structure holding mesh variants for a simplified scene.
 */

#include <memory>
#include <unordered_map>
#include <vector>

#include <geSG/Mesh.h>
#include <geSG/Scene.h>

/// @brief Holding and switching of simplified variants
class SimplifiedScene {
public:
    using scene_pointer_type = std::shared_ptr<ge::sg::Scene>;
    using mesh_pointer_type = std::shared_ptr<ge::sg::Mesh>;
    using mesh_variant_map = std::
        unordered_map<mesh_pointer_type *, std::vector<mesh_pointer_type>>;

    SimplifiedScene() noexcept = default;
    explicit SimplifiedScene(scene_pointer_type scene);

    /// @brief Access the stored scene.
    const scene_pointer_type &scene() const noexcept { return m_scene; }

    /// @brief Generate simplified variants.
    bool generate(std::size_t num_variants);
    /// @brief Select a simplification level.
    void select_level(std::size_t level_index);

private:
    scene_pointer_type m_scene = nullptr;  ///< Managed scene
    /** Simplified variants of scene's meshes.
     * 0 - original, others - simplifications.
     */
    mesh_variant_map m_variants = {};
};
