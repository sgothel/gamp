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
static bool SetGLAttribute(SDL_GLattr attr, int value) noexcept {
    return 0 == SDL_GL_SetAttribute(attr, value);
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

CapabilitiesPtr Surface::retrieveCaps(const SurfaceSRef& surface) noexcept {
    if (!surface || !surface->isValid()) {
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
    if (!ok) {
        return nullptr;
    }

    ok = GetGLAttribute(SDL_GL_DOUBLEBUFFER, v);
    if (ok) { caps->setDoubleBuffered(v); }

    GetGLAttribute(SDL_GL_DEPTH_SIZE, caps->depthBits());
    GetGLAttribute(SDL_GL_STENCIL_SIZE, caps->stencilBits());
    GetGLAttribute(SDL_GL_ACCUM_RED_SIZE, caps->accumRedBits());
    GetGLAttribute(SDL_GL_ACCUM_GREEN_SIZE, caps->accumGreenBits());
    GetGLAttribute(SDL_GL_ACCUM_BLUE_SIZE, caps->accumBlueBits());
    GetGLAttribute(SDL_GL_ACCUM_ALPHA_SIZE, caps->accumAlphaBits());
    ok = GetGLAttribute(SDL_GL_STEREO, v);
    if (ok) { caps->setStereo(v); }
    ok = GetGLAttribute(SDL_GL_ACCELERATED_VISUAL, v);
    if (ok) { caps->setHardwareAccelerated(v>0); }

    ok = GetGLAttribute(SDL_GL_MULTISAMPLEBUFFERS, v);
    if (ok && v > 0) {
        ok = GetGLAttribute(SDL_GL_MULTISAMPLESAMPLES, v);
        if (ok) { caps->setMultiSamplesCount(v); }
    }

    return caps;
}

void Surface::setCaps(handle_t surface_handle, const Capabilities& requested) noexcept {
    (void)surface_handle; // SDL2 uses a process static capabilities system (well)

    SetGLAttribute(SDL_GL_RED_SIZE, requested.redBits());
    SetGLAttribute(SDL_GL_GREEN_SIZE, requested.greenBits());
    SetGLAttribute(SDL_GL_BLUE_SIZE, requested.blueBits());
    SetGLAttribute(SDL_GL_ALPHA_SIZE, requested.alphaBits());

    if( requested.signature() == GLCapabilities::GLSignature() ) {
        const GLCapabilities &glcaps =  static_cast<const GLCapabilities&>(requested);
        SetGLAttribute(SDL_GL_DOUBLEBUFFER, glcaps.doubleBuffered() ? 1 : 0);

        SetGLAttribute(SDL_GL_DEPTH_SIZE, glcaps.depthBits());
        SetGLAttribute(SDL_GL_STENCIL_SIZE, glcaps.stencilBits());
        SetGLAttribute(SDL_GL_ACCUM_RED_SIZE, glcaps.accumRedBits());
        SetGLAttribute(SDL_GL_ACCUM_GREEN_SIZE, glcaps.accumGreenBits());
        SetGLAttribute(SDL_GL_ACCUM_BLUE_SIZE, glcaps.accumBlueBits());
        SetGLAttribute(SDL_GL_ACCUM_ALPHA_SIZE, glcaps.accumAlphaBits());
        SetGLAttribute(SDL_GL_STEREO, glcaps.stereo() ? 1 : 0);
        SetGLAttribute(SDL_GL_ACCELERATED_VISUAL, glcaps.hardwareAccelerated() ? 1 : 0);
        if (glcaps.hasMultiSamples()) {
            SetGLAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
            SetGLAttribute(SDL_GL_MULTISAMPLESAMPLES, glcaps.multiSamplesCount());
        } else {
            SetGLAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
            SetGLAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
        }
    }
}

GLContextSRef GLContext::create(const SurfaceSRef& surface,
                                const RenderProfile& profile,
                                RenderContextFlags contextFlags,
                                RenderContext* shareWith) noexcept {
    // const SurfaceSRef& surface, render::gl::GLProfileMask profile, render::gl::GLContextFlags contextFlags, render::gl::GL* shareWith
    if( !surface->isValid() ) {
        jau_ERR_PRINT3("Invalid surface: %s", surface->toString());
        return nullptr;
    }
    const bool verbose = is_set(contextFlags, RenderContextFlags::verbose);
    if( verbose ) {
        jau_fprintf_td(stdout, "GLContext::create: shareWith %p, profile %s, ctx %s, surface %s\n",
            shareWith, profile.toString(), contextFlags, surface->toString());
    }
    SDL_Window* sdl_win = reinterpret_cast<SDL_Window*>(surface->surfaceHandle()); // NOLINT
    // Create OpenGL context on SDL window
    // surface->setSwapIntervalImpl( surface->swapInterval() );
    {
        int ctxFlags = 0;
        if( is_set(contextFlags, RenderContextFlags::debug) ) {
            ctxFlags |= SDL_GL_CONTEXT_DEBUG_FLAG;
        }
        if( is_set(contextFlags, RenderContextFlags::robust) ) {
            ctxFlags |= SDL_GL_CONTEXT_ROBUST_ACCESS_FLAG;
        }
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, ctxFlags);
    }
    if( shareWith ) {
        // FIXME: shareWith should be current!
        SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
    }
    if( verbose ) {
        jau_fprintf_td(stdout, "GLContext::create: surface %s\n", surface->toString());
    }

    GLProfile glp_in;
    bool use_glp_core = false;
    if( GLProfile::isValid(profile) ) {
        glp_in = static_cast<const GLProfile&>(profile);
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
            jau_ERR_PRINT3("Creating %s context: %s", glp_in.toString(), SDL_GetError());
            return nullptr;
        }
        jau_fprintf_td(stdout, "GLContext::create: Failed GL ES 3 context: %s\n", SDL_GetError());
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        sdl_glc = SDL_GL_CreateContext(sdl_win);
        if (nullptr == sdl_glc) {
            jau_ERR_PRINT3("Creating GL ES 2 context: %s", SDL_GetError());
            return nullptr;
        }
    }
    if (0 != SDL_GL_MakeCurrent(sdl_win, sdl_glc)) {
        jau_ERR_PRINT3("Making GL context current: %s", SDL_GetError());
        SDL_GL_DeleteContext(sdl_glc);
        return nullptr;
    }
    const char* gl_version_cstr = reinterpret_cast<const char*>( glGetString(GL_VERSION) );
    if (nullptr == gl_version_cstr) {
        jau_ERR_PRINT3("Retrieving GL version: %s", SDL_GetError());
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
        jau_ERR_PRINT3("Retrieving GL context version information: %s", SDL_GetError());
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
    RenderContextFlags ctxFlags = RenderContextFlags::none;
    if( 0 != ( nContextFlags & SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG ) ) {
        ctxFlags |= RenderContextFlags::compatible;
    }
    if( 0 != ( nContextFlags & SDL_GL_CONTEXT_DEBUG_FLAG ) ) {
        ctxFlags |= RenderContextFlags::debug;
    }
    if( 0 != ( nContextFlags & SDL_GL_CONTEXT_ROBUST_ACCESS_FLAG ) ) {
        ctxFlags |= RenderContextFlags::robust;
    }
    if (verbose) {
        ctxFlags |= RenderContextFlags::verbose;
    }
    GLVersionNumber glv = GLVersionNumber::create(gl_version_cstr);
    GLProfile glp(glv, profileMask);
    if( verbose ) {
        GLProfile glp0(jau::util::VersionNumber(major, minor, 0), profileMask);
        jau_fprintf_td(stdout, "GLContext::create: GLProfile (query)   %s\n", glp0.toString());
        jau_fprintf_td(stdout, "GLContext::create: GLProfile (version) %s\n", glp.toString());
    }
    GLContextSRef ctx = std::make_shared<GLContext>(GLContext::Private(), surface, (handle_t)sdl_glc, std::move(glp), ctxFlags, glv); // current!
    surface->setContext(ctx);
    return ctx;
}

void Window::setRenderViewport(const jau::math::Recti &viewport) {
    ::glViewport(viewport.x(), viewport.y(), viewport.width(), viewport.height());
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
    const bool verbose = is_set(contextFlags(), RenderContextFlags::verbose);
    SDL_GLContext sdl_glc = reinterpret_cast<SDL_GLContext>(context()); // NOLINT
    if (verbose) {
        jau_fprintf_td(stdout, "GLContext::dispose: %s\n", toString());
    }
    if( sdl_glc ) {
        if( isCurrent() ) {
            releaseContext();
        }
        SDL_GL_DeleteContext(sdl_glc);
    }
    disposedNotify();
}

bool GLContext::makeCurrentImpl(const SurfaceSRef& surface, gamp::handle_t context) noexcept {
    if( !surface || !surface->isValid() || !context) {
        jau_ERR_PRINT3("Invalid surface/context: surface %s, context %p", surface ? surface->toString() : "nil", (void*)context); // NOLINT
        return false;
    }
    SDL_Window* sdl_win = reinterpret_cast<SDL_Window*>(surface->surfaceHandle()); // NOLINT
    SDL_GLContext sdl_glc = reinterpret_cast<SDL_GLContext>(context); // NOLINT
    if (0 != SDL_GL_MakeCurrent(sdl_win, sdl_glc)) {
        return false;
    }
    return true;
}

void GLContext::releaseContextImpl(const SurfaceSRef& surface) noexcept {
    if( !surface || !surface->isValid() ) {
        jau_ERR_PRINT3("Invalid surface: surface %s", surface ? surface->toString() : "nil");
        return;
    }
    SDL_Window* sdl_win = reinterpret_cast<SDL_Window*>(surface->surfaceHandle()); // NOLINT
    SDL_GL_MakeCurrent(sdl_win, nullptr);
}
