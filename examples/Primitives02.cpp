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
#include <vector>

#include <jau/basic_types.hpp>
#include <jau/darray.hpp>
#include <jau/file_util.hpp>
#include <jau/float_math.hpp>
#include <jau/float_types.hpp>
#include <jau/fraction_type.hpp>
#include <jau/math/vec3f.hpp>
#include <jau/math/vec4f.hpp>
#include <jau/math/vec4f.hpp>
#include <jau/math/quaternion.hpp>
#include <jau/math/geom/aabbox3f.hpp>
#include <jau/math/geom/geom3f.hpp>
#include <jau/math/geom/plane/affine_transform.hpp>

#include <gamp/Gamp.hpp>
#include <gamp/render/RenderContext.hpp>
#include <gamp/render/gl/data/GLArrayDataServer.hpp>
#include <gamp/render/gl/data/GLUniformData.hpp>
#include <gamp/render/gl/glsl/ShaderState.hpp>

#include <gamp/graph/OutlineShape.hpp>
#include <gamp/graph/tess/gl/GLUTesselator.hpp>

#include "../demos/GLLauncher01.hpp"

using namespace jau::math;
using namespace jau::math::util;
using namespace jau::math::geom;

using namespace gamp;
using namespace gamp::wt;
using namespace gamp::wt::event;

using namespace gamp::graph;
using namespace gamp::graph::tess;
using namespace gamp::render::gl::glsl;
using namespace gamp::render::gl::data;

struct PMVMat4fUniform {
    PMVMat4f         m;
    GLUniformDataRef u;

    PMVMat4fUniform()
    : m( PMVMat4f::INVERSE_PROJECTION | PMVMat4f::INVERSE_MODELVIEW | PMVMat4f::INVERSE_TRANSPOSED_MODELVIEW ),
      u( GLUniformSyncMatrices4f::create("mgl_PMVMatrix", m.getSyncPMvMviMvit()) )  // P, Mv, Mvi and Mvit
    {}
};

class Shape {
  private:
    ShaderState& m_st;
    PMVMat4fUniform&  m_pmvMat;
    OutlineShape m_oshape;
    GLUtilTesselator::SegmentList m_segments;

    Vec3f m_position;
    Quat4f m_rotation;
    Vec3f m_rotPivot;
    Vec3f m_scale = Vec3f(1, 1, 1);
    float m_zOffset;
    GLUniformVec4fRef m_uColor;

    Mat4f iMat;
    Mat4f tmpMat;
    bool iMatIdent = true;
    bool iMatDirty = false;

  public:
    Shape(ShaderState &st, PMVMat4fUniform& pmvMatU)
    : m_st(st), m_pmvMat(pmvMatU), m_oshape(3, 16)
    {
        m_uColor = GLUniformVec4f::create("mgl_StaticColor", Vec4f(0, 0, 0, 1));
        m_st.ownUniform(m_uColor, true);
    }

    constexpr const Vec3f& position() const noexcept { return m_position; }
    constexpr Vec3f& position() noexcept { iMatDirty=true; return m_position; }

    constexpr const float& zOffset() const noexcept { return m_zOffset; }
    constexpr float& zOffset() noexcept { iMatDirty=true; return m_zOffset; }

    constexpr const Quat4f& rotation() const noexcept { return m_rotation; }
    constexpr Quat4f& rotation() noexcept { iMatDirty=true; return m_rotation; }

    constexpr const Vec3f& rotationPivot() const noexcept { return m_rotPivot; }
    constexpr Vec3f& rotationPivot() noexcept { iMatDirty=true; return m_rotPivot; }

    constexpr const Vec3f& scale() const noexcept { return m_scale; }
    constexpr Vec3f& scale() noexcept { iMatDirty=true; return m_scale; }

    constexpr const OutlineShape& outlines() const noexcept { return m_oshape; }
    constexpr OutlineShape& outlines() noexcept { return m_oshape; }

    const Vec4f& color() const noexcept { return m_uColor->vec4f(); }
    void setColor(const Vec4f& c) noexcept { m_uColor->vec4f()=c; }

