/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright Gothel Software e.K.
 *
 * SPDX-License-Identifier: MIT
 *
 * This Source Code Form is subject to the terms of the MIT License
 * If a copy of the MIT was not distributed with this file,
 * you can obtain one at https://opensource.org/license/mit/.
 */

#include <gamp/Gamp.hpp>

#include <cstdio>
#include <cmath>
#include <memory>

#include <jau/basic_types.hpp>
#include <jau/fraction_type.hpp>
#include <jau/io/file_util.hpp>

#include <gamp/Gamp.hpp>
#include <gamp/render/gl/GLCapabilities.hpp>
#include <gamp/render/gl/GLTypes.hpp>
#include <gamp/render/RenderContext.hpp>
#include <gamp/wt/event/Event.hpp>
#include "gamp/render/gl/GLContext.hpp"

using namespace jau::math;
using namespace jau::math::util;

using namespace gamp::wt;
using namespace gamp::wt::event;

using namespace gamp::render::gl;

namespace {

    // Vertex shader
    static GLint u_pmv;
    static GLint a_vertices, a_colors;

    // 2 <= ES < 3: #version 100
    // ES >= 3: #version 300 es
    //
    // ES2/ES3 precision:
    //   precision highp float;
    //   precision highp int;
    static const GLchar* vertexSource =
        "#version 100\n"
        "precision highp float;\n"
        "precision highp int;\n"
        "\n"
        "#if __VERSION__ >= 130\n"
        "  #define attribute in\n"
        "  #define varying out\n"
        "#endif\n"
        "\n"
        "uniform mat4    mgl_PMVMatrix[2];\n"
        "attribute vec4    mgl_Vertex;\n"
        "attribute vec4    mgl_Color;\n"
        "varying vec4    frontColor;\n"
        "\n"
        "void main(void)\n"
        "{\n"
        "  frontColor=mgl_Color;\n"
        "  gl_Position = mgl_PMVMatrix[0] * mgl_PMVMatrix[1] * mgl_Vertex;\n"
        "}\n";

    // Fragment/pixel shader
    //
    // ES2 precision
    //   precision mediump float;
    //   precision mediump int;
    // ES3 precision:
    //   precision highp float;
    //   precision highp int;
    static const GLchar* fragmentSource =
        "#version 100\n"
        "precision mediump float;\n"
        "precision mediump int;\n"
        "\n"
        "#if __VERSION__ >= 130\n"
        "  #define varying in\n"
        "  out vec4 mgl_FragColor;\n"
        "#else\n"
        "  #define mgl_FragColor gl_FragColor\n"
        "#endif\n"
        "\n"
        "varying vec4 frontColor;\n"
        "\n"
        "void main (void)\n"
        "{\n"
        "    mgl_FragColor = frontColor;\n"
        "}\n";

    static std::vector<Vec3f> vertices2 = { Vec3f(-2,  2, 0), // NOLINT(bugprone-throwing-static-initialization)
                                            Vec3f( 2,  2, 0),
                                            Vec3f(-2, -2, 0),
                                            Vec3f( 2, -2, 0) };

    static std::vector<Vec4f> colors2 = { Vec4f( 1, 0, 0, 1), // NOLINT(bugprone-throwing-static-initialization)
                                          Vec4f( 0, 0, 1, 1),
                                          Vec4f( 1, 0, 0, 1),
                                          Vec4f( 1, 0, 0, 1) };

    static bool animating = true;

    class MyKeyListener : public KeyListener {
      public:
        void keyPressed(KeyEvent& e, const KeyboardTracker& kt) override {
            jau_fprintf_td(stdout, "KeyPressed: %s; keys %zu\n", e.toString(), kt.pressedKeyCodes().count());
            if( e.keySym() == VKeyCode::VK_ESCAPE ) {
                WindowSRef win = e.source().lock();
                if( win ) {
                    win->dispose(e.when());
                }
            } else if( e.keySym() == VKeyCode::VK_PAUSE || e.keySym() == VKeyCode::VK_P ) {
                animating = !animating;
            } else if( e.keySym() == VKeyCode::VK_W ) {
                WindowSRef win = e.source().lock();
                jau_fprintf_td(stdout, "Source: %s\n", win ? win->toString() : "null");
            }
        }
        void keyReleased(KeyEvent& e, const KeyboardTracker& kt) override {
            jau_fprintf_td(stdout, "KeyRelease: %s; keys %zu\n", e.toString(), kt.pressedKeyCodes().count());
        }
    };
    typedef std::shared_ptr<MyKeyListener> MyKeyListenerRef;

