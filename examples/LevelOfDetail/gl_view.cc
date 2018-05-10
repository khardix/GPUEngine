/** @file
 * @brief Implementation of QML Item for direct OpenGL rendering.
 * @author Jan Staněk <xstane32@stud.fit.vutbr.cz>
 */

#include <stdexcept>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <QQuickWindow>
#undef GL_GLEXT_VERSION  // use OpenGL through GPUEngine, respect its constants

#include <AssimpModelLoader.h>

#include "gl_view.hh"

/** Create internal signal-slot connections. */
GLView::GLView()
{
    connect(this, &QQuickItem::windowChanged, this, &GLView::change_window);
}

/** React to change of a parent window.
 * @post
 * 1. All appropriate slots are connected to the new window.
 * 2. Window is instructed not to clear away the rendering results.
 * @param[in] window Pointer to the new parent window.
 */
void GLView::change_window(QQuickWindow *window)
{
    if (window == nullptr) {
        return;
    }

    // Ensure synchronization between component and renderer
    connect(
        window,
        &QQuickWindow::beforeSynchronizing,
        this,
        &GLView::sync_renderer_state,
        Qt::DirectConnection);
    connect(
        window,
        &QQuickWindow::sceneGraphInvalidated,
        this,
        &GLView::reset_renderer,
        Qt::DirectConnection);

    // Repaint the scene after rotation and zoom
    connect(
        this,
        &GLView::update_rotation,
        window,
        &QQuickWindow::update,
        Qt::DirectConnection);
    connect(
        this,
        &GLView::update_zoom,
        window,
        &QQuickWindow::update,
        Qt::DirectConnection);

    window->setClearBeforeRendering(false);
}

/** Synchronize the state of QML and the renderer.
 * Creates the renderer, if necessary.
 */
void GLView::sync_renderer_state()
{
    auto *parent_window = QQuickItem::window();

    // create and connect new renderer, if necessary
    if (!m_renderer) {
        m_renderer = std::make_unique<Renderer>();

        auto connect_self = [this](auto signal, auto slot) {
            connect(this, signal, m_renderer.get(), slot, Qt::DirectConnection);
        };
        auto connect_renderer = [this](auto signal, auto slot) {
            connect(m_renderer.get(), signal, this, slot, Qt::DirectConnection);
        };
        auto connect_window = [this, &parent_window](auto signal, auto slot) {
            connect(
                parent_window,
                signal,
                m_renderer.get(),
                slot,
                Qt::DirectConnection);
        };

        // update scene for painting
        connect_self(&GLView::update_rotation, &Renderer::update_rotation);
        connect_self(&GLView::update_zoom, &Renderer::update_zoom);

        connect_self(&GLView::model_selected, &Renderer::load_scene);
        connect_self(&GLView::level_selected, &Renderer::select_level);
        connect_self(&GLView::generate_levels, &Renderer::generate_levels);

        connect_renderer(
            &Renderer::load_scene_failed, &GLView::errorEncountered);
        connect_renderer(&Renderer::scene_reset_finished, &GLView::sceneReset);
        connect_renderer(&Renderer::scene_reset_finished, &GLView::update);

        // paint the scene behind QML widgets
        connect_window(&QQuickWindow::beforeRendering, &Renderer::paint);
    }

    // point the renderer to correct window and reset viewport
    m_renderer->window(parent_window);
    m_renderer->viewport_size(parent_window->size());
}

/** Reset the renderer. */
void GLView::reset_renderer() noexcept
{
    m_renderer.reset(nullptr);
}

/** Calculate the difference between starting and current point
 * and notifies the renderer.
 * @param[in] target The target point of the rotation.
 */
void GLView::rotation_changed(QPointF target) noexcept
{
    // calculate the step size
    auto delta = target - m_rotation_origin;
    m_rotation_origin = std::move(target);  // make step

    // convert the step into renderer coordinates
    auto dx = static_cast<float>(delta.x());
    auto dy = static_cast<float>(delta.y());
    emit update_rotation(glm::radians(dx), glm::radians(dy));
}

/** Attempts to load a new scene from selected file.
 * @param[in] url The URL of the scene/model file to load.
 */
void GLView::Renderer::load_scene(const QUrl &url) noexcept
{
    m_scene = SimplifiedScene(std::shared_ptr<ge::sg::Scene>(
        AssimpModelLoader::loadScene(url.path().toLocal8Bit().constData())));
    if (m_scene.scene() == nullptr) {
        emit load_scene_failed(QStringLiteral("Cannot load scene!"));
    }
    emit scene_reset_finished(1);
}

/** Attempts to generate simplified levels from original meshes.
 * @param[in] level_count How many levels to generate.
 * @note The result will be level_count+1 (original) levels.
 */
void GLView::Renderer::generate_levels(unsigned level_count) try {
    clear();
    if (m_scene.generate(level_count)) {
        emit scene_reset_finished(level_count);
    }
    else {
        emit scene_reset_finished(0);
    }
}
catch (const std::runtime_error &exc) {
    emit load_scene_failed(exc.what());
}

/** Selects one from the generated levels to display.
 * @param[in] index The index of the level to select.
 */
void GLView::Renderer::select_level(unsigned index)
{
    m_scene.select_level(index);
}

/** Translate a rotation to a quaternion.
 * @param[in] delta The rotation change in X and Y axis, in radians.
 */
void GLView::Renderer::update_rotation(float dx, float dy) noexcept
{
    static const auto AROUND_X_AXIS = glm::vec3{1.f, 0.f, 0.f};
    static const auto AROUND_Y_AXIS = glm::vec3{0.f, 1.f, 0.f};

    // rotation in projected coordinates
    auto world_rotation = glm::normalize(
        glm::angleAxis(dx, AROUND_Y_AXIS) * glm::angleAxis(dy, AROUND_X_AXIS));

    m_rotation = glm::normalize(
        m_rotation * glm::inverse(m_rotation) * world_rotation * m_rotation);
}

/** Clear the screen. */
void GLView::Renderer::clear() const noexcept
{
    if (m_window == nullptr || !m_context) {
        return;
    }

    m_context->glClearColor(.0f, 0.f, .0f, 0.f);
    m_context->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

/** Render the item's contents. */
void GLView::Renderer::paint()
{
    if (m_window == nullptr) {
        return;
    }

    if (!m_context) {
        m_context = init_opengl();
    }

    if (!m_visualization) {
        m_visualization = std::make_unique<UniformVisualization>();
    }

    // calculate matrices
    const auto view_matrix = [this] {
        const auto position
            = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, m_zoom));
        const auto rotation = glm::mat4_cast(m_rotation);
        return position * rotation;
    }();
    const auto proj_matrix = [this] {
        const auto aspect_ratio = static_cast<float>(m_viewport_size.width())
            / static_cast<float>(m_viewport_size.height());
        return glm::perspective(45.f, aspect_ratio, 0.1f, 100.f);
    }();

    // clear screen to black
    m_context->glDisable(GL_BLEND);
    clear();

    // draw the scene
    m_visualization->view_matrix(view_matrix);
    m_visualization->projection_matrix(proj_matrix);
    m_visualization->draw(*m_context, m_scene.scene());

    // clean after OpenGL manipulations
    // WARNING: Zero-fills element buffer, unbind VAO before!
    m_context->glBindVertexArray(0);
    m_window->resetOpenGLState();
}
