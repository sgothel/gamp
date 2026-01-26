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
#ifndef JAU_GAMP_GRAPH_TESS_IMPL_LOOP_HPP_
#define JAU_GAMP_GRAPH_TESS_IMPL_LOOP_HPP_

#include <limits>

#include <jau/cpp_lang_util.hpp>
#include <jau/debug.hpp>
#include <jau/int_types.hpp>
#include <jau/math/geom/aabbox3f.hpp>
#include <jau/math/geom/geom3f2D.hpp>
#include <jau/string_util.hpp>

#include <gamp/GampTypes.hpp>
#include <gamp/graph/Graph.hpp>
#include <gamp/graph/Outline.hpp>
#include <gamp/graph/PrimTypes.hpp>
#include <gamp/graph/tess/impl/HEdge.hpp>
#include <gamp/graph/tess/impl/GraphOutline.hpp>

namespace gamp::graph::tess::impl {

    using namespace jau::math;
    using namespace jau::math::geom;
    using namespace gamp::graph;
    using namespace gamp::graph::tess;

    /** \addtogroup Gamp_GraphImpl
     *
     *  @{
     */

    class Loop;
    typedef std::shared_ptr<Loop> LoopRef;
    typedef std::vector<LoopRef> LoopRefList;

    class Loop {
      public:
        constexpr static bool FixedWindingRule = true;

      private:
        std::vector<HEdgeRef> m_hedges;
        AABBox3f m_box;
        GraphOutlineRef m_initialOutline;
        bool m_isComplex;
        GraphOutlineRefList m_outlines;
        HEdgePtr m_root;

        struct Private{ explicit Private() = default; };

        HEdgePtr createHEdge(const GraphVertexRef& vert, int type) {
           HEdgeRef owner = HEdge::create(vert, type);
           HEdgePtr ptr = owner.get();
           m_hedges.push_back(std::move(owner));
           return ptr;
        }

      public:
        Loop(Private, GraphOutlineRef polyline, int edgeType, bool isComplex)
        : m_hedges(), m_box(),
          m_initialOutline(std::move(polyline)),
          m_isComplex(isComplex),
          m_outlines(),
          m_root(initFromPolyline(m_initialOutline, edgeType))
        { }

