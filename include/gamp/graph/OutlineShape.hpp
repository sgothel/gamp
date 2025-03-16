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

#include <jau/math/geom/plane/affine_transform.hpp>

#include <gamp/graph/Outline.hpp>

namespace gamp::graph {

    using namespace jau::math;
    using namespace jau::math::geom;
    using jau::math::geom::plane::AffineTransform;

    /** \addtogroup Gamp_Graph
     *
     *  @{
     */

    class OutlineShape {
      public:
        static constexpr int DIRTY_BOUNDS = 1 << 0;
        static constexpr int DIRTY_VERTICES = 1 << 1;
        static constexpr int DIRTY_TRIANGLES = 1 << 2;
        static constexpr int DIRTY_CONVEX  = 1 << 3;
        static constexpr int OVERRIDE_CONVEX  = 1 << 4;

      private:
        size_t m_outlineVertCapacity;
        OutlineList m_outlines;
        Vec3f m_normal;
        // VertexList m_vertices;
        mutable AABBox3f m_bbox;
        mutable int m_dirtyBits;

      public:

        OutlineShape() : OutlineShape(2, 3) {}
        OutlineShape(size_t capacity, size_t outlineVertCapacity)
        : m_outlineVertCapacity(capacity),
          m_outlines(m_outlineVertCapacity),
          m_normal(0, 0, 1),
          m_bbox(),
          m_dirtyBits(0)
        {
            m_outlines.emplace_back(outlineVertCapacity);
        }

        constexpr void reserve(size_t newCapacity) { m_outlines.reserve(newCapacity); }

        constexpr int dirtyBits() const noexcept { return m_dirtyBits; }
        constexpr bool verticesDirty() const noexcept { return 0 != ( m_dirtyBits & DIRTY_VERTICES); }
        constexpr bool trianglesDirty() const noexcept { return 0 != ( m_dirtyBits & DIRTY_TRIANGLES); }
        void markClean(int flags) noexcept { m_dirtyBits &= ~flags; }

        /** Clears all data and reset all states as if this instance was newly created */
        void clear() {
            m_outlines.clear();
            m_outlines.emplace_back(m_outlineVertCapacity);
            // m_outlineState = VerticesState.UNDEFINED;
            m_bbox.reset();
            // m_triangles.clear();
            // m_addedVerticeCount = 0;
            m_dirtyBits = 0;
        }

      private:
        void validateBoundingBox() const noexcept {
            m_dirtyBits &= ~DIRTY_BOUNDS;
            m_bbox.reset();
            for (const auto & m_outline : m_outlines) {
                m_bbox.resize(m_outline.bounds());
            }
        }

      public:
        const AABBox3f& bounds() const noexcept {
            if ( 0 != ( m_dirtyBits & DIRTY_BOUNDS ) ) {
                validateBoundingBox();
            }
            return m_bbox;
        }

        bool empty() const noexcept { return m_outlines.empty(); }

        constexpr const Vec3f& normal() const noexcept { return m_normal; }
        constexpr Vec3f& normal() noexcept { return m_normal; }

        /** Returns the number of {@link Outline}s. */
        size_t outlineCount() const noexcept {
            return m_outlines.size();
        }

        /** Returns the total {@link Outline#getVertexCount() vertex number} of all {@link Outline}s. */
        size_t vertexCount() const noexcept {
            size_t res = 0;
            for(const Outline& o : m_outlines) {
                res += o.vertexCount();
            }
            return res;
        }

        const OutlineList& outlines() const noexcept { return m_outlines; }
        OutlineList& outlines() noexcept { return m_outlines; }

