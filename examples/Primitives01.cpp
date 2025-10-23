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

#include <cstdio>
#include <cmath>
#include <memory>

#include <jau/basic_types.hpp>
#include <jau/darray.hpp>
#include <jau/float_math.hpp>
#include <jau/float_types.hpp>
#include <jau/fraction_type.hpp>
#include <jau/io/file_util.hpp>
#include <jau/math/geom/aabbox3f.hpp>
#include <jau/math/geom/geom3f.hpp>
#include <jau/math/vec3f.hpp>
#include <jau/math/vec4f.hpp>
#include <jau/math/vec4f.hpp>
#include <jau/math/quaternion.hpp>

#include <gamp/Gamp.hpp>
#include <gamp/render/RenderContext.hpp>
#include <gamp/render/gl/data/GLArrayDataServer.hpp>
#include <gamp/render/gl/data/GLUniformData.hpp>
#include <gamp/render/gl/glsl/ShaderState.hpp>

#include "../demos/GLLauncher01.hpp"

using namespace jau::math;
using namespace jau::math::util;
using namespace jau::math::geom;

using namespace gamp::wt;
using namespace gamp::wt::event;

using namespace gamp::render::gl::glsl;
using namespace gamp::render::gl::data;

struct PMVMat4fUniform {
    PMVMat4f         m;
    GLUniformDataRef u;

    PMVMat4fUniform()
    : m( PMVData::inv_proj | PMVData::inv_mv | PMVData::inv_tps_mv ),
      u( GLUniformSyncMatrices4f::create("mgl_PMVMatrix", m.makeSyncPMvMviMvit()) )  // P, Mv, Mvi and Mvit
    {}
};

class Shape {
  private:
    GLenum m_type;
    ShaderState& m_st;
    PMVMat4fUniform&  m_pmvMatUni;
    GLFloatArrayDataServerRef m_array;

    Vec3f m_position;
    Quat4f m_rotation;
    Vec4f m_color = Vec4f(1, 0, 0, 1);
    GLUniformVec4fRef m_uColor;

  public:
    Shape(GLenum type, ShaderState &st, PMVMat4fUniform& pmvMatU)
    : m_type(type), m_st(st), m_pmvMatUni(pmvMatU), m_array( GLFloatArrayDataServer::createGLSLInterleaved(2*3, false, 4, GL_STATIC_DRAW) )
    {
        m_array->addGLSLSubArray("mgl_Vertex", 3, GL_ARRAY_BUFFER);
        m_array->addGLSLSubArray("mgl_Normal", 3, GL_ARRAY_BUFFER);
        m_st.ownAttribute(m_array, true);

        m_uColor = GLUniformVec4f::create("mgl_StaticColor", Vec4f(1, 0, 0, 1));
        m_st.ownUniform(m_uColor, true);
    }

    constexpr const Vec3f& position() const noexcept { return m_position; }
    constexpr Vec3f& position() noexcept { return m_position; }

    constexpr const Quat4f& rotation() const noexcept { return m_rotation; }
    constexpr Quat4f& rotation() noexcept { return m_rotation; }

    constexpr const GLFloatArrayDataServerRef& vertices() const noexcept { return m_array; }
    constexpr GLFloatArrayDataServerRef& vertices() noexcept { return m_array; }

    const Vec4f& color() const noexcept { return m_uColor->vec4f(); }
    void setColor(const Vec4f& c) noexcept { m_uColor->vec4f()=c; }

    void seal(GL&gl, bool seal) {
        m_array->seal(gl, seal);
    }
    void draw(GL &gl) {
        m_pmvMatUni.m.pushMv();
        m_pmvMatUni.m.translateMv(m_position); // identity + translate, scaled
        // Rotate shape around its scaled center
        m_pmvMatUni.m.rotateMv(m_rotation);
        m_st.pushUniform(gl, m_pmvMatUni.u); // automatic sync + update of Mvi + Mvit

        m_st.pushUniform(gl, m_uColor);

        m_array->enableBuffer(gl, true);
        ::glDrawArrays(m_type, 0, m_array->elemCount());
        m_array->enableBuffer(gl, false);
        m_pmvMatUni.m.popMv();
    }

};

class Primitives01 : public RenderListener {
  private:
    constexpr static jau::math::Vec3f lightPos = jau::math::Vec3f(0.0f, 5.0f, 10.0f);
    constexpr static float half = 0.5f;
    constexpr static float qter = 0.25f;
    constexpr static float zNear=  1.0f;
    constexpr static float zFar =100.0f;

    ShaderState m_st;
    Recti m_viewport;
    bool m_initialized;
    bool m_animating = true;
    jau::fraction_timespec m_tlast;
    PMVMat4fUniform  m_pmvMat;
    Vec4f mgl_ColorStatic = Vec4f(0, 0, 0, 1);
    Shape m_shape1, m_shape2;

