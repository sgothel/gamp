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
#include <jau/file_util.hpp>
#include <jau/string_util.hpp>

#include <gamp/graph/gl/GraphRenderer.hpp>
#include <gamp/graph/gl/RenderState.hpp>
#include <gamp/render/RenderContext.hpp>
#include <gamp/render/gl/glsl/ShaderCode.hpp>
#include <gamp/render/gl/glsl/ShaderState.hpp>

#include <gamp/graph/Graph.hpp>
#include <gamp/graph/OutlineShape.hpp>
#include <gamp/graph/impl/VertexMath.hpp>

#include <gamp/graph/tess/impl/CDTriangulator2D.hpp>
#include <gamp/graph/tess/impl/HEdge.hpp>

using namespace jau::enums;
using namespace jau::math;
using jau::math::geom::Winding;
using jau::math::geom::AABBox3f;
using jau::math::geom::plane::AffineTransform;

using namespace gamp::graph;
using namespace gamp::graph::gl;

//
//
//

jau::sc_atomic_int gamp::graph::gl::RenderState::nextID;

GraphRenderer::GLCallback GraphRenderer::defaultBlendEnable = [](GL&, GraphRenderer& renderer) {
    if( renderer.renderState().hintBitsSet(RenderState::BITHINT_GLOBAL_DEPTH_TEST_ENABLED) ) {
        ::glDepthMask(false);
        // ::glDisable(GL_DEPTH_TEST);
        // ::glDepthFunc(GL_ALWAYS);
    }
    ::glEnable(GL_BLEND);
    ::glBlendEquation(GL_FUNC_ADD); // default
    renderer.renderState().setHintBits(RenderState::BITHINT_BLENDING_ENABLED);
};

GraphRenderer::GLCallback GraphRenderer::defaultBlendDisable = [](GL&, GraphRenderer& renderer) {
    renderer.renderState().clearHintBits(RenderState::BITHINT_BLENDING_ENABLED);
    ::glDisable(GL_BLEND);
    if( renderer.renderState().hintBitsSet(RenderState::BITHINT_GLOBAL_DEPTH_TEST_ENABLED) ) {
        // ::glEnable(GL_DEPTH_TEST);
        // ::glDepthFunc(GL_LESS);
        ::glDepthMask(true);
    }
};
