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

#include <gamp/GampTypes.hpp>
#include <gamp/render/gl/data/GLArrayDataServer.hpp>
#include <gamp/render/gl/data/GLBuffers.hpp>
#include <gamp/render/gl/data/GLUniformData.hpp>
#include <gamp/render/gl/glsl/ShaderState.hpp>
#include <gamp/render/gl/GLTypes.hpp>
#include <gamp/wt/event/Event.hpp>
#include <gamp/wt/event/KeyEvent.hpp>
#include <gamp/wt/event/PointerEvent.hpp>

#include <jau/basic_types.hpp>
#include <jau/debug.hpp>
#include <jau/float_math.hpp>
#include <jau/functional.hpp>
#include <jau/math/mat4f.hpp>
#include <jau/math/vec2f.hpp>
#include <jau/math/vec3f.hpp>
#include <jau/math/geom/aabbox3f.hpp>
#include <jau/math/util/float_util.hpp>

using namespace jau::math;
using namespace jau::math::util;

using namespace gamp::wt;
using namespace gamp::wt::event;
using namespace gamp::render::gl::glsl;
using namespace gamp::render::gl::data;

class GearsObjectES2;
typedef jau::function<bool(const PointerEvent& e, const WindowRef& win, GearsObjectES2& shape)> PointerShapeAction;

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
    int               m_name;
    ShaderState&      m_st;
    PMVMat4f&         m_pmvMatrix;
    GLUniformDataRef  m_pmvMatrixUniform;
    GLUniformVec4fRef m_colorUniform;
    jau::math::Vec4f  m_gearColor;
    jau::math::Vec3f  m_tx;
    bool              m_useMappedBuffers;
    bool              m_picked;
    jau::math::geom::AABBox3f m_objbox, m_viewbox;
    jau::math::Mat4f  m_PMvi;

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
        array->addGLSLSubArray("mgl_Vertex", compsPerElem, GL_ARRAY_BUFFER);
        array->addGLSLSubArray("mgl_Normal", compsPerElem, GL_ARRAY_BUFFER);
    }

    void vert(GLFloatArrayDataServerRef& array, float x, float y, float z, const jau::math::Vec3f& n) {
        array->put3f(x, y, z);
        array->put3f(n);
        m_objbox.resize(x, y, z);
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
    GearsObjectES2(int name, ShaderState& st, const jau::math::Vec4f& gearColor,
                   float inner_radius, float outer_radius,
                   float     width,
                   GLsizei   teeth,
                   float     tooth_depth,
                   jau::math::Vec3f tx,
                   PMVMat4f& pmvMatrix, GLUniformDataRef pmvMatrixUniform, GLUniformVec4fRef colorUniform)
    : m_name(name), m_st(st), m_pmvMatrix(pmvMatrix), m_pmvMatrixUniform(std::move(pmvMatrixUniform)),
      m_colorUniform(std::move(colorUniform)), m_gearColor(gearColor), m_tx(tx),
      m_useMappedBuffers(false), m_picked(false), m_objbox(), m_viewbox(), m_PMvi() {
        const float dz = width * 0.5f;
        float       u, v, len;
        float       s[5];
        float       c[5];
        jau::math::Vec3f normal;
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
            normal = jau::math::Vec3f(0, 0, 1);

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
            normal = jau::math::Vec3f(0, 0, -1);

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
            normal = jau::math::Vec3f(v, -u, 0);

            vert(m_outwardFace, r1 * c[0], r1 * s[0],  dz, normal);
            vert(m_outwardFace, r1 * c[0], r1 * s[0], -dz, normal);
            vert(m_outwardFace, r2 * c[1], r2 * s[1],  dz, normal);
            vert(m_outwardFace, r2 * c[1], r2 * s[1], -dz, normal);

            normal[0] = c[0];
            normal[1] = s[0];
            vert(m_outwardFace, r2 * c[1], r2 * s[1],  dz, normal);
            vert(m_outwardFace, r2 * c[1], r2 * s[1], -dz, normal);
            vert(m_outwardFace, r2 * c[2], r2 * s[2],  dz, normal);
            vert(m_outwardFace, r2 * c[2], r2 * s[2], -dz, normal);

            normal[0] = (r1 * s[3] - r2 * s[2]);
            normal[1] = (r1 * c[3] - r2 * c[2]) * -1.0f;
            vert(m_outwardFace, r2 * c[2], r2 * s[2],  dz, normal);
            vert(m_outwardFace, r2 * c[2], r2 * s[2], -dz, normal);
            vert(m_outwardFace, r1 * c[3], r1 * s[3],  dz, normal);
            vert(m_outwardFace, r1 * c[3], r1 * s[3], -dz, normal);

            normal[0] = c[0];
            normal[1] = s[0];
            vert(m_outwardFace, r1 * c[3], r1 * s[3],  dz, normal);
            vert(m_outwardFace, r1 * c[3], r1 * s[3], -dz, normal);
            vert(m_outwardFace, r1 * c[0], r1 * s[0],  dz, normal);
            vert(m_outwardFace, r1 * c[0], r1 * s[0], -dz, normal);

            /* inside radius cylinder */
            normal[0] = c[0] * -1.0f;
            normal[1] = s[0] * -1.0f;
            normal[2] = 0.0f;
            vert(m_insideRadiusCyl, r0 * c[0], r0 * s[0], -dz, normal);
            vert(m_insideRadiusCyl, r0 * c[0], r0 * s[0],  dz, normal);
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
        vert(m_outwardFace, r1 * c[4], r1 * s[4],  dz, normal);
        vert(m_outwardFace, r1 * c[4], r1 * s[4], -dz, normal);
        m_outwardFace->seal(true);

        /* finish inside radius cylinder */
        normal[0] = c[4] * -1.0f;
        normal[1] = s[4] * -1.0f;
        normal[2] = 0.0f;
        vert(m_insideRadiusCyl, r0 * c[4], r0 * s[4], -dz, normal);
        vert(m_insideRadiusCyl, r0 * c[4], r0 * s[4],  dz, normal);
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

    void draw(GL& gl, float ang_rad) {
        m_pmvMatrix.pushMv();
        m_pmvMatrix.translateMv(m_tx);
        m_objbox.transform(m_pmvMatrix.getMv(), m_viewbox); // view-box for findPick
        m_pmvMatrix.rotateMv(ang_rad, 0.0f, 0.0f, 1.0f);
        m_st.pushUniform(gl, m_pmvMatrixUniform);  // automatic sync + update of Mvi + Mvit

        if( m_picked ) {
            const float gray = ( m_gearColor.x + m_gearColor.y + m_gearColor.z ) / 3.0f;
            m_colorUniform->vec4f() = jau::math::Vec4f(gray, gray, gray, m_gearColor.w);
        } else {
            m_colorUniform->vec4f() = m_gearColor;
        }
        m_st.pushUniform(gl, m_colorUniform);

        draw(gl, m_frontFace, GL_TRIANGLE_STRIP);
        draw(gl, m_frontSide, GL_TRIANGLES);
        draw(gl, m_backFace, GL_TRIANGLE_STRIP);
        draw(gl, m_backSide, GL_TRIANGLES);
        draw(gl, m_outwardFace, GL_TRIANGLE_STRIP);
        draw(gl, m_insideRadiusCyl, GL_TRIANGLE_STRIP);

        m_PMvi = m_pmvMatrix.getPMvi();
        m_pmvMatrix.popMv();
    }
    bool dispatchAction(const PointerShapeAction& action, const PointerEvent& e, const WindowRef& win) {
        m_pmvMatrix.pushMv();
        m_pmvMatrix.translateMv(m_tx);

        const bool done = action(e, win, *this);

        m_pmvMatrix.popMv();
        return done;
    }

    bool& picked() noexcept { return m_picked; }
    const jau::math::geom::AABBox3f& objBounds() const noexcept { return m_objbox; }
    const jau::math::geom::AABBox3f& viewBounds() const noexcept { return m_viewbox; }
    const jau::math::Mat4f& matPMvi() const noexcept { return m_PMvi; }
    constexpr int name() const noexcept { return m_name; }
    std::string toString() const noexcept { return std::string("GearsObjES2[").append(std::to_string(m_name).append(", view ").append(m_viewbox.toString())).append("]"); }
};

/**
 * GearsES2
 * @author Brian Paul (converted to C++ and added pixel-lightning by Sven Gothel)
 */
class GearsES2 : public RenderListener {
  private:
    constexpr static jau::math::Vec3f lightPos = jau::math::Vec3f(5.0f, 5.0f, 10.0f);

    ShaderState                m_st;
    PMVMat4f                   m_pmvMatrix;
    GLUniformSyncMatrices4fRef m_pmvMatrixUniform;
    GLUniformVec4fRef          m_colorUniform;
    bool                       m_useMappedBuffers;
    jau::math::Vec4f           m_gear1Color, m_gear2Color, m_gear3Color;
    GearsObjectES2             m_gear1, m_gear2, m_gear3;

    jau::math::Recti m_viewport;
    jau::math::Vec3f m_rotEuler = jau::math::Vec3f(jau::adeg_to_rad(20.0f), jau::adeg_to_rad(30.0f), jau::adeg_to_rad(0.0f));
    Vec3f m_pan;
    /// in angle degrees
    float m_teethAngle        = 0.0f;
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
      m_pmvMatrix(PMVMat4f::INVERSE_PROJECTION | PMVMat4f::INVERSE_MODELVIEW | PMVMat4f::INVERSE_TRANSPOSED_MODELVIEW),
      m_pmvMatrixUniform(GLUniformSyncMatrices4f::create("mgl_PMVMatrix", m_pmvMatrix.getSyncPMvMviMvit())),  // P, Mv, Mvi and Mvit
      m_colorUniform(GLUniformVec4f::create("mgl_StaticColor", GearsObjectES2::red)),
      m_useMappedBuffers(false),
      m_gear1Color(GearsObjectES2::red),
      m_gear2Color(GearsObjectES2::green),
      m_gear3Color(GearsObjectES2::blue),
      m_gear1(1, m_st, m_gear1Color, 1.0f, 4.0f, 1.0f, 20, 0.7f, Vec3f( -3.0f, -2.0f, 0.0f), m_pmvMatrix, m_pmvMatrixUniform, m_colorUniform),
      m_gear2(2, m_st, m_gear2Color, 0.5f, 2.0f, 2.0f, 10, 0.7f, Vec3f(  3.1f, -2.0f, 0.0f), m_pmvMatrix, m_pmvMatrixUniform, m_colorUniform),
      m_gear3(3, m_st, m_gear3Color, 1.3f, 2.0f, 0.5f, 10, 0.7f, Vec3f( -3.1f,  4.2f, 0.0f), m_pmvMatrix, m_pmvMatrixUniform, m_colorUniform)
    { }

    constexpr bool doRotate() const noexcept { return m_doRotate; }
    constexpr void setDoRotate(bool rotate) noexcept { m_doRotate = rotate; }
    constexpr void setClearBuffers(bool v) noexcept { m_clearBuffers = v; }
    constexpr void setFlipVerticalInGLOrientation(bool v) noexcept { m_flipVerticalInGLOrientation = v; }
    constexpr PMVMat4f& pmvMatrix() noexcept { return m_pmvMatrix; }
    constexpr const PMVMat4f& pmvMatrix() const noexcept { return m_pmvMatrix; }
    constexpr const jau::math::Recti& viewport() const noexcept { return m_viewport; }
    constexpr Vec3f& pan() noexcept { return m_pan; }
    constexpr jau::math::Vec3f& rotEuler() noexcept { return m_rotEuler; }

    void setClearColor(const jau::math::Vec4f& clearColor) noexcept {
        m_clearColor = clearColor;
    }

    void setGearsColors(const jau::math::Vec4f& gear1Color, const jau::math::Vec4f& gear2Color, const jau::math::Vec4f& gear3Color) noexcept {
        m_gear1Color = gear1Color;
        m_gear2Color = gear2Color;
        m_gear3Color = gear3Color;
    }
    constexpr const jau::math::Vec3f& rotEuler() const noexcept { return m_rotEuler; }
    constexpr bool  usingMappedBuffers() const noexcept { return m_useMappedBuffers; }

    void setZ(float zNear, float zFar, float zViewDist) {
        m_zNear     = zNear;
        m_zFar      = zFar;
        m_zViewDist = zViewDist;
    }
    const PMVMat4f& getPMVMatrix() const noexcept { return m_pmvMatrix; }

    bool init(const WindowRef& win, const jau::fraction_timespec& when) override {
        jau::fprintf_td(when.to_ms(), stdout, "RL::init: %s\n", toString().c_str());
        GL& gl = GL::downcast(win->renderContext());

        // m_st.setVerbose(true);
        ShaderCodeRef vp0 = ShaderCode::create(gl, GL_VERTEX_SHADER, "demos/glsl",
                                               "demos/glsl/bin", "SingleLight0");
        ShaderCodeRef fp0 = ShaderCode::create(gl, GL_FRAGMENT_SHADER, "demos/glsl",
                                               "demos/glsl/bin", "SingleLight0");
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
        m_st.ownUniform(m_pmvMatrixUniform, true);

        GLUniformVec3fRef lightU = GLUniformVec3f::create("mgl_LightPos", lightPos);
        m_st.ownUniform(lightU, true);

        m_colorUniform = GLUniformVec4f::create("color", GearsObjectES2::red);
        m_st.ownUniform(m_colorUniform, true);
        m_st.pushAllUniforms(gl);

        if( m_clearBuffers ) {
            ::glClearColor(m_clearColor.x, m_clearColor.y, m_clearColor.z, m_clearColor.w);
        }
        m_initialized = sp0->inUse();
        m_st.useProgram(gl, false);

        if( !m_initialized ) {
            jau::fprintf_td(when.to_ms(), stdout, "ERROR %s:%d: %s\n", E_FILE_LINE, toString().c_str());
            m_st.destroy(gl);
            win->dispose(when);
        }
        return m_initialized;
    }

    void dispose(const WindowRef& win, const jau::fraction_timespec& when) override {
        jau::fprintf_td(when.to_ms(), stdout, "RL::dispose: %s\n", toString().c_str());
        jau::fprintf_td(when.to_ms(), stdout, "RL::dispose: %s\n", win->toString().c_str());
        GL& gl = GL::downcast(win->renderContext());
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
        GL& gl = GL::downcast(win->renderContext());
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
                   m_teethAngle, left, right, bottom, top, w, h, l, r, b, t, _w, _h, m_flipVerticalInGLOrientation);
        }

        m_pmvMatrix.loadPIdentity();
        if( m_flipVerticalInGLOrientation && win->isBLOriented() ) {
            m_pmvMatrix.scaleP(1.0f, -1.0f, 1.0f);
        }
        m_pmvMatrix.frustumP(l, r, b, t, m_zNear, m_zFar);

        m_pmvMatrix.loadMvIdentity();
        m_pmvMatrix.translateMv(0.0f, 0.0f, -m_zViewDist);

        m_st.useProgram(gl, true);
        m_st.pushUniform(gl, m_pmvMatrixUniform);
        m_st.useProgram(gl, false);
    }

    static constexpr float getTeethRotation(int name, float adeg) noexcept {
        switch(name) {
            case 1: return jau::adeg_to_rad( 1.0f * adeg -  0.0f);
            case 2: return jau::adeg_to_rad(-2.0f * adeg -  9.0f);
            default: return jau::adeg_to_rad(-2.0f * adeg - 25.0f);
        }
    }

    void display(const WindowRef& win, const jau::fraction_timespec&) override {
        if( !m_initialized ) { return; }

        // Turn the gears' teeth
        if( m_doRotate ) {
            m_teethAngle += 0.5f;
        }
        GL& gl = GL::downcast(win->renderContext());
        // bool m_hasFocus = win->hasFocus();

        if( m_clearBuffers ) {
            ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        setGLStates(win, true);

        m_st.useProgram(gl, true);
        m_pmvMatrix.pushMv();
        m_pmvMatrix.translateMv(m_pan);
        m_pmvMatrix.rotateMv(m_rotEuler.x, 1.0f, 0.0f, 0.0f);
        m_pmvMatrix.rotateMv(m_rotEuler.y, 0.0f, 1.0f, 0.0f);
        m_pmvMatrix.rotateMv(m_rotEuler.z, 0.0f, 0.0f, 1.0f);

        m_gear1.draw(gl, getTeethRotation(1, m_teethAngle));
        m_gear2.draw(gl, getTeethRotation(2, m_teethAngle));
        m_gear3.draw(gl, getTeethRotation(3, m_teethAngle));
        m_pmvMatrix.popMv();
        m_st.useProgram(gl, false);

        setGLStates(win, false);
    }

    /// Fast loop through all shapes using PointerShapeAction w/o matrix traversal using view-coordinates
    GearsObjectES2* findPick(const PointerShapeAction& action, const PointerEvent& e, const WindowRef& win) {
        // Sort gears in z-axis descending order
        std::array<GearsObjectES2*, 3> gears{ &m_gear1, &m_gear2, &m_gear3 };
        struct ZDescending {
            bool operator()(GearsObjectES2* a, GearsObjectES2* b) const {
                return a->viewBounds().high().z > b->viewBounds().high().z;
            }
        } zDescending;
        std::sort(gears.begin(), gears.end(), zDescending);

        // We do not perform teeth-object rotation in object space for PointerEventAction!
        GearsObjectES2* res = nullptr;
        for(size_t i=0; !res && i<gears.size(); ++i) {
            if( action(e, win, *gears[i]) ) {
                res = gears[i];
            }
        }
        return res;
    }
    /// Dispatch PointerShapeAction to given shape w/ matrix traversal
    bool dispatchForShape(GearsObjectES2& shape, const PointerShapeAction& action, const PointerEvent& e, const WindowRef& win) {
        m_pmvMatrix.pushMv();
        m_pmvMatrix.translateMv(m_pan);
        m_pmvMatrix.rotateMv(m_rotEuler.x, 1.0f, 0.0f, 0.0f);
        m_pmvMatrix.rotateMv(m_rotEuler.y, 0.0f, 1.0f, 0.0f);
        m_pmvMatrix.rotateMv(m_rotEuler.z, 0.0f, 0.0f, 1.0f);

        const bool res = shape.dispatchAction(action, e, win);

        m_pmvMatrix.popMv();
        return res;
    }

  private:
    void setGLStates(const WindowRef& win, bool enable) {
        // Culling only possible if we do not flip the projection matrix
        const bool useCullFace = !(m_flipVerticalInGLOrientation && win->isBLOriented());
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
