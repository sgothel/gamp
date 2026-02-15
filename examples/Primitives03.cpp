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

#include <algorithm>
#include <cstdio>
#include <cmath>
#include <gamp/graph/Graph.hpp>
#include <gamp/graph/Outline.hpp>
#include <gamp/graph/PrimTypes.hpp>
#include <gamp/render/gl/data/GLBuffers.hpp>
#include <gamp/wt/event/KeyEvent.hpp>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <jau/basic_types.hpp>
#include <jau/cpp_lang_util.hpp>
#include <jau/darray.hpp>
#include <jau/debug.hpp>
#include <jau/float_math.hpp>
#include <jau/float_types.hpp>
#include <jau/fraction_type.hpp>
#include <jau/io/file_util.hpp>
#include <jau/math/geom/geom.hpp>
#include <jau/math/util/pmvmat4.hpp>
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

#include "../demos/graph/testshapes/Glyph03FreeMonoRegular_M.hpp"

#include "../demos/GLLauncher01.hpp"

#include <gamp/graph/tess/gl/GLUTesselator.hpp>

#include <cstdio>
#include <cmath>
#include <gamp/graph/PrimTypes.hpp>
#include <gamp/render/gl/data/GLBuffers.hpp>
#include <memory>
#include <vector>

#include <jau/basic_types.hpp>
#include <jau/darray.hpp>
#include <jau/float_math.hpp>
#include <jau/float_types.hpp>
#include <jau/fraction_type.hpp>
#include <jau/io/file_util.hpp>
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

#include "gamp/render/gl/GLCapabilities.hpp"
#include "model_cobramk3.hpp"

#include "../demos/GLLauncher01.hpp"

using namespace jau::math;
using namespace jau::math::util;
using namespace jau::math::geom;

using namespace gamp;
using namespace gamp::wt;
using namespace gamp::wt::event;

using namespace gamp::graph;
using namespace gamp::render::gl::glsl;
using namespace gamp::render::gl::data;

using namespace gamp::graph::tess;

#if 0
static void printOutlineShape(const std::string &tag, const OutlineShape& o, size_t idx=0) {
    size_t o_idx=0;
    printf("- %s: OutlineShape [%2zu]: %u outlines: %s\n", tag.c_str(), idx, o.outlines().size(), o.toString().c_str());
    for(const Outline& ol : o.outlines()){
        printf("  - Outline [%2zu][%2zu]:\n", idx, o_idx);
        for(const Vertex& v : ol.vertices()){
            printf("    - V[%2zu][%2zu]: %s\n", idx, o_idx, v.coord().toString().c_str());
        }
        ++o_idx;
    }
}

static void printOutlineShapes(const std::string &tag, const std::vector<OutlineShape>& oshapes) {
    printf("%s: %zu OutlineShapes\n", tag.c_str(), oshapes.size());
    size_t os_idx=0;
    for( const OutlineShape& o : oshapes ) {
        printOutlineShape(tag, o, os_idx);
        ++os_idx;
    }
}
#endif

class GraphRenderer {
  public:
    struct GraphRendererProps {
        bool m_isTwoPass = false;
        bool m_pass1 = true;
        bool m_hasFrustumClipping = false;
        bool m_hasNormalChannel = false;
        bool m_hasLight0 = true;
        bool m_hasColorChannel = false;
        bool m_hasColorTexture = false;
        bool m_hasDiscard = true;
    };
  private:
    constexpr static PMVData mat_req = PMVData::inv_proj | PMVData::inv_mv | PMVData::inv_tps_mv;
    constexpr static jau::math::Vec3f lightPos = jau::math::Vec3f(2.0f, 2.0f, 5.0f);
    GraphRendererProps m_props;
    ShaderState& m_st;
    GLUniformSyncPMVMat4f m_pmvMat;
    GLUniformVec3f m_light0Pos;
    GLUniformVec4f m_staticColor;
    bool m_initialized;

  public:
    constexpr bool usesNormal() const noexcept { return m_props.m_hasLight0 || m_props.m_hasNormalChannel; }
    constexpr GLsizei arrayCompsPerElement() const noexcept { return usesNormal()? 2*3 : 1*3; }

