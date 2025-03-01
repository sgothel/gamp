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

#ifndef GAMP_DEMOS_GEARSES02_HPP_
#define GAMP_DEMOS_GEARSES02_HPP_

#include <cmath>
#include <cstdio>
#include <memory>
#include <numbers>

#include <gamp/gamp_types.hpp>
#include <gamp/renderer/gl/data/glarraydataserver.hpp>
#include <gamp/renderer/gl/data/glbuffers.hpp>
#include <gamp/renderer/gl/data/gluniformdata.hpp>
#include <gamp/renderer/gl/glmisc.hpp>
#include <gamp/renderer/gl/glsl/shaderstate.hpp>
#include <gamp/renderer/gl/gltypes.hpp>
#include <gamp/wt/event/event.hpp>
#include <gamp/wt/event/keyevent.hpp>
#include <gamp/wt/event/pointerevent.hpp>

using namespace jau::math;
using namespace jau::math::util;

using namespace gamp::wt;
using namespace gamp::wt::event;
using namespace gamp::render::gl::glsl;
using namespace gamp::render::gl::data;

/**
 * GearsObjectES2
 * @author Brian Paul (converted to C++ and added pixel-lightning by Sven Gothel)
 */
class GearsObjectES2 {
  public:
    constexpr static jau::math::Vec4f red   = jau::math::Vec4f(0.8f, 0.1f, 0.0f, 0.7f);
    constexpr static jau::math::Vec4f green = jau::math::Vec4f(0.0f, 0.8f, 0.2f, 0.7f);
    constexpr static jau::math::Vec4f blue  = jau::math::Vec4f(0.2f, 0.2f, 1.0f, 0.7f);

  private:
    ShaderState&      m_st;
    PMVMat4f&         m_pmvMatrix;
    GLUniformDataRef  m_pmvMatrixUniform;
    GLUniformVec4fRef m_colorUniform;
    jau::math::Vec4f  m_gearColor;
    bool              m_useMappedBuffers = false;

    GLFloatArrayDataServerRef m_frontFace;
    GLFloatArrayDataServerRef m_frontSide;
    GLFloatArrayDataServerRef m_backFace;
    GLFloatArrayDataServerRef m_backSide;
    GLFloatArrayDataServerRef m_outwardFace;
    GLFloatArrayDataServerRef m_insideRadiusCyl;

  private:
    GLFloatArrayDataServerRef createInterleaved(bool useMappedBuffers, GLsizei compsPerElem, bool normalized, GLsizei initialSize, GLenum vboUsage) {
        if( useMappedBuffers ) {
            return GLFloatArrayDataServer::createGLSLInterleavedMapped(compsPerElem, normalized, initialSize, vboUsage);
        } else {
            return GLFloatArrayDataServer::createGLSLInterleaved(compsPerElem, normalized, initialSize, vboUsage);
        }
    }

    void addInterleavedVertexAndNormalArrays(GLFloatArrayDataServerRef& array, GLsizei compsPerElem) {
        array->addGLSLSubArray("vertices", compsPerElem, GL_ARRAY_BUFFER);
        array->addGLSLSubArray("normals", compsPerElem, GL_ARRAY_BUFFER);
    }

    static void vert(GLFloatArrayDataServerRef& array, float x, float y, float z, float n[]) {
        array->put3f(x, y, z);
        array->put3f(n[0], n[1], n[2]);
    }

    static void sincos(float x, float sin[], int sinIdx, float cos[], int cosIdx) {
        sin[sinIdx] = std::sin(x);
        cos[cosIdx] = std::cos(x);
    }

    void initGL(GL& gl, GLFloatArrayDataServerRef& array) {
        array->enableBuffer(gl, true);
        array->enableBuffer(gl, false);
    }

    void associate(ShaderState& st) {
        m_frontFace->associate(st, true);
        m_frontSide->associate(st, true);
        m_backFace->associate(st, true);
        m_backSide->associate(st, true);
        m_outwardFace->associate(st, true);
        m_insideRadiusCyl->associate(st, true);
    }

