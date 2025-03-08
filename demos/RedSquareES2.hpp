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
    ShaderState m_st;
    Recti m_viewport;
    PMVMat4f m_pmv;
    bool m_initialized;
    bool m_animating = true;
    jau::fraction_timespec m_tlast;

  public:
    RedSquareES2()
    : RenderListener(RenderListener::Private()),
      m_pmv(), m_initialized(false) {  }

    Recti& viewport() noexcept { return m_viewport; }
    const Recti& viewport() const noexcept { return m_viewport; }

    PMVMat4f& pmv() noexcept { return m_pmv; }
    const PMVMat4f& pmv() const noexcept { return m_pmv; }
    bool animating() const noexcept { return m_animating; }
    bool& animating() noexcept { return m_animating; }

    bool init(const WindowRef& win, const jau::fraction_timespec& when) override {
        jau::fprintf_td(when.to_ms(), stdout, "RL::init: %s\n", toString().c_str());
        m_tlast = when;

        GL& gl = GL::downcast(win->renderContext());
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
        m_st.attachShaderProgram(gl, sp0, true);

        // setup mgl_PMVMatrix
        m_pmv.getP().loadIdentity();
        m_pmv.getMv().loadIdentity();
        m_st.ownUniform(gl, GLUniformSyncMatrices4f::create("mgl_PMVMatrix", m_pmv.getSyncPMv()));

        // Allocate Vertex Array
        GLFloatArrayDataServerRef vertices = GLFloatArrayDataServer::createGLSL("mgl_Vertex", 3, false, 4, GL_STATIC_DRAW);
        vertices->reserve(4); // reserve 4 elements (4x3 components) upfront, otherwise growIfNeeded is used
        vertices->put3f(-2,  2, 0);
        vertices->put3f( 2,  2, 0);
        vertices->put3f(-2, -2, 0);
        vertices->put3f( 2, -2, 0);
        m_st.ownAttribute(vertices, true);
        vertices->seal(gl, true);

        // Allocate Color Array
        GLFloatArrayDataServerRef colors = GLFloatArrayDataServer::createGLSL("mgl_Color", 4, false, 4, GL_STATIC_DRAW);
        assert(GL_FLOAT == vertices->compType()); // determined via template type jau::float32_t
        colors->put4f(1, 0, 0, 1); // used implied growIfNeeded
        colors->put4f(0, 0, 1, 1);
        colors->put4f(1, 0, 0, 1);
        colors->put4f(1, 0, 0, 1);
        m_st.ownAttribute(colors, true);
        colors->seal(gl, true);

        ::glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
        ::glEnable(GL_DEPTH_TEST);

        m_initialized = sp0->inUse();
        if( !m_initialized ) {
            jau::fprintf_td(when.to_ms(), stdout, "ERROR %s:%d: %s\n", E_FILE_LINE, toString().c_str());
            m_st.destroy(gl);
            win->dispose(when);
        }
        return m_initialized;
    }

    void dispose(const WindowRef& win, const jau::fraction_timespec& when) override {
        jau::fprintf_td(when.to_ms(), stdout, "RL::dispose: %s\n", toString().c_str());
        m_st.destroy(GL::downcast(win->renderContext()));
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
        m_st.pushAllUniforms(GL::downcast(win->renderContext()));
    }

    void display(const WindowRef& win, const jau::fraction_timespec& when) override {
        // jau::fprintf_td(when.to_ms(), stdout, "RL::display: %s, %s\n", toString().c_str(), win->toString().c_str());
        if( !m_initialized ) {
            return;
        }
        GL& gl = GL::downcast(win->renderContext());
        ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_st.useProgram(gl, true);
        m_pmv.getMv().loadIdentity();
        m_pmv.translateMv(0, 0, -10);
        static float t_sum_ms = 0;
        if( m_animating ) {
            t_sum_ms += float( (when - m_tlast).to_ms() );
        }
        const float ang = jau::adeg_to_rad(t_sum_ms * 360.0f) / 4000.0f;
        m_pmv.rotateMv(ang, 0, 0, 1);
        m_pmv.rotateMv(ang, 0, 1, 0);
        m_st.pushAllUniforms(gl);

        // Draw a square
        ::glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        m_st.useProgram(gl, false);

        m_tlast = when;
    }

    std::string toStringImpl() const noexcept override { return "RedSquareES2"; }
};

#endif // #define GAMP_DEMOS_REDSQUAREES2_HPP_