  public:
    GraphRenderer(ShaderState& st)
    : m_st(st),
      m_pmvMat("gcu_PMVMatrix", mat_req), // P, Mv, Mvi and Mvit
      m_light0Pos("gcu_Light0Pos", lightPos),
      m_staticColor("gcu_StaticColor", Vec4f(0, 0, 0, 1)),
      m_initialized(false)
    {
        m_st.manage(m_pmvMat);
        m_st.manage(m_light0Pos);
        m_st.manage(m_staticColor);
    }

    constexpr bool initialized() const noexcept { return m_initialized; }

    bool init(GL& gl, const jau::fraction_timespec& when) {
        ShaderCode::DEBUG_CODE = true;
        // ShaderState::VERBOSE_STATE = true;

        ShaderCodeSRef rsVp = ShaderCode::create(gl, GL_VERTEX_SHADER, "demos/glsl",
                "demos/glsl/bin", "SingleLight0");
        ShaderCodeSRef rsFp = ShaderCode::create(gl, GL_FRAGMENT_SHADER, "demos/glsl",
                "demos/glsl/bin", "SingleLight0");
        if( !rsVp || !rsFp ) {
            jau::fprintf_td(when.to_ms(), stdout, "ERROR %s:%d\n", E_FILE_LINE);
            return false;
        }
        {
            std::string custom = "#define MAX_TEXTURE_UNITS 0\n";
            size_t vsPos = rsVp->defaultShaderCustomization(gl, true, true);
            size_t fsPos = rsFp->defaultShaderCustomization(gl, true, true);
            rsVp->insertShaderSource(0, vsPos, custom);
            rsFp->insertShaderSource(0, fsPos, custom);
        }
        ShaderProgramSRef sp0 = ShaderProgram::create();
        if( !sp0->add(gl, rsVp, true) || !sp0->add(gl, rsFp, true) ) {
            jau::fprintf_td(when.to_ms(), stdout, "ERROR %s:%d\n", E_FILE_LINE);
            sp0->destroy(gl);
            return false;
        }
        m_st.attachShaderProgram(gl, sp0, true);

        PMVMat4f& pmv = m_pmvMat.pmv();
        pmv.getP().loadIdentity();
        pmv.getMv().loadIdentity();

        m_st.send(gl, m_pmvMat);
        m_st.send(gl, m_light0Pos);
        m_st.send(gl, m_staticColor);

        m_initialized = sp0->inUse();
        if( !m_initialized ) {
            jau::fprintf_td(when.to_ms(), stdout, "ERROR %s:%d\n", E_FILE_LINE);
            m_st.destroy(gl);
        }
        return m_initialized;
    }

    void destroy(GL& gl) {
        m_st.destroyShaderProgram(gl);
    }

    void useProgram(GL& gl, bool on) {
        m_st.useProgram(gl, on);
    }

    void updateColor(GL& gl) {
        m_st.send(gl, m_staticColor);
    }
    void updatePMV(GL& gl) {
        m_st.send(gl, m_pmvMat);
    }
    void updateAll(GL& gl) {
        m_st.send(gl, m_pmvMat);
        m_st.send(gl, m_staticColor);
    }
    PMVMat4f& pmv() noexcept { return m_pmvMat.pmv(); }
    const PMVMat4f& pmv() const noexcept { return m_pmvMat.pmv(); }
    const Vec4f& color() const noexcept { return m_staticColor.vec4f(); }
    void setColor(const Vec4f& c) noexcept { m_staticColor.vec4f()=c; }
    ShaderState& st() noexcept { return m_st; }
    const ShaderState& st() const noexcept { return m_st; }
    GLUniformVec3f& lightPosition() noexcept { return m_light0Pos; }
    const GLUniformVec3f& lightPosition() const noexcept { return m_light0Pos; }
};

