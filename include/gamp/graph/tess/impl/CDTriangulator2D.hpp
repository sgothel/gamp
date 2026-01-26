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
#ifndef JAU_GAMP_GRAPH_TESS_IMPL_CDTesselator2D_HPP_
#define JAU_GAMP_GRAPH_TESS_IMPL_CDTesselator2D_HPP_

#include <gamp/GampTypes.hpp>
#include <gamp/graph/Graph.hpp>
#include <gamp/graph/Outline.hpp>
#include <gamp/graph/PrimTypes.hpp>
#include <gamp/graph/tess/impl/GraphOutline.hpp>
#include <gamp/graph/tess/impl/Loop.hpp>

#include <jau/backtrace.hpp>
#include <jau/cpp_lang_util.hpp>
#include <jau/debug.hpp>
#include <jau/int_types.hpp>
#include <jau/io/io_util.hpp>
#include <jau/math/geom/geom3f2D.hpp>
#include <jau/string_util.hpp>

namespace gamp::graph::tess::impl {

    using namespace jau::math;
    using namespace jau::math::geom;
    using namespace gamp::graph;

    /** \addtogroup Gamp_GraphImpl
     *
     *  @{
     */

    /**
     * Constrained Delaunay Triangulation
     * implementation of a list of Outlines that define a set of
     * Closed Regions with optional n holes.
     */
    class CDTriangulator2D {  // : public Triangulator {
      private:
        impl::LoopRefList loops;

        bool    complexShape;
        size_t  m_addedVerticeCount;
        int32_t m_maxTriID;

      public:
        /** Constructor for a new Delaunay triangulator
         */
        constexpr CDTriangulator2D() noexcept
        : complexShape(false) { reset(); }

        constexpr void setComplexShape(bool complex) noexcept { complexShape = complex; }

        constexpr void reset() noexcept {
            m_maxTriID          = 0;
            m_addedVerticeCount = 0;
            loops.clear();
        }

        constexpr size_t getAddedVerticeCount() const noexcept { return m_addedVerticeCount; }

        constexpr static bool FixedWindingRule = impl::Loop::FixedWindingRule;

        void addCurve(TriangleRefList& sink, Outline& polyline, float sharpness) {
            impl::LoopRef loop = getContainerLoop(polyline);

            if (!loop) {
                // HEdge.BOUNDARY -> Winding.CCW
                if constexpr (!FixedWindingRule) {
                    const Winding winding = polyline.getWinding();
                    if( Winding::CCW != winding ) {
                        jau_WARN_PRINT("CDT2.add.xx.BOUNDARY: !CCW but %s", to_string(winding).c_str());
                        // polyline.print(System.err);
                        polyline.setWinding(Winding::CCW);  // FIXME: Too late?
                    }
                }
                impl::GraphOutlineRef outline   = impl::GraphOutline::create(polyline);
                impl::GraphOutlineRef innerPoly = extractBoundaryTriangles(sink, outline, false /* hole */, sharpness);
                // vertices.addAll(polyline.getVertices());
                if (innerPoly->graphPoints().size() >= 3) {
                    loop = impl::Loop::createBoundary(innerPoly, complexShape);
                    if( loop ) {
                        loops.push_back(loop);
                    }
                } else if (Graph::DEBUG_MODE) {
                    /*
                     * Font FreeMono-Bold: ID 0 + 465: Glyph[id 465 'uni020F', advance 600, leftSideBearings 42, kerning[size 0, horiz true, cross true], shape true], OutlineShape@5e8a459[outlines 2, vertices 34]
                        Drop innerPoly ctrlpts < 3
                        - innerPo[vertices 2, ctrlpts 2] < 3
                        - outline[vertices 4, ctrlpts 4]
                        -   Input[vertices 4]
                     *
                     * Font FreeSans-Regular: ID 0 + 409: Glyph[id 409 'Udieresiscaron', advance 720, leftSideBearings 80, kerning[size 0, horiz true, cross false], shape true], OutlineShape@5eb97ced[outlines 3, vertices 33]
                        Drop innerPoly ctrlpts < 3
                        - innerPo[vertices 1, ctrlpts 1] < 3
                        - outline[vertices 1, ctrlpts 1]
                        -   Input[vertices 1]

                     * Stack:
                       at jogamp.graph.curve.tess.CDTriangulator2D.addCurve(CDTriangulator2D.java:97)
                       at com.jogamp.graph.curve.OutlineShape.triangulateImpl(OutlineShape.java:988)
                       at com.jogamp.graph.curve.OutlineShape.getTriangles(OutlineShape.java:1012)
                       at com.jogamp.graph.curve.Region.countOutlineShape(Region.java:503)
                       at com.jogamp.graph.ui.shapes.GlyphShape.<init>(GlyphShape.java:77)
                     */
                    jau_PLAIN_PRINT(true, "Drop innerPoly ctrlpts < 3");
                    // jau_PLAIN_PRINT(true, "- innerPo[vertices "+innerPoly.getOutline().getVertexCount()+", ctrlpts "+innerPoly.getGraphPoint().size()+"] < 3");
                    // jau_PLAIN_PRINT(true, "- outline[vertices "+outline.getOutline().getVertexCount()+", ctrlpts "+outline.getGraphPoint().size()+"]");
                    // jau_PLAIN_PRINT(true, "-   Input[vertices "+polyline.getVertexCount()+"]");
                    jau::print_backtrace(true, 4);
                }
            } else {
                // HEdge.HOLE -> Winding.CW, but Winding.CCW is also accepted!
                // Winding.CW not required, handled in Loop.initFromPolyline(): polyline.setWinding(winding);
                impl::GraphOutlineRef outline   = impl::GraphOutline::create(polyline);
                impl::GraphOutlineRef innerPoly = extractBoundaryTriangles(sink, outline, true /* hole */, sharpness);
                // vertices.addAll(innerPoly.getVertices());
                loop->addConstraintCurveHole(innerPoly);
            }
        }

