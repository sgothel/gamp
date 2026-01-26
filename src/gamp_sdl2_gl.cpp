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

#include <gamp/Gamp.hpp>
#include <gamp/GampTypes.hpp>
#include <gamp/render/gl/GLLiterals.hpp>
#include <gamp/render/gl/GLCapabilities.hpp>
#include <gamp/render/gl/GLContext.hpp>
#include <gamp/render/gl/GLTypes.hpp>
#include <gamp/render/gl/GLVersionNum.hpp>
#include <gamp/wt/Capabilities.hpp>
#include <gamp/wt/Window.hpp>
#include <memory>

#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>

using namespace jau::int_literals;
using namespace jau::fractions_i64_literals;
using namespace jau::enums;

using namespace gamp;
using namespace gamp::wt;
using namespace gamp::render;
using namespace gamp::render::gl;

static bool GetGLAttribute(SDL_GLattr attr, int& value) noexcept {
    return 0 == SDL_GL_GetAttribute(attr, &value);
}

bool Surface::setSwapIntervalImpl(int v) noexcept {
    if( 0 == SDL_GL_SetSwapInterval( v ) ) {
        m_swapInterval = v;
        return true;
    }
    if( -1 == v && 0 == SDL_GL_SetSwapInterval( 1 ) ) {
        m_swapInterval = 1;
        return true;
    }
    m_swapInterval = 0;
    return false;
}

CapabilitiesPtr Surface::retrieveCaps(const wt::SurfaceSRef& surface) noexcept {
    if( !surface->isValid() ) {
        return nullptr;
    }
    // SDL_Window* sdl_win = reinterpret_cast<SDL_Window*>(surface->surfaceHandle()); // NOLINT
    GLCapabilitiesPtr caps = std::make_unique<GLCapabilities>();
    bool ok;
    int v;

    ok = GetGLAttribute(SDL_GL_RED_SIZE, caps->redBits());
    ok = ok && GetGLAttribute(SDL_GL_GREEN_SIZE, caps->greenBits());
    ok = ok && GetGLAttribute(SDL_GL_BLUE_SIZE, caps->blueBits());
    ok = ok && GetGLAttribute(SDL_GL_ALPHA_SIZE, caps->alphaBits());
    if( !ok ) {
        return nullptr;
    }

    ok = GetGLAttribute(SDL_GL_DOUBLEBUFFER, v);
    if( ok ) { caps->setDoubleBuffered(v); }

    GetGLAttribute(SDL_GL_DEPTH_SIZE, caps->depthBits());
    GetGLAttribute(SDL_GL_STENCIL_SIZE, caps->stencilBits());
    GetGLAttribute(SDL_GL_ACCUM_RED_SIZE, caps->accumRedBits());
    GetGLAttribute(SDL_GL_ACCUM_GREEN_SIZE, caps->accumGreenBits());
    GetGLAttribute(SDL_GL_ACCUM_BLUE_SIZE, caps->accumBlueBits());
    GetGLAttribute(SDL_GL_ACCUM_ALPHA_SIZE, caps->accumAlphaBits());
    ok = GetGLAttribute(SDL_GL_STEREO, v);
    if( ok ) { caps->setStereo(v); }
    return caps;
}