class GraphRegion {
  public:
    typedef jau::darray<uint32_t, glmemsize_t> u32buffer_t;
  private:
    GraphRenderer& m_renderer;
    bool m_initialized;
    GLFloatArrayDataServerSRef m_array;
    GLUtilTesselator::SegmentList m_segments;

  public:
    GraphRegion(GraphRenderer& renderer)
    : m_renderer(renderer),
      m_initialized(m_renderer.initialized()),
      m_array(GLFloatArrayDataServer::createGLSLInterleaved(m_renderer.arrayCompsPerElement(), false, 256, GL_STATIC_DRAW))    {
        m_array->addGLSLSubArray("gca_Vertex", 3, GL_ARRAY_BUFFER);
        if( m_renderer.usesNormal() ) {
            m_array->addGLSLSubArray("gca_Normal", 3, GL_ARRAY_BUFFER);
        }
        m_renderer.st().manage(m_array);
        // m_st.manage(m_indices);
    }

    constexpr bool initialized() const noexcept { return m_initialized; }

    void destroy(GL& gl) {
        m_renderer.st().destroyAllData(gl);
        // m_array->destroy(gl); // owned by m_st
    }

    void seal(GL& gl, bool seal_) {
        if( !m_initialized ) {
            return;
        }
        m_array->seal(gl, seal_);
        m_array->enableBuffer(gl, false);
    }

  public:
    void addOutlineShape(OutlineShape& shape) {
        if( !m_initialized ) {
            return;
        }
        if( Graph::DEBUG_MODE ) {
            jau_PLAIN_PRINT(true, "add.0 array: %s", m_array->toString().c_str());
            jau_PLAIN_PRINT(true, "add.0 segments:\n%s", GLUtilTesselator::Segment::toString("- ", m_segments).c_str() );
        }
        // TODO use a GLUtilTesselator instance to be reused (perf)?
        // - Keep native tesselator instance in GLUtilTesselator, callback setup and etc
        {
            GLUtilTesselator glutess(GLUtilTesselator::FLAG_VERBOSE | GLUtilTesselator::FLAG_NORMAL, *m_array);
            GLUtilTesselator::SegmentList segs = glutess.tesselate(shape);
            m_segments.insert(m_segments.cend(), segs.cbegin(), segs.cend());
        }

        if( Graph::DEBUG_MODE ) {
            jau_PLAIN_PRINT(true, "add.x array: %s", m_array->toString().c_str());
            jau_PLAIN_PRINT(true, "add.x segments:\n%s", GLUtilTesselator::Segment::toString("- ", m_segments).c_str() );
        }
    }

    void draw(GL &gl) {
        if( !m_initialized ) {
            return;
        }
        m_renderer.useProgram(gl, true);

        m_array->enableBuffer(gl, true);

        ::glEnable(GL_BLEND);
        ::glBlendEquation(GL_FUNC_ADD); // default
        ::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        for(const GLUtilTesselator::Segment& s : m_segments ) {
            ::glDrawArrays(s.type, s.first, s.count);
        }

        m_array->enableBuffer(gl, false);
        // m_renderer.useProgram(gl, false);
    }
};

class Shape;
typedef std::shared_ptr<Shape> ShapeRef;

class Shape {
  private:
    std::vector<OutlineShape> m_oshapes;

    Vec3f m_position;
    Quat4f m_rotation;
    Vec3f m_rotPivot;
    Vec3f m_scale = Vec3f(1, 1, 1);
    float m_zOffset = 0.0f;
    Vec4f m_color = Vec4f(0, 0, 0, 1);
    GraphRenderer& m_renderer;
    GraphRegion m_region;

    Mat4f iMat;
    Mat4f tmpMat;
    bool iMatIdent = true;
    bool iMatDirty = false;

    float m_velo = 0; // m/s

    struct Private{ explicit Private() = default; };

  public:
    Shape(Private, GraphRenderer &renderer)
    : m_renderer(renderer), m_region(m_renderer)
    {
        std::cerr << "XXX ctor.x " << m_renderer.st() << "\n";
    }

    static ShapeRef createShared(GraphRenderer &renderer) {
        return std::make_shared<Shape>(Private(), renderer);
    }