      private:
        /**
         * Create a connected list of half edges (loop)
         * from the boundary profile
         * @param edgeType either {@link HEdge#BOUNDARY} requiring {@link Winding#CCW} or {@link HEdge#HOLE} using {@link Winding#CW} or even {@link Winding#CCW}
         */
        HEdgePtr initFromPolyline(const GraphOutlineRef& outline, int edgeType) {
            m_outlines.push_back(outline);
            const GraphVertexRefList& vertices = outline->graphPoints();

            if(vertices.size() < 3) {
                jau_ERR_PRINT2("Graph: Loop.initFromPolyline: GraphOutline's vertices < 3: %zu", vertices.size() );
                if( Graph::DEBUG_MODE ) {
                    jau::print_backtrace(true /* skip_anon_frames */, 4 /* max_frames */);
                }
                return nullptr;
            }
            Winding edgeWinding = HEdge::BOUNDARY == edgeType ? Winding::CCW : Winding::CW;
            Winding winding = FixedWindingRule ?  edgeWinding : outline->outline().getWinding();

            if( HEdge::BOUNDARY == edgeType && Winding::CCW != winding ) {
                // XXXX
                jau_WARN_PRINT("Loop.init.xx.01: BOUNDARY req CCW but has %s", to_string(winding).c_str());
                // outline->outline().print(stderr);
                jau::print_backtrace(true /* skip_anon_frames */, 4 /* max_frames */);
            }
            HEdgePtr firstEdge = nullptr;
            HEdgePtr lastEdge = nullptr;

            if( winding == edgeWinding || HEdge::BOUNDARY == edgeType ) {
                // Correct Winding or skipped CW -> CCW (no inversion possible here, too late)
                const size_t maxVertIdx = vertices.size() - 1;
                for(size_t index = 0; index <= maxVertIdx; ++index) {
                    GraphVertexRef v1 = vertices[index];
                    m_box.resize(v1->coord());

                    HEdgePtr edge = createHEdge(v1, edgeType);
                    v1->addEdge(edge);
                    if(lastEdge) {
                        lastEdge->setNext(edge);
                        edge->setPrev(lastEdge);
                        // jau_PLAIN_PRINT(true, "initFromPoly[%zu]: lastEdge %s -> new edge %s", index, lastEdge->toString().c_str(), edge->toString().c_str());
                    } else {
                        firstEdge = edge;
                        // jau_PLAIN_PRINT(true, "initFromPoly[%zu]: new firstEdge %s", index, firstEdge->toString().c_str());
                    }
                    if(index == maxVertIdx ) {
                        edge->setNext(firstEdge);
                        firstEdge->setPrev(edge);
                        // jau_PLAIN_PRINT(true, "initFromPoly[%zu]: last new edge %s -> firstEdge %s", index, edge->toString().c_str(), firstEdge->toString().c_str());
                    }
                    lastEdge = edge;
                }
            } else { // if( winding == Winding::CW ) {
                // CCW <-> CW
                for(size_t index = vertices.size(); index-- > 0;) {
                    GraphVertexRef v1 = vertices[index];
                    m_box.resize(v1->coord());

                    HEdgePtr edge = createHEdge(v1, edgeType);
                    v1->addEdge(edge);
                    if(lastEdge) {
                        lastEdge->setNext(edge);
                        edge->setPrev(lastEdge);
                    } else {
                        firstEdge = edge;
                    }

                    if (index == 0) {
                        edge->setNext(firstEdge);
                        firstEdge->setPrev(edge);
                    }
                    lastEdge = edge;
                }
            }
            // jau_PLAIN_PRINT(true, "initFromPoly.XX: firstEdge %s", firstEdge->toString().c_str());
            // firstEdge->printChain();
            return firstEdge;
        }

        /**
         * Locates the vertex and update the loops root
         * to have (root + vertex) as closest pair
         * @param polyline the control polyline to search for closestvertices in CW
         * @return the vertex that is closest to the newly set root Hedge.
         */
        GraphVertexRef locateClosestVertex(const GraphOutlineRef& polyline) {
            float minDistance = std::numeric_limits<float>::max();
            const GraphVertexRefList& initVertices = m_initialOutline->graphPoints();
            if( initVertices.size() < 2 ) {
                return nullptr;
            }
            const GraphVertexRefList& vertices = polyline->graphPoints();
            HEdgePtr closestE = nullptr;
            GraphVertexRef closestV;

            size_t initSz = initVertices.size();
            GraphVertexRef v0 = initVertices[0];
            for(size_t i=1; i< initSz; ++i){
                GraphVertexRef v1 = initVertices[i];
                for(size_t pos=0; pos<vertices.size(); ++pos) {
                    GraphVertexRef cand = vertices[pos];
                    float distance = v0->coord().dist(cand->coord());
                    if(distance < minDistance){
                        bool inside = false;
                        for (const GraphVertexRef& vert : vertices){
                            if( !( vert == v0 || vert == v1 || vert == cand) ) {
                                inside = isInCircle2D(v0->coord(), v1->coord(), cand->coord(), vert->coord());
                                if(inside){
                                    break;
                                }
                            }
                        }
                        if(!inside){
                            closestV = cand;
                            minDistance = distance;
                            closestE = v0->findBoundEdge();
                        }
                    }
                }
                v0 = v1;
            }
            if(closestE){
                m_root = closestE;
            }
            return closestV;
        }

