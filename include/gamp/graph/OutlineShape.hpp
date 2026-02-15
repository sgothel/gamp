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
#ifndef JAU_GAMP_GRAPH_OUTLINESHAPE_HPP_
#define JAU_GAMP_GRAPH_OUTLINESHAPE_HPP_

#include <jau/basic_algos.hpp>
#include <jau/basic_types.hpp>
#include <jau/debug.hpp>
#include <jau/enum_util.hpp>
#include <jau/math/geom/geom3f.hpp>
#include <jau/math/geom/plane/affine_transform.hpp>
#include <jau/math/vec3f.hpp>

#include <gamp/graph/Graph.hpp>
#include <gamp/graph/Outline.hpp>
#include <gamp/graph/PrimTypes.hpp>

namespace gamp::graph {

    using namespace jau::math;
    using namespace jau::math::geom;
    using namespace jau::enums;
    using jau::math::geom::plane::AffineTransform;

    /** \addtogroup Gamp_Graph
     *
     *  @{
     */

    /**
     * A Generic shape objects which is defined by a list of Outlines.
     * This Shape can be transformed to triangulations.
     * The list of triangles generated are render-able by a Region object.
     * The triangulation produced by this Shape will define the
     * closed region defined by the outlines.
     *
     * One or more OutlineShape Object can be associated to a region
     * this is left as a high-level representation of the Objects. For
     * optimizations, flexibility requirements for future features.

     * <a name="windingrules">
     * Outline shape general {@link Winding} rules
     * - Outer boundary-shapes are required as Winding::CCW
     * - Inner hole-shapes should be Winding::CW
     * - If unsure
     *   - You may check Winding via getWindingOfLastOutline() or Outline::getWinding() (optional, might be incorrect)
     *   - Use setWindingOfLastOutline(Winding) before {@link #closeLastOutline(boolean)} or {@link #closePath()} } to enforce Winding::CCW, or
     *   - use Outline::setWinding(Winding) on a specific Outline to enforce Winding::CCW.
     *   - If e.g. the Winding has changed for an Outline by above operations, its vertices have been reversed.
     * - Safe path: Simply create all outer boundary-shapes with Winding::CCW and inner hole-shapes with Winding::CW.
     *
     * Example to creating an Outline Shape:
     * <pre>
          addVertex(...)
          addVertex(...)
          addVertex(...)
          addEmptyOutline()
          addVertex(...)
          addVertex(...)
          addVertex(...)
     * </pre>
     *
     * The above will create two outlines each with three vertices. By adding these two outlines to
     * the OutlineShape, we are stating that the combination of the two outlines represent the shape.
     *
     * To specify that the shape is curved at a region, the on-curve flag should be set to false
     * for the vertex that is in the middle of the curved region (if the curved region is defined by 3
     * vertices (quadratic curve).
     *
     * In case the curved region is defined by 4 or more vertices the middle vertices should both have
     * the on-curve flag set to false.
     *
     * Example:
     * <pre>
          addVertex(0,0, true);
          addVertex(0,1, false);
          addVertex(1,1, false);
          addVertex(1,0, true);
     * </pre>
     *
     * The above snippet defines a cubic nurbs curve where (0,1 and 1,1)
     * do not belong to the final rendered shape.
     *
     * <i>Implementation Notes:</i><br>
     * - The first vertex of any outline belonging to the shape should be on-curve
     * - Intersections between off-curved parts of the outline is not handled
     *
     * @see Outline
     * @see Region
     */
    class OutlineShape {
      public:
        typedef uint32_t size_type;

        /// byte-size uint32_t limit: 1'073'741'823 (FIXME: Adjust to actual type, i.e. Vertex = 2x Vec3f?)
        constexpr static size_type max_elements = std::numeric_limits<uint32_t>::max() / sizeof(uint32_t);

        /** Initial sharpness() value, which can be modified via setSharpness(float). */
        static constexpr float DEFAULT_SHARPNESS = 0.5f;

