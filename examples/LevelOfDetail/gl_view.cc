/** @file
 * @brief Implementation of QML Item for direct OpenGL rendering.
 * @author Jan Staněk <xstane32@stud.fit.vutbr.cz>
 */

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <QQuickWindow>
#undef GL_GLEXT_VERSION  // use OpenGL through GPUEngine, respect its constants

#include <AssimpModelLoader.h>

#include "gl_view.hh"
#include "visualization.hh"

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
void GLView::select_model(const QUrl &url)
{
    auto scene = std::shared_ptr<ge::sg::Scene>(
        AssimpModelLoader::loadScene(url.path().toLocal8Bit().constData()));
    if (!scene) {
        emit errorEncountered(QStringLiteral("Cannot load scene!"));
    }
    else {
        emit scene_loaded(scene);
    }

    update();
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

/** Provides data for the shader program (currently hardcoded).
 * @returns Verter Array Object with loaded data.
 */
std::shared_ptr<ge::gl::VertexArray> GLView::Renderer::load_model()
{
    // static variable -> data available for the whole duration of program run
    // clang-format off
    static const auto vertices = std::vector<GLfloat>{
        -1.f, -1.f,  1.f,
         1.f, -1.f,  1.f,
        -1.f,  1.f,  1.f,
         1.f,  1.f,  1.f,
        -1.f, -1.f, -1.f,
         1.f, -1.f, -1.f,
        -1.f,  1.f, -1.f,
         1.f,  1.f, -1.f,
    };
    static const auto elements = std::vector<GLubyte>{
        0, 1, 3,
        0, 3, 2,
        1, 5, 7,
        1, 7, 3,
        5, 4, 6,
        5, 6, 7,
        4, 0, 2,
        4, 2, 6,
        0, 5, 1,
        0, 4, 5,
        2, 3, 7,
        2, 7, 6,
    };
    // clang-format on

    auto vertex_buffer = std::make_unique<ge::gl::Buffer>(
        vertices.size() * sizeof(GLfloat), vertices.data());
    auto element_buffer = std::make_unique<ge::gl::Buffer>(
        elements.size() * sizeof(GLubyte), elements.data());

    auto result = std::make_shared<ge::gl::VertexArray>();
    result->addAttrib(std::move(vertex_buffer), 0, 3, GL_FLOAT);
    result->addElementBuffer(std::move(element_buffer));

    return result;
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
        m_visualization = make_uniform_program();
    }
    if (!m_scene) {
        m_scene = load_model();
    }

    // calculate matrices
    const auto model_matrix = glm::mat4(1.f);
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
    m_context->glClearColor(.0f, 0.f, .0f, 0.f);
    m_context->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // calculate current model position
    m_visualization->use();
    m_visualization->setMatrix4fv("model", glm::value_ptr(model_matrix));
    m_visualization->setMatrix4fv("view", glm::value_ptr(view_matrix));
    m_visualization->setMatrix4fv("projection", glm::value_ptr(proj_matrix));

    // bind the vertices and run the program
    {
        m_scene->bind();

        m_context->glDrawElements(
            GL_TRIANGLES,
            static_cast<GLsizei>(
                m_scene->getElement()->getSize() / sizeof(GLubyte)),
            GL_UNSIGNED_BYTE,
            nullptr);

        m_scene->unbind();
    }

    // clean after OpenGL manipulations
    // WARNING: Zero-fills element buffer, unbind VAO before!
    m_window->resetOpenGLState();
}