    class RedSquareES2 : public RenderListener {
      private:
        Recti m_viewport;
        PMVMat4f m_pmv;
        bool m_initialized;
        jau::fraction_timespec m_tlast;

        void updatePMv()  {
            if( !m_initialized ) {
                return;
            }
            const PMVMat4f::SyncMats4& spmv = m_pmv.makeSyncPMv();
            glUniformMatrix4fv(u_pmv, (GLsizei)spmv.matrixCount(), false, spmv.floats());
        }

      public:
        RedSquareES2()
        : RenderListener(RenderListener::Private()), m_pmv(), m_initialized(false) {  }

        Recti& viewport() noexcept { return m_viewport; }
        const Recti& viewport() const noexcept { return m_viewport; }

        PMVMat4f& pmv() noexcept { return m_pmv; }
        const PMVMat4f& pmv() const noexcept { return m_pmv; }

        bool init(const WindowSRef&, const jau::fraction_timespec& when) override {
            jau_fprintf_td(stdout, "RL::init: %s\n", toString());
            m_tlast = when;
            // Create and compile vertex shader
            GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vertexShader, 1, &vertexSource, nullptr);
            glCompileShader(vertexShader);

            // Create and compile fragment shader
            GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
            glCompileShader(fragmentShader);

            // Link vertex and fragment shader into shader program and use it
            GLuint shaderProgram = glCreateProgram();
            glAttachShader(shaderProgram, vertexShader);
            glAttachShader(shaderProgram, fragmentShader);
            glLinkProgram(shaderProgram);
            glUseProgram(shaderProgram);

            // Get shader variables and initialize them

            u_pmv = glGetUniformLocation(shaderProgram, "mgl_PMVMatrix");
            a_vertices = glGetAttribLocation(shaderProgram, "mgl_Vertex");
            a_colors = glGetAttribLocation(shaderProgram, "mgl_Color");

            {
                // Create vertex buffer object and copy vertex data into it
                GLuint vbos[2];
                glGenBuffers(2, vbos);

                // Specify the layout of the shader vertex data (positions only, 3 floats)
                glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
                glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices2.size() * Vec3f::byte_size), vertices2.data(), GL_STATIC_DRAW);
                glEnableVertexAttribArray(a_vertices);
                glVertexAttribPointer(a_vertices, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

                // Specify the layout of the shader vertex data (positions only, 4 floats)
                glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
                glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(colors2.size() * Vec4f::byte_size), colors2.data(), GL_STATIC_DRAW);
                glEnableVertexAttribArray(a_colors);
                glVertexAttribPointer(a_colors, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
            }

            m_pmv.getP().loadIdentity();
            m_pmv.getMv().loadIdentity();
            m_pmv.translateMv(0, 0, -10);

            glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
            glEnable(GL_DEPTH_TEST);

