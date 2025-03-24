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
#ifndef JAU_GAMP_GRAPH_TESS_IMPL_GRAPHVERTEX_HPP_
#define JAU_GAMP_GRAPH_TESS_IMPL_GRAPHVERTEX_HPP_

#include <jau/cpp_lang_util.hpp>
#include <jau/debug.hpp>
#include <jau/int_types.hpp>
#include <jau/string_util.hpp>

#include <gamp/GampTypes.hpp>
#include <gamp/graph/PrimTypes.hpp>
#include <gamp/graph/tess/impl/HEdge.hpp>

namespace gamp::graph::tess::impl {

    using namespace jau::math;
    using namespace jau::math::geom;
    using namespace gamp::graph;
    using namespace gamp::graph::tess;

    /** \addtogroup Gamp_GraphImpl
     *
     *  @{
     */

    class GraphVertex : public std::enable_shared_from_this<GraphVertex> {
      private:
        Vertex& m_vertex;
        HEdgePtrList m_edges;
        bool m_boundaryContained = false;

        struct Private{ explicit Private() = default; };

      public:
        GraphVertex(Private, Vertex& point) noexcept
        : m_vertex(point)
        {}

        static GraphVertexRef create(Vertex& point) noexcept {
            return std::make_shared<GraphVertex>(Private(), point);
        }

        constexpr Vertex& vertex() const noexcept { return m_vertex; }
        constexpr Vec3f& coord() const noexcept { return m_vertex.coord(); }

        constexpr const HEdgePtrList& getEdges() const noexcept { return m_edges; }
        constexpr HEdgePtrList& getEdges() noexcept { return m_edges; }

        void addEdge(HEdgePtr edge) {
            m_edges.push_back(edge);
        }
        void removeEdge(const HEdgePtr& edge) noexcept {
            std::erase(m_edges, edge);
        }
        HEdgePtr findNextEdge(const GraphVertexRef& nextVert) const noexcept {
            for(HEdgePtr e : m_edges) {
                if(e->getNext()->getGraphPoint() == nextVert) {
                    return e;
                }
            }
            return nullptr;
        }
        HEdgePtr findBoundEdge() const noexcept {
            for(HEdgePtr e : m_edges) {
                if((e->getType() == HEdge::BOUNDARY) || (e->getType() == HEdge::HOLE)) {
                    return e;
                }
            }
            return nullptr;
        }
        HEdgePtr findPrevEdge(const GraphVertexRef& prevVert) const noexcept {
            for(HEdgePtr e : m_edges) {
                if(e->getPrev()->getGraphPoint() == prevVert) {
                    return e;
                }
            }
            return nullptr;
        }

        constexpr bool isBoundaryContained() const noexcept { return m_boundaryContained; }

        void setBoundaryContained(bool boundaryContained) noexcept {
            m_boundaryContained = boundaryContained;
        }

        std::string toString() noexcept {
            return std::string("GraphVertex[contained[")
              .append(jau::to_string(m_boundaryContained)).append(", ")
              .append(m_vertex.toString()).append("]");
        }
    };

    inline bool HEdge::vertexOnCurveVertex() const noexcept {
        return m_vert->vertex().onCurve();
    }

    /**@}*/

} // namespace gamp::graph::tess::impl

#endif /*  JAU_GAMP_GRAPH_TESS_IMPL_GRAPHVERTEX_HPP_ */