        enum class DirtyBits : uint16_t {
            none = 0,
            bounds = 1 << 0,
            vertices = 1 << 1,  /// <Modified shape, requires to update the vertices and triangles, here: vertices
            triangles = 1 << 2, /// <Modified shape, requires to update the vertices and triangles, here: triangulation.
            convex = 1 << 3,    /// <Modified shape, requires to update the convex determination
            convexOverride = 1 << 4
        };
        JAU_MAKE_BITFIELD_ENUM_STRING_MEMBER(DirtyBits, bounds, vertices, triangles, convex, convexOverride);

        enum class VertexState : uint16_t { undefined = 0, quadratic_nurbs = 1 };
        JAU_MAKE_ENUM_STRING_MEMBER(VertexState, quadratic_nurbs);

      private:
        size_type m_outlineVertCapacity;
        Vec3f m_normal;
        OutlineList m_outlines;
        mutable AABBox3f m_bbox;
        mutable DirtyBits m_dirtyBits;
        size_type m_addedVertexCount;
        mutable bool m_complexShape;
        VertexState m_outlineState;
        float m_sharpness;
        VertexList m_vertices;
        TriangleRefList m_triangles;

      public:

        OutlineShape() : OutlineShape(2, 3) {}
        OutlineShape(size_type capacity, size_type outlineVertCapacity)
        : m_outlineVertCapacity(capacity),
          m_normal(0, 0, 1),
          m_outlines(m_outlineVertCapacity),
          m_bbox(),
          m_dirtyBits(DirtyBits::none),
          m_addedVertexCount(0),
          m_complexShape(false),
          m_outlineState(VertexState::undefined),
          m_sharpness(DEFAULT_SHARPNESS)
        {
            m_outlines.emplace_back(outlineVertCapacity);
        }


        /** Normal vector, optionally used by tesselator to add (interleaved) normals.  */
        constexpr const Vec3f& normal() const noexcept { return m_normal; }
        /** Writing the normal vector, optionally used by tesselator to add (interleaved) normals.  */
        constexpr Vec3f& normal() noexcept { return m_normal; }
        /// Set the normal using given 3 points
        void setNormal(Point3f p0, Point3f p1, Point3f p2) noexcept {
            m_normal.cross((p1 - p0), (p2 - p0)).normalize();
        }
        /// Set the normal using first outline's 3 points
        void setNormal() noexcept {
            if(m_outlines.size() > 0){
                const VertexList& v = m_outlines[0].vertices();
                if(v.size() > 2){
                    setNormal(v[0].coord(), v[1].coord(), v[2].coord());
                }
            }
        }
        /**
         * Return the number of newly added vertices during getTriangles(VerticesState)
         * while transforming the outlines to VerticesState::QUADRATIC_NURBS and triangulation.
         * @see setIsQuadraticNurbs()
         */
        constexpr size_type addedVertexCount() const noexcept { return m_addedVertexCount; }

        /** Sharpness value, defaults to DEFAULT_SHARPNESS. */
        constexpr float sharpness() const noexcept { return m_sharpness; }

        /** Sets sharpness, defaults to DEFAULT_SHARPNESS. */
        void setSharpness(float s) noexcept {
            if( m_sharpness != s ) {
                clearCache();
                m_sharpness=s;
            }
        }

        /** Clears all data and reset all states as if this instance was newly created */
        void clear() {
            m_outlines.clear();
            m_outlines.emplace_back(m_outlineVertCapacity);
            m_outlineState = VertexState::undefined;
            m_bbox.reset();
            m_vertices.clear();
            m_triangles.clear();
            m_addedVertexCount = 0;
            m_complexShape = false;
            m_dirtyBits = DirtyBits::none;
        }

        /** Clears cached triangulated data, i.e. {@link #getTriangles(VerticesState)} and {@link #getVertices()}.  */
        void clearCache() noexcept {
            m_vertices.clear();
            m_triangles.clear();
            m_dirtyBits |= DirtyBits::triangles | DirtyBits::vertices | DirtyBits::convex;
        }

        constexpr void reserve(size_type newCapacity) { m_outlines.reserve(newCapacity); }