    void draw(GL& gl, GLFloatArrayDataServerRef& array, int mode) {
        if( 0 < array->vboName() ) {
            array->enableBuffer(gl, true);
            ::glDrawArrays(mode, 0, array->elemCount());
            array->enableBuffer(gl, false);
        }
    }

  public:
    GearsObjectES2(ShaderState& st, const jau::math::Vec4f& gearColor,
                   float inner_radius, float outer_radius,
                   float     width,
                   GLsizei   teeth,
                   float     tooth_depth,
                   PMVMat4f& pmvMatrix, GLUniformDataRef pmvMatrixUniform, GLUniformVec4fRef colorUniform)
    : m_st(st), m_pmvMatrix(pmvMatrix), m_pmvMatrixUniform(std::move(pmvMatrixUniform)), m_colorUniform(std::move(colorUniform)), m_gearColor(gearColor), m_useMappedBuffers(false) {
        const float dz = width * 0.5f;
        float       u, v, len;
        float       s[5];
        float       c[5];
        float       normal[3];
        // const int tris_per_tooth = 32;

        const float r0 = inner_radius;
        const float r1 = outer_radius - tooth_depth / 2.0f;
        const float r2 = outer_radius + tooth_depth / 2.0f;

        const float da = 2.0f * std::numbers::pi_v<float> / float(teeth) / 4.0f;

        s[4] = 0;  // sin(0f)
        c[4] = 1;  // cos(0f)

        const GLenum vboUsage = GL_STATIC_DRAW;

        m_frontFace = createInterleaved(m_useMappedBuffers, 6, false, 4 * teeth + 2, vboUsage);
        addInterleavedVertexAndNormalArrays(m_frontFace, 3);
        m_backFace = createInterleaved(m_useMappedBuffers, 6, false, 4 * teeth + 2, vboUsage);
        addInterleavedVertexAndNormalArrays(m_backFace, 3);
        m_frontSide = createInterleaved(m_useMappedBuffers, 6, false, 6 * teeth, vboUsage);
        addInterleavedVertexAndNormalArrays(m_frontSide, 3);
        m_backSide = createInterleaved(m_useMappedBuffers, 6, false, 6 * teeth, vboUsage);
        addInterleavedVertexAndNormalArrays(m_backSide, 3);
        m_outwardFace = createInterleaved(m_useMappedBuffers, 6, false, 4 * 4 * teeth + 2, vboUsage);
        addInterleavedVertexAndNormalArrays(m_outwardFace, 3);
        m_insideRadiusCyl = createInterleaved(m_useMappedBuffers, 6, false, 2 * teeth + 2, vboUsage);
        addInterleavedVertexAndNormalArrays(m_insideRadiusCyl, 3);

        for( GLsizei i = 0; i < teeth; i++ ) {
            const float angle = float(i) * 2.0f * std::numbers::pi_v<float> / float(teeth);
            sincos(angle + da * 0.0f, s, 0, c, 0);
            sincos(angle + da * 1.0f, s, 1, c, 1);
            sincos(angle + da * 2.0f, s, 2, c, 2);
            sincos(angle + da * 3.0f, s, 3, c, 3);

            /* front  */
            normal[0] = 0.0f;
            normal[1] = 0.0f;
            normal[2] = 1.0f;

            /* front face - GL.GL_TRIANGLE_STRIP */
            vert(m_frontFace, r0 * c[0], r0 * s[0], dz, normal);
            vert(m_frontFace, r1 * c[0], r1 * s[0], dz, normal);
            vert(m_frontFace, r0 * c[0], r0 * s[0], dz, normal);
            vert(m_frontFace, r1 * c[3], r1 * s[3], dz, normal);

            /* front sides of teeth - GL.GL_TRIANGLES */
            vert(m_frontSide, r1 * c[0], r1 * s[0], dz, normal);
            vert(m_frontSide, r2 * c[1], r2 * s[1], dz, normal);
            vert(m_frontSide, r2 * c[2], r2 * s[2], dz, normal);
            vert(m_frontSide, r1 * c[0], r1 * s[0], dz, normal);
            vert(m_frontSide, r2 * c[2], r2 * s[2], dz, normal);
            vert(m_frontSide, r1 * c[3], r1 * s[3], dz, normal);

            /* back */
            normal[0] = 0.0f;
            normal[1] = 0.0f;
            normal[2] = -1.0f;

            /* back face - GL.GL_TRIANGLE_STRIP */
            vert(m_backFace, r1 * c[0], r1 * s[0], -dz, normal);
            vert(m_backFace, r0 * c[0], r0 * s[0], -dz, normal);
            vert(m_backFace, r1 * c[3], r1 * s[3], -dz, normal);
            vert(m_backFace, r0 * c[0], r0 * s[0], -dz, normal);

            /* back sides of teeth - GL.GL_TRIANGLES*/
            vert(m_backSide, r1 * c[3], r1 * s[3], -dz, normal);
            vert(m_backSide, r2 * c[2], r2 * s[2], -dz, normal);
            vert(m_backSide, r2 * c[1], r2 * s[1], -dz, normal);
            vert(m_backSide, r1 * c[3], r1 * s[3], -dz, normal);
            vert(m_backSide, r2 * c[1], r2 * s[1], -dz, normal);
            vert(m_backSide, r1 * c[0], r1 * s[0], -dz, normal);

            /* outward faces of teeth */
            u          = r2 * c[1] - r1 * c[0];
            v          = r2 * s[1] - r1 * s[0];
            len        = std::sqrt(u * u + v * v);
            u         /= len;
            v         /= len;
            normal[0]  = v;
            normal[1]  = -u;
            normal[2]  = 0.0f;

            vert(m_outwardFace, r1 * c[0], r1 * s[0], dz, normal);
            vert(m_outwardFace, r1 * c[0], r1 * s[0], -dz, normal);
            vert(m_outwardFace, r2 * c[1], r2 * s[1], dz, normal);
            vert(m_outwardFace, r2 * c[1], r2 * s[1], -dz, normal);

            normal[0] = c[0];
            normal[1] = s[0];
            vert(m_outwardFace, r2 * c[1], r2 * s[1], dz, normal);
            vert(m_outwardFace, r2 * c[1], r2 * s[1], -dz, normal);
            vert(m_outwardFace, r2 * c[2], r2 * s[2], dz, normal);
            vert(m_outwardFace, r2 * c[2], r2 * s[2], -dz, normal);

            normal[0] = (r1 * s[3] - r2 * s[2]);
            normal[1] = (r1 * c[3] - r2 * c[2]) * -1.0f;
            vert(m_outwardFace, r2 * c[2], r2 * s[2], dz, normal);
            vert(m_outwardFace, r2 * c[2], r2 * s[2], -dz, normal);
            vert(m_outwardFace, r1 * c[3], r1 * s[3], dz, normal);
            vert(m_outwardFace, r1 * c[3], r1 * s[3], -dz, normal);

            normal[0] = c[0];
            normal[1] = s[0];
            vert(m_outwardFace, r1 * c[3], r1 * s[3], dz, normal);
            vert(m_outwardFace, r1 * c[3], r1 * s[3], -dz, normal);
            vert(m_outwardFace, r1 * c[0], r1 * s[0], dz, normal);
            vert(m_outwardFace, r1 * c[0], r1 * s[0], -dz, normal);

            /* inside radius cylinder */
            normal[0] = c[0] * -1.0f;
            normal[1] = s[0] * -1.0f;
            normal[2] = 0.0f;
            vert(m_insideRadiusCyl, r0 * c[0], r0 * s[0], -dz, normal);
            vert(m_insideRadiusCyl, r0 * c[0], r0 * s[0], dz, normal);
        }
        /* finish front face */
        normal[0] = 0.0f;
        normal[1] = 0.0f;
        normal[2] = 1.0f;
        vert(m_frontFace, r0 * c[4], r0 * s[4], dz, normal);
        vert(m_frontFace, r1 * c[4], r1 * s[4], dz, normal);
        m_frontFace->seal(true);

        /* finish back face */
        normal[2] = -1.0f;
        vert(m_backFace, r1 * c[4], r1 * s[4], -dz, normal);
        vert(m_backFace, r0 * c[4], r0 * s[4], -dz, normal);
        m_backFace->seal(true);

        m_backSide->seal(true);
        m_frontSide->seal(true);

        /* finish outward face */
        sincos(da * 1.0f, s, 1, c, 1);
        u          = r2 * c[1] - r1 * c[4];
        v          = r2 * s[1] - r1 * s[4];
        len        = std::sqrt(u * u + v * v);
        u         /= len;
        v         /= len;
        normal[0]  = v;
        normal[1]  = -u;
        normal[2]  = 0.0f;
        vert(m_outwardFace, r1 * c[4], r1 * s[4], dz, normal);
        vert(m_outwardFace, r1 * c[4], r1 * s[4], -dz, normal);
        m_outwardFace->seal(true);

        /* finish inside radius cylinder */
        normal[0] = c[4] * -1.0f;
        normal[1] = s[4] * -1.0f;
        normal[2] = 0.0f;
        vert(m_insideRadiusCyl, r0 * c[4], r0 * s[4], -dz, normal);
        vert(m_insideRadiusCyl, r0 * c[4], r0 * s[4], dz, normal);
        m_insideRadiusCyl->seal(true);

        associate(m_st);
    }

