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
    constexpr GLsizei arrayCompsPerElement() const noexcept { return usesNormal()? 3*3 : 2*3; }

    static constexpr std::string_view GLSL_PARAM_COMMENT_START = "\n// Gamp Graph Parameter Start\n";
    static constexpr std::string_view GLSL_PARAM_COMMENT_END = "// Gamp Graph Parameter End\n\n";
    static constexpr std::string_view GLSL_USE_COLOR_CHANNEL = "#define USE_COLOR_CHANNEL 1\n";
    static constexpr std::string_view GLSL_USE_NORMAL_CHANNEL = "#define USE_NORMAL_CHANNEL 1\n";
    static constexpr std::string_view GLSL_USE_LIGHT0 = "#define USE_LIGHT0 1\n";
    static constexpr std::string_view GLSL_USE_COLOR_TEXTURE = "#define USE_COLOR_TEXTURE 1\n";
    static constexpr std::string_view GLSL_USE_FRUSTUM_CLIPPING = "#define USE_FRUSTUM_CLIPPING 1\n";
    static constexpr std::string_view GLSL_USE_DISCARD = "#define USE_DISCARD 1\n";
    static constexpr std::string_view GLSL_DEF_SAMPLE_COUNT = "#define SAMPLE_COUNT ";
    static constexpr std::string_view GLSL_CONST_SAMPLE_COUNT = "const float sample_count = ";
    static constexpr std::string_view GLSL_MAIN_BEGIN = "void main (void)\n{\n";
    static constexpr std::string_view gcuTexture2D = "gcuTexture2D";
    static constexpr std::string_view colTexLookupFuncName = "texture2D";
    static constexpr std::string_view shader_basename = "curverenderer01";
    static constexpr std::string_view source_dir = "impl/graph/glsl";
    static constexpr std::string_view bin_dir = "impl/graph/glsl/bin";

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

        std::string vertexShaderName, fragmentShaderName;
        vertexShaderName.append(shader_basename);
        if( m_props.m_isTwoPass ) {
            vertexShaderName.append("-pass").append(m_props.m_pass1 ? "1":"2");
        } else {
            vertexShaderName.append("-single");
        }
        fragmentShaderName.append(shader_basename).append("-segment-head");

        ShaderCodeRef rsVp = ShaderCode::create(gl, GL_VERTEX_SHADER, source_dir, bin_dir, vertexShaderName);
        ShaderCodeRef rsFp = ShaderCode::create(gl, GL_FRAGMENT_SHADER, source_dir, bin_dir, fragmentShaderName);
        if( !rsVp || !rsFp ) {
            jau::fprintf_td(when.to_ms(), stdout, "ERROR %s:%d\n", E_FILE_LINE);
            return false;
        }
        {
            size_t posVp = rsVp->defaultShaderCustomization(gl);
            size_t posFp = rsFp->defaultShaderCustomization(gl);
            if( posVp == std::string::npos || posFp == std::string::npos ) {
                jau::fprintf_td(when.to_ms(), stdout, "ERROR %s:%d\n", E_FILE_LINE);
                return false;
            }

            // GLSL append from here on
            posFp = -1;

            posVp = rsVp->insertShaderSource(0, posVp, GLSL_PARAM_COMMENT_START);
            posFp = rsFp->insertShaderSource(0, posFp, GLSL_PARAM_COMMENT_START);

            // if( !gl.getContext().hasRendererQuirk(GLRendererQuirks.GLSLBuggyDiscard) ) {
            if( m_props.m_hasDiscard ) {
                posFp = rsFp->insertShaderSource(0, posFp, GLSL_USE_DISCARD);
            }

            if( m_props.m_hasFrustumClipping ) {
                posVp = rsVp->insertShaderSource(0, posVp, GLSL_USE_FRUSTUM_CLIPPING);
                posFp = rsFp->insertShaderSource(0, posFp, GLSL_USE_FRUSTUM_CLIPPING);
            }

            if( usesNormal() ) {
                posVp = rsVp->insertShaderSource(0, posVp, GLSL_USE_NORMAL_CHANNEL);
                posFp = rsFp->insertShaderSource(0, posFp, GLSL_USE_NORMAL_CHANNEL);
            }
            if( m_props.m_hasLight0 ) {
                posVp = rsVp->insertShaderSource(0, posVp, GLSL_USE_LIGHT0);
                posFp = rsFp->insertShaderSource(0, posFp, GLSL_USE_LIGHT0);
            }
            if( m_props.m_hasColorChannel ) {
                posVp = rsVp->insertShaderSource(0, posVp, GLSL_USE_COLOR_CHANNEL);
                posFp = rsFp->insertShaderSource(0, posFp, GLSL_USE_COLOR_CHANNEL);
            }
            if( m_props.m_hasColorTexture ) {
                        rsVp->insertShaderSource(0, posVp, GLSL_USE_COLOR_TEXTURE);
                posFp = rsFp->insertShaderSource(0, posFp, GLSL_USE_COLOR_TEXTURE);
            }
            /*if( !pass1 ) {
                posFp = rsFp->insertShaderSource(0, posFp, GLSL_DEF_SAMPLE_COUNT+sms.sampleCount+"\n");
                posFp = rsFp->insertShaderSource(0, posFp, GLSL_CONST_SAMPLE_COUNT+sms.sampleCount+".0;\n");
            } */

            posVp = rsVp->insertShaderSource(0, posVp, GLSL_PARAM_COMMENT_END);
            if( posVp == std::string::npos ) {
                jau::fprintf_td(when.to_ms(), stdout, "ERROR %s:%d\n", E_FILE_LINE);
                return false;
            }

            posFp = rsFp->insertShaderSource(0, posFp, GLSL_PARAM_COMMENT_END);

            posFp = rsFp->insertShaderSourceFile(0, posFp, string_t(source_dir).append("/constants.glsl"));
            if( posFp == std::string::npos ) {
                jau::fprintf_td(when.to_ms(), stdout, "ERROR %s:%d\n", E_FILE_LINE);
                return false;
            }
            posFp = rsFp->insertShaderSourceFile(0, posFp, string_t(source_dir).append("/uniforms.glsl"));
            if( posFp == std::string::npos ) {
                jau::fprintf_td(when.to_ms(), stdout, "ERROR %s:%d\n", E_FILE_LINE);
                return false;
            }
            posFp = rsFp->insertShaderSourceFile(0, posFp, string_t(source_dir).append("/varyings.glsl"));
            if( posFp == std::string::npos ) {
                jau::fprintf_td(when.to_ms(), stdout, "ERROR %s:%d\n", E_FILE_LINE);
                return false;
            }
            if( m_props.m_hasColorTexture || m_props.m_hasFrustumClipping ) {
                posFp = rsFp->insertShaderSourceFile(0, posFp, string_t(source_dir).append("/functions.glsl"));
                if( posFp == std::string::npos ) {
                    jau::fprintf_td(when.to_ms(), stdout, "ERROR %s:%d\n", E_FILE_LINE);
                    return false;
                }
            }
            /*if( hasColorTexture ) {
                posFp = rsFp->insertShaderSource(0, posFp, "uniform "+colorTexSeq.getTextureSampler2DType()+" "+UniformNames.gcu_ColorTexUnit+";\n");
                posFp = rsFp->insertShaderSource(0, posFp, colorTexSeq.getTextureLookupFragmentShaderImpl());
            }*/

            posFp = rsFp->insertShaderSource(0, posFp, GLSL_MAIN_BEGIN);

            std::string passS = m_props.m_pass1 ? "-pass1-" : "-pass2-";
            std::string shaderSegment = string_t(source_dir).append("/").append(shader_basename).append(passS).append("curve_simple").append(".glsl"); // sms.tech+sms.sub+".glsl";
            if( Graph::DEBUG_MODE || ShaderCode::DEBUG_CODE ) {
                jau::PLAIN_PRINT(true, "RegionRenderer.createShaderProgram.1: segment %s", shaderSegment.c_str());
            }
            posFp = rsFp->insertShaderSourceFile(0, posFp, shaderSegment);
            if( posFp == std::string::npos ) {
                jau::fprintf_td(when.to_ms(), stdout, "ERROR %s:%d\n", E_FILE_LINE);
                return false;
            }
            posFp = rsFp->insertShaderSource(0, posFp, "}\n");
            if( posFp == std::string::npos ) {
                jau::fprintf_td(when.to_ms(), stdout, "ERROR %s:%d\n", E_FILE_LINE);
                return false;
            }

            if( m_props.m_hasColorTexture ) {
                rsFp->replaceInShaderSource(std::string(gcuTexture2D), std::string(colTexLookupFuncName));
            }

        }
        ShaderProgramRef sp0 = ShaderProgram::create();
        if( !sp0->add(gl, rsVp, true) || !sp0->add(gl, rsFp, true) ) {
            jau::fprintf_td(when.to_ms(), stdout, "ERROR %s:%d\n", E_FILE_LINE);
            sp0->destroy(gl);
            return false;
        }
        m_st.attachShaderProgram(gl, sp0, true);

        PMVMat4f& pmv = m_pmvMat.pmv();
        pmv.getP().loadIdentity();
        pmv.getMv().loadIdentity();

        m_st.pushUniform(gl, m_pmvMat);
        m_st.pushUniform(gl, m_light0Pos);
        m_st.pushUniform(gl, m_staticColor);

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

    void updateColor(GL& gl) noexcept {
        m_st.pushUniform(gl, m_staticColor);
    }
    void updatePMV(GL& gl) noexcept {
        m_st.pushUniform(gl, m_pmvMat);
    }
    void updateAll(GL& gl) noexcept {
        m_st.pushUniform(gl, m_pmvMat);
        m_st.pushUniform(gl, m_staticColor);
    }
    PMVMat4f& pmv() noexcept { return m_pmvMat.pmv(); }
    const PMVMat4f& pmv() const noexcept { return m_pmvMat.pmv(); }
    const Vec4f& color() const noexcept { return m_staticColor.vec4f(); }
    void setColor(const Vec4f& c) noexcept { m_staticColor.vec4f()=c; }
    ShaderState& st() noexcept { return m_st; }
    const ShaderState& st() const noexcept { return m_st; }
};

