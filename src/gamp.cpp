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

#include "gamp/Gamp.hpp"

#include <ctime>
#include <jau/debug.hpp>
#include <jau/environment.hpp>
#include <jau/string_util.hpp>
#include <jau/io/file_util.hpp>

#include <gamp/render/RenderContext.hpp>
#include <gamp/render/gl/glsl/ShaderCode.hpp>
#include <gamp/render/gl/glsl/ShaderState.hpp>
#include <gamp/graph/Graph.hpp>

static std::string m_asset_dir;

//
//
//

std::string gamp::lookup_and_register_asset_dir(const char* exe_path, const char* asset_file, const char* asset_install_subdir) noexcept {
    m_asset_dir = jau::io::fs::lookup_asset_dir(exe_path, asset_file, asset_install_subdir);
    return m_asset_dir;
}
std::string gamp::asset_dir() noexcept { return m_asset_dir; }

std::string gamp::resolve_asset(const std::string &asset_file, bool lookup_direct) noexcept {
    if( lookup_direct && jau::io::fs::exists(asset_file) ) {
        return asset_file;
    }
    if( m_asset_dir.size() ) {
        std::string fname1 = m_asset_dir+"/"+asset_file;
        if( jau::io::fs::exists(fname1) ) {
            return fname1;
        }
    }
    return "";
}

gamp::GampEnv::GampEnv() noexcept
: DEBUG_GLOBAL( jau::environment::get("gamp").debug ),
  exploding( true ), // jau::environment::getExplodingProperties("gamp_debug") ),
  DEBUG_WT_EVENT( jau::environment::getBooleanProperty("gamp.debug.wt.event", false) ),
  DEBUG_RENDER_GL_GLSL_CODE( jau::environment::getBooleanProperty("gamp.debug.render.gl.glsl.code", false) ),
  DEBUG_RENDER_GL_GLSL_STATE( jau::environment::getBooleanProperty("gamp.debug.render.gl.glsl.state", false) ),
  DEBUG_GRAPH_RENDER( jau::environment::getBooleanProperty("gamp.debug.graph.render", false) )
{
    jau::INFO_PRINT("GampEnv: Debug[global %d, wt.event %d, render.gl.glsl[code %d, state %d]]",
        DEBUG_GLOBAL, DEBUG_WT_EVENT, DEBUG_RENDER_GL_GLSL_CODE, DEBUG_RENDER_GL_GLSL_STATE);
}

std::string gamp::render::RenderContext::toString() const {
    return std::string("RC[")
       .append(signature().name()).append(", ")
       .append(jau::toHexString(m_context)).append(", ")
       .append(to_string(contextFlags())).append(", ")
       .append(version().toString()).append(" -> surface ")
       .append(m_surface?jau::toHexString(m_surface->surfaceHandle()):"nil").append("]");
}

bool gamp::render::gl::glsl::ShaderCode::DEBUG_CODE = GampEnv::get().DEBUG_RENDER_GL_GLSL_CODE;
bool gamp::render::gl::glsl::ShaderState::DEBUG_STATE = GampEnv::get().DEBUG_RENDER_GL_GLSL_STATE;
bool gamp::graph::Graph::DEBUG_MODE = GampEnv::get().DEBUG_GRAPH_RENDER;
