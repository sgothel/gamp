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

#ifndef GAMP_DEMOS_REDSQUAREES2_HPP_
#define GAMP_DEMOS_REDSQUAREES2_HPP_

#include <cstdio>
#include <memory>

#include <gamp/render/gl/GLTypes.hpp>
#include <gamp/render/gl/data/GLArrayDataServer.hpp>
#include <gamp/render/gl/data/GLUniformData.hpp>
#include <gamp/render/gl/glsl/ShaderState.hpp>

using namespace jau::math;
using namespace jau::math::util;

using namespace gamp::wt;
using namespace gamp::wt::event;
using namespace gamp::render::gl::glsl;
using namespace gamp::render::gl::data;

class RedSquareES2 : public RenderListener {
  private:
    class MyKeyListener : public KeyListener {
      private:
        RedSquareES2& m_parent;
      public:
        MyKeyListener(RedSquareES2& p) : m_parent(p) {}

        void keyPressed(const KeyEvent& e, const KeyboardTracker& kt) override {
            jau::fprintf_td(e.when().to_ms(), stdout, "KeyPressed: %s; keys %zu\n", e.toString().c_str(), kt.pressedKeyCodes().bitCount());
            if( e.keySym() == VKeyCode::VK_ESCAPE ) {
                WindowRef win = e.source().lock();
                if( win ) {
                    win->dispose(e.when());
                }
            } else if( e.keySym() == VKeyCode::VK_PAUSE || e.keySym() == VKeyCode::VK_P ) {
                m_parent.animating = !m_parent.animating;
            } else if( e.keySym() == VKeyCode::VK_W ) {
                WindowRef win = e.source().lock();
                jau::fprintf_td(e.when().to_ms(), stdout, "Source: %s\n", win ? win->toString().c_str() : "null");
            }
        }
        void keyReleased(const KeyEvent& e, const KeyboardTracker& kt) override {
            jau::fprintf_td(e.when().to_ms(), stdout, "KeyRelease: %s; keys %zu\n", e.toString().c_str(), kt.pressedKeyCodes().bitCount());
        }
    };
    typedef std::shared_ptr<MyKeyListener> MyKeyListenerRef;

    ShaderState st;
    Recti m_viewport;
    PMVMat4f m_pmv;
    MyKeyListenerRef m_kl;
    bool m_initialized;
    bool animating = true;
    jau::fraction_timespec t_last;

  public:
    RedSquareES2()
    : RenderListener(RenderListener::Private()),
      m_pmv(), m_kl(std::make_shared<MyKeyListener>(*this)), m_initialized(false) {  }

    Recti& viewport() noexcept { return m_viewport; }
    const Recti& viewport() const noexcept { return m_viewport; }

    PMVMat4f& pmv() noexcept { return m_pmv; }
    const PMVMat4f& pmv() const noexcept { return m_pmv; }