    void destroy(GL& gl) {
        m_region.destroy(gl);
    }

    constexpr const Vec3f& position() const noexcept { return m_position; }
    constexpr Vec3f& position() noexcept { iMatDirty=true; return m_position; }
    constexpr void set_position(Vec3f new_pos) noexcept { m_position = new_pos; }

    constexpr const float& zOffset() const noexcept { return m_zOffset; }
    constexpr float& zOffset() noexcept { iMatDirty=true; return m_zOffset; }

    constexpr const Quat4f& rotation() const noexcept { return m_rotation; }
    constexpr Quat4f& rotation() noexcept { iMatDirty=true; return m_rotation; }

    constexpr const Vec3f& rotationPivot() const noexcept { return m_rotPivot; }
    constexpr Vec3f& rotationPivot() noexcept { iMatDirty=true; return m_rotPivot; }

    constexpr const Vec3f& scale() const noexcept { return m_scale; }
    constexpr Vec3f& scale() noexcept { iMatDirty=true; return m_scale; }

    constexpr const std::vector<OutlineShape>& outlineShapes() const noexcept { return m_oshapes; }
    constexpr std::vector<OutlineShape>& outlineShapes() noexcept { return m_oshapes; }

    const Vec4f& color() const noexcept { return m_color; }
    void setColor(const Vec4f& c) noexcept { m_color = c; }

    void update(GL& gl) {
        for(OutlineShape& o : m_oshapes){
            m_region.addOutlineShape(o);
        }
        m_region.seal(gl, true);
    }

    void draw(GL &gl) {
        PMVMat4f& pmv = m_renderer.pmv();
        pmv.pushMv();
        applyMatToMv(pmv);

        m_renderer.setColor(m_color);
        m_renderer.updateAll(gl); // PMV + Color

        m_region.draw(gl);
        pmv.popMv();
    }

    /// Game ..
    void tick(float dt) {
        if( !jau::is_zero(m_velo) ) {
            iMatDirty = true;
            m_rotation.rotateByAngleZ( M_PI_2);
            Vec3f dir = m_rotation.rotateVector(Vec3f(1, 0, 0));
            m_rotation.rotateByAngleZ(-M_PI_2);
            Vec3f d_p = dir * m_velo * dt;
            m_position += d_p;
        }
    }
    float& velo() noexcept { return m_velo; }

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
        for(OutlineShape& o : m_oshapes){
            const Vec3f& ctr = o.bounds().center();
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
    }
};

class Primitives03 : public RenderListener {
  private:
    constexpr static float zNear=  1.0f;
    constexpr static float zFar =100.0f;

    ShaderState m_st;
    Recti m_viewport;
    bool m_initialized;
    bool m_animating = true;
    bool m_oneframe = false;
    jau::fraction_timespec m_tlast;
    GraphRenderer m_renderer;
    std::vector<ShapeRef> m_shapes;
    bool m_once = true;
    GLUniformSyncPMVMat4f m_pmvMat;
    GLUniformVec3f m_light0Pos;
    GLUniformVec4f m_staticColor;

  public:
    Primitives03()
    : RenderListener(RenderListener::Private()),
      m_initialized(false),
      m_renderer(m_st),
      m_pmvMat("gcu_PMVMatrix", PMVData::inv_proj | PMVData::inv_mv | PMVData::inv_tps_mv), // P, Mv, Mvi and Mvit
      m_light0Pos("gcu_Light0Pos", m_renderer.lightPosition().vec3f()),
      m_staticColor("gcu_StaticColor", Vec4f(0.05f, 0.05f, 0.5f, 1))
    {
        m_st.manage(m_pmvMat);
        m_st.manage(m_light0Pos);
        m_st.manage(m_staticColor);
    }

    Recti& viewport() noexcept { return m_viewport; }
    const Recti& viewport() const noexcept { return m_viewport; }
    std::vector<ShapeRef>& shapes() noexcept { return m_shapes; }

    bool animating() const noexcept { return m_animating; }
    bool& animating() noexcept { return m_animating; }
    void setOneFrame() noexcept { m_animating=false; m_oneframe=true; }