            m_initialized = 0 != shaderProgram;
            return m_initialized;
        }

        void dispose(const WindowSRef&, const jau::fraction_timespec& /*when*/) override {
            jau_fprintf_td(stdout, "RL::dispose: %s\n", toString());
            m_initialized = false;
        }

        void display(const WindowSRef&, const jau::fraction_timespec& when) override {
            // jau_fprintf_td(stdout, "RL::display: %s, %s\n", toString(), win->toString());
            if( !m_initialized ) {
                return;
            }
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            m_pmv.getMv().loadIdentity();
            m_pmv.translateMv(0, 0, -10);
            static float t_sum_ms = 0;
            if( animating ) {
                t_sum_ms += float( (when - m_tlast).to_ms() );
            }
            const float ang = jau::adeg_to_rad(t_sum_ms * 360.0f) / 4000.0f;
            m_pmv.rotateMv(ang, 0, 0, 1);
            m_pmv.rotateMv(ang, 0, 1, 0);
            updatePMv();

            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            m_tlast = when;
        }

        void reshape(const WindowSRef&, const jau::math::Recti& viewport, const jau::fraction_timespec& /*when*/) override {
            jau_fprintf_td(stdout, "RL::reshape: %s\n", toString());
            m_viewport = viewport;

            m_pmv.getP().loadIdentity();

            const float aspect = 1.0f;
            const float fovy_deg=45.0f;
            const float aspect2 = ( (float) m_viewport.width() / (float) m_viewport.height() ) / aspect;
            const float zNear=1.0f;
            const float zFar=100.0f;
            m_pmv.perspectiveP(jau::adeg_to_rad(fovy_deg), aspect2, zNear, zFar);

            updatePMv();
        }

    };

}

int main(int argc, char *argv[]) // NOLINT(bugprone-exception-escape)
{
    std::string_view sfile(__FILE__);
    std::string demo_name = std::string("gamp ").append(jau::io::fs::basename(sfile, ".cpp"));
    std::cout << "Launching: " << demo_name << ", source " << sfile << " , exe " << argv[0] << "\n";

    int win_width = 1920, win_height = 1000;
    #if defined(__EMSCRIPTEN__)
        win_width = 1024, win_height = 576; // 16:9
    #endif
    {
        for(int i=1; i<argc; ++i) {
            if( 0 == strcmp("-width", argv[i]) && i+1<argc) {
                jau::fromIntString(win_width, argv[++i]);
                ++i;
            } else if( 0 == strcmp("-height", argv[i]) && i+1<argc) {
                jau::fromIntString(win_height, argv[++i]);
                ++i;
            } else if( 0 == strcmp("-fps", argv[i]) && i+1<argc) {
                int v;
                if (jau::fromIntString(v, argv[++i]).b) {
                    gamp::set_gpu_forced_fps(v);
                }
                ++i;
            }
        }
        jau_fprintf_td(stdout, "-fps: %d\n", gamp::gpu_forced_fps());
    }

    if( !gamp::init_gfx_subsystem(argv[0]) ) {
        jau_fprintf_td(stdout, "Exit (0)...");
        return 1;
    }
    {
        WindowSRef main_win = Window::create(demo_name.c_str(), win_width, win_height, GLCapabilities(), true /* verbose */);
        if( !main_win ) {
            jau_fprintf_td(stdout, "Exit (1): Failed to create window.\n");
            return 1;
        }
        {
            const int w = main_win->surfaceSize().x;
            const int h = main_win->surfaceSize().y;
            const float a = (float)w / (float)h;
            jau_fprintf_td(stdout, "FB %d x %d [w x h], aspect %f [w/h]; Win %s\n", w, h, a, main_win->windowBounds().toString());
        }
        GLContextSRef ctx = GLContext::create(main_win, GLProfile(GLProfile::GLES2), gamp::render::RenderContextFlags::verbose);
        if (!ctx) {
            jau_fprintf_td(stdout, "Exit (2): Failed to create context\n");
            main_win->dispose(jau::getMonotonicTime());
            return 1;
        }
        jau_fprintf_td(stdout, "GL Context: %s (ref-cnt %zd)\n", ctx->toString(), ctx.use_count());

        main_win->addKeyListener(std::make_shared<MyKeyListener>());
        main_win->addRenderListener(std::make_shared<RedSquareES2>());
    }

    #if defined(__EMSCRIPTEN__)
        emscripten_set_main_loop(gamp::mainloop_void, 0, 1);
    #else
        while( gamp::mainloop_default() ) { }
    #endif
    jau_fprintf_td(stdout, "Exit: RedSquare01\n");
    return 0;
}