    void initGL(GL& gl) {
        /** Init VBO and data .. */
        initGL(gl, m_frontFace);
        initGL(gl, m_frontSide);
        initGL(gl, m_backFace);
        initGL(gl, m_backSide);
        initGL(gl, m_outwardFace);
        initGL(gl, m_insideRadiusCyl);
    }
    void dispose(GL& gl) {
        // could be already destroyed by shared configuration
        if( m_frontFace ) {
            m_frontFace->destroy(gl);
        }
        if( m_frontSide ) {
            m_frontSide->destroy(gl);
        }
        if( m_backFace ) {
            m_backFace->destroy(gl);
        }
        if( m_backSide ) {
            m_backSide->destroy(gl);
        }
        if( m_outwardFace ) {
            m_outwardFace->destroy(gl);
        }
        if( m_insideRadiusCyl ) {
            m_insideRadiusCyl->destroy(gl);
        }
        m_frontFace       = nullptr;
        m_frontSide       = nullptr;
        m_backFace        = nullptr;
        m_backSide        = nullptr;
        m_outwardFace     = nullptr;
        m_insideRadiusCyl = nullptr;
    }

    void draw(GL& gl, float x, float y, float ang_rad) {
        m_pmvMatrix.pushMv();
        m_pmvMatrix.translateMv(x, y, 0.0f);
        m_pmvMatrix.rotateMv(ang_rad, 0.0f, 0.0f, 1.0f);
        m_st.pushUniform(gl, m_pmvMatrixUniform);  // automatic sync + update of Mvi + Mvit

        m_colorUniform->vec4f() = m_gearColor;
        m_st.pushUniform(gl, m_colorUniform);

        draw(gl, m_frontFace, GL_TRIANGLE_STRIP);
        draw(gl, m_frontSide, GL_TRIANGLES);
        draw(gl, m_backFace, GL_TRIANGLE_STRIP);
        draw(gl, m_backSide, GL_TRIANGLES);
        draw(gl, m_outwardFace, GL_TRIANGLE_STRIP);
        draw(gl, m_insideRadiusCyl, GL_TRIANGLE_STRIP);

        m_pmvMatrix.popMv();
    }

