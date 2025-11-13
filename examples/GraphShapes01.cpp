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
#include <gamp/graph/Graph.hpp>
#include <gamp/graph/PrimTypes.hpp>
#include <gamp/render/gl/data/GLBuffers.hpp>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
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

#include "../demos/graph/testshapes/Glyph05FreeSerifBoldItalic_ae.hpp"
#include "../demos/graph/testshapes/Glyph03FreeMonoRegular_M.hpp"

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
    GraphRendererProps m_props;
    ShaderState& m_st;
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
      m_initialized(false)
    { }

    constexpr bool initialized() const noexcept { return m_initialized; }

    bool init(GL& gl, const jau::fraction_timespec& when) {
        ShaderCode::DEBUG_CODE = true;

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

        m_st.pushAllUniforms(gl);

        m_initialized = sp0->inUse();
        if( !m_initialized ) {
            jau::fprintf_td(when.to_ms(), stdout, "ERROR %s:%d\n", E_FILE_LINE);
            m_st.destroy(gl);
        }
        return m_initialized;
    }

    void useProgram(GL& gl, bool on) {
        m_st.useProgram(gl, on);
    }

};

class GraphRegion {
  public:
    typedef jau::darray<uint32_t, glmemsize_t> u32buffer_t;
  private:
    GraphRenderer& m_renderer;
    ShaderState& m_st;
    bool m_initialized;
    GLFloatArrayDataServerRef m_array;
    GLUIntArrayDataServerRef m_indices;
    int m_num_vertices, m_num_indices;