        constexpr DirtyBits dirtyBits() const noexcept { return m_dirtyBits; }
        constexpr bool verticesDirty() const noexcept { return is_set(m_dirtyBits, DirtyBits::vertices); }
        constexpr bool trianglesDirty() const noexcept { return is_set(m_dirtyBits, DirtyBits::triangles); }
        void markClean(DirtyBits flags) noexcept { m_dirtyBits &= ~flags; }

      private:
        void validateBoundingBox() const noexcept {
            m_dirtyBits &= ~DirtyBits::bounds;
            m_bbox.reset();
            for (const auto & m_outline : m_outlines) {
                m_bbox.resize(m_outline.bounds());
            }
        }

      public:
        const AABBox3f& bounds() const noexcept {
            if ( is_set(m_dirtyBits, DirtyBits::bounds) ) {
                validateBoundingBox();
            }
            return m_bbox;
        }

        bool empty() const noexcept { return m_outlines.empty(); }

        /** Returns the number of {@link Outline}s. */
        size_type outlineCount() const noexcept {
            return m_outlines.size();
        }

        /** Returns the total {@link Outline#getVertexCount() vertex number} of all {@link Outline}s. */
        size_type vertexCount() const noexcept {
            size_type res = 0;
            for(const Outline& o : m_outlines) {
                res += o.vertexCount();
            }
            return res;
        }

        const OutlineList& outlines() const noexcept { return m_outlines; }
        OutlineList& outlines() noexcept { return m_outlines; }

        const Outline& outline(size_type i) const noexcept { return m_outlines[i]; }
        Outline& outline(size_type i) noexcept { return m_outlines[i]; }

        /**
         * Get the last added outline to the list
         * of outlines that define the shape
         * @return the last outline
         */
        const Outline& lastOutline() const noexcept {
            return m_outlines[m_outlines.size()-1];
        }
        /**
         * Get the last added outline to the list
         * of outlines that define the shape
         * @return the last outline
         */
        Outline& lastOutline() noexcept {
            return m_outlines[m_outlines.size()-1];
        }

        /**
         * Compute the {@link Winding} of the {@link #getLastOutline()} using the {@link VectorUtil#area(ArrayList)} function over all of its vertices.
         * @return {@link Winding#CCW} or {@link Winding#CW}
         */
        Winding windingOfLastOutline() const noexcept {
            return lastOutline().getWinding();
        }

        /**
         * Sets the enforced {@link Winding} of the {@link #getLastOutline()}.
         */
        void setWindingOfLastOutline(Winding enforced) {
            lastOutline().setWinding(enforced);
        }

        /**
         * Returns cached or computed result if at least one `polyline` outline(size_type) is a complex shape, see Outline::isComplex().
         * <p>
         * A polyline with less than 3 elements is marked a simple shape for simplicity.
         * </p>
         * <p>
         * The result is cached.
         * </p>
         * @see #setOverrideConvex(boolean)
         * @see #clearOverrideConvex()
         */
        bool isComplex() const noexcept {
            if( !is_set(m_dirtyBits, DirtyBits::convexOverride) &&
                 is_set(m_dirtyBits, DirtyBits::convex) )
            {
                m_complexShape = false;
                size_type sz = outlineCount();
                for(size_type i=0; i<sz && !m_complexShape; ++i) {
                    m_complexShape = outline(i).isComplex();
                }
                m_dirtyBits &= ~DirtyBits::convex;
            }
            return m_complexShape;
        }
        /**
         * Overrides {@link #isComplex()} using the given value instead of computing via {@link Outline#isComplex()}.
         * @see #clearOverrideConvex()
         * @see #isComplex()
         */
        void setOverrideConvex(bool convex) noexcept {
            m_dirtyBits |= DirtyBits::convexOverride;
            m_complexShape = convex;
        }

        /**
         * Clears the {@link #isComplex()} override done by {@link #setOverrideConvex(boolean)}
         * @see #setOverrideConvex(boolean)
         * @see #isComplex()
         */
        void clearOverrideConvex() noexcept {
            m_dirtyBits &= ~DirtyBits::convexOverride;
            m_dirtyBits |= DirtyBits::convex;
        }

