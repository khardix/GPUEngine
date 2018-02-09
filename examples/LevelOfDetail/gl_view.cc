/** @file
 * @brief Implementation of QML Item for direct OpenGL rendering.
 * @author Jan Staněk <xstane32@stud.fit.vutbr.cz>
 */

#include <QQuickWindow>
#undef GL_GLEXT_VERSION  // use OpenGL through GPUEngine, respect its constants

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
        connect(
            parent_window,
            &QQuickWindow::beforeRendering,
            m_renderer.get(),
            &Renderer::paint,
            Qt::DirectConnection);
    }

    // point the renderer to correct window and reset viewport
    m_renderer->window(parent_window);
    m_renderer->viewport(parent_window->size());
}

/** Reset the renderer. */
void GLView::reset_renderer() noexcept
{
    m_renderer.reset(nullptr);
}

/** Render the item's contents. */
void GLView::Renderer::paint()
{
    if (m_window == nullptr) {
        return;
    }

    if (!m_context) {
        ge::gl::init();
        m_context = std::make_unique<ge::gl::Context>();
    }

    // set the viewport
    m_context->glViewport(
        0, 0, m_viewport_size.width(), m_viewport_size.height());

    // paint the whole screen purple
    m_context->glDisable(GL_DEPTH_TEST);
    m_context->glClearColor(.8f, 0.f, .8f, 1.f);
    m_context->glClear(GL_COLOR_BUFFER_BIT);

    // clean after OpenGL manipulations
    m_window->resetOpenGLState();
}