    void update(const GLFloatArrayDataServerRef& array) {
        m_segments = GLUtilTesselator(GLUtilTesselator::FLAG_VERBOSE | GLUtilTesselator::FLAG_NORMAL, *array).tesselate(m_oshape);
        jau::INFO_PRINT("\n%s", GLUtilTesselator::Segment::toString("- ", m_segments).c_str() );
    }
    void draw(GL &gl) {
        m_pmvMat.m.pushMv();
        applyMatToMv(m_pmvMat.m);
        m_st.pushUniform(gl, m_pmvMat.u); // automatic sync + update of Mvi + Mvit

        m_st.pushUniform(gl, m_uColor);

        for(const GLUtilTesselator::Segment& s : m_segments ) {
            ::glDrawArrays(s.type, s.first, s.count);
        }

        m_pmvMat.m.popMv();
    }

  private:
    /**
     * Applies the internal {@link Matrix4f} to the given {@link PMVMatrix4f#getMv() modelview matrix},
     * i.e. {@code pmv.mulMv( getMat() )}.
     * <p>
     * Calls {@link #updateMat()} if dirty.
     * </p>
     * In case {@link #isMatIdentity()} is {@code true}, implementation is a no-operation.
     * </p>
     * @param pmv the matrix
     * @see #isMatIdentity()
     * @see #updateMat()
     * @see #getMat()
     * @see PMVMatrix4f#mulMv(Matrix4f)
     */
    void applyMatToMv(PMVMat4f& pmvMat) noexcept {
        if( iMatDirty ) {
            updateMat();
        }
        if( !iMatIdent ) {
            pmvMat.mulMv(iMat);
        }
    }
    void updateMat() noexcept {
        bool hasPos = !m_position.is_zero();
        bool hasScale = m_scale != Vec3f::one;
        bool hasRotate = !m_rotation.isIdentity();
        bool hasRotPivot = false; // null != rotPivot;
        const Vec3f& ctr = m_oshape.bounds().center();
        bool sameScaleRotatePivot = hasScale && hasRotate && ( !hasRotPivot || m_rotPivot == ctr );

        if( sameScaleRotatePivot ) {
            iMatIdent = false;
            iMat.setToTranslation(m_position); // identity + translate, scaled
            // Scale shape from its center position and rotate around its center
            iMat.translate(Vec3f(ctr).mul(m_scale)); // add-back center, scaled
            iMat.rotate(m_rotation);
            iMat.scale(m_scale);
            iMat.translate(-ctr); // move to center
        } else if( hasRotate || hasScale ) {
            iMatIdent = false;
            iMat.setToTranslation(m_position); // identity + translate, scaled
            if( hasRotate ) {
                if( hasRotPivot ) {
                    // Rotate shape around its scaled pivot
                    iMat.translate(Vec3f(m_rotPivot).mul(m_scale)); // pivot back from rot-pivot, scaled
                    iMat.rotate(m_rotation);
                    iMat.translate(Vec3f(-m_rotPivot).mul(m_scale)); // pivot to rot-pivot, scaled
                } else {
                    // Rotate shape around its scaled center
                    iMat.translate(Vec3f(ctr).mul(m_scale)); // pivot back from center-pivot, scaled
                    iMat.rotate(m_rotation);
                    iMat.translate(Vec3f(-ctr).mul(m_scale)); // pivot to center-pivot, scaled
                }
            }
            if( hasScale ) {
                // Scale shape from its center position
                iMat.translate(Vec3f(ctr).mul(m_scale)); // add-back center, scaled
                iMat.scale(m_scale);
                iMat.translate(Vec3f(-ctr).mul(m_scale)); // move to center
            }
        } else if( hasPos ) {
            iMatIdent = false;
            iMat.setToTranslation(m_position); // identity + translate, scaled

        } else {
            iMatIdent = true;
            iMat.loadIdentity();
        }
        iMatDirty = false;
    }
};
typedef std::shared_ptr<Shape> ShapeRef;

class Primitives02 : public RenderListener {
  private:
    constexpr static jau::math::Vec3f lightPos = jau::math::Vec3f(0.0f, 5.0f, 10.0f);
    constexpr static float zNear=  1.0f;
    constexpr static float zFar =100.0f;

    ShaderState m_st;
    Recti m_viewport;
    bool m_initialized;
    bool m_animating = true;
    bool m_oneframe = false;
    jau::fraction_timespec m_tlast;
    PMVMat4fUniform  m_pmvMat;
    Vec4f mgl_ColorStatic = Vec4f(0, 0, 0, 1);
    GLFloatArrayDataServerRef m_array;
    std::vector<ShapeRef> m_shapes;