        void generate(TriangleRefList& sink) {
            size_t loopsSize = loops.size();
            size_t size;

            for (size_t i = 0; i < loopsSize; i++) {
                impl::LoopRef &loop = loops[i];
                size_t numTries     = 0;
                size                = loop->computeLoopSize();

                while (!loop->isSimplex()) {
                    TriangleRef tri;
                    bool        delaunay;
                    if (numTries > size) {
                        tri      = loop->cut(false);
                        delaunay = false;
                    } else {
                        tri      = loop->cut(true);
                        delaunay = true;
                    }
                    numTries++;

                    if (tri) {
                        tri->id() = m_maxTriID++;
                        sink.push_back(tri);
                        // if (Graph::DEBUG_MODE ) {
                        //   jau_PLAIN_PRINT(true, "CDTri.gen["+i+"].0: delaunay "+delaunay+", tries "+numTries+", size "+size+", "+tri); // FIXME
                        // }
                        numTries = 0;
                        size--;
                    }
                    if (numTries > size * 2) {
                        // if (Graph::DEBUG_MODE ) {
                        //   jau_PLAIN_PRINT("CDTri.gen["+i+"].X: Triangulation not complete!"); // FIXME
                        // }
                        break;
                    }
                    (void)delaunay;
                }
                TriangleRef tri = loop->cut(true);
                if (tri) {
                    tri->id() = m_maxTriID++;
                    sink.push_back(tri);
                    // if (Graph::DEBUG_MODE ) {
                    //   jau_PLAIN_PRINT("CDTri.gen["+i+"].1: size "+size+"/"+loopsSize+", "+tri); // FIXME
                    // }
                }
            }
        }

      private:
        impl::GraphOutlineRef extractBoundaryTriangles(TriangleRefList& sink, const impl::GraphOutlineRef& outline, bool hole, float sharpness) {
            impl::GraphOutlineRef           innerOutline = impl::GraphOutline::create();
            const impl::GraphVertexRefList& outVertices  = outline->graphPoints();
            size_t                          size         = outVertices.size();

            for (size_t i = 0; i < size; i++) {
                const impl::GraphVertexRef &gv1 = outVertices[i];                      // currentVertex
                const impl::GraphVertexRef& gv0 = outVertices[(i + size - 1) % size];  // -1
                const impl::GraphVertexRef& gv2 = outVertices[(i + 1) % size];         // +1

                if (!gv1->vertex().onCurve()) {
                    Vertex v0 = gv0->vertex().clone();  // deep copy w/ a max id marker
                    Vertex v2            = gv2->vertex().clone();
                    Vertex v1            = gv1->vertex().clone();
                    m_addedVerticeCount += 3;
                    Triangle::tribit_t boundaryVertices;
                    boundaryVertices.put(0, true);
                    boundaryVertices.put(1, true);
                    boundaryVertices.put(2, true);

                    gv0->setBoundaryContained(true);
                    gv1->setBoundaryContained(true);
                    gv2->setBoundaryContained(true);

                    const bool holeLike = !is2DCCW(v0.coord(), v1.coord(), v2.coord());
                    if (hole || holeLike) {
                        v0.texCoord() = Vec3f(0.0f,            -0.1f, 0);
                        v2.texCoord() = Vec3f(1.0f,            -0.1f, 0);
                        v1.texCoord() = Vec3f(0.5f, -sharpness -0.1f, 0);
                        innerOutline->addVertex(gv1);
                    } else {
                        v0.texCoord() = Vec3f(0.0f,             0.1f, 0);
                        v2.texCoord() = Vec3f(1.0f,             0.1f, 0);
                        v1.texCoord() = Vec3f(0.5f, sharpness + 0.1f, 0);
                    }
                    TriangleRef t;
                    if (holeLike) {
                        t = Triangle::create(v2, v1, v0, boundaryVertices);
                    } else {
                        t = Triangle::create(v0, v1, v2, boundaryVertices);
                    }
                    t->id() = m_maxTriID++;
                    sink.push_back(t);
                    // if (Graph::DEBUG_MODE ) {
                    //   PLAIN_PRINT(true, "CDTri.ebt[%zu].0: hole %d %s, %s", i, (hole || holeLike), gv1->toString().c_str(), t->toString().c_str());
                    // }
                } else {
                    if( !gv2->vertex().onCurve() || !gv0->vertex().onCurve() ) {
                        gv1->setBoundaryContained(true);
                    }
                    innerOutline->addVertex(gv1);
                    // if (Graph::DEBUG_MODE ) {
                    //   PLAIN_PRINT(true, "CDTri.ebt[%zu].1: %s", i, gv1->toString().c_str());
                    // }
                }
            }
            return innerOutline;
        }

        impl::LoopRef getContainerLoop(const Outline& polyline) const noexcept {
            size_t count = loops.size();
            if (0 < count) {
                const VertexList &vertices = polyline.vertices();
                for (const impl::LoopRef &loop : loops) {
                    for (size_t j = 0; j < vertices.size(); ++j) {  // NOLINT
                        if (loop->checkInside(vertices[j])) {
                            return loop;
                        }
                    }
                }
            }
            return nullptr;
        }
    };

    /**@}*/

}  // namespace gamp::graph::tess::impl

#endif /*  JAU_GAMP_GRAPH_TESS_IMPL_CDTesselator2D_HPP_ */
