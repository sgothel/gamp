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
#include <jau/float_types.hpp>
#include <jau/fraction_type.hpp>
#include <jau/io/file_util.hpp>
#include <jau/math/vec4f.hpp>

#include <gamp/Gamp.hpp>
#include <gamp/render/RenderContext.hpp>

#include <gamp/render/gl/GLTypes.hpp>
#include <gamp/render/gl/data/GLArrayDataServer.hpp>
#include <gamp/render/gl/data/GLUniformData.hpp>
#include <gamp/render/gl/glsl/ShaderState.hpp>

#include "../demos/GLLauncher01.hpp"

using namespace jau::math;
using namespace jau::math::util;

using namespace gamp::wt;
using namespace gamp::wt::event;
using namespace gamp::render::gl::glsl;
using namespace gamp::render::gl::data;

class Example : public RenderListener {
  public:
    static constexpr float radius = 1.00f;
    static constexpr float coreRadius = 0.20f;
    static constexpr float coreRadiusHalfStep = 0.015f;

  private:
    ShaderState m_st;
    Recti m_viewport;
    GLUniformSyncPMVMat4fRef m_pmvMatUni;

    GLUniformVec4fRef m_uWinCenter;
    GLUniformVec4fRef m_uCoreColor, m_uHaloColor, m_uBackColor;
    GLUniformScalarF32Ref m_uWinRadius, m_uCoreRadius, m_uSeam;

    bool m_initialized;
    jau::fraction_timespec m_tlast;

    class MyKeyListener : public KeyListener {
      public:
        MyKeyListener(Example &) { }

        void keyPressed(KeyEvent &e, const KeyboardTracker &kt) override {
            jau::fprintf_td(e.when().to_ms(), stdout, "KeyPressed: %s; keys %zu\n", e.toString().c_str(), kt.pressedKeyCodes().count());
            if (e.keySym() == VKeyCode::VK_ESCAPE) {
                WindowRef win = e.source().lock();
                if (win) {
                    win->dispose(e.when());
                }
            } else if (e.keySym() == VKeyCode::VK_W) {
                WindowRef win = e.source().lock();
                jau::fprintf_td(e.when().to_ms(), stdout, "Source: %s\n", win ? win->toString().c_str() : "null");
            }
        }
        void keyReleased(KeyEvent &e, const KeyboardTracker &kt) override {
            jau::fprintf_td(e.when().to_ms(), stdout, "KeyRelease: %s; keys %zu\n", e.toString().c_str(), kt.pressedKeyCodes().count());
        }
    };
    typedef std::shared_ptr<MyKeyListener> MyKeyListenerRef;
    MyKeyListenerRef m_kl;

  public:

    Example()
    : RenderListener(RenderListener::Private()),
      m_pmvMatUni(GLUniformSyncPMVMat4f::create("gcu_PMVMatrix")),
      m_uWinCenter(GLUniformVec4f::create("gcu_solInSpace.winCenter", Vec4f())),
      m_uCoreColor(GLUniformVec4f::create("gcu_solInSpace.coreColor", Vec4f(1.0f, 1.0f, 0.0f, 1.0f))),
      m_uHaloColor(GLUniformVec4f::create("gcu_solInSpace.haloColor", Vec4f(1.0f, 0.99f, 0.0f, 1.0f))),
      m_uBackColor(GLUniformVec4f::create("gcu_solInSpace.bgColor", Vec4f(0.0f, 0.0f, 0.0f, 0.5f))),
      m_uWinRadius(GLUniformScalarF32::create("gcu_solInSpace.winRadius", 0)),
      m_uCoreRadius(GLUniformScalarF32::create("gcu_solInSpace.coreRadius", coreRadius)),
      m_uSeam(GLUniformScalarF32::create("gcu_solInSpace.seam", 0.005f)),
      m_initialized(false),
      m_kl(std::make_shared<MyKeyListener>(*this))
    {  }

    Recti& viewport() noexcept { return m_viewport; }
    const Recti& viewport() const noexcept { return m_viewport; }

    PMVMat4f& pmv() noexcept { return m_pmvMatUni->pmv(); }
    const PMVMat4f& pmv() const noexcept { return m_pmvMatUni->pmv(); }