gamp::render::RenderContextPtr Surface::createContext(const wt::SurfaceSRef& surface,
                                          const gamp::render::RenderProfile& profile,
                                          const gamp::render::RenderContextFlags& contextFlags,
                                          gamp::render::RenderContext* shareWith) noexcept {
    // const wt::SurfaceSRef& surface, render::gl::GLProfileMask profile, render::gl::GLContextFlags contextFlags, render::gl::GL* shareWith
    if( !surface->isValid() ) {
        printf("SDL: Error creating GL context: Invalid surface: %s\n", surface->toString().c_str());
        return nullptr;
    }
    const bool verbose = is_set(contextFlags, gamp::render::RenderContextFlags::verbose);
    if( verbose ) {
        printf("Surface::createContext: shareWith %p, profile %s, ctx %s, surface %s\n",
            (void*)shareWith, profile.toString().c_str(), to_string(contextFlags).c_str(), surface->toString().c_str());
    }
    SDL_Window* sdl_win = reinterpret_cast<SDL_Window*>(surface->surfaceHandle()); // NOLINT
    // Create OpenGL context on SDL window
    surface->setSwapIntervalImpl( surface->swapInterval() );
    {
        int ctxFlags = 0;
        if( is_set(contextFlags, gamp::render::RenderContextFlags::debug) ) {
            ctxFlags |= SDL_GL_CONTEXT_DEBUG_FLAG;
        }
        if( is_set(contextFlags, gamp::render::RenderContextFlags::robust) ) {
            ctxFlags |= SDL_GL_CONTEXT_ROBUST_ACCESS_FLAG;
        }
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, ctxFlags);
    }
    if( shareWith ) {
        // FIXME: shareWith should be current!
        SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
    }
    if( verbose ) {
        printf("Surface::createContext.1: surface %s\n", surface->toString().c_str());
    }

    GLProfile glp_in;
    bool use_glp_core = false;
    if( profile.signature() == GLProfile::GLSignature() ) {
        glp_in = GLProfile::downcast(profile);
        if( !glp_in.isGLES() ) {
            use_glp_core = true;
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            if( glp_in.version().major() >= 4 ) {
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
            } else if( glp_in.version().major() >= 3 ) {
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            }
        }
    }
    if( !use_glp_core ) {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    }
    SDL_GLContext sdl_glc = SDL_GL_CreateContext(sdl_win);
    if (nullptr == sdl_glc) {
        if( use_glp_core ) {
            printf("SDL: Error creating %s context: %s\n", glp_in.toString().c_str(), SDL_GetError());
            return nullptr;
        }
        printf("SDL: Error creating GL ES 3 context: %s\n", SDL_GetError());
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        sdl_glc = SDL_GL_CreateContext(sdl_win);
        if (nullptr == sdl_glc) {
            printf("SDL: Error creating GL ES 2 context: %s\n", SDL_GetError());
            return nullptr;
        }
    }
    if (0 != SDL_GL_MakeCurrent(sdl_win, sdl_glc)) {
        printf("SDL: Error making GL context current: %s\n", SDL_GetError());
        SDL_GL_DeleteContext(sdl_glc);
        return nullptr;
    }
    const char* gl_version_cstr = reinterpret_cast<const char*>( glGetString(GL_VERSION) );
    if (nullptr == gl_version_cstr) {
        printf("SDL: Error retrieving GL version: %s\n", SDL_GetError());
        SDL_GL_DeleteContext(sdl_glc);
        return nullptr;
    }
    bool ok;
    int major, minor, nContextFlags, nProfileMask;

    ok = GetGLAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major);
    ok = ok && GetGLAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor);
    ok = ok && GetGLAttribute(SDL_GL_CONTEXT_FLAGS, nContextFlags);
    ok = ok && GetGLAttribute(SDL_GL_CONTEXT_PROFILE_MASK, nProfileMask);
    if( !ok ) {
        printf("SDL: Error retrieving GL context version information: %s\n", SDL_GetError());
        SDL_GL_DeleteContext(sdl_glc);
        return nullptr;
    }
    render::gl::GLProfileMask profileMask = render::gl::GLProfileMask::none;
    if( 0 != ( nProfileMask & SDL_GL_CONTEXT_PROFILE_CORE ) ) {
        profileMask |= render::gl::GLProfileMask::core;
    } else if( 0 != ( nProfileMask & SDL_GL_CONTEXT_PROFILE_COMPATIBILITY ) ) {
        profileMask |= render::gl::GLProfileMask::compat;
    } else if( 0 != ( nProfileMask & SDL_GL_CONTEXT_PROFILE_ES ) ) {
        profileMask |= render::gl::GLProfileMask::es;
    }
    gamp::render::RenderContextFlags ctxFlags = gamp::render::RenderContextFlags::none;
    if( 0 != ( nContextFlags & SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG ) ) {
        ctxFlags |= gamp::render::RenderContextFlags::compatible;
    }
    if( 0 != ( nContextFlags & SDL_GL_CONTEXT_DEBUG_FLAG ) ) {
        ctxFlags |= gamp::render::RenderContextFlags::debug;
    }
    if( 0 != ( nContextFlags & SDL_GL_CONTEXT_ROBUST_ACCESS_FLAG ) ) {
        ctxFlags |= gamp::render::RenderContextFlags::robust;
    }
    GLVersionNumber glv = GLVersionNumber::create(gl_version_cstr);
    render::gl::GLProfile glp_out(glv, profileMask);
    if( verbose ) {
        render::gl::GLProfile glp_out0(jau::util::VersionNumber(major, minor, 0), profileMask);
        printf("Surface::createContext.2: GLProfile (query)   %s\n", glp_out0.toString().c_str());
        printf("Surface::createContext.2: GLProfile (version) %s\n", glp_out.toString().c_str());
    }
    return render::gl::GLContext::create(surface, (handle_t)sdl_glc, std::move(glp_out), ctxFlags, gl_version_cstr); // current!
}


bool Window::surfaceSwap() noexcept {
    if( isValid() ) {
        SDL_GL_SwapWindow(reinterpret_cast<SDL_Window*>(windowHandle()));  // NOLINT
        return true;
    }
    return false;
}

//
//
//

thread_local render::gl::GLContext* render::gl::GLContext::m_current = nullptr;

void render::gl::GLContext::dispose() noexcept {
    SDL_GLContext sdl_glc = reinterpret_cast<SDL_GLContext>(context()); // NOLINT
    if( sdl_glc ) {
        if( isCurrent() ) {
            releaseContext();
        }
        SDL_GL_DeleteContext(sdl_glc);
    }
    disposedNotify();
}

bool gamp::render::gl::GLContext::makeCurrentImpl(const gamp::wt::SurfaceSRef& surface, gamp::handle_t context) noexcept {
    if( !surface || !surface->isValid() || !context) {
        printf("SDL: Error GLContext::makeCurrent: Invalid surface/context: surface %s, context %p\n",
            surface ? surface->toString().c_str() : "nil", (void*)context); // NOLINT
        return false;
    }
    SDL_Window* sdl_win = reinterpret_cast<SDL_Window*>(surface->surfaceHandle()); // NOLINT
    SDL_GLContext sdl_glc = reinterpret_cast<SDL_GLContext>(context); // NOLINT
    if (0 != SDL_GL_MakeCurrent(sdl_win, sdl_glc)) {
        return false;
    }
    return true;
}

void gamp::render::gl::GLContext::releaseContextImpl(const gamp::wt::SurfaceSRef& surface) noexcept {
    if( !surface || !surface->isValid() ) {
        printf("SDL: Error GLContext::release: Invalid surface: surface %s\n",
            surface ? surface->toString().c_str() : "nil");
        return;
    }
    SDL_Window* sdl_win = reinterpret_cast<SDL_Window*>(surface->surfaceHandle()); // NOLINT
    SDL_GL_MakeCurrent(sdl_win, nullptr);
}