class GraphRegion {
  public:
    typedef jau::darray<uint32_t, glmemsize_t> u32buffer_t;
  private:
    GraphRenderer& m_renderer;
    bool m_initialized;
    GLFloatArrayDataServerRef m_array;
    GLUIntArrayDataServerRef m_indices;
    int m_num_vertices, m_num_indices;

  public:
    GraphRegion(GraphRenderer& renderer)
    : m_renderer(renderer),
      m_initialized(m_renderer.initialized()),
      m_array(GLFloatArrayDataServer::createGLSLInterleaved(m_renderer.arrayCompsPerElement(), false, 256, GL_STATIC_DRAW)),
      m_indices(GLUIntArrayDataServer::createData(3, 256, GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER)),
      m_num_vertices(0), m_num_indices(0)
    {
        m_array->addGLSLSubArray("gca_Vertex", 3, GL_ARRAY_BUFFER);
        m_array->addGLSLSubArray("gca_CurveParam", 3, GL_ARRAY_BUFFER);
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
        m_indices->destroy(gl);
        m_num_vertices = 0;
        m_num_indices = 0;
    }

    void seal(GL& gl, bool seal_) {
        if( !m_initialized ) {
            return;
        }
        m_array->seal(gl, seal_);
        m_indices->seal(gl, seal_);
        m_array->enableBuffer(gl, false);
        m_indices->enableBuffer(gl, false);
    }

