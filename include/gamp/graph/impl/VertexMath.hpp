/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright Gothel Software e.K. and the authors
 *
 * SPDX-License-Identifier: MIT
 *
 * This Source Code Form is subject to the terms of the MIT License
 * If a copy of the MIT was not distributed with this file,
 * you can obtain one at https://opensource.org/license/mit/.
 */
#ifndef JAU_GAMP_GRAPH_IMPL_VERTEXMATH_HPP_
#define JAU_GAMP_GRAPH_IMPL_VERTEXMATH_HPP_

#include <sys/types.h>
#include <cstring>
#include <limits>

#include <jau/darray.hpp>
#include <jau/bitfield.hpp>
#include <jau/math/geom/geom.hpp>
#include <jau/math/vec3f.hpp>
#include <jau/math/geom/geom3f.hpp>
#include <jau/math/geom/geom3f2D.hpp>
#include <jau/math/geom/plane/affine_transform.hpp>

#include <gamp/GampTypes.hpp>
#include <gamp/graph/PrimTypes.hpp>

namespace gamp::graph::impl {

    using namespace jau::math;
    using jau::math::geom::Winding;

    /** \addtogroup Gamp_GraphImpl
     *
     *  @{
     */

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

    /**
     * Returns whether the given on-curve {@code polyline} points denotes a convex shape with O(n) complexity.
     * <p>
     * See [Determine whether a polygon is convex based on its vertices](https://math.stackexchange.com/questions/1743995/determine-whether-a-polygon-is-convex-based-on-its-vertices/1745427#1745427)
     * </p>
     * <p>
     * All off-curve points are ignored.
     * </p>
     * @param polyline connected {@link Vert2fImmutable}, i.e. a poly-line
     * @param shortIsConvex return value if {@code vertices} have less than three elements, allows simplification
     */
    constexpr static bool isConvex1(const VertexList& polyline, bool shortIsConvex) noexcept {
        auto cmod = [](ssize_t i, ssize_t count) noexcept -> ssize_t {
            if( i >= 0 ) {
                return i % count;
            } else {
                return i + count;
            }
        };
        const ssize_t polysz = static_cast<ssize_t>(polyline.size());
        if( polysz < 3 ) {
            return shortIsConvex;
        }
        constexpr float eps = std::numeric_limits<float>::epsilon();

        float wSign = 0;    // First nonzero orientation (positive or negative)

        int xSign = 0;
        int xFirstSign = 0; //  Sign of first nonzero edge vector x
        int xFlips = 0;     //  Number of sign changes in x

        int ySign = 0;
        int yFirstSign = 0; //  Sign of first nonzero edge vector y
        int yFlips = 0;     //  Number of sign changes in y

        ssize_t offset=-3;
        Vertex v0, v1;
        {
            do {
                ++offset; // -2
                v0 = polyline[cmod(offset, polysz)];   // current, polyline[-2] if on-curve
            } while( !v0.onCurve() && offset < polysz );
            if( offset >= polysz ) {
                return shortIsConvex;
            }
            do {
                ++offset; // -1
                v1 = polyline[cmod(offset, polysz)];   //  next, polyline[-1] if both on-curve
            } while( !v1.onCurve() && offset < polysz );
            if( offset >= polysz ) {
                return shortIsConvex;
            }
        }

        while( offset < polysz ) {
            Vertex vp = v0;                               //  previous on-curve vertex
            v0 = v1;                                      //  current on-curve vertex
            do {
                ++offset; // 0, ...
                v1 = polyline[cmod(offset, polysz)];  //  next on-curve vertex
            } while( !v1.onCurve() && offset < polysz );
            if( offset >= polysz ) {
                break;
            }

            //  Previous edge vector ("before"):
            const float bx = v0.coord().x - vp.coord().x;
            const float by = v0.coord().y - vp.coord().y;

            //  Next edge vector ("after"):
            const float ax = v1.coord().x - v0.coord().x;
            const float ay = v1.coord().y - v0.coord().y;

            //  Calculate sign flips using the next edge vector ("after"),
            //  recording the first sign.
            if( ax > eps ) {
                if( xSign == 0 ) {
                    xFirstSign = +1;
                } else if( xSign < 0 ) {
                    xFlips = xFlips + 1;
                }
                xSign = +1;
            } else if( ax < -eps ) {
                if( xSign == 0 ) {
                    xFirstSign = -1;
                } else if ( xSign > 0 ) {
                    xFlips = xFlips + 1;
                }
                xSign = -1;
            }
            if( xFlips > 2 ) {
                return false;
            }

            if( ay > eps ) {
                if( ySign == 0 ) {
                    yFirstSign = +1;
                } else if( ySign < 0 ) {
                    yFlips = yFlips + 1;
                }
                ySign = +1;
            } else if( ay < -eps ) {
                if( ySign == 0 ) {
                    yFirstSign = -1;
                } else if( ySign > 0 ) {
                    yFlips = yFlips + 1;
                }
                ySign = -1;
            }
            if( yFlips > 2 ) {
                return false;
            }

            //  Find out the orientation of this pair of edges,
            //  and ensure it does not differ from previous ones.
            const float w = bx*ay - ax*by;
            if( jau::is_zero(wSign) && !jau::is_zero(w) ) {
                wSign = w;
            } else if( wSign > eps && w < -eps ) {
                return false;
            } else if( wSign < -eps && w > eps ) {
                return false;
            }
        }

        //  Final/wraparound sign flips:
        if( xSign != 0 && xFirstSign != 0 && xSign != xFirstSign ) {
            xFlips = xFlips + 1;
        }
        if( ySign != 0 && yFirstSign != 0 && ySign != yFirstSign ) {
            yFlips = yFlips + 1;
        }

        //  Concave polygons have two sign flips along each axis.
        if( xFlips != 2 || yFlips != 2 ) {
            return false;
        }

        //  This is a convex polygon.
        return true;
    }

    /**
     * Check if a segment intersects with a triangle using {@link FloatUtil#EPSILON} w/o considering collinear-case
     * <p>
     * Implementation uses float precision.
     * </p>
     * @param a vertex 1 of the triangle
     * @param b vertex 2 of the triangle
     * @param c vertex 3 of the triangle
     * @param d vertex 1 of first segment
     * @param e vertex 2 of first segment
     * @return true if the segment intersects at least one segment of the triangle, false otherwise
     * @see #testSeg2SegIntersection(Vert2fImmutable, Vert2fImmutable, Vert2fImmutable, Vert2fImmutable, float, boolean)
     */
    constexpr bool testTri2SegIntersection2D(const Vertex& a, const Vertex& b, const Vertex& c,
                                             const Vertex& d, const Vertex& e) {
        using namespace jau::math::geom;
        return testSeg2SegIntersection2D(a.coord(), b.coord(), d.coord(), e.coord()) ||
               testSeg2SegIntersection2D(b.coord(), c.coord(), d.coord(), e.coord()) ||
               testSeg2SegIntersection2D(a.coord(), c.coord(), d.coord(), e.coord()) ;
    }

    /**@}*/

} // namespace gamp::graph

#endif /*  JAU_GAMP_GRAPH_VERTEXMATH_HPP_ */