    std::string toString() const noexcept { return "GearsObjectES2"; }
};

/**
 * GearsES2
 * @author Brian Paul (converted to C++ and added pixel-lightning by Sven Gothel)
 */
class GearsES2 : public RenderListener {
  private:
    constexpr static jau::math::Vec3f lightPos = jau::math::Vec3f(5.0f, 5.0f, 10.0f);

    class MyKeyListener : public KeyListener {
      private:
        GearsES2& m_parent;

      public:
        MyKeyListener(GearsES2& p): m_parent(p) { }

        void keyPressed(const KeyEvent& e, const KeyboardTracker&) override {
            const VKeyCode kc = e.keySym();
            if( e.keySym() == VKeyCode::VK_ESCAPE ) {
                WindowRef win = e.source().lock();
                if( win ) {
                    win->dispose(e.when());
                }
            } else if( kc == VKeyCode::VK_PAUSE || kc == VKeyCode::VK_P ) {
                m_parent.setDoRotate(!m_parent.doRotate());
            } else if( kc == VKeyCode::VK_W ) {
                WindowRef win = e.source().lock();
                jau::fprintf_td(e.when().to_ms(), stdout, "Source: %s\n", win ? win->toString().c_str() : "null");
            } else if( VKeyCode::VK_LEFT == kc ) {
                m_parent.setRotY(m_parent.rotY() - 1);
            } else if( VKeyCode::VK_RIGHT == kc ) {
                m_parent.setRotY(m_parent.rotY() + 1);
            } else if( VKeyCode::VK_UP == kc ) {
                m_parent.setRotX(m_parent.rotX() - 1);
            } else if( VKeyCode::VK_DOWN == kc ) {
                m_parent.setRotX(m_parent.rotX() + 1);
            }
        }
    };
    typedef std::shared_ptr<MyKeyListener> MyKeyListenerRef;