  private:
    void pushVertex(const Vertex& v, const Vec3f& normal) {
        // jau::PLAIN_PRINT(true, "pushVertex.0[%d]: v %s]", m_num_vertices, v.toString().c_str());
        m_array->put3f(v.coord());
        m_array->put3f(v.texCoord());
        if( m_renderer.usesNormal() ) {
            m_array->put3f(normal);
        }
        ++m_num_vertices;
    }
    void pushIndices(uint32_t i, uint32_t j, uint32_t k) {
        // jau::PLAIN_PRINT(true, "pushIndices.0[%d]: %u, %u, %u]", m_num_indices, i, j, k);
        m_indices->putN(i, j, k);
        m_num_indices += 3;
    }
    void pushNewVerticesIdx(const Vertex& vertIn1, const Vertex& vertIn2, const Vertex& vertIn3, const Vec3f& normal) {
        pushIndices(m_num_vertices, m_num_vertices+1, m_num_vertices+2);
        pushVertex(vertIn1, normal);
        pushVertex(vertIn2, normal);
        pushVertex(vertIn3, normal);
    }

  public:
    void addOutlineShape(OutlineShape& shape) {
        if( !m_initialized ) {
            return;
        }
        if( Graph::DEBUG_MODE ) {
            jau::PLAIN_PRINT(true, "add.0 num[vertices %d, indices %d]", m_num_vertices, m_num_indices);
            jau::PLAIN_PRINT(true, "add.0 array: %s", m_array->toString().c_str());
            jau::PLAIN_PRINT(true, "add.0 indices: %s", m_indices->toString().c_str());
        }
        const TriangleRefList& trisIn = shape.getTriangles();
        const VertexList& vertsIn = shape.getVertices();
        if( Graph::DEBUG_MODE ) {
            jau::PLAIN_PRINT(true, "add.0 triangles %u, vertices %u", trisIn.size(), vertsIn.size());
        }
        {
            glmemsize_t verticeCount = (glmemsize_t)vertsIn.size() + shape.addedVertexCount();
            glmemsize_t indexCount = (glmemsize_t)trisIn.size() * 3;
            m_array->growIfNeeded(verticeCount * m_array->compsPerElem());
            m_indices->growIfNeeded(indexCount * m_indices->compsPerElem());
        }
        uint32_t idxOffset = m_num_vertices;
        if( vertsIn.size() >= 3 ) {
            //
            // Processing Vertices
            //
            for(const Vertex& v : vertsIn) {
                pushVertex(v, shape.normal());
            }
            constexpr static uint32_t max_index = std::numeric_limits<uint32_t>::max() / sizeof(uint32_t);
            OutlineShape::size_type trisIn_sz = trisIn.size();
            for(OutlineShape::size_type i=0; i < trisIn_sz; ++i) {
                const TriangleRef& triIn = trisIn[i];
                // triEx.addVertexIndicesOffset(idxOffset);
                // triangles.add( triEx );
                Triangle::trivert_t& triInVertices = triIn->vertices();
                uint32_t tv0Idx = triInVertices[0].id();
                if ( max_index - idxOffset > tv0Idx ) {
                    // valid 'known' idx - move by offset
                    pushIndices(tv0Idx+idxOffset,
                                triInVertices[1].id()+idxOffset,
                                triInVertices[2].id()+idxOffset);
                } else {
                    // FIXME: If exceeding max_indices, we would need to generate a new buffer w/ indices
                    pushNewVerticesIdx(triInVertices[0], triInVertices[1], triInVertices[2], shape.normal());
                }
            }
        }
        if( Graph::DEBUG_MODE ) {
            jau::PLAIN_PRINT(true, "add.x num[vertices %d, indices %d]", m_num_vertices, m_num_indices);
            jau::PLAIN_PRINT(true, "add.x array: %s", m_array->toString().c_str());
            jau::PLAIN_PRINT(true, "add.x indices: %s", m_indices->toString().c_str());
        }
    }

