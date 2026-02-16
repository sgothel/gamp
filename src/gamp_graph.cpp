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

#include <ctime>
#include <jau/debug.hpp>
#include <jau/environment.hpp>
#include <jau/string_util.hpp>
#include <jau/io/file_util.hpp>

#include <gamp/render/RenderContext.hpp>

#include <gamp/graph/Graph.hpp>
#include <gamp/graph/OutlineShape.hpp>
#include <gamp/graph/impl/VertexMath.hpp>

#include <gamp/graph/tess/impl/CDTriangulator2D.hpp>
#include <gamp/graph/tess/impl/HEdge.hpp>

using namespace jau::enums;
using namespace jau::math;
using jau::math::geom::Winding;
// using jau::math::geom::AABBox3f;
// using jau::math::geom::plane::AffineTransform;

using namespace gamp::graph;

//
//
//

Winding Outline::computeWinding() const noexcept {
    return gamp::graph::impl::getWinding( m_vertices );
}

bool Outline::computeIsComplex() const noexcept {
    return gamp::graph::impl::isConvex1(m_vertices, true);
}

//
// OutlineShape prost-processing
//

static struct SizeDescending {
    bool operator()(Outline& a, Outline& b) const {
        return a.compareTo(b) >= 1;
    }
} sizeDescending;

void OutlineShape::sortOutlines() {
    std::sort(m_outlines.begin(), m_outlines.end(), sizeDescending); // NOLINT(modernize-use-ranges)
}

uint32_t OutlineShape::generateVertexIds() {
    uint32_t maxVertexId = 0;
    for(size_type i=0; i<m_outlines.size(); ++i) {
        VertexList& vertices = outline(i).vertices();
        for(auto & vertice : vertices) {
            vertice.id() = maxVertexId++;
        }
    }
    return maxVertexId;
}

void OutlineShape::subdivideTriangle(Outline& outline, const Vertex& a, Vertex& b, const Vertex& c, size_type index) {
    Vec3f v1 = midpoint( a.coord(), b.coord() );
    Vec3f v3 = midpoint( b.coord(), c.coord() );
    Vec3f v2 = midpoint( v1, v3 );

    // COLOR
    // tmpC1.set(a.getColor()).add(b.getColor()).scale(0.5f);
    // tmpC3.set(b.getColor()).add(b.getColor()).scale(0.5f);
    // tmpC2.set(tmpC1).add(tmpC1).scale(0.5f);

    //drop off-curve vertex to image on the curve
    b.coord() = v2;
    b.onCurve() = true;

    outline.addVertex(index, Vertex(v1, false));
    outline.addVertex(index+2, Vertex(v3, false));

    m_addedVertexCount += 2;
}

Vertex* OutlineShape::checkTriOverlaps0(const Vertex& a, const Vertex& b, const Vertex& c) {
    size_type count = outlineCount();
    for (size_type cc = 0; cc < count; ++cc) {
        Outline& ol = outline(cc);
        size_type vertexCount = ol.vertexCount();
        for(size_type i=0; i < vertexCount; ++i) {
            Vertex& currV = ol.vertex(i);
            if( !currV.onCurve() && currV != a && currV != b && currV != c) {
                Vertex& nextV = ol.vertex((i+1)%vertexCount);
                Vertex& prevV = ol.vertex((i+vertexCount-1)%vertexCount);

                //skip neighboring triangles
                if(prevV != c && nextV != a) {
                    if( isInTriangle3(a.coord(), b.coord(), c.coord(),
                                      currV.coord(), nextV.coord(), prevV.coord()) ) {
                        return &currV;
                    }
                    if(gamp::graph::impl::testTri2SegIntersection2D(a, b, c, prevV, currV) ||
                       gamp::graph::impl::testTri2SegIntersection2D(a, b, c, currV, nextV) ||
                       gamp::graph::impl::testTri2SegIntersection2D(a, b, c, prevV, nextV) ) {
                        return &currV;
                    }
                }
            }
        }
    }
    return nullptr;
}

void OutlineShape::checkOverlaps() {
    VertexList overlaps(3);
    const size_type count = outlineCount();
    bool firstpass = true;
    do {
        for (size_type cc = 0; cc < count; ++cc) {
            Outline& ol = outline(cc);
            size_type vertexCount = ol.vertexCount();
            for(size_type i=0; i < ol.vertexCount(); ++i) {
                Vertex& currentVertex = ol.vertex(i);
                if ( !currentVertex.onCurve()) {
                    const Vertex& nextV = ol.vertex((i+1)%vertexCount);
                    const Vertex& prevV = ol.vertex((i+vertexCount-1)%vertexCount);
                    Vertex* overlap;

                    // check for overlap even if already set for subdivision
                    // ensuring both triangular overlaps get divided
                    // for pref. only check in first pass
                    // second pass to clear the overlaps array(reduces precision errors)
                    if( firstpass ) {
                        overlap = checkTriOverlaps0(prevV, currentVertex, nextV);
                    } else {
                        overlap = nullptr;
                    }
                    if( overlap || jau::contains(overlaps, currentVertex) ) {
                        jau::eraseFirst(overlaps, currentVertex);

                        subdivideTriangle(ol, prevV, currentVertex, nextV, i);
                        i+=3;
                        vertexCount+=2;
                        m_addedVertexCount+=2;

                        if(overlap && !overlap->onCurve()) {
                            if(!jau::contains(overlaps, *overlap)) {
                                overlaps.push_back(*overlap);
                            }
                        }
                    }
                }
            }
        }
        firstpass = false;
    } while( !overlaps.empty() );
}