    bool init(const WindowRef& win, const jau::fraction_timespec& when) override {
        jau::fprintf_td(when.to_ms(), stdout, "RL::init: %s\n", toString().c_str());
        t_last = when;

        GL& gl = win->renderContext();
        ShaderCodeRef vp0 = ShaderCode::create(gl, GL_VERTEX_SHADER, "demos/glsl",
                "demos/glsl/bin", "RedSquareShader");
        ShaderCodeRef fp0 = ShaderCode::create(gl, GL_FRAGMENT_SHADER, "demos/glsl",
                "demos/glsl/bin", "RedSquareShader");
        if( !vp0 || !fp0 ) {
            jau::fprintf_td(when.to_ms(), stdout, "ERROR %s:%d: %s\n", E_FILE_LINE, toString().c_str());
            win->dispose(when);
            return false;
        }
        vp0->defaultShaderCustomization(gl, true, true);
        fp0->defaultShaderCustomization(gl, true, true);
        ShaderProgramRef sp0 = ShaderProgram::create();
        if( !sp0->add(gl, vp0, true) || !sp0->add(gl, fp0, true) ) {
            jau::fprintf_td(when.to_ms(), stdout, "ERROR %s:%d: %s\n", E_FILE_LINE, toString().c_str());
            sp0->destroy(gl);
            win->dispose(when);
            return false;
        }
        st.attachShaderProgram(gl, sp0, true);

        // setup mgl_PMVMatrix
        m_pmv.getP().loadIdentity();
        m_pmv.getMv().loadIdentity();
        st.ownUniform(gl, GLUniformSyncMatrices4f::create("mgl_PMVMatrix", m_pmv.getSyncPMv()));

        // Allocate Vertex Array
        GLFloatArrayDataServerRef vertices = GLFloatArrayDataServer::createGLSL("mgl_Vertex", 3, false, 4, GL_STATIC_DRAW);
        vertices->reserve(4); // reserve 4 elements (4x3 components) upfront, otherwise growIfNeeded is used
        vertices->put3f(-2,  2, 0);
        vertices->put3f( 2,  2, 0);
        vertices->put3f(-2, -2, 0);
        vertices->put3f( 2, -2, 0);
        st.ownAttribute(vertices, true);
        vertices->seal(gl, true);

        // Allocate Color Array
        GLFloatArrayDataServerRef colors = GLFloatArrayDataServer::createGLSL("mgl_Color", 4, false, 4, GL_STATIC_DRAW);
        assert(GL_FLOAT == vertices->compType()); // determined via template type jau::float32_t
        colors->put4f(1, 0, 0, 1); // used implied growIfNeeded
        colors->put4f(0, 0, 1, 1);
        colors->put4f(1, 0, 0, 1);
        colors->put4f(1, 0, 0, 1);
        st.ownAttribute(colors, true);
        colors->seal(gl, true);

        ::glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
        ::glEnable(GL_DEPTH_TEST);

        m_initialized = sp0->inUse();
        if( m_initialized ) {
            win->addKeyListener(m_kl);
        } else {
            jau::fprintf_td(when.to_ms(), stdout, "ERROR %s:%d: %s\n", E_FILE_LINE, toString().c_str());
            st.destroy(gl);
            win->dispose(when);
        }
        return m_initialized;
    }

    void dispose(const WindowRef& win, const jau::fraction_timespec& when) override {
        jau::fprintf_td(when.to_ms(), stdout, "RL::dispose: %s\n", toString().c_str());
        win->removeKeyListener(m_kl);
        st.destroy(win->renderContext());
        m_initialized = false;
    }

    void reshape(const WindowRef& win, const jau::math::Recti& viewport, const jau::fraction_timespec& when) override {
        jau::fprintf_td(when.to_ms(), stdout, "RL::reshape: %s\n", toString().c_str());
        m_viewport = viewport;

        m_pmv.getP().loadIdentity();

        const float aspect = 1.0f;
        const float fovy_deg=45.0f;
        const float aspect2 = ( (float) m_viewport.width() / (float) m_viewport.height() ) / aspect;
        const float zNear=1.0f;
        const float zFar=100.0f;
        m_pmv.perspectiveP(jau::adeg_to_rad(fovy_deg), aspect2, zNear, zFar);
        st.pushAllUniforms(win->renderContext());
    }

    void display(const WindowRef& win, const jau::fraction_timespec& when) override {
        // jau::fprintf_td(when.to_ms(), stdout, "RL::display: %s, %s\n", toString().c_str(), win->toString().c_str());
        if( !m_initialized ) {
            return;
        }
        GL& gl = win->renderContext();
        ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        st.useProgram(gl, true);
        m_pmv.getMv().loadIdentity();
        m_pmv.translateMv(0, 0, -10);
        static float t_sum_ms = 0;
        if( animating ) {
            t_sum_ms += float( (when - t_last).to_ms() );
        }
        const float ang = jau::adeg_to_rad(t_sum_ms * 360.0f) / 4000.0f;
        m_pmv.rotateMv(ang, 0, 0, 1);
        m_pmv.rotateMv(ang, 0, 1, 0);
        st.pushAllUniforms(win->renderContext());

        // Draw a square
        ::glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        st.useProgram(gl, false);

        t_last = when;
    }

    std::string toStringImpl() const noexcept override { return "RedSquareES2"; }
};

#endif // #define GAMP_DEMOS_REDSQUAREES2_HPP_