        /**
         * Add a new empty {@link Outline}
         * to the end of this shape's outline list.
         * <p>If the {@link #getLastOutline()} is empty already, no new one will be added.</p>
         *
         * After a call to this function all new vertices added
         * will belong to the new outline
         */
        void addEmptyOutline() {
            if( !lastOutline().empty() ) {
                m_outlines.emplace_back(m_outlineVertCapacity);
            }
        }

        /**
         * Appends the {@link Outline} element to the end,
         * ensuring a clean tail.
         *
         * <p>A clean tail is ensured, no double empty Outlines are produced
         * and a pre-existing empty outline will be replaced with the given one. </p>
         *
         * @param outline Outline object to be added
         * @throws NullPointerException if the  {@link Outline} element is null
         */
        void addOutline(const Outline& outline) {
            addOutline(m_outlines.size(), outline);
        }

        /**
         * Insert the {@link Outline} element at the given {@code position}.
         *
         * <p>If the {@code position} indicates the end of this list,
         * a clean tail is ensured, no double empty Outlines are produced
         * and a pre-existing empty outline will be replaced with the given one. </p>
         *
         * @param position of the added Outline
         * @param outline Outline object to be added
         * @throws NullPointerException if the  {@link Outline} element is null
         * @throws IndexOutOfBoundsException if position is out of range (position < 0 || position > getOutlineNumber())
         */
        void addOutline(size_type position, const Outline& outline) {
            if( m_outlines.size() == position ) {
                const Outline& last = lastOutline();
                if( outline.empty() && last.empty() ) {
                    return;
                }
                if( last.empty() ) {
                    m_outlines[position-1] = outline;
                    if( !is_set(m_dirtyBits, DirtyBits::bounds) ) {
                        m_bbox.resize(outline.bounds());
                    }
                    // vertices.addAll(outline.getVertices()); // FIXME: can do and remove DIRTY_VERTICES ?
                    m_dirtyBits |= DirtyBits::triangles | DirtyBits::vertices | DirtyBits::convex;
                    return;
                }
            }
            m_outlines.insert(position, outline);
            if( !is_set(m_dirtyBits, DirtyBits::bounds) ) {
                m_bbox.resize(outline.bounds());
            }
            m_dirtyBits |= DirtyBits::triangles | DirtyBits::vertices | DirtyBits::convex;
        }

        /**
         * Insert the {@link OutlineShape} elements of type {@link Outline}, .. at the end of this shape,
         * using {@link #addOutline(Outline)} for each element.
         * <p>Closes the current last outline via {@link #closeLastOutline(boolean)} before adding the new ones.</p>
         * @param outlineShape OutlineShape elements to be added.
         * @throws NullPointerException if the  {@link OutlineShape} is null
         * @throws IndexOutOfBoundsException if position is out of range (position < 0 || position > getOutlineNumber())
         */
        void addOutlineShape(const OutlineShape& outlineShape) {
            closeLastOutline(true);
            for(size_type i=0; i<outlineShape.outlineCount(); i++) {
                addOutline(outlineShape.outline(i));
            }
        }

        /**
         * Replaces the {@link Outline} element at the given {@code position}.
         * <p>Sets the bounding box dirty, hence a next call to {@link #getBounds()} will validate it.</p>
         *
         * @param position of the replaced Outline
         * @param outline replacement Outline object
         * @throws NullPointerException if the  {@link Outline} element is null
         * @throws IndexOutOfBoundsException if position is out of range (position < 0 || position >= getOutlineNumber())
         */
        void setOutline(size_type position, const Outline& outline) {
            m_outlines.insert(position, outline);
            m_dirtyBits |= DirtyBits::bounds | DirtyBits::triangles | DirtyBits::vertices | DirtyBits::convex;
        }

        /**
         * Removes the {@link Outline} element at the given {@code position}.
         * <p>Sets the bounding box dirty, hence a next call to {@link #getBounds()} will validate it.</p>
         *
         * @param position of the to be removed Outline
         * @throws IndexOutOfBoundsException if position is out of range (position < 0 || position >= getOutlineNumber())
         */
        void removeOutline(size_type position) {
            m_dirtyBits |= DirtyBits::bounds | DirtyBits::triangles | DirtyBits::vertices | DirtyBits::convex;
            m_outlines.erase(position);
        }

