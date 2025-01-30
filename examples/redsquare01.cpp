/*
 * Author: Sven Gothel <sgothel@jausoft.com> and Svenson Han Gothel
 * Copyright (c) 2022-2024 Gothel Software e.K.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <gamp/gamp.hpp>

#include <gamp/wt/wtevent.hpp>
#include <gamp/wt/wtkeyevent.hpp>
#include <cstdio>
#include <cmath>
#include <gamp/wt/wtwindow.hpp>
#include <memory>
#include <jau/basic_types.hpp>
#include <jau/fraction_type.hpp>

#include <GLES2/gl2.h>
#include <SDL2/SDL_opengles2.h>

using namespace jau::math;
using namespace jau::math::util;

// Vertex shader
static GLint u_pmv;
static GLint a_vertices, a_colors;

static_assert(sizeof(GLfloat) == sizeof(float) );

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

static std::vector<Vec3f> vertices2 = { Vec3f(-2,  2, 0),
                                        Vec3f( 2,  2, 0),
                                        Vec3f(-2, -2, 0),
                                        Vec3f( 2, -2, 0) };

static std::vector<Vec4f> colors2 = { Vec4f( 1, 0, 0, 1),
                                      Vec4f( 0, 0, 1, 1),
                                      Vec4f( 1, 0, 0, 1),
                                      Vec4f( 1, 0, 0, 1) };

using namespace gamp::wt::event;
static gamp::wt::WindowRef main_win;
static bool animating = true;

class MyKeyListener : public KeyListener {
  public:
    void keyPressed(const KeyEvent& e, const KeyboardTracker& kt) override {
        printf("KeyPressed: %s; keys %zu\n", e.toString().c_str(), kt.getPressedKeyCodes().bitCount());
        if( e.keySym() == gamp::wt::event::VKeyCode::VK_ESCAPE ) {
            gamp::shutdown();
        } else if( e.keySym() == gamp::wt::event::VKeyCode::VK_PAUSE || e.keySym() == gamp::wt::event::VKeyCode::VK_P ) {
            animating = !animating;
        } else if( e.keySym() == gamp::wt::event::VKeyCode::VK_W ) {
            gamp::wt::WindowRef win = e.source().lock();
            printf("Source: %s\n", win ? win->toString().c_str() : "null");
        }
    }
    void keyReleased(const KeyEvent& e, const KeyboardTracker& kt) override {
        printf("KeyRelease: %s; keys %zu\n", e.toString().c_str(), kt.getPressedKeyCodes().bitCount());
    }
};
typedef std::shared_ptr<MyKeyListener> MyKeyListenerRef;

class MyRenderListener : public gamp::wt::RenderListener {
  private:
    Recti m_viewport;
    PMVMat4f m_pmv;
    bool m_initialized;
    jau::fraction_timespec t_last;

    void updatePMv()  {
        if( !m_initialized ) {
            return;
        }
        const PMVMat4f::SyncMats4& spmv = m_pmv.getSyncPMv();
        glUniformMatrix4fv(u_pmv, (GLsizei)spmv.matrixCount(), false, spmv.floats());
    }

  public:
    MyRenderListener()
    : m_pmv(), m_initialized(false) {  }

    Recti& viewport() noexcept { return m_viewport; }
    const Recti& viewport() const noexcept { return m_viewport; }

    PMVMat4f& pmv() noexcept { return m_pmv; }
    const PMVMat4f& pmv() const noexcept { return m_pmv; }

    void init(const gamp::wt::WindowRef&, const jau::fraction_timespec& when) override {
        printf("RL::init: %s\n", toString().c_str());
        t_last = when;
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

    }

    void dispose(const gamp::wt::WindowRef&, const jau::fraction_timespec& /*when*/) override {
        printf("RL::dispose: %s\n", toString().c_str());
        m_initialized = false;
    }

    void display(const gamp::wt::WindowRef&, const jau::fraction_timespec& when) override {
        // printf("RL::display: %s, %s\n", toString().c_str(), win->toString().c_str());
        if( !m_initialized ) {
            return;
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_pmv.getMv().loadIdentity();
        m_pmv.translateMv(0, 0, -10);
        static float t_sum_ms = 0;
        if( animating ) {
            t_sum_ms += float( (when - t_last).to_ms() );
        }
        const float ang = jau::adeg_to_rad(t_sum_ms * 360.0f) / 4000.0f;
        m_pmv.rotateMv(ang, 0, 0, 1);
        m_pmv.rotateMv(ang, 0, 1, 0);
        updatePMv();

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        t_last = when;
    }

    void reshape(const gamp::wt::WindowRef&, const jau::math::Recti& viewport, const jau::fraction_timespec& /*when*/) override {
        printf("RL::reshape: %s\n", toString().c_str());
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

int main(int argc, char *argv[]) // NOLINT(bugprone-exception-escape)
{
    int win_width = 1920, win_height = 1000;
    #if defined(__EMSCRIPTEN__)
        win_width = 1024, win_height = 576; // 16:9
    #endif
    {
        for(int i=1; i<argc; ++i) {
            if( 0 == strcmp("-width", argv[i]) && i+1<argc) {
                win_width = atoi(argv[i+1]);
                ++i;
            } else if( 0 == strcmp("-height", argv[i]) && i+1<argc) {
                win_height = atoi(argv[i+1]);
                ++i;
            } else if( 0 == strcmp("-fps", argv[i]) && i+1<argc) {
                gamp::set_gpu_forced_fps( atoi(argv[i+1]) );
                ++i;
            }
        }
        printf("-fps: %d\n", gamp::gpu_forced_fps());
    }

    if( !gamp::init_gfx_subsystem(argv[0]) ) {
        printf("Exit (0)...");
        return 1;
    }
    main_win = gamp::createWindow("gamp example01", win_width, win_height, true /* vsync */);
    if( !main_win ) {
        printf("Exit (1)...");
        return 1;
    }
    {
        const int w = main_win->surfaceSize().x;
        const int h = main_win->surfaceSize().y;
        const float a = (float)w / (float)h;
        printf("FB %d x %d [w x h], aspect %f [w/h]; Win %s\n", w, h, a, main_win->windowBounds().toString().c_str());
    }
    main_win->addKeyListener(std::make_shared<MyKeyListener>());
    main_win->addRenderListener(std::make_shared<MyRenderListener>());

    #if defined(__EMSCRIPTEN__)
        emscripten_set_main_loop(gamp::mainloop_default(), 0, 1);
    #else
        while( true ) { gamp::mainloop_default(); }
    #endif
}