void OutlineShape::cleanupOutlines() {
    const bool transformOutlines2Quadratic = VertexState::quadratic_nurbs != m_outlineState;
    size_type count = outlineCount();
    for (size_type cc = 0; cc < count; ++cc) {
        Outline& ol = outline(cc);
        size_type vertexCount = ol.vertexCount();

        if( transformOutlines2Quadratic ) {
            for(size_type i=0; i < vertexCount; ++i) {
                Vertex& currentVertex = ol.vertex(i);
                size_type j = (i+1)%vertexCount;
                Vertex& nextVertex = ol.vertex(j);
                if ( !currentVertex.onCurve() && !nextVertex.onCurve() ) {
                    Vec3f mp = midpoint(currentVertex.coord(), nextVertex.coord());
                    // System.err.println("XXX: Cubic: "+i+": "+currentVertex+", "+j+": "+nextVertex);
                    Vertex v(mp, true);
                    // COLOR: tmpC1.set(currentVertex.getColor()).add(nextVertex.getColor()).scale(0.5f)
                    ++i;
                    ++vertexCount;
                    ++m_addedVertexCount;
                    ol.addVertex(i, v);
                }
            }
        }
        if( 0 == vertexCount ) {
            jau::eraseFirst(m_outlines, ol); // empty
            --cc;
            --count;
        } else if( 0 < vertexCount &&
                   ol.vertex(0).coord() == ol.lastVertex().coord() )
        {
            ol.removeVertex(vertexCount-1); // closing vertex
        }
    }
    m_outlineState = VertexState::quadratic_nurbs;
    checkOverlaps();
}

void OutlineShape::triangulateImpl() {
    if( 0 < m_outlines.size() ) {
        sortOutlines();
        generateVertexIds();

        m_triangles.clear();
        tess::impl::CDTriangulator2D triangulator2d;
        triangulator2d.setComplexShape( isComplex() );
        for(Outline& ol : m_outlines) {
            triangulator2d.addCurve(m_triangles, ol, m_sharpness);
        }
        triangulator2d.generate(m_triangles);
        m_addedVertexCount += triangulator2d.getAddedVerticeCount();
        triangulator2d.reset();
    }
}

const TriangleRefList& OutlineShape::getTriangles(VertexState destinationType) {
    bool updated = false;
    if(destinationType != VertexState::quadratic_nurbs) {
        throw jau::IllegalStateError("destinationType "+to_string(destinationType)+" not supported (currently "+to_string(m_outlineState)+")", E_FILE_LINE);
    }
    if( is_set(m_dirtyBits, DirtyBits::triangles ) ) {
        cleanupOutlines();
        triangulateImpl();
        updated = true;
        m_dirtyBits |= DirtyBits::vertices;
        m_dirtyBits &= ~DirtyBits::triangles;
    } else {
        updated = false;
    }
    if(Graph::DEBUG_MODE) {
        jau_PLAIN_PRINT(true, "OutlineShape.getTriangles().X: %u, updated %d", m_triangles.size(), updated);
        if( updated ) {
            size_type i=0;
            for(TriangleRef& t : m_triangles) {
                jau_PLAIN_PRINT(false, "- [%u]: %s", i++, t->toString());
            }
        }
    }
    return m_triangles;
}

//
//
//

const VertexList& OutlineShape::getVertices() {
    bool updated = false;
    if( is_set(m_dirtyBits, DirtyBits::vertices ) ) {
        m_vertices.clear();
        for(const Outline& ol : m_outlines) {
            const VertexList& v = ol.vertices();
            m_vertices.push_back(v.begin(), v.end());
        }
        m_dirtyBits &= ~DirtyBits::vertices;
        updated = true;
    }
    if(Graph::DEBUG_MODE) {
        jau_PLAIN_PRINT(true, "OutlineShape.getVertices().X: %u, updated %d", m_vertices.size(), updated);
        if( updated ) {
            size_type i=0;
            for(Vertex& v : m_vertices) {
                jau_PLAIN_PRINT(false, "- [%u]: %s", i++, v);
            }
        }
    }
    return m_vertices;
}

//
//
//