    bool init(const WindowSRef& win, const jau::fraction_timespec& when) override {
        jau::fprintf_td(when.to_ms(), stdout, "RL::init: %s\n", toString().c_str());
        m_tlast = when;

        GL& gl = GL::downcast(win->renderContext());

        if( !m_renderer.init(gl, when) ) {
            jau::fprintf_td(when.to_ms(), stdout, "ERROR %s:%d: %s\n", E_FILE_LINE, toString().c_str());
            win->dispose(when);
            return false;
        }

        //const float lineWidth = 1/2.5f;
        ShapeRef cobraMkIII_Shape = Shape::createShared(m_renderer);
        models::appendCobraMkIII(cobraMkIII_Shape->outlineShapes());
        cobraMkIII_Shape->setColor(Vec4f(0.05f, 0.05f, 0.5f, 1.0f));
        cobraMkIII_Shape->rotation().rotateByAngleX(-M_PI / 4.0f);
        cobraMkIII_Shape->update(gl);
        m_shapes.push_back(cobraMkIII_Shape);
        if ( false ) {
            ShapeRef frontShape = Shape::createShared(m_renderer);
            m_shapes.push_back(frontShape);
            std::vector<OutlineShape>& oshapes = frontShape->outlineShapes();
            for(OutlineShape& o : oshapes){
                Glyph03FreeMonoRegular_M::addShapeToRegion(o);

                OutlineShape back = o.flipFace(); // -dz);
                oshapes.push_back(back);
            }
            frontShape->update(gl);
            frontShape->setColor(Vec4f(0.05f, 0.5f, 0.05f, 1));
            frontShape->position().x = 1.5f;
            frontShape->position().y = 0.5f;
            frontShape->scale().x *= 2.0f;
            frontShape->scale().y *= 2.0f;
        }
        //::glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
        ::glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        ::glEnable(GL_DEPTH_TEST);
        // ::glEnable(GL_CULL_FACE);
        ::glDisable(GL_CULL_FACE);

        m_initialized = true;
        if( !m_initialized ) {
            jau::fprintf_td(when.to_ms(), stdout, "ERROR %s:%d: %s\n", E_FILE_LINE, toString().c_str());
            m_st.destroy(gl);
            win->dispose(when);
        }
        return m_initialized;
    }

    void dispose(const WindowSRef& win, const jau::fraction_timespec& when) override {
        GL& gl = GL::downcast(win->renderContext());
        jau::fprintf_td(when.to_ms(), stdout, "RL::dispose: %s\n", toString().c_str());
        for(ShapeRef& s : m_shapes) {
            s->destroy(gl);
        }
        m_renderer.destroy(gl);
        m_st.destroy(GL::downcast(win->renderContext()));
        m_initialized = false;
    }

    void reshape(const WindowSRef& win, const jau::math::Recti& viewport, const jau::fraction_timespec& when) override {
        GL& gl = GL::downcast(win->renderContext());
        jau::fprintf_td(when.to_ms(), stdout, "RL::reshape: %s\n", toString().c_str());
        m_viewport = viewport;

        PMVMat4f& pmv = m_renderer.pmv();
        pmv.getP().loadIdentity();
        const float aspect = 1.0f;
        const float fovy_deg=45.0f;
        const float aspect2 = ( (float) m_viewport.width() / (float) m_viewport.height() ) / aspect;
        pmv.perspectiveP(jau::adeg_to_rad(fovy_deg), aspect2, zNear, zFar);
        pmv.getMv().loadIdentity();
        pmv.translateMv(0, 0, -5);

        m_st.useProgram(gl, true);
        // m_st.useProgram(gl, false);
    }