        //
        //

        /**
         * Adds a vertex to the last open outline to the shape's tail.
         *
         * @param v the vertex to be added to the OutlineShape
         * @see <a href="#windingrules">see winding rules</a>
         */
        void addVertex(const Vertex& v) {
            Outline& lo = lastOutline();
            lo.addVertex(v);
            if( !is_set(m_dirtyBits, DirtyBits::bounds) ) {
                m_bbox.resize(v.coord());
            }
            // vertices.add(v); // FIXME: can do and remove DIRTY_VERTICES ?
            m_dirtyBits |= DirtyBits::triangles | DirtyBits::vertices | DirtyBits::convex;
        }

        /**
         * Adds a vertex to the last open outline to the shape at {@code position}
         *
         * @param position index within the last open outline, at which the vertex will be added
         * @param v the vertex to be added to the OutlineShape
         * @see <a href="#windingrules">see winding rules</a>
         */
        void addVertex(size_type position, const Vertex& v) {
            Outline& lo = lastOutline();
            lo.addVertex(position, v);
            if( !is_set(m_dirtyBits, DirtyBits::bounds) ) {
                m_bbox.resize(v.coord());
            }
            // vertices.add(v); // FIXME: can do and remove DIRTY_VERTICES ?
            m_dirtyBits |= DirtyBits::triangles | DirtyBits::vertices | DirtyBits::convex;
        }

        /**
         * Add a 3D {@link Vertex} to the last open outline to the shape's tail.
         *
         * @param x the x coordinate
         * @param y the y coordinate
         * @param z the z coordinate
         * @param onCurve flag if this vertex is on the curve or defines a curved region of the shape around this vertex.
         * @see <a href="#windingrules">see winding rules</a>
         */
        void addVertex(float x, float y, float z, bool onCurve) {
            addVertex(Vertex(x, y, z, onCurve));
        }

        /**
         * Add a 3D {@link Vertex} to the last open outline to the shape's tail.
         *
         * @param v the Vec3f coordinates
         * @param onCurve flag if this vertex is on the curve or defines a curved region of the shape around this vertex.
         * @see <a href="#windingrules">see winding rules</a>
         */
        void addVertex(const Vec3f& v, bool onCurve) {
            addVertex(Vertex(v, onCurve));
        }

        /**
         * Add a 2D {@link Vertex} to the last open outline to the shape's tail.
         *
         * @param x the x coordinate
         * @param y the y coordinate
         * @param onCurve flag if this vertex is on the curve or defines a curved region of the shape around this vertex.
         * @see <a href="#windingrules">see winding rules</a>
         */
        void addVertex(float x, float y, bool onCurve) {
            addVertex(Vertex(x, y, onCurve));
        }

        /**
         * Add a 2D {@link Vertex} to the last open outline to the shape's tail.
         *
         * @param v the Vec2f coordinates
         * @param onCurve flag if this vertex is on the curve or defines a curved region of the shape around this vertex.
         * @see <a href="#windingrules">see winding rules</a>
         */
        void addVertex(const Vec2f& v, bool onCurve) {
            addVertex(Vertex(v, onCurve));
        }

        /**
         * Add a 3D {@link Vertex} to the last open outline to the shape at {@code position}.
         *
         * @param position index within the last open outline, at which the vertex will be added
         * @param x the x coordinate
         * @param y the y coordniate
         * @param z the z coordinate
         * @param onCurve flag if this vertex is on the curve or defines a curved region of the shape around this vertex.
         * @see <a href="#windingrules">see winding rules</a>
         */
        void addVertex(size_type position, float x, float y, float z, bool onCurve) {
            addVertex(position, Vertex(x, y, z, onCurve));
        }