    bool init(const WindowRef& win, const jau::fraction_timespec& when) override {
        ShaderCode::DEBUG_CODE = true;

        jau::fprintf_td(when.to_ms(), stdout, "RL::init: %s\n", toString().c_str());
        m_tlast = when;

        GL& gl = GL::downcast(win->renderContext());
        ShaderCodeRef vp0 = ShaderCode::create(gl, GL_VERTEX_SHADER, "demos/glsl",
                "demos/glsl/bin", "default");
        ShaderCodeRef fp0 = ShaderCode::create(gl, GL_FRAGMENT_SHADER, "demos/glsl",
                "demos/glsl/bin", "SolInSpace");
        if( !vp0 || !fp0 ) {
            jau::fprintf_td(when.to_ms(), stdout, "ERROR %s:%d: %s\n", E_FILE_LINE, toString().c_str());
            win->dispose(when);
            return false;
        }
        vp0->defaultShaderCustomization(gl);
        fp0->defaultShaderCustomization(gl);
        ShaderProgramRef sp0 = ShaderProgram::create();
        if( !sp0->add(gl, vp0, true) || !sp0->add(gl, fp0, true) ) {
            jau::fprintf_td(when.to_ms(), stdout, "ERROR %s:%d: %s\n", E_FILE_LINE, toString().c_str());
            sp0->destroy(gl);
            win->dispose(when);
            return false;
        }
        m_st.attachShaderProgram(gl, sp0, true);

        // setup mgl_PMVMatrix
        PMVMat4f &pmv = m_pmvMatUni->pmv();
        pmv.getP().loadIdentity();
        pmv.getMv().loadIdentity();
        m_st.ownUniform(m_pmvMatUni, true);

        m_st.ownUniform(m_uWinCenter, true);
        m_st.ownUniform(m_uCoreColor, true);
        m_st.ownUniform(m_uHaloColor, true);
        m_st.ownUniform(m_uBackColor, true);
        m_st.ownUniform(m_uWinRadius, true);
        m_st.ownUniform(m_uCoreRadius, true);
        m_st.ownUniform(m_uSeam, true);

        // ::glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
        // m_uBackColor->vec4f().set(1.0f, 1.0f, 1.0f, 0.5f);
        ::glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        m_uBackColor->vec4f().set(0.0f, 0.0f, 0.0f, 0.5f);

        // Allocate Vertex Array
        GLFloatArrayDataServerRef vertices = GLFloatArrayDataServer::createGLSL("gca_Vertex", 3, false, 4, GL_STATIC_DRAW);
        vertices->reserve(4); // reserve 4 elements (4x3 components) upfront, otherwise growIfNeeded is used
        vertices->put( { -radius,  radius, 0, // 1st vertex
                          radius,  radius, 0, // burst transfer, instead of 4x `put3f` for single vertice-value
                         -radius, -radius, 0,
                          radius, -radius, 0 } );
        m_st.ownAttribute(vertices, true);
        vertices->seal(gl, true);

        ::glEnable(GL_DEPTH_TEST);

        m_initialized = sp0->inUse();
        if( !m_initialized ) {
            jau::fprintf_td(when.to_ms(), stdout, "ERROR %s:%d: %s\n", E_FILE_LINE, toString().c_str());
            m_st.destroy(gl);
            win->dispose(when);
        }
        m_st.pushAllUniforms(gl);
        m_st.useProgram(gl, false);

        win->addKeyListener(m_kl);
        return m_initialized;
    }

    void dispose(const WindowRef& win, const jau::fraction_timespec& when) override {
        jau::fprintf_td(when.to_ms(), stdout, "RL::dispose: %s\n", toString().c_str());
        win->removeKeyListener(m_kl);
        m_st.destroy(GL::downcast(win->renderContext()));
        m_initialized = false;
    }

    void reshape(const WindowRef& win, const jau::math::Recti& viewport, const jau::fraction_timespec& when) override {
        GL& gl = GL::downcast(win->renderContext());
        jau::fprintf_td(when.to_ms(), stdout, "RL::reshape: %s\n", toString().c_str());
        m_viewport = viewport;

        const float aspect = 1.0f;
        const float fovy_deg=45.0f;
        const float aspect2 = ( (float) m_viewport.width() / (float) m_viewport.height() ) / aspect;
        const float zNear=1.0f;
        const float zFar=100.0f;
        PMVMat4f &pmv = m_pmvMatUni->pmv();
        pmv.setToPerspective(jau::adeg_to_rad(fovy_deg), aspect2, zNear, zFar);

        m_st.useProgram(gl, true);
        m_st.pushUniform(gl, m_pmvMatUni);
        m_st.useProgram(gl, false);
    }

    bool m_once = true;

    void display(const WindowRef& win, const jau::fraction_timespec& when) override {
        // jau::fprintf_td(when.to_ms(), stdout, "RL::display: %s, %s\n", toString().c_str(), win->toString().c_str());
        if( !m_initialized ) {
            return;
        }
        GL& gl = GL::downcast(win->renderContext());
        ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_st.useProgram(gl, true);
        PMVMat4f &pmv = m_pmvMatUni->pmv();
        pmv.getMv().loadIdentity();
        pmv.translateMv(0, 0, -3);

        {
            const Vec3f center(0,0,0);
            Vec3f winCenter;
            pmv.mapObjToWin(center, m_viewport, winCenter);
            m_uWinCenter->vec4f().set(winCenter, 1.0f);

            const Vec3f p1 = center + Vec3f(radius,0,0);
            Vec3f winP1;
            pmv.mapObjToWin(p1, m_viewport, winP1);
            Vec3f winR = winP1 - winCenter;
            m_uWinRadius->scalar() = winR.length();

            static float dr_dir = 1;
            constexpr float dr_min = coreRadius * 1.0f-coreRadiusHalfStep;
            constexpr float dr_max = coreRadius * 1.0f+coreRadiusHalfStep;
            const float dt = (float)(when - m_tlast).to_ms() / 1000.0f;
            float r = m_uCoreRadius->scalar() + coreRadiusHalfStep * dt * dr_dir;
            if( r <= dr_min ) {
                dr_dir = 1;
                r = dr_min;
            } else if( r >= dr_max ) {
                dr_dir = -1;
                r = dr_max;
            }
            m_uCoreRadius->scalar() = r;
        }
        m_st.pushUniform(gl, m_pmvMatUni);
        m_st.pushUniform(gl, m_uWinCenter);
        m_st.pushUniform(gl, m_uWinRadius);
        m_st.pushUniform(gl, m_uCoreRadius);
        if( m_once ) {
            std::cerr << "XXX: " << m_st << "\n";
            m_once = false;
        }

        if( true ) {
            ::glEnable(GL_BLEND);
            ::glBlendEquation(GL_FUNC_ADD); // default
            ::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        // Draw a square
        ::glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        m_st.useProgram(gl, false);

        m_tlast = when;
    }

    std::string toStringImpl() const noexcept override { return "SolInSpace"; }
};

int main(int argc, char *argv[]) // NOLINT(bugprone-exception-escape)
{
    return launch("SolInSpace",
                  GLLaunchProps{.profile=GLProfile(GLProfile::GLES2), .contextFlags=gamp::render::RenderContextFlags::verbose},
                  std::make_shared<Example>(), argc, argv);
}