  public:
    Primitives02()
    : RenderListener(RenderListener::Private()),
      m_initialized(false),
      m_array(GLFloatArrayDataServer::createGLSLInterleaved(2*3, false, 256, GL_STATIC_DRAW))
    {
        m_array->addGLSLSubArray("mgl_Vertex", 3, GL_ARRAY_BUFFER);
        m_array->addGLSLSubArray("mgl_Normal", 3, GL_ARRAY_BUFFER);
        m_st.ownAttribute(m_array, true);
    }

    Recti& viewport() noexcept { return m_viewport; }
    const Recti& viewport() const noexcept { return m_viewport; }

    PMVMat4f& pmv() noexcept { return m_pmvMat.m; }
    const PMVMat4f& pmv() const noexcept { return m_pmvMat.m; }
    bool animating() const noexcept { return m_animating; }
    bool& animating() noexcept { return m_animating; }
    void setOneFrame() noexcept { m_animating=false; m_oneframe=true; }

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

        const float lineWidth = 1/2.5f;
        const float dz = 0.001f;
        {
            // Rectangle
            const float width = 1.5f;
            const float height = 1.5f;
            const float x1 = -width/2.0f;
            const float y1 = -height/2.0f;
            const float x2 =  x1 + width;
            const float y2 =  y1 + height;
            float z = dz;
            ShapeRef frontShape = std::make_shared<Shape>(m_st, m_pmvMat);
            m_shapes.push_back(frontShape);
            OutlineShape& oshape = frontShape->outlines();
            {
                // Outer OutlineShape as Winding.CCW.
                oshape.moveTo(x1, y1, z);
                oshape.lineTo(x2, y1, z);
                oshape.lineTo(x2, y2, z);
                oshape.lineTo(x1, y2, z);
                oshape.lineTo(x1, y1, z);
                oshape.closePath();
            }
            {
                // Inner OutlineShape as Winding.CW.
                // final float dxy0 = getWidth() < getHeight() ? getWidth() : getHeight();
                const float dxy = lineWidth; // dxy0 * getDebugBox();
                oshape.moveTo(x1+dxy, y1+dxy, z);
                oshape.lineTo(x1+dxy, y2-dxy, z);
                oshape.lineTo(x2-dxy, y2-dxy, z);
                oshape.lineTo(x2-dxy, y1+dxy, z);
                oshape.lineTo(x1+dxy, y1+dxy, z);
                oshape.closePath();
            }
            frontShape->update(m_array);
            frontShape->setColor(Vec4f(0.05f, 0.05f, 0.5f, 1));
            frontShape->position().x =  1.5f;

            ShapeRef backShape = std::make_shared<Shape>(m_st, m_pmvMat);
            m_shapes.push_back(backShape);
            backShape->outlines() = oshape.flipFace();
            backShape->update(m_array);
            backShape->setColor(Vec4f(0.4f, 0.4f, 0.1f, 1));
            backShape->position().x =  1.5f;
        }
        {
            // Cross / Plus
            const float width = 1.5f;
            const float height = 1.5f;

            float lwh = lineWidth/2.0f;

            float twh = width/2.0f;
            float thh = height/2.0f;

            float ctrX = 0, ctrY = 0, ctrZ = dz;
            ShapeRef frontShape = std::make_shared<Shape>(m_st, m_pmvMat);
            m_shapes.push_back(frontShape);
            OutlineShape& oshape = frontShape->outlines();
            // CCW
            oshape.moveTo(ctrX-lwh, ctrY+thh, ctrZ); // vert: left-top
            oshape.lineTo(ctrX-lwh, ctrY+lwh, ctrZ);
            oshape.lineTo(ctrX-twh, ctrY+lwh, ctrZ); // horz: left-top
            oshape.lineTo(ctrX-twh, ctrY-lwh, ctrZ); // horz: left-bottom
            oshape.lineTo(ctrX-lwh, ctrY-lwh, ctrZ);
            oshape.lineTo(ctrX-lwh, ctrY-thh, ctrZ); // vert: left-bottom
            oshape.lineTo(ctrX+lwh, ctrY-thh, ctrZ); // vert: right-bottom
            oshape.lineTo(ctrX+lwh, ctrY-lwh, ctrZ);
            oshape.lineTo(ctrX+twh, ctrY-lwh, ctrZ); // horz: right-bottom
            oshape.lineTo(ctrX+twh, ctrY+lwh, ctrZ); // horz: right-top
            oshape.lineTo(ctrX+lwh, ctrY+lwh, ctrZ);
            oshape.lineTo(ctrX+lwh, ctrY+thh, ctrZ); // vert: right-top
            oshape.lineTo(ctrX-lwh, ctrY+thh, ctrZ); // vert: left-top
            oshape.closePath();
            // shape1->seal(gl, true);
            frontShape->update(m_array);
            frontShape->setColor(Vec4f(0.5f, 0.05f, 0.05f, 1));
            frontShape->position().x = -1.5f;

            ShapeRef backShape = std::make_shared<Shape>(m_st, m_pmvMat);
            m_shapes.push_back(backShape);
            backShape->outlines() = oshape.flipFace();
            backShape->update(m_array);
            backShape->setColor(Vec4f(0.2f, 0.2f, 0.2f, 1));
            backShape->position().x = -1.5f;
        }
        m_array->seal(gl, true);