        bool intersectsOutline(const Vertex& a1, const Vertex& a2, const Vertex& b) const noexcept {
            for(const GraphOutlineRef &outline : m_outlines) {
                const GraphVertexRefList &vertices = outline->graphPoints();
                size_t sz = vertices.size();
                if( sz >= 2 ) {
                    const Vertex* v0 = &vertices[0]->vertex();
                    for(size_t i=1; i< sz; i++){
                        const Vertex& v1 = vertices[i]->vertex();
                        if( *v0 != b && v1 != b ) {
                            if( *v0 != a1 && v1 != a1 &&
                                testSeg2SegIntersection2D(a1.coord(), b.coord(), v0->coord(), v1.coord()) ) {
                                return true;
                            }
                            if( *v0 != a2 && v1 != a2 &&
                                testSeg2SegIntersection2D(a2.coord(), b.coord(), v0->coord(), v1.coord()) ) {
                                return true;
                            }
                        }
                        v0 = &v1;
                    }
                }
            }
            return false;
        }
        HEdgePtr isValidNeighbor(HEdgePtr candEdge, bool delaunay) const noexcept {
            const GraphVertexRef& rootGPoint = m_root->getGraphPoint();
            const GraphVertexRef& nextGPoint = m_root->getNext()->getGraphPoint();
            const Vertex& rootPoint = rootGPoint->vertex();
            const Vertex& nextPoint = nextGPoint->vertex();
            const Vertex& candPoint = candEdge->getGraphPoint()->vertex();
            if( !is2DCCW(rootPoint.coord(), nextPoint.coord(), candPoint.coord()) ||
                ( m_isComplex && intersectsOutline(rootPoint, nextPoint, candPoint) ) ) {
                return nullptr;
            }
            if( !delaunay ) {
                return candEdge;
            }
            HEdgePtr e = candEdge->getNext();
            while (e != candEdge){
                const GraphVertexRef& egp = e->getGraphPoint();
                const Vertex& ep = egp->vertex();
                if(egp != rootGPoint &&
                   egp != nextGPoint &&
                   ep != candPoint )
                {
                    if( isInCircle2D(rootPoint.coord(), nextPoint.coord(), candPoint.coord(), ep.coord()) ) {
                        return nullptr;
                    }
                }
                e = e->getNext();
            }
            return candEdge;
        }

        /** Create a triangle from the param vertices only if
         * the triangle is valid. IE not outside region.
         * @param v1 vertex 1
         * @param v2 vertex 2
         * @param v3 vertex 3
         * @param root and edge of this triangle
         * @return the triangle iff it satisfies, null otherwise
         */
        TriangleRef createTriangle(Vertex& v1, Vertex& v2, Vertex& v3, HEdgePtr rootT) const noexcept {
            return Triangle::create(v1, v2, v3, checkVerticesBoundary(rootT));
        }

        Triangle::tribit_t checkVerticesBoundary(HEdgePtr rootT) const noexcept {
            Triangle::tribit_t boundary;
            boundary.put(0, rootT->getGraphPoint()->isBoundaryContained());
            boundary.put(1, rootT->getNext()->getGraphPoint()->isBoundaryContained());
            boundary.put(2, rootT->getNext()->getNext()->getGraphPoint()->isBoundaryContained());
            return boundary;
        }

      public:
        static LoopRef createBoundary(const GraphOutlineRef& polyline, bool isConvex) {
            LoopRef res = std::make_shared<Loop>(Private(), polyline, HEdge::BOUNDARY, isConvex);
            if( !res->m_root ) {
                return nullptr;
            }
            return res;
        }

        const HEdgePtr& getHEdge() const noexcept { return m_root; }

        bool isSimplex() const noexcept {
            return (m_root->getNext()->getNext()->getNext() == m_root);
        }

