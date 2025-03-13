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
#ifndef JAU_GAMP_GRAPH_OUTLINE_HPP_
#define JAU_GAMP_GRAPH_OUTLINE_HPP_

#include <limits>
#include <jau/darray.hpp>
#include <jau/math/geom/geom.hpp>
#include <jau/math/vec3f.hpp>
#include <jau/math/geom/geom3f.hpp>
#include <jau/math/geom/aabbox3f.hpp>
#include <jau/math/geom/plane/affine_transform.hpp>

namespace gamp::graph {

    using namespace jau::math;
    using jau::math::geom::Winding;
    using jau::math::geom::AABBox3f;
    using jau::math::geom::plane::AffineTransform;

    /** @defgroup Gamp_Graph Gamp Graph
     *  Basic Graph Framework
     *
     *  @{
     */

    // typedef Vec3f Vertex;
    class Vertex {
      private:
        int m_id;
        Vec3f m_coord;
        Vec3f m_color;
        Vec3f m_texCoord;
        bool m_onCurve;

      public:
        constexpr Vertex() noexcept
        : m_id(std::numeric_limits<int>::max()),
          m_coord(), m_color(), m_texCoord(), m_onCurve(true) {}

        constexpr Vertex(const float x_, const float y_, const float z_, bool onCurve) noexcept
        : m_id(std::numeric_limits<int>::max()),
          m_coord(x_, y_, z_), m_color(), m_texCoord(), m_onCurve(onCurve) {}

        constexpr Vertex(const float x_, const float y_, bool onCurve) noexcept
        : m_id(std::numeric_limits<int>::max()),
          m_coord(x_, y_, 0), m_color(), m_texCoord(), m_onCurve(onCurve) {}

        constexpr int id() const noexcept { return m_id; }
        constexpr int& id() noexcept { return m_id; }

        constexpr const Vec3f& coord() const noexcept { return m_coord; }
        constexpr Vec3f& coord() noexcept { return m_coord; }

        constexpr const Vec3f& color() const noexcept { return m_color; }
        constexpr Vec3f& color() noexcept { return m_color; }

        constexpr const Vec3f& texCoord() const noexcept { return m_texCoord; }
        constexpr Vec3f& texCoord() noexcept { return m_texCoord; }

        constexpr bool onCurve() const noexcept { return m_onCurve; }
        constexpr bool& onCurve() noexcept { return m_onCurve; }

        std::string toString() const noexcept {
            return std::string("Vert[")
                .append(std::to_string(m_id))
                .append(", onCurve ").append(std::to_string(m_onCurve))
                .append(", p ").append(m_coord.toString())
                .append("]");
        }
    };

    typedef jau::darray<Vertex> VertexList;

    /**
     * Computes the area of a list of vertices via shoelace formula.
     *
     * This method is utilized e.g. to reliably compute the {@link Winding} of complex shapes.
     *
     * Implementation uses double precision.
     *
     * @param vertices
     * @return positive area if ccw else negative area value
     * @see #getWinding()
     */
    constexpr double area2D(const VertexList& vertices) noexcept {
        size_t n = vertices.size();
        double area = 0.0;
        for (size_t p = n - 1, q = 0; q < n; p = q++) {
            const Vec3f& pCoord = vertices[p].coord();
            const Vec3f& qCoord = vertices[q].coord();
            area += (double)pCoord.x * (double)qCoord.y - (double)qCoord.x * (double)pCoord.y;
        }
        return area;
    }

    /**
     * Compute the winding using the area2D() function over all vertices for complex shapes.
     *
     * Uses the {@link #area(List)} function over all points
     * on complex shapes for a reliable result!
     *
     * Implementation uses double precision.
     *
     * @param vertices array of Vertices
     * @return Winding::CCW or Winding::CLW
     * @see area2D()
     */
    constexpr Winding getWinding(const VertexList& vertices) noexcept {
        return area2D(vertices) >= 0 ? Winding::CCW : Winding::CW ;
    }

    class Outline {
      private:
        static constexpr Vertex zeroVec = Vertex();
        static constexpr int DIRTY_BOUNDS = 1 << 0;
        static constexpr int DIRTY_WINDING = 1 << 1;
        static constexpr int DIRTY_COMPLEXSHAPE = 1 << 2;

        VertexList m_vertices;
        bool m_closed;
        mutable AABBox3f m_bbox;
        mutable Winding m_winding;
        // bool m_complexShape;
        mutable int m_dirtyBits;

      public:
        constexpr Outline() : Outline(3) {}

        constexpr Outline(size_t vertCapacity)
        : m_vertices(vertCapacity),
          m_closed(false), m_bbox(),
          m_winding(Winding::CCW),
          // m_complexShape(false),
          m_dirtyBits(0)
        {}

        constexpr void reserve(size_t newVertCapacity) { m_vertices.reserve(newVertCapacity); }

        constexpr bool empty() const noexcept { return m_vertices.empty(); }

        constexpr size_t vertexCount() const noexcept { return m_vertices.size(); }