        ::glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
        ::glEnable(GL_DEPTH_TEST);
        ::glEnable(GL_CULL_FACE);

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

        m_array->enableBuffer(gl, true);
        for(const ShapeRef& s : m_shapes) {
            if( animating() || m_oneframe ) {
                constexpr double angle_per_sec = 30;
                const float rad = (float) ( (when - m_tlast).to_double() * angle_per_sec );
                s->rotation().rotateByAngleY(jau::adeg_to_rad(  rad ));
            }
            s->draw(gl);
        }
        m_oneframe = false;
        m_array->enableBuffer(gl, false);
        m_st.useProgram(gl, false);

        m_tlast = when;
    }

    std::string toStringImpl() const noexcept override { return "Primitives02"; }
};

class Example : public Primitives02 {
  private:
    class MyKeyListener : public KeyListener {
      private:
        Primitives02& m_parent;
      public:
        MyKeyListener(Primitives02& p) : m_parent(p) {}

        void keyPressed(KeyEvent& e, const KeyboardTracker& kt) override {
            jau::fprintf_td(e.when().to_ms(), stdout, "KeyPressed: %s; keys %zu\n", e.toString().c_str(), kt.pressedKeyCodes().bitCount());
            if( e.keySym() == VKeyCode::VK_ESCAPE ) {
                WindowRef win = e.source().lock();
                if( win ) {
                    win->dispose(e.when());
                }
            } else if( e.keySym() == VKeyCode::VK_PAUSE || e.keySym() == VKeyCode::VK_P ) {
                m_parent.animating() = !m_parent.animating();
            } else if( e.keySym() == VKeyCode::VK_PERIOD ) {
                m_parent.setOneFrame();
            } else if( e.keySym() == VKeyCode::VK_W ) {
                WindowRef win = e.source().lock();
                jau::fprintf_td(e.when().to_ms(), stdout, "Source: %s\n", win ? win->toString().c_str() : "null");
            }
        }
        void keyReleased(KeyEvent& e, const KeyboardTracker& kt) override {
            jau::fprintf_td(e.when().to_ms(), stdout, "KeyRelease: %s; keys %zu\n", e.toString().c_str(), kt.pressedKeyCodes().bitCount());
        }
    };
    typedef std::shared_ptr<MyKeyListener> MyKeyListenerRef;
    MyKeyListenerRef m_kl;

  public:
    Example()
    : Primitives02(),
      m_kl(std::make_shared<MyKeyListener>(*this)) {  }

    bool init(const WindowRef& win, const jau::fraction_timespec& when) override {
        if( !Primitives02::init(win, when) ) {
            return false;
        }
        win->addKeyListener(m_kl);
        return true;
    }
    void dispose(const WindowRef& win, const jau::fraction_timespec& when) override {
        win->removeKeyListener(m_kl);
        Primitives02::dispose(win, when);
    }
};

int main(int argc, char *argv[]) // NOLINT(bugprone-exception-escape)
{
    ShaderCode::DEBUG_CODE = true;

    return launch("Primitives02.cpp",
                  GLLaunchProps{GLProfile(GLProfile::GLES2), gamp::render::RenderContextFlags::verbose},
                  std::make_shared<Example>(), argc, argv);
}