  public:
    GraphRegion(GraphRenderer& renderer, ShaderState& st)
    : m_renderer(renderer), m_st(st),
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
        m_st.ownAttribute(m_array, true);
        // m_st.ownAttribute(m_indices, true);
    }

    constexpr bool initialized() const noexcept { return m_initialized; }

    void seal(GL& gl, bool seal_) {
        if( !m_initialized ) {
            return;
        }
        m_array->seal(gl, seal_);
        m_indices->seal(gl, seal_);
        m_array->enableBuffer(gl, false);
        m_indices->enableBuffer(gl, false);
    }

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

    void addOutlineShape(OutlineShape& shape) {
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
    ShaderState& m_st;
    GLUniformSyncPMVMat4fRef m_pmvMatUni;
    OutlineShape m_oshape;

    Vec3f m_position;
    Quat4f m_rotation;
    Vec3f m_rotPivot;
    Vec3f m_scale = Vec3f(1, 1, 1);
    float m_zOffset;
    GraphRenderer& m_renderer;
    GraphRegion m_region;
    GLUniformVec4fRef m_uColor;

    Mat4f iMat;
    Mat4f tmpMat;
    bool iMatIdent = true;
    bool iMatDirty = false;

    struct Private{ explicit Private() = default; };

  public:
    Shape(Private, ShaderState &st, GLUniformSyncPMVMat4fRef pmvMatU, GraphRenderer& renderer)
    : m_st(st), m_pmvMatUni(std::move(pmvMatU)), m_oshape(3, 16),
      m_renderer(renderer), m_region(m_renderer, m_st)
    {
        m_uColor = GLUniformVec4f::create("gcu_StaticColor", Vec4f(0, 0, 0, 1));
        m_st.ownUniform(m_uColor, true);
    }

    static ShapeRef create(ShaderState &st, GLUniformSyncPMVMat4fRef pmvMatU, GraphRenderer& renderer) {
        return std::make_shared<Shape>(Private(), st, std::move(pmvMatU), renderer);
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

    void update(GL& gl) {
        m_region.addOutlineShape(m_oshape);
        m_region.seal(gl, true);
    }

    void draw(GL &gl) {
        PMVMat4f& pmv = m_pmvMatUni->pmv();
        pmv.pushMv();
        applyMatToMv(pmv);
        m_st.pushUniform(gl, m_pmvMatUni); // automatic sync + update of Mvi + Mvit

        m_st.pushUniform(gl, m_uColor);
        m_region.draw(gl);
        pmv.popMv();
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

class GraphShapes01 : public RenderListener {
  private:
    constexpr static float zNear=  1.0f;
    constexpr static float zFar =100.0f;
    constexpr static PMVData mat_req = PMVData::inv_proj | PMVData::inv_mv | PMVData::inv_tps_mv;

    ShaderState m_st;
    Recti m_viewport;
    bool m_initialized;
    bool m_animating = true;
    bool m_oneframe = false;
    jau::fraction_timespec m_tlast;
    GLUniformSyncPMVMat4fRef m_pmvMatUni;
    GraphRenderer m_renderer;
    std::vector<ShapeRef> m_shapes;
    const jau::math::Vec3f lightPos = jau::math::Vec3f(0.0f, 5.0f, 10.0f);

  public:
    GraphShapes01()
    : RenderListener(RenderListener::Private()),
      m_initialized(false),
      m_pmvMatUni(GLUniformSyncPMVMat4f::create("gcu_PMVMatrix", mat_req)), // P, Mv, Mvi and Mvit
      m_renderer(m_st)
    {
    }

    Recti& viewport() noexcept { return m_viewport; }
    const Recti& viewport() const noexcept { return m_viewport; }

    PMVMat4f& pmv() noexcept { return m_pmvMatUni->pmv(); }
    const PMVMat4f& pmv() const noexcept { return m_pmvMatUni->pmv(); }
    bool animating() const noexcept { return m_animating; }
    bool& animating() noexcept { return m_animating; }
    void setOneFrame() noexcept { m_animating=false; m_oneframe=true; }

    bool init(const WindowRef& win, const jau::fraction_timespec& when) override {
        jau::fprintf_td(when.to_ms(), stdout, "RL::init: %s\n", toString().c_str());
        m_tlast = when;

        GL& gl = GL::downcast(win->renderContext());

        // setup mgl_PMVMatrix
        PMVMat4f& pmv = m_pmvMatUni->pmv();
        pmv.getP().loadIdentity();
        pmv.getMv().loadIdentity();
        m_st.ownUniform(m_pmvMatUni, true);

        if( !m_renderer.init(gl, when) ) {
            jau::fprintf_td(when.to_ms(), stdout, "ERROR %s:%d: %s\n", E_FILE_LINE, toString().c_str());
            win->dispose(when);
            return false;
        }

        GLUniformVec3fRef lightU = GLUniformVec3f::create("gcu_Light0Pos", lightPos);
        m_st.ownUniform(lightU, true);
        m_st.pushAllUniforms(gl);

        const float lineWidth = 1/2.5f;
        const float dz = 0.005f;
        if( true ) {
            // Cross / Plus
            const float width = 1.5f;
            const float height = 1.5f;

            float lwh = lineWidth/2.0f;

            float twh = width/2.0f;
            float thh = height/2.0f;

            float ctrX = 0, ctrY = 0, ctrZ = dz;
            ShapeRef frontShape = Shape::create(m_st, m_pmvMatUni, m_renderer);
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
            frontShape->update(gl);
            frontShape->setColor(Vec4f(0.5f, 0.05f, 0.05f, 1));
            frontShape->position().x = -2.0f;

            ShapeRef backShape = Shape::create(m_st, m_pmvMatUni, m_renderer);
            m_shapes.push_back(backShape);
            backShape->outlines() = oshape.flipFace(); // -dz);
            backShape->outlines().clearCache();
            backShape->update(gl);
            backShape->setColor(Vec4f(0.2f, 0.5f, 0.2f, 1));
            backShape->position().x = -2.0f;
        }
        if( true) {
            ShapeRef frontShape = Shape::create(m_st, m_pmvMatUni, m_renderer);
            m_shapes.push_back(frontShape);
            OutlineShape& oshape = frontShape->outlines();
            // Outer boundary-shapes are required as Winding::CCW (is CCW)
            oshape.moveTo(0.0f,-10.0f, 0);
            oshape.lineTo(15.0f,-10.0f, 0);
            oshape.quadTo(10.0f,5.0f,0, 15.0f,10.0f,0);
            oshape.cubicTo(6.0f,15.0f,0, 5.0f,8.0f,0, 0.0f,10.0f,0);
            oshape.closePath();
            // Inner hole-shapes should be Winding::CW (is CCW and will be fixed to CW below)
            oshape.moveTo(5.0f,-5.0f,0);
            oshape.quadTo(10.0f,-5.0f,0, 10.0f,0.0f,0);
            oshape.quadTo(5.0f,0.0f,0, 5.0f,-5.0f,0);
            oshape.closePath();
            {
                const Winding w10 = oshape.outlines()[0].getWinding();
                const Winding w11 = oshape.outlines()[1].getWinding();
                oshape.outlines()[0].setWinding(Winding::CCW);
                oshape.outlines()[1].setWinding(Winding::CW);
                jau::PLAIN_PRINT(true, "Special.frontShape.10.winding_area: %s -> %s",
                    to_string(w10).c_str(), to_string(oshape.outlines()[0].getWinding()).c_str());
                jau::PLAIN_PRINT(true, "Special.frontShape.11.winding_area: %s -> %s",
                    to_string(w11).c_str(), to_string(oshape.outlines()[1].getWinding()).c_str());
            }

            frontShape->update(gl);
            frontShape->setColor(Vec4f(0.4f, 0.4f, 0.1f, 1));
            frontShape->position().x = -1.0f;
            frontShape->scale().x *= 0.1f;
            frontShape->scale().y *= 0.1f;

            ShapeRef backShape = Shape::create(m_st, m_pmvMatUni, m_renderer);
            m_shapes.push_back(backShape);
            backShape->outlines() = oshape.flipFace(-dz); // winding preserved and is correct
            backShape->outlines().clearCache();
            backShape->update(gl);
            backShape->setColor(Vec4f(0.2f, 0.2f, 0.5f, 1));
            backShape->position().x = -1.0f;
            backShape->scale().x *= 0.1f;
            backShape->scale().y *= 0.1f;
            jau::PLAIN_PRINT(true, "Special.backShape.10.winding_area: %s",
                to_string(backShape->outlines().outlines()[0].getWinding()).c_str());
            jau::PLAIN_PRINT(true, "Special.backShape.11.winding_area: %s",
                to_string(backShape->outlines().outlines()[1].getWinding()).c_str());
        }
        if ( true ) {
            ShapeRef frontShape = Shape::create(m_st, m_pmvMatUni, m_renderer);
            m_shapes.push_back(frontShape);
            OutlineShape& oshape = frontShape->outlines();
            Glyph05FreeSerifBoldItalic_ae::addShapeToRegion(oshape);
            frontShape->update(gl);
            frontShape->setColor(Vec4f(0.05f, 0.05f, 0.5f, 1));
            frontShape->position().x =  1.5f;
            frontShape->position().y = -1.0f;
            frontShape->scale().x *= 2.0f;
            frontShape->scale().y *= 2.0f;

            ShapeRef backShape = Shape::create(m_st, m_pmvMatUni, m_renderer);
            m_shapes.push_back(backShape);
            backShape->outlines() = oshape.flipFace(-dz);
            backShape->outlines().clearCache();
            backShape->update(gl);
            backShape->setColor(Vec4f(0.4f, 0.4f, 0.1f, 1));
            backShape->position().x =  1.5f;
            backShape->position().y = -1.0f;
            backShape->scale().x *= 2.0f;
            backShape->scale().y *= 2.0f;
        }
        if ( true ) {
            ShapeRef frontShape = Shape::create(m_st, m_pmvMatUni, m_renderer);
            m_shapes.push_back(frontShape);
            OutlineShape& oshape = frontShape->outlines();
            Glyph03FreeMonoRegular_M::addShapeToRegion(oshape);
            frontShape->update(gl);
            frontShape->setColor(Vec4f(0.05f, 0.5f, 0.05f, 1));
            frontShape->position().x = 1.5f;
            frontShape->position().y = 0.5f;
            frontShape->scale().x *= 2.0f;
            frontShape->scale().y *= 2.0f;

            ShapeRef backShape = Shape::create(m_st, m_pmvMatUni, m_renderer);
            m_shapes.push_back(backShape);
            backShape->outlines() = oshape.flipFace(-dz);
            backShape->outlines().clearCache();
            backShape->update(gl);
            backShape->setColor(Vec4f(0.5f, 0.1f, 0.1f, 1));
            backShape->position().x =  1.5f;
            backShape->position().y =  0.5f;
            backShape->scale().x *= 2.0f;
            backShape->scale().y *= 2.0f;
        }

        ::glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
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
        jau::fprintf_td(when.to_ms(), stdout, "RL::dispose: %s\n", toString().c_str());
        m_st.destroy(GL::downcast(win->renderContext()));
        m_initialized = false;
    }

    void reshape(const WindowRef& win, const jau::math::Recti& viewport, const jau::fraction_timespec& when) override {
        GL& gl = GL::downcast(win->renderContext());
        jau::fprintf_td(when.to_ms(), stdout, "RL::reshape: %s\n", toString().c_str());
        m_viewport = viewport;

        PMVMat4f& pmv = m_pmvMatUni->pmv();
        pmv.getP().loadIdentity();
        const float aspect = 1.0f;
        const float fovy_deg=45.0f;
        const float aspect2 = ( (float) m_viewport.width() / (float) m_viewport.height() ) / aspect;
        pmv.perspectiveP(jau::adeg_to_rad(fovy_deg), aspect2, zNear, zFar);
        m_st.useProgram(gl, true);
        m_st.pushUniform(gl, m_pmvMatUni); // automatic sync + update of Mvi + Mvit
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
        PMVMat4f& pmv = m_pmvMatUni->pmv();
        pmv.getMv().loadIdentity();
        pmv.translateMv(0, 0, -5);

        for(const ShapeRef& s : m_shapes) {
            if( animating() || m_oneframe ) {
                constexpr double angle_per_sec = 30;
                const float rad = (float) ( (when - m_tlast).to_double() * angle_per_sec );
                s->rotation().rotateByAngleY(jau::adeg_to_rad(  rad ));
            }
            s->draw(gl);
        }

        m_oneframe = false;
        // m_st.useProgram(gl, false);

        m_tlast = when;
    }

    std::string toStringImpl() const noexcept override { return "GraphShapes01"; }
};

class Example : public GraphShapes01 {
  private:
    class MyKeyListener : public KeyListener {
      private:
        GraphShapes01& m_parent;
      public:
        MyKeyListener(GraphShapes01& p) : m_parent(p) {}

        void keyPressed(KeyEvent& e, const KeyboardTracker& kt) override {
            jau::fprintf_td(e.when().to_ms(), stdout, "KeyPressed: %s; keys %zu\n", e.toString().c_str(), kt.pressedKeyCodes().count());
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
            jau::fprintf_td(e.when().to_ms(), stdout, "KeyRelease: %s; keys %zu\n", e.toString().c_str(), kt.pressedKeyCodes().count());
        }
    };
    typedef std::shared_ptr<MyKeyListener> MyKeyListenerRef;
    MyKeyListenerRef m_kl;

  public:
    Example()
    : GraphShapes01(),
      m_kl(std::make_shared<MyKeyListener>(*this)) {  }

    bool init(const WindowRef& win, const jau::fraction_timespec& when) override {
        if( !GraphShapes01::init(win, when) ) {
            return false;
        }
        win->addKeyListener(m_kl);
        return true;
    }
    void dispose(const WindowRef& win, const jau::fraction_timespec& when) override {
        win->removeKeyListener(m_kl);
        GraphShapes01::dispose(win, when);
    }
};

int main(int argc, char *argv[]) // NOLINT(bugprone-exception-escape)
{
    return launch("GraphShapes01.cpp",
                  GLLaunchProps{.profile=GLProfile(GLProfile::GLES2),
                                .contextFlags=gamp::render::RenderContextFlags::verbose}, // | gamp::render::RenderContextFlags::debug},
                  std::make_shared<Example>(), argc, argv);
}
