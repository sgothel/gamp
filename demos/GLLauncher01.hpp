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
#include <gamp/Gamp.hpp>

#include <cstdio>

#include <jau/basic_types.hpp>
#include <jau/debug.hpp>
#include <jau/io/file_util.hpp>

#include <gamp/Gamp.hpp>
#include <gamp/render/RenderContext.hpp>
#include <gamp/render/gl/GLLiterals.hpp>
#include <gamp/render/gl/GLContext.hpp>

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

struct GLLaunchProps {
    gamp::render::gl::GLProfile profile;
    gamp::render::RenderContextFlags contextFlags;
};

inline int launch(std::string_view sfile, GLLaunchProps props, const RenderListenerSRef& demo, int argc, char *argv[]) // NOLINT(bugprone-exception-escape)
{
    std::string demo_name = std::string("Gamp ").append(jau::io::fs::basename(sfile, {{".cpp"}, {".hpp"}}));
    std::cout << "Launching: " << demo_name << ", source " << sfile << " , exe " << argv[0] << "\n";

    #if defined(__EMSCRIPTEN__)
        int win_width = 1024, win_height = 576; // 16:9
    #else
        int win_width = 1920, win_height = 1000;
    #endif
    {
        for(int i=1; i<argc; ++i) {
            if( 0 == strcmp("-gl", argv[i]) && i+1<argc) {
                if( gamp::render::gl::GLProfile::isValidTag(argv[i+1]) ) {
                    props.profile = gamp::render::gl::GLProfile(argv[i+1]);
                }
            } else if( 0 == strcmp("-width", argv[i]) && i+1<argc) {
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
        std::cout << "-profile " << props.profile << ", context flags " << props.contextFlags << "\n";
        printf("-window %d x %d, fps %d\n", win_width, win_height, gamp::gpu_forced_fps());
    }

    if( !gamp::init_gfx_subsystem(argv[0]) ) {
        printf("Exit (0)...");
        return 1;
    }
    const bool verbose = is_set(props.contextFlags, gamp::render::RenderContextFlags::verbose);
    WindowSRef main_win = Window::create(demo_name.c_str(), win_width, win_height, verbose);
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
    // main_win->createContext(const gamp::render::RenderProfile &profile, const gamp::render::RenderContextFlags &contextFlags);

    if( !main_win->createContext(props.profile, props.contextFlags) ) {
        printf("Exit (2): Failed to create context\n");
        main_win->dispose(jau::getMonotonicTime());
        return 1;
    }
    printf("Window: %s\n", main_win->toString().c_str());
    {
        gamp::render::gl::GL& gl = gamp::render::gl::GL::downcast(main_win->renderContext());

        printf("GL Context: %s\n", gl.toString().c_str());

        #if !defined(__EMSCRIPTEN__)
            // TODO: Should be hooked with a GLDebugListener manager
            if( is_set(gl.contextFlags(), gamp::render::RenderContextFlags::debug) ) {
                ::glDebugMessageCallback(GLDebugCallback, nullptr);
                GLuint ids[] = { 0 };
                ::glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, ids, GL_TRUE);
            }
        #endif
    }
    main_win->addRenderListener(demo);

    #if defined(__EMSCRIPTEN__)
        emscripten_set_main_loop(gamp::mainloop_void, 0, 1);
    #else
        while( gamp::mainloop_default() ) { }
    #endif
    printf("Exit: %s\n", main_win->toString().c_str());
    return 0;
}

#endif // #define GAMP_DEMOS_LAUNCHER01_HPP_