        /**
         * Start a new position for the next line segment at given point x/y (P1).
         *
         * @param x point (P1)
         * @param y point (P1)
         * @param z point (P1)
         * @see Path2F#moveTo(float, float)
         * @see #addPath(com.jogamp.math.geom.plane.Path2F.Iterator, boolean)
         * @see <a href="#windingrules">see winding rules</a>
         */
        void moveTo(float x, float y, float z) {
            if ( 0 == lastOutline().vertexCount() ) {
                addVertex(x, y, z, true);
            } else {
                closeLastOutline(false);
                addEmptyOutline();
                addVertex(x, y, z, true);
            }
        }

        /**
         * Add a line segment, intersecting the last point and the given point x/y (P1).
         *
         * @param x final point (P1)
         * @param y final point (P1)
         * @param z final point (P1)
         * @see Path2F#lineTo(float, float)
         * @see #addPath(com.jogamp.math.geom.plane.Path2F.Iterator, boolean)
         * @see <a href="#windingrules">see winding rules</a>
         */
        void lineTo(float x, float y, float z) {
            addVertex(x, y, z, true);
        }

        /**
         * Add a quadratic curve segment, intersecting the last point and the second given point x2/y2 (P2).
         *
         * @param x1 quadratic parametric control point (P1)
         * @param y1 quadratic parametric control point (P1)
         * @param z1 quadratic parametric control point (P1)
         * @param x2 final interpolated control point (P2)
         * @param y2 final interpolated control point (P2)
         * @param z2 quadratic parametric control point (P2)
         * @see Path2F#quadTo(float, float, float, float)
         * @see #addPath(com.jogamp.math.geom.plane.Path2F.Iterator, boolean)
         * @see <a href="#windingrules">see winding rules</a>
         */
        void quadTo(float x1, float y1, float z1, float x2, float y2, float z2) {
            addVertex(x1, y1, z1, false);
            addVertex(x2, y2, z2, true);
        }

        /**
         * Add a cubic Bézier curve segment, intersecting the last point and the second given point x3/y3 (P3).
         *
         * @param x1 Bézier control point (P1)
         * @param y1 Bézier control point (P1)
         * @param z1 Bézier control point (P1)
         * @param x2 Bézier control point (P2)
         * @param y2 Bézier control point (P2)
         * @param z2 Bézier control point (P2)
         * @param x3 final interpolated control point (P3)
         * @param y3 final interpolated control point (P3)
         * @param z3 final interpolated control point (P3)
         * @see Path2F#cubicTo(float, float, float, float, float, float)
         * @see #addPath(com.jogamp.math.geom.plane.Path2F.Iterator, boolean)
         * @see <a href="#windingrules">see winding rules</a>
         */
        void cubicTo(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3) {
            addVertex(x1, y1, z1, false);
            addVertex(x2, y2, z2, false);
            addVertex(x3, y3, z3, true);
        }

        /**
         * Closes the last outline in the shape.
         * <p>
         * Checks whether the last vertex equals to the first of the last outline.
         * If not equal, it either appends a copy of the first vertex
         * or prepends a copy of the last vertex, depending on <code>closeTail</code>.
         * </p>
         * @param closeTail if true, a copy of the first vertex will be appended,
         *                  otherwise a copy of the last vertex will be prepended.
         */
        void closeLastOutline(bool closeTail) {
            if( lastOutline().setClosed( closeTail ) ) {
                m_dirtyBits |= DirtyBits::triangles | DirtyBits::vertices | DirtyBits::convex;
            }
        }

        /**
         * Closes the current sub-path segment by drawing a straight line back to the coordinates of the last moveTo.
         * If the path is already closed, no additional lineTo is issued.
         *
         * Method sets the normal using the first outline's 3 points, see setNormal()
         * @see Path2F#closePath()
         * @see #addPath(com.jogamp.math.geom.plane.Path2F.Iterator, boolean)
         * @see setNormal
         */
        void closePath() {
            if ( 0 < lastOutline().vertexCount() ) {
                closeLastOutline(true);
                setNormal();
                addEmptyOutline();
            }
        }

        VertexState outlineState() const noexcept { return m_outlineState; }

