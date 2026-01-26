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
#ifndef JAU_GAMP_GRAPH_TESS_GL_GLUTESSELATOR_HPP_
#define JAU_GAMP_GRAPH_TESS_GL_GLUTESSELATOR_HPP_

#include <string_view>
#include <vector>

#include <GL/glutess2.h>

#include <jau/cpp_lang_util.hpp>
#include <jau/debug.hpp>
#include <jau/int_types.hpp>
#include <jau/string_util.hpp>

#include <gamp/GampTypes.hpp>
#include <gamp/graph/Outline.hpp>
#include <gamp/graph/OutlineShape.hpp>
#include <gamp/render/gl/data/GLArrayDataServer.hpp>

namespace gamp::graph::tess {

    using namespace jau::math;
    using namespace jau::math::geom;
    using namespace gamp::graph;
    using namespace gamp::render::gl::data;

    /** \addtogroup Gamp_Graph
     *
     *  @{
     */

    /**
     * GLUtilTesselator transform OutlineShapes to triangles using glutess2
     * - The triangle's vertices are added to a given GLFloatArrayDataServer
     * - If selected by flags, normals are added to the same GLFloatArrayDataServer (interleaved)
     * - The triangle (strip, fan or plain) segments are captured via GLUtilTesselator::Segment
     *   and returned by the GLUtilTesselator::tesselate() command
     * - Utilizes glutess2 in single precision float
     */
    class GLUtilTesselator {
      public:
        static constexpr int FLAG_NORMAL  = 1 << 0;
        static constexpr int FLAG_COLOR   = 1 << 1;
        static constexpr int FLAG_TEXTURE = 1 << 2;
        static constexpr int FLAG_VERBOSE = 1 << 3;

        struct Segment;
        typedef std::vector<Segment> SegmentList;

        struct Segment {
            /// render implementation specific triangle type, i.e. triangle-strip, -fan or just triangles.
            GLenum type;
            /// index of first element in data-sink for this segment
            GLint first;
            /// number of elements in data-sink for this segment
            GLsizei count;

            inline static std::string toString(const std::string& pre, const SegmentList& segments) noexcept {
                size_t i=0;
                std::string r;
                for(const Segment& s : segments ) {
                    r.append( jau::format_string_n(256, "%sSegment[%zu]: %s\n", pre.c_str(), i++, s.toString().c_str()) );
                }
                return r;
            }

            std::string toString() const {
                std::string_view type_s;
                switch( type ) {
                    case GL_TRIANGLE_STRIP:
                        type_s = "triangle_strip"; break;
                    case GL_TRIANGLE_FAN:
                        type_s = "triangle_fan"; break;
                    case GL_TRIANGLES:
                        type_s = "triangles"; break;
                    default:
                        type_s = "undefined";
                }
                return std::string("Segment[").append(type_s)
                    .append(", 1st ").append(std::to_string(first))
                    .append(", count ").append(std::to_string(count)).append("]");
            }
        };

      private:
        GLFloatArrayDataServer& m_array;
        SegmentList m_segments;
        std::vector<std::shared_ptr<Vertex>> m_vcache;
        Vec3f m_normal;
        uint32_t m_nextSegment = 0;
        uint32_t m_curSegment = 0;
        int m_flags;

      public:
        GLUtilTesselator(int flags, GLFloatArrayDataServer& array) noexcept
        : m_array(array), m_flags(flags)
        {
        }

        constexpr bool useNormal() const noexcept { return 0 != ( m_flags & FLAG_NORMAL ); }
        constexpr bool useColor() const noexcept { return 0 != ( m_flags & FLAG_COLOR ); }
        constexpr bool useTexture() const noexcept { return 0 != ( m_flags & FLAG_TEXTURE ); }
        constexpr bool verbose() const noexcept { return 0 != ( m_flags & FLAG_VERBOSE ); }

        /** Clears all internal data, not passed array or indices. */
        void clear() {
            m_segments.clear();
            m_nextSegment = 0;
            m_curSegment = 0;
            m_vcache.clear();
        }

        /// Returns true if segments() is empty
        bool empty() const noexcept { return m_segments.empty(); }

        jau::nsize_t elementCount() const noexcept { return m_array.elemCount(); }

        constexpr const std::vector<Segment>& segments() const noexcept { return m_segments; }

      private:

