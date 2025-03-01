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

#ifndef GAMP_DEMOS_LAUNCHER01_HPP_
#define GAMP_DEMOS_LAUNCHER01_HPP_

#include <GLES3/gl32.h>
#include <gamp/gamp.hpp>

#include <cstdio>

#include <jau/basic_types.hpp>
#include <jau/debug.hpp>
#include <jau/file_util.hpp>

#include <gamp/gamp.hpp>
#include <gamp/renderer/gl/glliterals.hpp>

using namespace jau::math;
using namespace jau::math::util;

using namespace gamp::wt;
using namespace gamp::wt::event;

#if !defined(__EMSCRIPTEN__)
    // TODO: Should be moved into a GLDebugListener
    static void GLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
        (void)userParam;
        jau::PLAIN_PRINT(true, "GL-Debug: src 0x%x, type 0x%x, id 0x%x, serverity 0x%x, len %zu: %s", source, type, id, severity, length, message);
    }
#endif

struct LaunchProps {
    gamp::render::gl::GLProfileMask provileMask;
    gamp::render::gl::GLContextFlags contextFlags;
};

int launch(std::string_view sfile, const LaunchProps& props, const RenderListenerRef& demo, int argc, char *argv[]) // NOLINT(bugprone-exception-escape)
{
    std::string demo_name = std::string("Gamp ").append(jau::fs::basename(sfile, {{".cpp"}, {".hpp"}}));
    printf("Launching: %s, source %s, exe %s\n", demo_name.c_str(), sfile.data(), argv[0]);

    int win_width = 1920, win_height = 1000;
    #if defined(__EMSCRIPTEN__)
        win_width = 1024, win_height = 576; // 16:9
    #endif
    {
        for(int i=1; i<argc; ++i) {
            if( 0 == strcmp("-width", argv[i]) && i+1<argc) {
                win_width = atoi(argv[i+1]);
                ++i;
            } else if( 0 == strcmp("-height", argv[i]) && i+1<argc) {
                win_height = atoi(argv[i+1]);
                ++i;
            } else if( 0 == strcmp("-fps", argv[i]) && i+1<argc) {
                gamp::set_gpu_forced_fps( atoi(argv[i+1]) );
                ++i;
            }
        }
        printf("-fps: %d\n", gamp::gpu_forced_fps());
    }

    if( !gamp::init_gfx_subsystem(argv[0]) ) {
        printf("Exit (0)...");
        return 1;
    }
    const bool verbose = is_set(props.contextFlags, gamp::render::gl::GLContextFlags::verbose);
    WindowRef main_win = Window::create(demo_name.c_str(), win_width, win_height, verbose);
    if( !main_win ) {
        printf("Exit (1): Failed to create window.\n");
        return 1;
    }
    {
        const int w = main_win->surfaceSize().x;
        const int h = main_win->surfaceSize().y;
        const float a = (float)w / (float)h;
        printf("FB %d x %d [w x h], aspect %f [w/h]; Win %s\n", w, h, a, main_win->windowBounds().toString().c_str());
    }
    if( !main_win->createContext(props.provileMask, props.contextFlags) ) {
        printf("Exit (2): Failed to create context\n");
        main_win->dispose(jau::getMonotonicTime());
        return 1;
    }
    printf("GL Context: %s\n", main_win->renderContext().toLongString().c_str());

    #if !defined(__EMSCRIPTEN__)
        // TODO: Should be hooked with a GLDebugListener manager
        if( is_set(main_win->renderContext().contextFlags(), gamp::render::gl::GLContextFlags::debug) ) {
            ::glDebugMessageCallback(GLDebugCallback, nullptr);
            GLuint ids[] = { 0 };
            ::glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, ids, GL_TRUE);
        }
    #endif

    main_win->addRenderListener(demo);

    #if defined(__EMSCRIPTEN__)
        emscripten_set_main_loop(gamp::mainloop_default, 0, 1);
    #else
        while( true ) { gamp::mainloop_default(); }
    #endif
    return 0;
}

#endif // #define GAMP_DEMOS_LAUNCHER01_HPP_
