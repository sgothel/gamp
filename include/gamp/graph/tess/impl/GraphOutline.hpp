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
#ifndef JAU_GAMP_GRAPH_TESS_IMPL_GRAPHOUTLINE_HPP_
#define JAU_GAMP_GRAPH_TESS_IMPL_GRAPHOUTLINE_HPP_

#include <jau/cpp_lang_util.hpp>
#include <jau/debug.hpp>
#include <jau/int_types.hpp>
#include <jau/string_util.hpp>

#include <gamp/GampTypes.hpp>
#include <gamp/graph/Outline.hpp>
#include <gamp/graph/tess/impl/GraphVertex.hpp>
#include <memory>

namespace gamp::graph::tess::impl {

    using namespace jau::math;
    using namespace jau::math::geom;
    using namespace gamp::graph;
    using namespace gamp::graph::tess;

    /** @defgroup Gamp_GraphImpl Gamp Graph Implementation Detail
     *  Gamp Graph Implementation Detail
     *
     *  @{
     */

    class GraphOutline;
    typedef std::shared_ptr<GraphOutline> GraphOutlineRef;
    typedef jau::darray<GraphOutlineRef, uint32_t> GraphOutlineRefList;

    class GraphOutline {
      private:

        typedef std::unique_ptr<Outline, jau::OptDeleter<Outline>> OutlinePtr;
        OutlinePtr m_outline;
        GraphVertexRefList m_controlpoints;

      public:
        GraphOutline()
        : m_outline(new Outline(), jau::OptDeleter<Outline>(true)),
          m_controlpoints()
        {
            m_controlpoints.reserve(3);
        }

        /**
         * Create a control polyline of control vertices
         * the curve pieces can be identified by onCurve flag
         * of each cp the control polyline is open by default
         *
         * @param ol the source {@link Outline}
         */
        GraphOutline(Outline& ol)
        : m_outline(const_cast<Outline*>(&ol), jau::OptDeleter<Outline>(false))
        {
            VertexList& vs = ol.vertices();
            m_controlpoints.reserve(vs.size());
            for(Vertex& v : vs) {
                m_controlpoints.push_back(GraphVertex::create(v));
            }
        }

        static GraphOutlineRef create() {
            return std::make_shared<GraphOutline>();
        }
        static GraphOutlineRef create(Outline& ol) {
            return std::make_shared<GraphOutline>(ol);
        }

        const Outline& outline() const noexcept { return *m_outline; }

        const GraphVertexRefList& graphPoints() const noexcept { return m_controlpoints; }

        const VertexList& vertices() const noexcept { return m_outline->vertices(); }

        void addVertex(const GraphVertexRef& v) {
            m_controlpoints.push_back(v);
            m_outline->addVertex(v->vertex());
        }
    };

    /**@}*/

} // namespace gamp::graph::tess::impl

#endif /*  JAU_GAMP_GRAPH_TESS_IMPL_GRAPHOUTLINE_HPP_ */

