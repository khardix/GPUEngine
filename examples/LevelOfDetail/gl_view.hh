/** @file
 * @brief QML Item for direct OpenGL rendering.
 * @author Jan Staněk <xstane32@stud.fit.vutbr.cz>
 */

#pragma once

#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <QObject>
#include <QPointF>
#include <QQuickItem>

#include <geGL/geGL.h>
#include <geSG/Scene.h>


/** @brief QML component that displays the rendering.
 * @warning Do NOT combine this class with its renderer!
 * Each of them is expected to live in different thread.
 */
class GLView : public QQuickItem {
    Q_OBJECT

public:
    explicit GLView();

signals:
    /// @brief Notify the renderer of a rotation change.
    void update_rotation(float dx, float dy);
    /// @brief Notify the renderer of a zoom change.
    void update_zoom(float delta);

public slots:
    /// @brief Synchronize renderer and QML state.
    void sync_renderer_state();
    /// @brief Reset renderer (i.e. on invalidated scene graph).
    void reset_renderer() noexcept;

    /// @brief User has started rotation.
    void rotation_start(QPointF origin) noexcept;
    /// @brief Rotation was changed.
    void rotation_changed(QPointF target) noexcept;
    /// @brief Current rotation was finished.
    void rotation_finished() noexcept;

private slots:
    /// @brief Attach self to new window.
    void change_window(QQuickWindow *window);

private:
    class Renderer;

    std::unique_ptr<Renderer> m_renderer = nullptr;
    QPointF                   m_rotation_origin = {0.0, 0.0};
};

/** @brief OpenGL renderer for the GLView component. */
class GLView::Renderer : public QObject {
    Q_OBJECT

public:
    explicit Renderer() = default;
    Renderer(const Renderer &) = delete;
    Renderer(Renderer &&) = default;
    virtual ~Renderer() = default;

    Renderer &operator=(const Renderer &) = delete;
    Renderer &operator=(Renderer &&) = default;

    /// @brief Reset parent window of this renderer's component.
    Renderer &window(QQuickWindow *window) noexcept;

public slots:
    /// @brief Set viewport size.
    void viewport_size(QSize size) noexcept;
    /// @brief Render the scene.
    void paint();
    /// @brief Update scene rotation.
    void update_rotation(float dx, float dy) noexcept;
    /// @brief Update scene zoom.
    void update_zoom(float delta) noexcept;

protected:
    /// @brief Initialize OpenGL context.
    static std::unique_ptr<ge::gl::Context> init_opengl();
    /// @brief Load model data into buffers.
    static std::shared_ptr<ge::gl::VertexArray> load_model();

private:
    // Qt data
    QQuickWindow *m_window = nullptr;
    QSize         m_viewport_size = {};

    // GPUEngine data
    std::unique_ptr<ge::gl::Context> m_context = nullptr;

    // Scene and its state
    std::shared_ptr<ge::gl::VertexArray> m_scene = nullptr;
    glm::fquat                           m_rotation = {};  // identity
    float                                m_zoom = -10.f;

    // Visualisation
    std::unique_ptr<ge::gl::Program> m_visualization = nullptr;
};

// Inline and template members {{{
/** Remembers the staring point of current rotation.
 * @param[in] origin Starting point of the rotation.
 */
inline void GLView::rotation_start(QPointF origin) noexcept
{
    m_rotation_origin = std::move(origin);
}

/** Reset the rotation. */
inline void GLView::rotation_finished() noexcept
{
    m_rotation_origin = QPointF{0.0, 0.0};
}

/**
 * @param[in] size New viewport size.
 */
inline void GLView::Renderer::viewport_size(QSize size) noexcept
{
    m_viewport_size = std::move(size);
}

/**
 * @param[in] window Pointer to new parent window.
 * @returns Reference to modified self.
 */
inline GLView::Renderer &GLView::Renderer::window(QQuickWindow *window) noexcept
{
    m_window = window;
    return *this;
}

/**
 * @returns Interface to OpenGL calls.
 */
inline std::unique_ptr<ge::gl::Context> GLView::Renderer::init_opengl()
{
    ge::gl::init();
    return std::make_unique<ge::gl::Context>();
}

/** Updates scene zoom.
 * @param[in] delta Positive means zoom in, negative zoom out.
 */
inline void GLView::Renderer::update_zoom(float delta) noexcept
{
    // value adjustments chosen empirically for better feel
    m_zoom += glm::radians(delta / 2);
}
// Inline and template members }}}