  public:
    Primitives01()
    : RenderListener(RenderListener::Private()),
      m_initialized(false),
      m_shape1(GL_TRIANGLE_STRIP, m_st, m_pmvMat), m_shape2(GL_LINE_STRIP, m_st, m_pmvMat) {  }

    Recti& viewport() noexcept { return m_viewport; }
    const Recti& viewport() const noexcept { return m_viewport; }

    PMVMat4f& pmv() noexcept { return m_pmvMat.m; }
    const PMVMat4f& pmv() const noexcept { return m_pmvMat.m; }
    bool animating() const noexcept { return m_animating; }
    bool& animating() noexcept { return m_animating; }

    bool init(const WindowRef& win, const jau::fraction_timespec& when) override {
        jau::fprintf_td(when.to_ms(), stdout, "RL::init: %s\n", toString().c_str());
        m_tlast = when;

        GL& gl = GL::downcast(win->renderContext());
        ShaderCodeRef vp0 = ShaderCode::create(gl, GL_VERTEX_SHADER, "demos/glsl",
                "demos/glsl/bin", "SingleLight0");
        ShaderCodeRef fp0 = ShaderCode::create(gl, GL_FRAGMENT_SHADER, "demos/glsl",
                "demos/glsl/bin", "SingleLight0");
        if( !vp0 || !fp0 ) {
            jau::fprintf_td(when.to_ms(), stdout, "ERROR %s:%d: %s\n", E_FILE_LINE, toString().c_str());
            win->dispose(when);
            return false;
        }
        {
            std::string custom = "#define MAX_TEXTURE_UNITS 0\n";
            size_t vsPos = vp0->defaultShaderCustomization(gl, true, true);
            size_t fsPos = fp0->defaultShaderCustomization(gl, true, true);
            vp0->insertShaderSource(0, vsPos, custom);
            fp0->insertShaderSource(0, fsPos, custom);
        }

        ShaderProgramRef sp0 = ShaderProgram::create();
        if( !sp0->add(gl, vp0, true) || !sp0->add(gl, fp0, true) ) {
            jau::fprintf_td(when.to_ms(), stdout, "ERROR %s:%d: %s\n", E_FILE_LINE, toString().c_str());
            sp0->destroy(gl);
            win->dispose(when);
            return false;
        }
        m_st.attachShaderProgram(gl, sp0, true);

        // setup mgl_PMVMatrix
        m_pmvMat.m.getP().loadIdentity();
        m_pmvMat.m.getMv().loadIdentity();
        m_st.ownUniform(m_pmvMat.u, true);

        GLUniformVec3fRef lightU = GLUniformVec3f::create("mgl_LightPos", lightPos);
        m_st.ownUniform(lightU, true);

        m_st.pushAllUniforms(gl);

        const Vec3f frontNormal(0, 0,  1);
        const Vec3f backNormal(0, 0, -1);
        {
            GLFloatArrayDataServerRef& v = m_shape1.vertices();
            float dz = 0.001f;
            v->put3f(-half,  half, dz); v->put3f(frontNormal);
            v->put3f( half,  half, dz); v->put3f(frontNormal);
            v->put3f(-half, -half, dz); v->put3f(frontNormal);
            v->put3f( half, -half, dz); v->put3f(frontNormal);

            dz = -0.001f;
            v->put3f(-half,  half, dz); v->put3f(backNormal);
            v->put3f( half,  half, dz); v->put3f(backNormal);
            v->put3f(-half, -half, dz); v->put3f(backNormal);
            v->put3f( half, -half, dz); v->put3f(backNormal);
            m_shape1.seal(gl, true);
            m_shape1.setColor(Vec4f(0, 1, 0, 1));
            m_shape1.position().x =  1.5f;
        }
        {
            const float lineWidth = 1/10.0f;
            const float tw = 1.0f;
            const float th = 1.0f;

            float lwh = lineWidth/2.0f;

            float twh = tw/2.0f;
            float thh = th/2.0f;

            float ctrX = 0, ctrY = 0;
            float ctrZ = 0;

            GLFloatArrayDataServerRef& v = m_shape2.vertices();
            // CCW
            v->put3f(ctrX-lwh, ctrY+thh, ctrZ); v->put3f(frontNormal); // vert: left-top
            v->put3f(ctrX-lwh, ctrY+lwh, ctrZ); v->put3f(frontNormal);
            v->put3f(ctrX-twh, ctrY+lwh, ctrZ); v->put3f(frontNormal); // horz: left-top
            v->put3f(ctrX-twh, ctrY-lwh, ctrZ); v->put3f(frontNormal); // horz: left-bottom
            v->put3f(ctrX-lwh, ctrY-lwh, ctrZ); v->put3f(frontNormal);
            v->put3f(ctrX-lwh, ctrY-thh, ctrZ); v->put3f(frontNormal); // vert: left-bottom
            v->put3f(ctrX+lwh, ctrY-thh, ctrZ); v->put3f(frontNormal); // vert: right-bottom
            v->put3f(ctrX+lwh, ctrY-lwh, ctrZ); v->put3f(frontNormal);
            v->put3f(ctrX+twh, ctrY-lwh, ctrZ); v->put3f(frontNormal); // horz: right-bottom
            v->put3f(ctrX+twh, ctrY+lwh, ctrZ); v->put3f(frontNormal); // horz: right-top
            v->put3f(ctrX+lwh, ctrY+lwh, ctrZ); v->put3f(frontNormal);
            v->put3f(ctrX+lwh, ctrY+thh, ctrZ); v->put3f(frontNormal); // vert: right-top
            v->put3f(ctrX-lwh, ctrY+thh, ctrZ); v->put3f(frontNormal); // vert: left-top
            m_shape2.seal(gl, true);
            m_shape2.setColor(Vec4f(0, 0, 1, 1));
            m_shape2.position().x = -1.5f;
        }

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
        GL& gl = GL::downcast(win->renderContext());
        jau::fprintf_td(when.to_ms(), stdout, "RL::reshape: %s\n", toString().c_str());
        m_viewport = viewport;

        m_pmvMat.m.getP().loadIdentity();
        const float aspect = 1.0f;
        const float fovy_deg=45.0f;
        const float aspect2 = ( (float) m_viewport.width() / (float) m_viewport.height() ) / aspect;
        m_pmvMat.m.perspectiveP(jau::adeg_to_rad(fovy_deg), aspect2, zNear, zFar);
        m_st.useProgram(gl, true);
        m_st.pushUniform(gl, m_pmvMat.u); // automatic sync + update of Mvi + Mvit
        m_st.useProgram(gl, true);
    }

