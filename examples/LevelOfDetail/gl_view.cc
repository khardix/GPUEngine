/** @file
 * @brief Implementation of QML Item for direct OpenGL rendering.
 * @author Jan Staněk <xstane32@stud.fit.vutbr.cz>
 */

#include <vector>

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

/** Provides data for the shader program (currently hardcoded).
 * @returns Verter Array Object with loaded data.
 */
std::shared_ptr<ge::gl::VertexArray> GLView::Renderer::load_model()
{
    // static variable -> data available for the whole duration of program run
    // clang-format off
    static const auto vertices = std::vector<GLfloat>{
        -1.f, -1.f, 0.f,
         1.f, -1.f, 0.f,
         0.f,  1.f, 0.f,
    };
    // clang-format on

    auto buffer = std::make_shared<ge::gl::Buffer>(
        vertices.size() * sizeof(GLfloat), vertices.data());
    auto result = std::make_shared<ge::gl::VertexArray>();
    result->addAttrib(buffer, 0, 3, GL_FLOAT);

    return result;
}

/** Provides the shader program.
 * The shader source is currently hardcoded.
 * @returns Compiled and linked shader program.
 */
std::shared_ptr<ge::gl::Program> GLView::Renderer::link_shader_program()
{
    using namespace std::literals::string_literals;
    using ge::gl::Program;
    using ge::gl::Shader;

    static const auto vertex_shader_code = R"vertex(
        #version 430 core

        layout(location = 0) in vec3 model_vertex;

        void main() {
            gl_Position = vec4(model_vertex, 1.0);
        }
    )vertex"s;
    static const auto fragment_shader_code = R"fragment(
        #version 430 core

        out vec4 color;

        void main() {
            color = vec4(1, 0, 0, 1);
        }
    )fragment"s;

    return std::make_shared<Program>(
        std::make_shared<Shader>(GL_VERTEX_SHADER, vertex_shader_code),
        std::make_shared<Shader>(GL_FRAGMENT_SHADER, fragment_shader_code));
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
    if (!m_program) {
        m_program = link_shader_program();
    }
    if (!m_vao) {
        m_vao = load_model();
    }

    // paint the whole screen purple
    m_context->glClearColor(.0f, 0.f, .0f, 0.f);
    m_context->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // bind the vertices and run the program
    m_program->use();
    m_vao->bind();

    m_context->glDrawArrays(GL_TRIANGLES, 0, 3);

    // clean after OpenGL manipulations
    m_window->resetOpenGLState();
}