    class MyPointerListener : public PointerListener {
      private:
        GearsES2&        m_parent;
        jau::math::Vec2i preWinPos;
        jau::math::Vec3f preObjPos;

        bool mapWinToObj(const jau::math::Vec2i& pos, jau::math::Vec3f& objPos) noexcept {
            const float orthoDist = m_parent.m_zViewDist; // assume orthogonal plane at m_zViewDist
            const float winZ = jau::math::util::getOrthoWinZ(orthoDist, m_parent.m_zNear, m_parent.m_zFar);
            return m_parent.m_pmvMatrix.mapWinToObj((float)pos.x, (float)pos.y, winZ, m_parent.m_viewport, objPos);
        }

        void navigate(const PointerEvent& e) noexcept {
            const jau::math::Vec2i& pos = e.position();
            WindowRef               win = e.source().lock();
            if( win ) {
                jau::math::Vec3f objPos;
                bool mapOK = mapWinToObj(pos, objPos);
                if( e.isControlDown() ) {
                    if( mapOK ) {
                        m_parent.m_panX += (objPos.x - preObjPos.x); // positive -> left
                        m_parent.m_panY -= (objPos.y - preObjPos.y); // positive -> up
                    }
                } else {
                    const jau::math::Vec2i& sdim    = win->surfaceSize();
                    const float thetaY  = 360.0f * ((float)(pos.x - preWinPos.x) / (float)sdim.x);
                    const float thetaX  = 360.0f * ((float)(preWinPos.y - pos.y) / (float)sdim.y);
                    m_parent.m_view_rotx += thetaX;
                    m_parent.m_view_roty += thetaY;
                }
                preWinPos = pos;
                preObjPos = objPos;
            }
        }

      public:
        MyPointerListener(GearsES2& p): m_parent(p) { (void)m_parent; }
        void pointerPressed(const PointerEvent& e) override {
            if( e.pointerCount() == 1 ) {
                preWinPos = e.position();
                mapWinToObj(e.position(), preObjPos);
            }
        }
        void pointerMoved(const PointerEvent& e) override {
            if( e.isConfined() ) {
                navigate(e);
            } else {
                // track prev. position so we don't have 'jumps'
                // in case we move to confined navigation.
                preWinPos = e.position();
                mapWinToObj(e.position(), preObjPos);
            }
        }
        void pointerDragged(const PointerEvent& e) override {
            navigate(e);
        }
        void pointerWheelMoved(const PointerEvent& e) override {
            const jau::math::Vec3f& rot = e.rotation();
            if( e.isControlDown() ) {
                // alternative zoom
                float incr       = e.isShiftDown() ? rot.x : rot.y * 0.5f;
                m_parent.m_panZ += incr;
            } else {
                // panning
                m_parent.m_panX -= rot.x;  // positive -> left
                m_parent.m_panY += rot.y;  // positive -> up
            }
        }
    };