        /**
         * Claim this outline's vertices are all VertexState::quadratic_nurbs,
         * hence no cubic transformations will be performed.
         */
        void setIsQuadraticNurbs() noexcept {
            m_outlineState = VertexState::quadratic_nurbs;
            // checkPossibleOverlaps = false;
        }

        /**
         * Return a transformed instance with all {@link Outline}s are copied and transformed.
         * <p>
         * Note: Triangulated data is lost in returned instance!
         * </p>
         */
        OutlineShape transform(const AffineTransform& t) const {
            OutlineShape newOutlineShape;
            size_type osize = m_outlines.size();
            for(size_type i=0; i<osize; i++) {
                newOutlineShape.addOutline( m_outlines[i].transform(t) );
            }
            return newOutlineShape;
        }

        /// Returns a copy of this instance with normal() pointing to the opposite direction and all outlines() vertices()'s z-axis sign-flipped,
        /// used to generate a back-face from a front-face shape.
        OutlineShape flipFace(float zoffset=0) const {
            OutlineShape nshape = *this;
            nshape.normal() *= -1;
            for(Outline& o : nshape.outlines()) {
                for(Vertex& v : o.vertices()) {
                    v.coord().z = ( v.coord().z * -1.0f ) + zoffset;
                }
            }
            return nshape;
        }

        /**
         * Compare two outline shape's Bounding Box size.
         * @return <0, 0, >0 if this this object is less than, equal to, or greater than the other object.
         * @see AABBox3f::size()
         */
        int compareTo(const OutlineShape& other) const noexcept {
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
        constexpr bool operator==(const OutlineShape& o) const noexcept {
            if( this == &o) {
                return true;
            }
            if(outlineState() != o.outlineState()) {
                return false;
            }
            if(outlineCount() != o.outlineCount()) {
                return false;
            }
            if( bounds() != o.bounds() ) {
                return false;
            }
            for (size_type i=outlineCount(); i-- > 0;) {
                if( outline(i) != o.outline(i) ) {
                    return false;
                }
            }
            return true;
        }

      private:
        //
        // prost-processing
        //

        void subdivideTriangle(Outline& outline, const Vertex& a, Vertex& b, const Vertex& c, size_type index);

        /**
         * Check overlaps between curved triangles
         * first check if any vertex in triangle a is in triangle b
         * second check if edges of triangle a intersect segments of triangle b
         * if any of the two tests is true we divide current triangle
         * and add the other to the list of overlaps
         *
         * Loop until overlap array is empty. (check only in first pass)
         */
        void checkOverlaps();
        Vertex* checkTriOverlaps0(const Vertex& a, const Vertex& b, const Vertex& c);
        void cleanupOutlines();
        uint32_t generateVertexIds();

        /**
         * Sort the outlines in descending size from large
         * to small depending on the AABBox
         */
        void sortOutlines();

        void triangulateImpl();

      public:
        /**
         * Triangulate the {@link OutlineShape} generating a list of triangles,
         * while {@link #transformOutlines(VerticesState)} beforehand.
         *
         * Triangles are cached until marked dirty.
         *
         * After generating a the triangles, getVertices() can be used for all vertices.
         *
         * @return an arraylist of triangles representing the filled region
         * which is produced by the combination of the outlines
         *
         * @see getVertices()
         */
        const TriangleRefList& getTriangles(VertexState destinationType = VertexState::quadratic_nurbs);

        /**
         * Return list of concatenated vertices associated with all
         * {@code Outline}s of this object.
         *
         * Vertices are cached until marked dirty.
         *
         * Should always be called <i>after</i> getTriangles(VerticesState),
         * since the latter will mark all cached vertices dirty!
         */
        const VertexList& getVertices();

        std::string toString() const noexcept {
            std::string r("OutlineShape[");
            r.append("dirty ").append(to_string(m_dirtyBits))
             .append(", outlines ").append(std::to_string(outlineCount()))
             .append(", vertices ").append(std::to_string(vertexCount()))
             .append("]");
            return r;
        }

      private:

    };


    /**@}*/

} // namespace gamp::graph

#endif /*  JAU_GAMP_GRAPH_OUTLINESHAPE_HPP_ */