        TriangleRef cut(bool delaunay) {
            HEdgePtr next1 = m_root->getNext();
            if( isSimplex() ){
                Vertex& rootPoint = m_root->getGraphPoint()->vertex();
                Vertex& nextPoint = next1->getGraphPoint()->vertex();
                Vertex& candPoint = next1->getNext()->getGraphPoint()->vertex();
                if( m_isComplex && intersectsOutline(rootPoint, nextPoint, candPoint) ) {
                    return nullptr;
                }
                return Triangle::create(rootPoint, nextPoint, candPoint, checkVerticesBoundary(m_root));
            }
            HEdgePtr prev = m_root->getPrev();
            HEdgePtr next2 = isValidNeighbor(next1->getNext(), delaunay);
            if(!next2) {
                m_root = m_root->getNext();
                return nullptr;
            }
            GraphVertexRef v1 = m_root->getGraphPoint();
            GraphVertexRef v2 = next1->getGraphPoint();
            GraphVertexRef v3 = next2->getGraphPoint();

            HEdgePtr v3Edge = createHEdge(v3, HEdge::INNER);

            HEdge::connect(v3Edge, m_root);
            HEdge::connect(next1, v3Edge);

            HEdgePtr v3EdgeSib = v3Edge->getSibling();
            if(!v3EdgeSib){
                v3EdgeSib = createHEdge(v3Edge->getNext()->getGraphPoint(), HEdge::INNER);
                HEdge::makeSiblings(v3Edge, v3EdgeSib);
            }
            HEdge::connect(prev, v3EdgeSib);
            HEdge::connect(v3EdgeSib, next2);

            TriangleRef t = createTriangle(v1->vertex(), v2->vertex(), v3->vertex(), m_root);
            m_root = next2;
            return t;
        }

        void addConstraintCurveHole(const GraphOutlineRef& polyline) {
            //        GraphOutline outline = new GraphOutline(polyline);
            /**needed to generate vertex references.*/
            if( !initFromPolyline(polyline, HEdge::HOLE) ) {
                return;
            }
            GraphVertexRef v3 = locateClosestVertex(polyline);
            if( !v3 ) {
                jau_WARN_PRINT("Graph: Loop.locateClosestVertex returns null; root valid? %d", (nullptr!=m_root));
                if( Graph::DEBUG_MODE ) {
                    jau::print_backtrace(true /* skip_anon_frames */, 4 /* max_frames */);
                }
                return;
            }
            HEdgePtr v3Edge = v3->findBoundEdge();
            HEdgePtr v3EdgeP = v3Edge->getPrev();
            HEdgePtr crossEdge = createHEdge(m_root->getGraphPoint(), HEdge::INNER);

            HEdge::connect(m_root->getPrev(), crossEdge);
            HEdge::connect(crossEdge, v3Edge);

            HEdgePtr crossEdgeSib = crossEdge->getSibling();
            if(!crossEdgeSib) {
                crossEdgeSib = createHEdge(crossEdge->getNext()->getGraphPoint(), HEdge::INNER);
                HEdge::makeSiblings(crossEdge, crossEdgeSib);
            }

            HEdge::connect(v3EdgeP, crossEdgeSib);
            HEdge::connect(crossEdgeSib, m_root);
        }

        bool checkInside(const Vertex& v) const noexcept {
            if(!m_box.contains(v.coord())){
                return false;
            }
            bool inside = false;
            HEdgePtr current = m_root;
            HEdgePtr next = m_root->getNext();
            const Vec3f& vc = v.coord();
            do {
                const Vec3f& v2c = current->getGraphPoint()->vertex().coord();
                const Vec3f& v1c = next->getGraphPoint()->vertex().coord();

                if ( ( (v1c.y > vc.y) != (v2c.y > vc.y) ) &&
                     ( vc.x < (v2c.x - v1c.x) * (vc.y - v1c.y) / (v2c.y - v1c.y) + v1c.x ) ) {
                    inside = !inside;
                }
                current = next;
                next = current->getNext();
            } while(current != m_root);
            return inside;
        }

        size_t computeLoopSize() const noexcept {
            size_t size = 0;
            HEdgePtr e = m_root;
            do{
                size++;
                e = e->getNext();
            } while(e != m_root);
            return size;
        }
    };


    /**@}*/

} // namespace gamp::graph::tess::impl

#endif /*  JAU_GAMP_GRAPH_TESS_IMPL_LOOP_HPP_ */