    typedef std::shared_ptr<MyPointerListener> MyPointerListenerRef;

    ShaderState                m_st;
    PMVMat4f                   m_pmvMatrix;
    GLUniformSyncMatrices4fRef m_pmvMatrixUniform;
    GLUniformVec4fRef          m_colorUniform;
    bool                       m_useMappedBuffers;
    jau::math::Vec4f           m_gear1Color, m_gear2Color, m_gear3Color;
    GearsObjectES2             m_gear1, m_gear2, m_gear3;
    MyKeyListenerRef           m_kl;
    MyPointerListenerRef       m_pl;

    jau::math::Recti m_viewport;
    /// in angle degrees
    float m_view_rotx = 20.0f, m_view_roty = 30.0f, m_view_rotz = 0.0f;
    float m_panX = 0.0f, m_panY = 0.0f, m_panZ = 0.0f;
    /// in angle degrees
    float m_angle        = 0.0f;
    int   m_swapInterval = 1;

    bool             m_flipVerticalInGLOrientation = false;
    bool             m_doRotate                    = true;
    jau::math::Vec4f m_clearColor                  = jau::math::Vec4f(0.0f);
    bool             m_clearBuffers                = true;
    bool             m_initialized                 = false;

    float m_zNear     = 5.0f;
    float m_zFar      = 10000.0f;
    float m_zViewDist = 40.0f;

  public:
    GearsES2()
    : RenderListener(RenderListener::Private()),
      m_st(),
      m_pmvMatrix(PMVMat4f::INVERSE_MODELVIEW | PMVMat4f::INVERSE_TRANSPOSED_MODELVIEW),
      m_pmvMatrixUniform(GLUniformSyncMatrices4f::create("pmvMatrix", m_pmvMatrix.getSyncPMvMviMvit())),  // P, Mv, Mvi and Mvit
      m_colorUniform(GLUniformVec4f::create("color", GearsObjectES2::red)),
      m_useMappedBuffers(false),
      m_gear1Color(GearsObjectES2::red),
      m_gear2Color(GearsObjectES2::green),
      m_gear3Color(GearsObjectES2::blue),
      m_gear1(m_st, m_gear1Color, 1.0f, 4.0f, 1.0f, 20, 0.7f, m_pmvMatrix, m_pmvMatrixUniform, m_colorUniform),
      m_gear2(m_st, m_gear2Color, 0.5f, 2.0f, 2.0f, 10, 0.7f, m_pmvMatrix, m_pmvMatrixUniform, m_colorUniform),
      m_gear3(m_st, m_gear3Color, 1.3f, 2.0f, 0.5f, 10, 0.7f, m_pmvMatrix, m_pmvMatrixUniform, m_colorUniform),
      m_kl(std::make_shared<MyKeyListener>(*this)),
      m_pl(std::make_shared<MyPointerListener>(*this)) { }

    constexpr bool doRotate() const noexcept { return m_doRotate; }
    void           setDoRotate(bool rotate) noexcept { m_doRotate = rotate; }
    void           setClearBuffers(bool v) noexcept { m_clearBuffers = v; }
    void           setFlipVerticalInGLOrientation(bool v) noexcept { m_flipVerticalInGLOrientation = v; }

    void setClearColor(const jau::math::Vec4f& clearColor) noexcept {
        m_clearColor = clearColor;
    }

    void setGearsColors(const jau::math::Vec4f& gear1Color, const jau::math::Vec4f& gear2Color, const jau::math::Vec4f& gear3Color) noexcept {
        m_gear1Color = gear1Color;
        m_gear2Color = gear2Color;
        m_gear3Color = gear3Color;
    }
    constexpr float rotX() const noexcept { return m_view_rotx; }
    constexpr float rotY() const noexcept { return m_view_roty; }
    constexpr bool  usingMappedBuffers() const noexcept { return m_useMappedBuffers; }

