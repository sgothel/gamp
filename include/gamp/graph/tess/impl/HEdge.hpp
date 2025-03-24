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

#ifndef JAU_GAMP_GRAPH_TESS_IMPL_HEDGE_HPP_
#define JAU_GAMP_GRAPH_TESS_IMPL_HEDGE_HPP_

#include <vector>

#include <jau/cpp_lang_util.hpp>
#include <jau/debug.hpp>
#include <jau/int_types.hpp>
#include <jau/string_util.hpp>

#include <gamp/GampTypes.hpp>
#include <gamp/graph/PrimTypes.hpp>

namespace gamp::graph::tess::impl {

    using namespace jau::math;
    using namespace jau::math::geom;
    using namespace gamp::graph;
    using namespace gamp::graph::tess;

    /** \addtogroup Gamp_GraphImpl
     *
     *  @{
     */

    class GraphVertex;
    typedef std::shared_ptr<GraphVertex> GraphVertexRef;
    typedef std::vector<GraphVertexRef> GraphVertexRefList;

    class HEdge;
    typedef std::unique_ptr<HEdge> HEdgeRef; /// <Unique HEdge reference (pointer) w/ ownership, help at caller site
    typedef HEdge* HEdgePtr; /// <Plain naked HEdge pointer w/o ownership for simplification and efficiency
    typedef std::vector<HEdgePtr> HEdgePtrList;

    class HEdge {
      public:
        constexpr static int BOUNDARY = 3;
        constexpr static int INNER = 1;
        constexpr static int HOLE = 2;

      private:
        GraphVertexRef m_vert;
        HEdgePtr m_prev;
        HEdgePtr m_next;
        HEdgePtr m_sibling;
        int m_type; //  = BOUNDARY;

      public:
        HEdge(GraphVertexRef vert, int type)
        : m_vert(std::move(vert)), m_prev(nullptr), m_next(nullptr), m_sibling(nullptr),
          m_type(type)
        {}

        HEdge(GraphVertexRef vert, HEdgePtr prev, HEdgePtr next, HEdgePtr sibling, int type)
        : m_vert(std::move(vert)),
          m_prev(prev), m_next(next), m_sibling(sibling),
          m_type(type)
        { }

      public:
        static HEdgeRef create(const GraphVertexRef& vert, int type) {
            return std::make_unique<HEdge>(vert, type);
        }
        static HEdgeRef create(const GraphVertexRef& vert, HEdgePtr prev, HEdgePtr next, HEdgePtr sibling, int type) {
            return std::make_unique<HEdge>(vert, prev, next, sibling, type);
        }
        constexpr const GraphVertexRef& getGraphPoint() const noexcept { return m_vert; }
        void setVert(const GraphVertexRef& vert) noexcept { m_vert = vert; }

        constexpr HEdgePtr getPrev() noexcept { return m_prev; }
        void setPrev(HEdgePtr prev) noexcept { m_prev = prev; }

        constexpr HEdgePtr getNext() noexcept { return m_next; }
        void setNext(HEdgePtr next) noexcept { m_next = next; }

        constexpr HEdgePtr getSibling() noexcept { return m_sibling; }
        void setSibling(HEdgePtr sibling) noexcept { m_sibling = sibling; }

        constexpr int getType() const noexcept { return m_type; }

        void setType(int type) noexcept { m_type = type; }

        static void connect(HEdgePtr first, HEdgePtr next) noexcept {
            first->setNext(next);
            next->setPrev(first);
        }

        static void makeSiblings(HEdgePtr first, HEdgePtr second) noexcept {
            first->setSibling(second);
            second->setSibling(first);
        }

        std::string toString() {
            return "HEdge{this "+jau::to_hexstring(this)+", prev "+jau::to_hexstring(m_prev)+", next "+jau::to_hexstring(m_next)+"}";
        }
        void printChain() {
            int i=0;
            HEdgePtr current = this;
            HEdgePtr next = getNext();
            jau::PLAIN_PRINT(true, "HEdge[%d: root %p]", i, this);
            do {
                jau::PLAIN_PRINT(true, "HEdge[%d: current %p, next %p]", i, current, next); ++i;
                if( !next ) {
                    break;
                }
                current = next;
                next = current->getNext();

            } while(current != this);
        }

        bool vertexOnCurveVertex() const noexcept ;
    };

    /**@}*/

} // namespace gamp::graph::tess::impl

#endif /*  JAU_GAMP_GRAPH_TESS_IMPL_HEDGE_HPP_ */