    void display(const WindowSRef& win, const jau::fraction_timespec& when) override {
        // jau::fprintf_td(when.to_ms(), stdout, "RL::display: %s, %s\n", toString().c_str(), win->toString().c_str());
        if( !m_initialized ) {
            return;
        }
        GL& gl = GL::downcast(win->renderContext());
        ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_st.useProgram(gl, true);

        const float dt = float( (when - m_tlast).to_double() );
        for(const ShapeRef& s : m_shapes) {
            if( (animating() || m_oneframe) && s != m_shapes[0]) {
                constexpr float angle_per_sec = 30;
                const float rad = dt * angle_per_sec;
                s->rotation().rotateByAngleX(jau::adeg_to_rad( -rad ));
                s->rotation().rotateByAngleY(jau::adeg_to_rad(  rad ));
                //s->rotation().rotateByAngleZ(jau::adeg_to_rad(  rad ));
            }
            s->tick(dt);
            s->draw(gl);
        }
        m_oneframe = false;

        if( m_once ) {
            m_once = false;
            std::cerr << "XXX draw " << m_st << "\n";
        }

        // m_st.useProgram(gl, false);

        m_tlast = when;
    }

    std::string toStringImpl() const noexcept override { return "GraphShapes01"; }
};

class Example : public Primitives03 {
  private:
    class MyKeyListener : public KeyListener {
      private:
        Primitives03& m_parent;
      public:
        MyKeyListener(Primitives03& p) : m_parent(p) {}

        void keyPressed(KeyEvent& e, const KeyboardTracker& kt) override {
            jau::fprintf_td(e.when().to_ms(), stdout, "KeyPressed: %s; keys %zu\n", e.toString().c_str(), kt.pressedKeyCodes().count());
            std::vector<ShapeRef>& shapeList = m_parent.shapes();
            if( e.keySym() == VKeyCode::VK_ESCAPE ) {
                WindowSRef win = e.source().lock();
                if( win ) {
                    win->dispose(e.when());
                }
            } else if( e.keySym() == VKeyCode::VK_PAUSE || e.keySym() == VKeyCode::VK_P ) {
                m_parent.animating() = !m_parent.animating();
            } else if( e.keySym() == VKeyCode::VK_PERIOD ) {
                m_parent.setOneFrame();
            } else if( e.keySym() == VKeyCode::VK_W ) {
                WindowSRef win = e.source().lock();
                jau::fprintf_td(e.when().to_ms(), stdout, "Source: %s\n", win ? win->toString().c_str() : "null");
            } else if( e.keySym() == VKeyCode::VK_UP   ) {
                shapeList[0]->rotation().rotateByAngleX(-M_PI / 50);
            } else if( e.keySym() == VKeyCode::VK_DOWN ) {
                shapeList[0]->rotation().rotateByAngleX( M_PI / 50);
            } else if( e.keySym() == VKeyCode::VK_SHIFT   ) {
                shapeList[0]->velo() += 0.1f;
            } else if( e.keySym() == VKeyCode::VK_ENTER ) {
                shapeList[0]->velo() = std::max(shapeList[0]->velo() - 0.1f, 0.0f);
            } else if( e.keySym() == VKeyCode::VK_RIGHT) {
                shapeList[0]->rotation().rotateByAngleY( M_PI / 50);
            } else if( e.keySym() == VKeyCode::VK_LEFT ) {
                shapeList[0]->rotation().rotateByAngleY(-M_PI / 50);
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
    : Primitives03(),
      m_kl(std::make_shared<MyKeyListener>(*this)) {  }

    bool init(const WindowSRef& win, const jau::fraction_timespec& when) override {
        if( !Primitives03::init(win, when) ) {
            return false;
        }
        win->addKeyListener(m_kl);
        return true;
    }
    void dispose(const WindowSRef& win, const jau::fraction_timespec& when) override {
        win->removeKeyListener(m_kl);
        Primitives03::dispose(win, when);
    }
};

int main(int argc, char *argv[]) // NOLINT(bugprone-exception-escape)
{
    return launch("Primitives03.cpp",
                  GLLaunchProps{.profile=GLProfile(GLProfile::GLES2),
                                .contextFlags=gamp::render::RenderContextFlags::verbose, // | gamp::render::RenderContextFlags::debug,
                                .requestedCaps=GLCapabilities()},
                  std::make_shared<Example>(), argc, argv);
}