    void setRotX(float v) noexcept { m_view_rotx = v; }
    void setRotY(float v) noexcept { m_view_roty = v; }
    void setZ(float zNear, float zFar, float zViewDist) {
        m_zNear     = zNear;
        m_zFar      = zFar;
        m_zViewDist = zViewDist;
    }
    const PMVMat4f& getPMVMatrix() const noexcept { return m_pmvMatrix; }

    bool init(const WindowRef& win, const jau::fraction_timespec& when) override {
        jau::fprintf_td(when.to_ms(), stdout, "RL::init: %s\n", toString().c_str());
        GL& gl = win->renderContext();

        // m_st.setVerbose(true);
        ShaderCodeRef vp0 = ShaderCode::create(gl, GL_VERTEX_SHADER, "demos/glsl",
                                               "demos/glsl/bin", "gears");
        ShaderCodeRef fp0 = ShaderCode::create(gl, GL_FRAGMENT_SHADER, "demos/glsl",
                                               "demos/glsl/bin", "gears");
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

        m_gear1.initGL(gl);
        m_gear2.initGL(gl);
        m_gear3.initGL(gl);

        // st.attachObject("pmvMatrix", pmvMatrix);
        m_st.ownUniform(gl, m_pmvMatrixUniform);
        m_st.pushUniform(gl, m_pmvMatrixUniform);

        GLUniformVec3fRef lightU = GLUniformVec3f::create("lightPos", lightPos);
        m_st.ownUniform(gl, lightU);
        m_st.pushUniform(gl, lightU);

        m_colorUniform = GLUniformVec4f::create("color", GearsObjectES2::red);
        m_st.ownUniform(gl, m_colorUniform);
        m_st.pushUniform(gl, m_colorUniform);

        if( m_clearBuffers ) {
            ::glClearColor(m_clearColor.x, m_clearColor.y, m_clearColor.z, m_clearColor.w);
        }
        m_initialized = sp0->inUse();
        m_st.useProgram(gl, false);

        if( m_initialized ) {
            win->addKeyListener(m_kl);
            win->addPointerListener(m_pl);
        } else {
            jau::fprintf_td(when.to_ms(), stdout, "ERROR %s:%d: %s\n", E_FILE_LINE, toString().c_str());
            m_st.destroy(gl);
            win->dispose(when);
        }
        return m_initialized;
    }

    void dispose(const WindowRef& win, const jau::fraction_timespec& when) override {
        jau::fprintf_td(when.to_ms(), stdout, "RL::dispose: %s\n", toString().c_str());
        win->removeKeyListener(m_kl);
        win->removePointerListener(m_pl);
        GL& gl = win->renderContext();
        m_st.useProgram(gl, false);
        m_gear1.dispose(gl);
        m_gear2.dispose(gl);
        m_gear3.dispose(gl);
        m_st.destroy(gl);
        m_initialized = false;
    }

    void reshape(const WindowRef& win, const jau::math::Recti& viewport, const jau::fraction_timespec& when) override {
        jau::fprintf_td(when.to_ms(), stdout, "RL::reshape: %s\n", toString().c_str());
        if( !m_initialized ) { return; }
        m_viewport = viewport;
        reshapeImpl(win, viewport, float(viewport.width()), float(viewport.height()), when);
    }

