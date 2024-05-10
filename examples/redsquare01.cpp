/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2022 Gothel Software e.K.
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
#include "gamp/gamp.hpp"
#include <random>
#include <cstdio>
#include <cmath>
#include <iostream>

#include <jau/os/os_support.hpp>

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
// ES2 precision:
//   precision highp float;
//   precision highp int;
//   // precision lowp sampler2D;
//   // precision lowp samplerCube;
// ES2/ES3 precision:
//   precision highp float;
//   precision highp int;
//   // precision lowp sampler2D;
//   // precision lowp samplerCube;
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
// ES2/ES3 precision
//   precision mediump float;
//   precision mediump int;
//   precision lowp sampler2D;
//   // precision lowp samplerCube;
static const GLchar* fragmentSource =
    "#version 100\n"
    "precision highp float;\n"
    "precision highp int;\n"
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

class RenderContext {
  public:
    typedef bool (*init_func)(RenderContext& pmv);
    
  private:    
    Recti& m_viewport;
    PMVMat4f m_pmv;
    bool m_initialized;
    
  public:    
    RenderContext(init_func init)
    : m_viewport(gamp::viewport), m_pmv(), m_initialized(false) {
        m_initialized = init(*this);
    }
    Recti& viewport() noexcept { return m_viewport; }
    const Recti& viewport() const noexcept { return m_viewport; }
    
    PMVMat4f& pmv() noexcept { return m_pmv; }
    const PMVMat4f& pmv() const noexcept { return m_pmv; }
    
    bool initialized() const noexcept { return m_initialized; }
};

void updatePMv(const PMVMat4f& pmv)
{
    const PMVMat4f::SyncMats4& spmv = pmv.getSyncPMv();
    glUniformMatrix4fv(u_pmv, (GLsizei)spmv.matrixCount(), false, spmv.floats());
}

void reshape(RenderContext& rc) {
    rc.pmv().getP().loadIdentity();

    const float aspect = 1.0f;
    const float fovy_deg=45.0f;
    const float aspect2 = ( (float) rc.viewport().width() / (float) rc.viewport().height() ) / aspect;
    const float zNear=1.0f;
    const float zFar=100.0f;
    rc.pmv().perspectiveP(gamp::adeg_to_rad(fovy_deg), aspect2, zNear, zFar);
    
    updatePMv(rc.pmv());
}

bool initialize(RenderContext& rc)
{
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
        
    PMVMat4f& pmv = rc.pmv();
    pmv.getP().loadIdentity();
    pmv.getMv().loadIdentity();
    pmv.translateMv(0, 0, -10);
    reshape(rc);    

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    
    return shaderProgram;
}

static uint64_t t0; // [ms]

void mainloop() {
    static uint64_t t_last = gamp::getElapsedMillisecond(); // [ms]
    static gamp::input_event_t event;
    static RenderContext renderContext(initialize);    

    gamp::handle_events(event);
    if( event.pressed_and_clr( gamp::input_event_type_t::WINDOW_CLOSE_REQ ) ) {
        printf("Exit Application\n");
        #if defined(__EMSCRIPTEN__)
            emscripten_cancel_main_loop();
        #else
            exit(0);
        #endif
    } else if( event.pressed_and_clr( gamp::input_event_type_t::WINDOW_RESIZED ) ) {
        reshape(renderContext);
    }
    const bool animating = !event.paused();
    (void)animating;

    const uint64_t t1 = gamp::getElapsedMillisecond(); // [ms]
    const float dt = (float)( t1 - t_last ) / 1000.0f; // [s]
    (void)dt;
    t_last = t1;

    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    
        
        PMVMat4f& pmv = renderContext.pmv();
        pmv.getMv().loadIdentity();
        pmv.translateMv(0, 0, -10);
        
        const float ang = gamp::adeg_to_rad(static_cast<float>(t1 - t0) * 360.0f) / 4000.0f;
        pmv.rotateMv(ang, 0, 0, 1);
        pmv.rotateMv(ang, 0, 1, 0);
            
        updatePMv(pmv);
        
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    gamp::swap_gpu_buffer();
}

int main(int argc, char *argv[])
{
    printf("%s\n", jau::os::get_platform_info().c_str());
    
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
            }
        }
    }
    if( !gamp::init_gfx_subsystem("gamp example01", win_width, win_height, true /* vsync */) ) {
        printf("Exit...");
        return 1;
    }

    {
        const int w = gamp::viewport.width();
        const int h = gamp::viewport.width();
        const float a = (float)w / (float)h;
        printf("FB %d x %d [w x h], aspect %f [w/h]; Win %d x %d\n", w, h, a, gamp::win_width, gamp::win_height);
    }

    t0 = gamp::getElapsedMillisecond(); // [ms]
    
    #if defined(__EMSCRIPTEN__)
        emscripten_set_main_loop(mainloop, 0, 1);
    #else
        while( true ) { mainloop(); }
    #endif
}
