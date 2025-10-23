/*
 * Author: Sven Gothel <sgothel@jausoft.com> (C++, Java) and Rami Santina (Java)
 * Copyright Gothel Software e.K. and the authors
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

#include <jau/basic_types.hpp>
#include <jau/darray.hpp>
#include <jau/int_math.hpp>
#include <jau/int_types.hpp>
#include <jau/math/vec3f.hpp>
#include <jau/math/geom/geom.hpp>
#include <jau/math/geom/geom3f.hpp>
#include <jau/math/geom/aabbox3f.hpp>
#include <jau/math/geom/plane/affine_transform.hpp>

#include <gamp/graph/PrimTypes.hpp>

namespace gamp::graph {

    using namespace jau::enums;
    using namespace jau::math;
    using jau::math::geom::Winding;
    using jau::math::geom::AABBox3f;
    using jau::math::geom::plane::AffineTransform;

    /** \addtogroup Gamp_Graph
     *
     *  @{
     */

    /** Define a single continuous stroke by control vertices.
     *  The vertices define the shape of the region defined by this
     *  outline. The Outline can contain a list of off-curve and on-curve
     *  vertices which define curved regions.
     *
     *  Note: An outline should be closed to be rendered as a region.
     *
     *  @see OutlineShape
     *  @see Region
     */
    class Outline {
      private:
        static constexpr Vertex zeroVec = Vertex();

        enum class DirtyBits : uint16_t {
            none = 0,
            bounds = 1 << 0,
            winding = 1 << 1,
            complexShape = 1 << 2
        };
        JAU_MAKE_BITFIELD_ENUM_STRING_MEMBER(DirtyBits, bounds, winding, complexShape);

        VertexList m_vertices;
        bool m_closed;
        mutable AABBox3f m_bbox;
        mutable Winding m_winding;
        mutable DirtyBits m_dirtyBits;
        mutable bool m_complexShape;

      public:
        typedef uint32_t size_type;
        /// byte-size uint32_t limit: 1'073'741'823 (FIXME: Adjust to actual type, i.e. Vertex = 2x Vec3f?)
        constexpr static size_type max_elements = std::numeric_limits<uint32_t>::max() / sizeof(uint32_t);

        constexpr Outline() : Outline(3) {}

        constexpr Outline(size_type vertCapacity)
        : m_vertices(vertCapacity),
          m_closed(false), m_bbox(),
          m_winding(Winding::CCW),
          m_dirtyBits(DirtyBits::none),
          m_complexShape(false)
        {}

        constexpr void reserve(size_type newVertCapacity) { m_vertices.reserve(newVertCapacity); }

        constexpr bool empty() const noexcept { return m_vertices.empty(); }

        constexpr size_type vertexCount() const noexcept { return m_vertices.size(); }

        constexpr const VertexList& vertices() const noexcept { return m_vertices; }
        constexpr VertexList& vertices() noexcept { return m_vertices; }

        constexpr const Vertex& vertex(size_type i) const noexcept { return m_vertices[i]; }
        constexpr Vertex& vertex(size_type i) noexcept { return m_vertices[i]; }

        /**
         * Removes the {@link Vertex} element at the given {@code position}.
         *
         * Sets the bounding box dirty, hence a next call to bounds() will validate it.
         *
         * @param position of the to be removed Vertex
         */
        void removeVertex(size_type position) {
            if( position < m_vertices.size() ) {
                m_dirtyBits |= DirtyBits::bounds | DirtyBits::winding | DirtyBits::complexShape;
                m_vertices.erase(position);
            }
        }

        bool isEmpty() const noexcept {
            return m_vertices.size() == 0;
        }

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
            if( !is_set(m_dirtyBits, DirtyBits::winding) ) {
                return m_winding;
            }
            size_type count = m_vertices.size();
            if( 3 > count ) {
                m_winding = Winding::CCW;
            } else {
                m_winding = computeWinding();
            }
            m_dirtyBits &= ~DirtyBits::winding;
            return m_winding;
        }

      private:
        Winding computeWinding() const noexcept;

      public:
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
                size_type count = m_vertices.size();
                VertexList ccw(count);
                for(size_type i=count; i-- > 0; ) {
                    ccw.push_back(m_vertices[i]);
                }
                m_vertices = ccw;
                m_winding = enforce;
                m_dirtyBits &= ~DirtyBits::winding;
            }
        }

        /**
         * Returns cached or computed result if whether this {@link Outline}s {@code polyline} is a complex shape.
         * <p>
         * A polyline with less than 3 elements is marked a simple shape for simplicity.
         * </p>
         * <p>
         * The result is cached.
         * </p>
         */
        bool isComplex() const noexcept {
            if( !is_set(m_dirtyBits, DirtyBits::complexShape) ) {
                return m_complexShape;
            }
            m_complexShape = !computeIsComplex();
            // complexShape = isSelfIntersecting1(m_vertices);
            m_dirtyBits &= ~DirtyBits::complexShape;
            return m_complexShape;
        }

      private:
        bool computeIsComplex() const noexcept;

        void validateBoundingBox() const noexcept {
            m_dirtyBits &= ~DirtyBits::bounds;
            m_bbox.reset();
            for(const Vertex& v : m_vertices) {
                m_bbox.resize(v.coord());
            }
        }

      public:
        const AABBox3f& bounds() const noexcept {
            if ( is_set(m_dirtyBits, DirtyBits::bounds) ) {
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
            if( m_vertices.size() == max_elements ) {
                throw jau::RuntimeException("Max elements "+std::to_string(max_elements)+" reached", E_FILE_LINE);
            }
            m_vertices.push_back(vertex);
            if ( !is_set(m_dirtyBits, DirtyBits::bounds) ) {
                m_bbox.resize(vertex.coord());
            }
            m_dirtyBits |= DirtyBits::winding | DirtyBits::complexShape;
        }

        /**
         * Insert the {@link Vertex} element at the given {@code position} to the outline loop/strip.
         * @param position of the added Vertex
         * @param vertex Vertex object to be added
         * @throws NullPointerException if the  {@link Vertex} element is null
         * @throws IndexOutOfBoundsException if position is out of range (position < 0 || position > getVertexNumber())
         */
        void addVertex(size_type position, const Vertex& vertex) {
            if( m_vertices.size() == max_elements ) {
                throw jau::RuntimeException("Max elements "+std::to_string(max_elements)+" reached", E_FILE_LINE);
            }
            m_vertices.insert(position, vertex);
            if ( !is_set(m_dirtyBits, DirtyBits::bounds) ) {
                m_bbox.resize(vertex.coord());
            }
            m_dirtyBits |= DirtyBits::winding | DirtyBits::complexShape;
        }

        /// Returns a transformed copy of this instance using the given AffineTransform.
        Outline transform(const AffineTransform& t) const {
            Outline newOutline;
            for(const Vertex& v : m_vertices) {
                newOutline.addVertex(v.transform(t));
            }
            newOutline.m_closed = m_closed;
            return newOutline;
        }

        /**
         * Compare two outline's Bounding Box size.
         * @return <0, 0, >0 if this this object is less than, equal to, or greater than the other object.
         * @see AABBox3f::size()
         */
        int compareTo(const Outline& other) const noexcept {
            const float thisSize = bounds().size();
            const float otherSize = other.bounds().size();
            if( jau::equals2(thisSize, otherSize) ) {
                return 0;
            } else if(thisSize < otherSize){
                return -1;
            } else {
                return 1;
            }
        }

        /**
         * @return true if {@code o} equals bounds and vertices in the same order
         */
        constexpr bool operator==(const Outline& o) const noexcept {
            if( this == &o) {
                return true;
            }
            if(vertexCount() != o.vertexCount()) {
                return false;
            }
            if( bounds() != o.bounds() ) {
                return false;
            }
            for (size_type i=vertexCount(); i-- > 0;) {
                if( vertex(i) != o.vertex(i) ) {
                    return false;
                }
            }
            return true;
        }

    };
    // typedef std::shared_ptr<Outline> OutlineRef;
    typedef jau::darray<Outline, uint32_t> OutlineList;


    /**@}*/

} // namespace gamp::graph

#endif /*  JAU_GAMP_GRAPH_OUTLINE_HPP_ */