        constexpr const VertexList& vertices() const noexcept { return m_vertices; }
        constexpr VertexList& vertices() noexcept { return m_vertices; }

        constexpr const Vertex& vertex(size_t i) const noexcept { return m_vertices[i]; }
        constexpr Vertex& vertex(size_t i) noexcept { return m_vertices[i]; }

        const Vertex& lastVertex() const noexcept {
            if( empty() ) {
                return zeroVec;
            }
            return m_vertices[m_vertices.size()-1];
        }

        constexpr bool isClosed() const noexcept { return m_closed; }

        /**
         * Ensure this outline is closed.
         * <p>
         * Checks whether the last vertex equals to the first.
         * If not equal, it either appends a copy of the first vertex
         * or prepends a copy of the last vertex, depending on <code>closeTail</code>.
         * </p>
         * @param closeTail if true, a copy of the first vertex will be appended,
         *                  otherwise a copy of the last vertex will be prepended.
         * @return true if closing performed, otherwise false for NOP
         */
        bool setClosed(bool closeTail) {
            m_closed = true;
            if( !empty() ) {
                const Vertex& first = vertex(0);
                const Vertex& last = lastVertex();
                if( first.coord() != last.coord() ) {
                    if( closeTail ) {
                        m_vertices.push_back(first);
                    } else {
                        m_vertices.insert(m_vertices.begin(), last);
                    }
                    return true;
                }
            }
            return false;
        }

        /**
         * Returns the cached or computed winding of this {@link Outline}s {@code polyline} using {@link VectorUtil#area(ArrayList)}.
         * <p>
         * The result is cached.
         * </p>
         * @return {@link Winding#CCW} or {@link Winding#CW}
         */
        Winding getWinding() const noexcept {
            if( 0 == ( m_dirtyBits & DIRTY_WINDING ) ) {
                return m_winding;
            }
            size_t count = m_vertices.size();
            if( 3 > count ) {
                m_winding = Winding::CCW;
            } else {
                m_winding = gamp::graph::getWinding( m_vertices );
            }
            m_dirtyBits &= ~DIRTY_WINDING;
            return m_winding;
        }

        /**
         * Sets Winding to this outline
         *
         * If the enforced Winding doesn't match this Outline, the vertices are reversed.
         *
         * @param enforce to be enforced {@link Winding}
         */
        void setWinding(Winding enforce) {
            Winding had_winding = getWinding();
            if( enforce != had_winding ) {
                size_t count = m_vertices.size();
                VertexList ccw(count);
                for(size_t i=count; i-- > 0; ) {
                    ccw.push_back(m_vertices[i]);
                }
                m_vertices = ccw;
                m_winding = enforce;
            }
        }

      private:
        void validateBoundingBox() const noexcept {
            m_dirtyBits &= ~DIRTY_BOUNDS;
            m_bbox.reset();
            for(const Vertex& v : m_vertices) {
                m_bbox.resize(v.coord());
            }
        }

      public:
        const AABBox3f& bounds() const noexcept {
            if ( 0 != ( m_dirtyBits & DIRTY_BOUNDS ) ) {
                validateBoundingBox();
            }
            return m_bbox;
        }

        /**
         * Appends a vertex to the outline loop/strip.
         * @param vertex Vertex to be added
         * @throws NullPointerException if the  {@link Vertex} element is null
         */
        void addVertex(const Vertex& vertex) {
            m_vertices.push_back(vertex);
            if ( 0 == ( m_dirtyBits & DIRTY_BOUNDS ) ) {
                m_bbox.resize(vertex.coord());
            }
            m_dirtyBits |= DIRTY_WINDING | DIRTY_COMPLEXSHAPE;
        }

        /**
         * Insert the {@link Vertex} element at the given {@code position} to the outline loop/strip.
         * @param position of the added Vertex
         * @param vertex Vertex object to be added
         * @throws NullPointerException if the  {@link Vertex} element is null
         * @throws IndexOutOfBoundsException if position is out of range (position < 0 || position > getVertexNumber())
         */
        void addVertex(size_t position, const Vertex& vertex) {
            m_vertices.insert(position, vertex);
            if ( 0 == ( m_dirtyBits & DIRTY_BOUNDS ) ) {
                m_bbox.resize(vertex.coord());
            }
            m_dirtyBits |= DIRTY_WINDING | DIRTY_COMPLEXSHAPE;
        }

        /**
         * Return a transformed instance with all vertices are copied and transformed.
         */
        Outline transform(const AffineTransform& t) const {
            Outline newOutline;
            const size_t vsize = m_vertices.size();
            for(size_t i=0; i<vsize; ++i) {
                const Vertex& v = m_vertices[i];
                Vertex w(v);
                t.transform(v.coord(), w.coord());
                newOutline.addVertex(w);
            }
            newOutline.m_closed = m_closed;
            return newOutline;
        }

    };
    typedef jau::darray<Outline> OutlineList;


    /**@}*/

} // namespace gamp::graph

#endif /*  JAU_GAMP_GRAPH_OUTLINE_HPP_ */