    void display(const WindowRef& win, const jau::fraction_timespec& when) override {
        // jau::fprintf_td(when.to_ms(), stdout, "RL::display: %s, %s\n", toString().c_str(), win->toString().c_str());
        if( !m_initialized ) {
            return;
        }
        GL& gl = GL::downcast(win->renderContext());
        ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_st.useProgram(gl, true);
        m_pmvMat.m.getMv().loadIdentity();
        m_pmvMat.m.translateMv(0, 0, -5);

        if( animating() ) {
            constexpr double angle_per_sec = 30;
            const float rad = (float) ( (when - m_tlast).to_double() * angle_per_sec );
            m_shape1.rotation().rotateByAngleY(jau::adeg_to_rad(  rad ));
            m_shape2.rotation().rotateByAngleY(jau::adeg_to_rad( -rad ));
        }
        m_shape1.draw(gl);
        m_shape2.draw(gl);

        m_st.useProgram(gl, false);

        m_tlast = when;
    }

    std::string toStringImpl() const noexcept override { return "Primitives01"; }
};

class Example : public Primitives01 {
  private:
    class MyKeyListener : public KeyListener {
      private:
        Primitives01& m_parent;
      public:
        MyKeyListener(Primitives01& p) : m_parent(p) {}

        void keyPressed(KeyEvent& e, const KeyboardTracker& kt) override {
            jau::fprintf_td(e.when().to_ms(), stdout, "KeyPressed: %s; keys %zu\n", e.toString().c_str(), kt.pressedKeyCodes().count());
            if( e.keySym() == VKeyCode::VK_ESCAPE ) {
                WindowRef win = e.source().lock();
                if( win ) {
                    win->dispose(e.when());
                }
            } else if( e.keySym() == VKeyCode::VK_PAUSE || e.keySym() == VKeyCode::VK_P ) {
                m_parent.animating() = !m_parent.animating();
            } else if( e.keySym() == VKeyCode::VK_W ) {
                WindowRef win = e.source().lock();
                jau::fprintf_td(e.when().to_ms(), stdout, "Source: %s\n", win ? win->toString().c_str() : "null");
            }
        }
        void keyReleased(KeyEvent& e, const KeyboardTracker& kt) override {
            jau::fprintf_td(e.when().to_ms(), stdout, "KeyRelease: %s; keys %zu\n", e.toString().c_str(), kt.pressedKeyCodes().count());
        }
    };
    typedef std::shared_ptr<MyKeyListener> MyKeyListenerRef;
    MyKeyListenerRef m_kl;

  public:
    Example()
    : Primitives01(),
      m_kl(std::make_shared<MyKeyListener>(*this)) {  }

    bool init(const WindowRef& win, const jau::fraction_timespec& when) override {
        if( !Primitives01::init(win, when) ) {
            return false;
        }
        win->addKeyListener(m_kl);
        return true;
    }
    void dispose(const WindowRef& win, const jau::fraction_timespec& when) override {
        win->removeKeyListener(m_kl);
        Primitives01::dispose(win, when);
    }
};

int main(int argc, char *argv[]) // NOLINT(bugprone-exception-escape)
{
    ShaderCode::DEBUG_CODE = true;

    return launch("Primitives01.cpp",
                  GLLaunchProps{.profile=GLProfile(GLProfile::GLES2), .contextFlags=gamp::render::RenderContextFlags::verbose},
                  std::make_shared<Example>(), argc, argv);
}
