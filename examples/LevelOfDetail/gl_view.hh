/** @file
 * @brief QML Item for direct OpenGL rendering.
 * @author Jan Staněk <xstane32@stud.fit.vutbr.cz>
 */

#pragma once

#include <memory>

#include <QObject>
#include <QQuickItem>

#include <geGL/geGL.h>


/** @brief QML component that displays the rendering.
 * @warning Do NOT combine this class with its renderer!
 * Each of them is expected to live in different thread.
 */
class GLView : public QQuickItem {
    Q_OBJECT

public:
    explicit GLView();

public slots:
    /// @brief Synchronize renderer and QML state.
    void sync_renderer_state();
    /// @brief Reset renderer (i.e. on invalidated scene graph).
    void reset_renderer() noexcept;

private slots:
    /// @brief Attach self to new window.
    void change_window(QQuickWindow *window);

private:
    class Renderer;

    std::unique_ptr<Renderer> m_renderer = nullptr;
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
    /// @brief Reset the available viewport size.
    Renderer &viewport(QSize size) noexcept(
        std::is_nothrow_move_assignable<QSize>::value);

public slots:
    /// @brief Render the scene.
    void paint();

protected:
    /// @brief Initialize OpenGL context.
    static std::unique_ptr<ge::gl::Context> init_opengl();
    /// @brief Compile and link the shader program.
    static std::shared_ptr<ge::gl::Program> link_shader_program();
    /// @brief Load model data into buffers.
    static std::shared_ptr<ge::gl::VertexArray> load_model();

private:
    // Qt data
    QQuickWindow *m_window = nullptr;
    QSize         m_viewport_size = {0, 0};

    // GPUEngine data
    std::unique_ptr<ge::gl::Context>     m_context = nullptr;
    std::shared_ptr<ge::gl::VertexArray> m_vao = nullptr;
    std::shared_ptr<ge::gl::Program>     m_program = nullptr;
};

// Inline and template members {{{
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
 * @param[in] size New size of the viewport.
 * @returns Reference to modified self.
 */
inline GLView::Renderer &GLView::Renderer::viewport(QSize size) noexcept(
    std::is_nothrow_move_assignable<QSize>::value)
{
    m_viewport_size = std::move(size);
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
// Inline and template members }}}