    void reshapeImpl(const WindowRef& win, const jau::math::Recti& viewport, float imageWidth, float imageHeight, const jau::fraction_timespec& when) {
        const bool msaa = false;  // gl.getContext().getGLDrawable().getChosenGLCapabilities().getSampleBuffers();
        jau::fprintf_td(when.to_ms(), stdout, "GearsES2.reshape %s of %fx%f, swapInterval %d, msaa %d, tileRendererInUse %d\n",
                        viewport.toString().c_str(), imageWidth, imageHeight, m_swapInterval, msaa, false);
        GL& gl = win->renderContext();

        // compute projection parameters 'normal'
        float left, right, bottom, top;
        if( imageHeight > imageWidth ) {
            const float a = (float)imageHeight / (float)imageWidth;
            left          = -1.0f;
            right         = 1.0f;
            bottom        = -a;
            top           = a;
        } else {
            const float a = (float)imageWidth / (float)imageHeight;
            left          = -a;
            right         = a;
            bottom        = -1.0f;
            top           = 1.0f;
        }
        const float w = right - left;
        const float h = top - bottom;

        // compute projection parameters 'tiled'
        const float l = left   + float(viewport.x())      * w / imageWidth;
        const float r = l      + float(viewport.width())  * w / imageWidth;
        const float b = bottom + float(viewport.y())      * h / imageHeight;
        const float t = b      + float(viewport.height()) * h / imageHeight;
        {
            const float _w = r - l;
            const float _h = t - b;
            jau::fprintf_td(when.to_ms(), stdout, ">> GearsES2 angle %f, [l %f, r %f, b %f, t %f] %fx%f -> [l %f, r %f, b %f, t %f] %fx%f, v-flip %d",
                   m_angle, left, right, bottom, top, w, h, l, r, b, t, _w, _h, m_flipVerticalInGLOrientation);
        }

        m_pmvMatrix.loadPIdentity();
        if( m_flipVerticalInGLOrientation && win->isGLOriented() ) {
            m_pmvMatrix.scaleP(1.0f, -1.0f, 1.0f);
        }
        m_pmvMatrix.frustumP(l, r, b, t, m_zNear, m_zFar);

        m_pmvMatrix.loadMvIdentity();
        m_pmvMatrix.translateMv(0.0f, 0.0f, -m_zViewDist);
        m_st.useProgram(gl, true);
        m_st.pushUniform(gl, m_pmvMatrixUniform);
        m_st.useProgram(gl, false);
    }

    void display(const WindowRef& win, const jau::fraction_timespec&) override {
        if( !m_initialized ) { return; }

        // Turn the gears' teeth
        if( m_doRotate ) {
            m_angle += 0.5f;
        }
        GL& gl = win->renderContext();
        // bool m_hasFocus = win->hasFocus();

        if( m_clearBuffers ) {
            ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        setGLStates(win, true);

        m_st.useProgram(gl, true);
        m_pmvMatrix.pushMv();
        m_pmvMatrix.translateMv(m_panX, m_panY, m_panZ);
        m_pmvMatrix.rotateMv(jau::adeg_to_rad(m_view_rotx), 1.0f, 0.0f, 0.0f);
        m_pmvMatrix.rotateMv(jau::adeg_to_rad(m_view_roty), 0.0f, 1.0f, 0.0f);
        m_pmvMatrix.rotateMv(jau::adeg_to_rad(m_view_rotz), 0.0f, 0.0f, 1.0f);

        m_gear1.draw(gl, -3.0f, -2.0f, jau::adeg_to_rad(1.0f * m_angle - 0.0f));
        m_gear2.draw(gl,  3.1f, -2.0f, jau::adeg_to_rad(-2.0f * m_angle - 9.0f));
        m_gear3.draw(gl, -3.1f,  4.2f, jau::adeg_to_rad(-2.0f * m_angle - 25.0f));
        m_pmvMatrix.popMv();
        m_st.useProgram(gl, false);

        setGLStates(win, false);
    }

  private:
    void setGLStates(const WindowRef& win, bool enable) {
        // Culling only possible if we do not flip the projection matrix
        const bool useCullFace = !(m_flipVerticalInGLOrientation && win->isGLOriented());
        if( enable ) {
            ::glEnable(GL_DEPTH_TEST);
            if( useCullFace ) {
                ::glEnable(GL_CULL_FACE);
            }
        } else {
            ::glDisable(GL_DEPTH_TEST);
            if( useCullFace ) {
                ::glDisable(GL_CULL_FACE);
            }
        }
    }

  public:
    std::string toStringImpl() const noexcept override {
        std::string r("GearsES2[isInit ");
        r.append(std::to_string(m_initialized).append(", usesShared ").append(std::to_string(false)))
        .append(", 1 ")
        .append(m_gear1.toString())
        .append(", 2 ")
        .append(m_gear2.toString())
        .append(", 3 ")
        .append(m_gear3.toString())
        .append("]");
        return r;
    }
};

#endif  // GAMP_DEMO_GEARSES02_HPP_