    void draw(GL &gl) {
        if( !m_initialized ) {
            return;
        }
        m_renderer.useProgram(gl, true);

        m_array->enableBuffer(gl, true);
        m_indices->bindBuffer(gl, true); // keeps VBO binding

        ::glEnable(GL_BLEND);
        ::glBlendEquation(GL_FUNC_ADD); // default
        ::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        ::glDrawElements(GL_TRIANGLES, m_indices->elemCount() * m_indices->compsPerElem(), GL_UNSIGNED_INT, nullptr);

        m_indices->bindBuffer(gl, false);
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

class GraphShapes02 : public RenderListener {
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

  public:
    GraphShapes02()
    : RenderListener(RenderListener::Private()),
      m_initialized(false),
      m_renderer(m_st)
    {
    }

    Recti& viewport() noexcept { return m_viewport; }
    const Recti& viewport() const noexcept { return m_viewport; }
	std::vector<ShapeRef>& shapes() noexcept { return m_shapes; }

    bool animating() const noexcept { return m_animating; }
    bool& animating() noexcept { return m_animating; }
    void setOneFrame() noexcept { m_animating=false; m_oneframe=true; }

    bool init(const WindowRef& win, const jau::fraction_timespec& when) override {
        jau::fprintf_td(when.to_ms(), stdout, "RL::init: %s\n", toString().c_str());
        m_tlast = when;

        GL& gl = GL::downcast(win->renderContext());

        if( !m_renderer.init(gl, when) ) {
            jau::fprintf_td(when.to_ms(), stdout, "ERROR %s:%d: %s\n", E_FILE_LINE, toString().c_str());
            win->dispose(when);
            return false;
        }
        ShapeRef cobraMkIII_Shape = Shape::createShared(m_renderer);
        models::appendCobraMkIII(cobraMkIII_Shape->outlineShapes());
        cobraMkIII_Shape->setColor(Vec4f(0.05f, 0.05f, 0.5f, 1.0f));
        cobraMkIII_Shape->update(gl);
        m_shapes.push_back(cobraMkIII_Shape);

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

    void dispose(const WindowRef& win, const jau::fraction_timespec& when) override {
        GL& gl = GL::downcast(win->renderContext());
        jau::fprintf_td(when.to_ms(), stdout, "RL::dispose: %s\n", toString().c_str());
        for(ShapeRef& s : m_shapes) {
            s->destroy(gl);
        }
        m_renderer.destroy(gl);
        m_st.destroy(GL::downcast(win->renderContext()));
        m_initialized = false;
    }

    void reshape(const WindowRef& win, const jau::math::Recti& viewport, const jau::fraction_timespec& when) override {
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

    void display(const WindowRef& win, const jau::fraction_timespec& when) override {
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

class Example : public GraphShapes02 {
  private:
    class MyKeyListener : public KeyListener {
      private:
        GraphShapes02& m_parent;
      public:
        MyKeyListener(GraphShapes02& p) : m_parent(p) {}

        void keyPressed(KeyEvent& e, const KeyboardTracker& kt) override {
            jau::fprintf_td(e.when().to_ms(), stdout, "KeyPressed: %s; keys %zu\n", e.toString().c_str(), kt.pressedKeyCodes().count());
            std::vector<ShapeRef>& shapeList = m_parent.shapes();
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
    : GraphShapes02(),
      m_kl(std::make_shared<MyKeyListener>(*this)) {  }

    bool init(const WindowRef& win, const jau::fraction_timespec& when) override {
        if( !GraphShapes02::init(win, when) ) {
            return false;
        }
        win->addKeyListener(m_kl);
        return true;
    }
    void dispose(const WindowRef& win, const jau::fraction_timespec& when) override {
        win->removeKeyListener(m_kl);
        GraphShapes02::dispose(win, when);
    }
};

int main(int argc, char *argv[]) // NOLINT(bugprone-exception-escape)
{
    return launch("GraphShapes02.cpp",
                  GLLaunchProps{.profile=GLProfile(GLProfile::GLES2),
                                .contextFlags=gamp::render::RenderContextFlags::verbose}, // | gamp::render::RenderContextFlags::debug},
                  std::make_shared<Example>(), argc, argv);
}