        static void cbBeginData( GLenum type, void *polygonData ) {
            GLUtilTesselator* os = reinterpret_cast<GLUtilTesselator*>(polygonData);
            os->m_curSegment = os->m_nextSegment++;
            Segment s{.type=type, .first=castOrThrow<size_t, GLint>(os->m_array.elemCount()), .count=0 };
            os->m_segments.push_back(s);
            if( os->verbose() ) {
                jau_INFO_PRINT("GLUtess begin %02u, type 0x%X, %s", os->m_curSegment, type, s.toString());
            }
        }
        static void cbVertexData( void *data, void *polygonData ) {
            GLUtilTesselator* os = reinterpret_cast<GLUtilTesselator*>(polygonData);
            Vertex* v = reinterpret_cast<Vertex*>(data);
            if( os->verbose() ) {
                jau_INFO_PRINT("GLUtess vertex %02u, %s", os->m_curSegment, v->coord().toString());
            }
            os->m_array.put3f(v->coord());
            if( os->useNormal() ) {
                os->m_array.put3f(os->m_normal);
            }
        }
        static void cbEndData( void *polygonData ) {
            GLUtilTesselator* os = reinterpret_cast<GLUtilTesselator*>(polygonData);
            Segment& s = os->m_segments.at(os->m_segments.size()-1);
            s.count = castOrThrow<size_t, GLsizei>(os->m_array.elemCount() - s.first);
            if( os->verbose() ) {
                jau_INFO_PRINT("GLUtess end %02u, %s", os->m_curSegment, s.toString());
            }
        }
        static void cbErrorData( GLenum errnum, void *polygonData ) {
            GLUtilTesselator* os = reinterpret_cast<GLUtilTesselator*>(polygonData);
            if( os->verbose() ) {
                // jau_INFO_PRINT("GLUtess error %02d, errnum 0x%X, %s", os->m_curIndex, errnum, gluErrorString(errnum));
                jau_INFO_PRINT("GLUtess error %02u, errnum 0x%X", os->m_curSegment, errnum);
            }
        }
        static void cbCombineData( GLUTessFloat coords[3], void *data[4],
                                   GLfloat weight[4], void **outData,
                                   void *polygonData ) {
            GLUtilTesselator* os = reinterpret_cast<GLUtilTesselator*>(polygonData);
            if( os->verbose() ) {
                jau_INFO_PRINT("GLUtess combine %02u, %f, %f, %f", os->m_curSegment, coords[0], coords[1], coords[2]);
                // jau_INFO_PRINT("GLUtess combine %p, %p, %p, %p", data[0], data[1], data[2], data[3]);
            }
            std::shared_ptr<Vertex> v = std::make_shared<Vertex>((float)coords[0], (float)coords[1], (float)coords[2], true);
            os->m_vcache.push_back(v);
            for (int i = 0; i < 4; ++i) {
                Vertex* s = (Vertex*) data[i];
                if( !s ) { break; }
                for (int j = 0; j < 3; ++j) {
                    // v->color()[j]    += (float)( weight[i] * s->color()[j] );
                    v->texCoord()[j] += (float)( weight[i] * s->texCoord()[j] );
                }
            }
            outData[0] = v.get();
        }

      public:
        static SegmentList tesselate(int flags, GLFloatArrayDataServer& array, OutlineShape& outlines) {
            GLUtilTesselator glutess(flags, array);
            return glutess.tesselate(outlines);
        }
        const SegmentList& tesselate(OutlineShape& outlines) {
            const bool odirty = outlines.verticesDirty() || outlines.trianglesDirty();
            size_t outlineCount = 0;
            if( odirty || empty() ) {
                clear();
                GLUtesselator* tess = gluNewTess();
                if( !tess ) { return m_segments; }
                m_normal = outlines.normal();
                if( verbose() ) {
                    jau_INFO_PRINT("GLUtess: normal: %s", m_normal.toString().c_str());
                }
                gluTessNormal(tess, m_normal.x, m_normal.y, m_normal.z);
                gluTessCallback(tess, GLU_TESS_BEGIN_DATA,   (_GLUfuncptr)cbBeginData);
                gluTessCallback(tess, GLU_TESS_END_DATA,     (_GLUfuncptr)cbEndData);
                gluTessCallback(tess, GLU_TESS_VERTEX_DATA,  (_GLUfuncptr)cbVertexData);
                gluTessCallback(tess, GLU_TESS_COMBINE_DATA, (_GLUfuncptr)cbCombineData);
                gluTessCallback(tess, GLU_TESS_ERROR_DATA,   (_GLUfuncptr)cbErrorData);
                {
                    gluTessBeginPolygon(tess, this);
                    for(const Outline& o : outlines.outlines()) {
                        if( verbose() ) {
                            jau_INFO_PRINT("GLUtess: outline %zu: Vertices %zu, Winding %s", outlineCount, o.vertexCount(), to_string(o.getWinding()).c_str());
                        }
                        ++outlineCount;
                        gluTessBeginContour(tess);
                        if constexpr ( sizeof(GLfloat) == sizeof(GLUTessFloat) ) {
                            for(const Vertex& v : o.vertices()) {
                                gluTessVertex(tess, const_cast<GLfloat*>(v.coord().cbegin()), (void*)&v);
                            }
                        } else {
                            for(const Vertex& v : o.vertices()) {
                                GLUTessFloat coords[] = { v.coord().x, v.coord().y, v.coord().z };
                                gluTessVertex(tess, coords, (void*)&v);
                            }
                        }
                        gluTessEndContour(tess);
                    }
                    gluTessEndPolygon(tess);
                }
                gluDeleteTess(tess);
                // FIXME outlines.markClean(OutlineShape::DirtyBits::vertices | OutlineShape::DirtyBits::triangles);
            }
            if( verbose() ) {
                jau_INFO_PRINT("GLUtess: outlines: %zu", outlineCount);
                jau_INFO_PRINT("GLUtess: segments: %zu", m_segments.size());
                jau_INFO_PRINT("\n%s", Segment::toString("- ", m_segments).c_str() );
                jau_INFO_PRINT("GLUtess: outline dirty: %d", odirty);
                jau_INFO_PRINT("GLUtess: index next: %u", m_nextSegment);
                jau_INFO_PRINT("GLUtess: vcache: %zu", m_vcache.size());
                jau_INFO_PRINT("GLUtess: vertices: %s", m_array.toString().c_str());
            }
            return m_segments;
        }
    };


    /**@}*/

} // namespace gamp::graph::tess

#endif /*  JAU_GAMP_GRAPH_TESS_GL_GLUTESSELATOR_HPP_ */
