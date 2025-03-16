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
#ifndef JAU_GAMP_GRAPH_PRIMTYPES_HPP_
#define JAU_GAMP_GRAPH_PRIMTYPES_HPP_

#include <array>
#include <cstring>
#include <limits>
#include <memory>
#include <jau/darray.hpp>
#include <jau/bitfield.hpp>
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

    /** \addtogroup Gamp_Graph
     *
     *  @{
     */

    // typedef Vec3f Vertex;
    class Vertex {
      private:
        int m_id;
        Vec3f m_coord;
        Vec3f m_texCoord;
        bool m_onCurve;

      public:
        constexpr Vertex() noexcept
        : m_id(std::numeric_limits<int>::max()),
          m_coord(), m_texCoord(), m_onCurve(true) {}

        constexpr Vertex(const float x_, const float y_, const float z_, bool onCurve) noexcept
        : m_id(std::numeric_limits<int>::max()),
          m_coord(x_, y_, z_), m_texCoord(), m_onCurve(onCurve) {}

        constexpr Vertex(const Vec2f& coord, bool onCurve) noexcept
        : m_id(std::numeric_limits<int>::max()),
          m_coord(coord.x, coord.y, 0), m_texCoord(), m_onCurve(onCurve)
        {}

        constexpr Vertex(const float x_, const float y_, bool onCurve) noexcept
        : m_id(std::numeric_limits<int>::max()),
          m_coord(x_, y_, 0), m_texCoord(), m_onCurve(onCurve) {}

        constexpr Vertex(const Vec3f& coord, bool onCurve) noexcept
        : m_id(std::numeric_limits<int>::max()),
          m_coord(coord), m_texCoord(), m_onCurve(onCurve)
        {}

        constexpr Vertex(int id, bool onCurve, const Vec3f& texCoord) noexcept
        : m_id(id),
          m_coord(), m_texCoord(texCoord), m_onCurve(onCurve)
        {}

        /// Returns a transformed copy of this instance using the given AffineTransform.
        Vertex transform(const AffineTransform& t) const noexcept {
            Vertex v(m_id, m_onCurve, m_texCoord);
            t.transform(m_coord, v.coord());
            return v;
        }

        constexpr int id() const noexcept { return m_id; }
        constexpr int& id() noexcept { return m_id; }

        constexpr const Vec3f& coord() const noexcept { return m_coord; }
        constexpr Vec3f& coord() noexcept { return m_coord; }

        constexpr const Vec3f& texCoord() const noexcept { return m_texCoord; }
        constexpr Vec3f& texCoord() noexcept { return m_texCoord; }

        constexpr bool onCurve() const noexcept { return m_onCurve; }
        constexpr bool& onCurve() noexcept { return m_onCurve; }

        constexpr bool equals(const Vertex& o, const float epsilon=std::numeric_limits<float>::epsilon()) const noexcept {
            if( this == &o ) {
                return true;
            } else {
                return onCurve() == o.onCurve() &&
                       texCoord().equals( o.texCoord(), epsilon ) &&
                       coord().equals( o.coord(), epsilon );
            }
        }
        constexpr bool operator==(const Vertex& rhs) const noexcept {
            return equals(rhs);
        }

        std::string toString() const noexcept {
            return std::string("Vert[id ")
                .append(std::to_string(m_id))
                .append(", onCurve ").append(jau::to_string(m_onCurve))
                .append(", p ").append(m_coord.toString())
                .append(", t ").append(m_texCoord.toString())
                .append("]");
        }
    };
    typedef jau::darray<Vertex> VertexList;

    class Triangle;
    typedef std::shared_ptr<Triangle> TriangleRef;
    typedef std::vector<TriangleRef> TriangleRefList;

    class Triangle {
      public:
        typedef std::array<Vertex, 3> trivert_t;
        typedef jau::bitfield<3> tribit_t;

      private:
        int m_id;
        trivert_t m_vertices;
        tribit_t m_boundaryEdges;
        tribit_t m_boundaryVertices;

        constexpr Triangle(Vertex v1, Vertex v2, Vertex v3, const tribit_t& boundaryVertices) noexcept
        : m_id(std::numeric_limits<int>::max()),
          m_vertices(), m_boundaryVertices(boundaryVertices)
        {
            m_vertices[0] = v1;
            m_vertices[1] = v2;
            m_vertices[2] = v3;
        }
        constexpr Triangle(const trivert_t& vertices, const tribit_t& boundaryVertices) noexcept
        : m_id(std::numeric_limits<int>::max()),
          m_vertices(vertices), m_boundaryVertices(boundaryVertices)
        { }

        constexpr Triangle(int id, const tribit_t& boundaryEdges, const tribit_t& boundaryVertices) noexcept
        : m_id(id), m_boundaryEdges(boundaryEdges), m_boundaryVertices(boundaryVertices)
        { }

        static TriangleRef createImpl(int id, const tribit_t& boundaryEdges, const tribit_t& boundaryVertices) noexcept {
            return std::make_shared<Triangle>(id, boundaryEdges, boundaryVertices);
        }

      public:
        static TriangleRef create(Vertex v1, Vertex v2, Vertex v3, const tribit_t& boundaryVertices) noexcept {
            return std::make_shared<Triangle>(v1, v2, v3, boundaryVertices);
        }
        static TriangleRef create(const trivert_t& vertices, const tribit_t& boundaryVertices) noexcept {
            return std::make_shared<Triangle>(vertices, boundaryVertices);
        }

        /// Returns a transformed copy of this instance using the given AffineTransform.
        TriangleRef transform(const AffineTransform& t) const noexcept {
            TriangleRef tri = createImpl(m_id, m_boundaryEdges, m_boundaryVertices);
            tri->m_vertices[0] = m_vertices[0].transform(t);
            tri->m_vertices[1] = m_vertices[1].transform(t);
            tri->m_vertices[2] = m_vertices[2].transform(t);
            return tri;
        }

        /// Returns true if all vertices are on-curve, otherwise false.
        constexpr bool onCurve() const noexcept {
            return m_vertices[0].onCurve() && m_vertices[1].onCurve() && m_vertices[2].onCurve();
        }

        /// Returns true if all vertices are lines, i.e. zero tex-coord, otherwise false.
        bool isLine() const noexcept {
            return m_vertices[0].texCoord().is_zero() &&
                   m_vertices[1].texCoord().is_zero() &&
                   m_vertices[2].texCoord().is_zero() ;
        }

        constexpr int id() const noexcept { return m_id; }
        constexpr int& id() noexcept { return m_id; }

        /// Returns array of 3 vertices, denominating the triangle.
        constexpr const trivert_t& vertices() const noexcept { return m_vertices; }
        /// Returns array of 3 vertices, denominating the triangle.
        constexpr trivert_t& vertices() noexcept { return m_vertices; }

        constexpr const tribit_t& boundaryEdges() const noexcept { return m_boundaryEdges; }
        constexpr tribit_t& boundaryEdges() noexcept { return m_boundaryEdges; }

        constexpr const tribit_t& boundaryVertices() const noexcept { return m_boundaryVertices; }
        constexpr tribit_t& boundaryVertices() noexcept { return m_boundaryVertices; }

        constexpr bool isEdgesBoundary() const noexcept {
            return m_boundaryEdges[0] || m_boundaryEdges[1] || m_boundaryEdges[2];
        }

        constexpr bool isVerticesBoundary() const noexcept {
            return m_boundaryVertices[0] || m_boundaryVertices[1] || m_boundaryVertices[2];
        }

        std::string toString() const noexcept {
            return std::string("Tri[id ")
                .append(std::to_string(m_id))
                .append(", onCurve ").append(std::to_string(onCurve()))
                .append(m_vertices[0].toString()).append(", bound ").append(jau::to_string(m_boundaryVertices[0])).append("\n\t")
                .append(m_vertices[1].toString()).append(", bound ").append(jau::to_string(m_boundaryVertices[1])).append("\n\t")
                .append(m_vertices[2].toString()).append(", bound ").append(jau::to_string(m_boundaryVertices[2]))
                .append("]");
        }
    };

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

    /**@}*/

} // namespace gamp::graph

#endif /*  JAU_GAMP_GRAPH_PRIMTYPES_HPP_ */