        const Outline& outline(size_t i) const noexcept { return m_outlines[i]; }
        Outline& outline(size_t i) noexcept { return m_outlines[i]; }

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
        void addOutline(size_t position, const Outline& outline) {
            if( m_outlines.size() == position ) {
                const Outline& last = lastOutline();
                if( outline.empty() && last.empty() ) {
                    return;
                }
                if( last.empty() ) {
                    m_outlines[position-1] = outline;
                    if( 0 == ( m_dirtyBits & DIRTY_BOUNDS ) ) {
                        m_bbox.resize(outline.bounds());
                    }
                    // vertices.addAll(outline.getVertices()); // FIXME: can do and remove DIRTY_VERTICES ?
                    m_dirtyBits |= DIRTY_TRIANGLES | DIRTY_VERTICES | DIRTY_CONVEX;
                    return;
                }
            }
            m_outlines.insert(position, outline);
            if( 0 == ( m_dirtyBits & DIRTY_BOUNDS ) ) {
                m_bbox.resize(outline.bounds());
            }
            m_dirtyBits |= DIRTY_TRIANGLES | DIRTY_VERTICES | DIRTY_CONVEX;
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
            for(size_t i=0; i<outlineShape.outlineCount(); i++) {
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
        void setOutline(size_t position, const Outline& outline) {
            m_outlines.insert(position, outline);
            m_dirtyBits |= DIRTY_BOUNDS | DIRTY_TRIANGLES | DIRTY_VERTICES | DIRTY_CONVEX;
        }

        /**
         * Removes the {@link Outline} element at the given {@code position}.
         * <p>Sets the bounding box dirty, hence a next call to {@link #getBounds()} will validate it.</p>
         *
         * @param position of the to be removed Outline
         * @throws IndexOutOfBoundsException if position is out of range (position < 0 || position >= getOutlineNumber())
         */
        void removeOutline(size_t position) {
            m_dirtyBits |= DIRTY_BOUNDS | DIRTY_TRIANGLES | DIRTY_VERTICES | DIRTY_CONVEX;
            m_outlines.erase(position);
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
            if( 0 == ( m_dirtyBits & DIRTY_BOUNDS ) ) {
                m_bbox.resize(v.coord());
            }
            // vertices.add(v); // FIXME: can do and remove DIRTY_VERTICES ?
            m_dirtyBits |= DIRTY_TRIANGLES | DIRTY_VERTICES | DIRTY_CONVEX;
        }

        /**
         * Adds a vertex to the last open outline to the shape at {@code position}
         *
         * @param position index within the last open outline, at which the vertex will be added
         * @param v the vertex to be added to the OutlineShape
         * @see <a href="#windingrules">see winding rules</a>
         */
        void addVertex(size_t position, const Vertex& v) {
            Outline& lo = lastOutline();
            lo.addVertex(position, v);
            if( 0 == ( m_dirtyBits & DIRTY_BOUNDS ) ) {
                m_bbox.resize(v.coord());
            }
            // vertices.add(v); // FIXME: can do and remove DIRTY_VERTICES ?
            m_dirtyBits |= DIRTY_TRIANGLES | DIRTY_VERTICES | DIRTY_CONVEX;
        }

        /**
         * Add a 2D {@link Vertex} to the last open outline to the shape's tail.
         * The 2D vertex will be represented as Z=0.
         *
         * @param x the x coordinate
         * @param y the y coordniate
         * @param onCurve flag if this vertex is on the curve or defines a curved region of the shape around this vertex.
         * @see <a href="#windingrules">see winding rules</a>
         */
        void addVertex(float x, float y, bool onCurve) {
            addVertex(Vertex(x, y, onCurve));
        }

        /**
         * Add a 2D {@link Vertex} to the last open outline to the shape at {@code position}.
         * The 2D vertex will be represented as Z=0.
         *
         * @param position index within the last open outline, at which the vertex will be added
         * @param x the x coordinate
         * @param y the y coordniate
         * @param onCurve flag if this vertex is on the curve or defines a curved region of the shape around this vertex.
         * @see <a href="#windingrules">see winding rules</a>
         */
        void addVertex(size_t position, float x, float y, bool onCurve) {
            addVertex(position, Vertex(x, y, onCurve));
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
         * Add a 3D {@link Vertex} to the last open outline to the shape at {@code position}.
         *
         * @param position index within the last open outline, at which the vertex will be added
         * @param x the x coordinate
         * @param y the y coordniate
         * @param z the z coordinate
         * @param onCurve flag if this vertex is on the curve or defines a curved region of the shape around this vertex.
         * @see <a href="#windingrules">see winding rules</a>
         */
        void addVertex(size_t position, float x, float y, float z, bool onCurve) {
            addVertex(position, Vertex(x, y, z, onCurve));
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
                m_dirtyBits |= DIRTY_TRIANGLES | DIRTY_VERTICES | DIRTY_CONVEX;
            }
        }

        /**
         * Closes the current sub-path segment by drawing a straight line back to the coordinates of the last moveTo. If the path is already closed then this method has no effect.
         * @see Path2F#closePath()
         * @see #addPath(com.jogamp.math.geom.plane.Path2F.Iterator, boolean)
         */
        void closePath() {
            if ( 0 < lastOutline().vertexCount() ) {
                closeLastOutline(true);
                addEmptyOutline();
            }
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
         * Return a transformed instance with all {@link Outline}s are copied and transformed.
         * <p>
         * Note: Triangulated data is lost in returned instance!
         * </p>
         */
        OutlineShape transform(const AffineTransform& t) const {
            OutlineShape newOutlineShape;
            size_t osize = m_outlines.size();
            for(size_t i=0; i<osize; i++) {
                newOutlineShape.addOutline( m_outlines[i].transform(t) );
            }
            return newOutlineShape;
        }

        /// Returns a copy of this instance with normal() and all outlines() vertices()'s z-axis sign-flipped,
        /// used to generate a back-face from a front-face shape.
        OutlineShape flipFace() const {
            OutlineShape nshape = *this;
            nshape.normal().z *= -1;
            for(Outline& o : nshape.outlines()) {
                for(Vertex& v : o.vertices()) {
                    v.coord().z *= -1.0f;
                }
            }
            return nshape;
        }

    };


    /**@}*/

} // namespace gamp::graph

#endif /*  JAU_GAMP_GRAPH_OUTLINESHAPE_HPP_ */
