/*
 * Author: Sven Gothel <sgothel@jausoft.com> and Svenson Han Gothel
 * Copyright (c) 2022-2024 Gothel Software e.K.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <gamp/gamp.hpp>

#include <jau/basic_types.hpp>
#include <jau/cow_darray.hpp>
#include <jau/enum_util.hpp>
#include <jau/environment.hpp>
#include <jau/fraction_type.hpp>
#include <jau/math/vec2i.hpp>
#include <jau/secmem.hpp>
#include <jau/util/VersionNumber.hpp>

#include <cstdint>
#include <gamp/renderer/gl/gltypes.hpp>
#include <gamp/renderer/gl/glversionnum.hpp>

#include "gamp/version.hpp"

#include <GLES2/gl2.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengles2.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>

#include <SDL2/SDL_scancode.h>
// #include <SDL2/SDL_surface.h>
// #include <SDL2/SDL_ttf.h>
// #include <SDL2/SDL_image.h>

using namespace gamp;

static int monitor_frames_per_sec=60;
static int gpu_forced_fps_ = -1;
static bool gpu_fps_resync = true;
static float gpu_fps = 0.0f;
static jau::fraction_timespec gpu_fdur;
static int64_t gpu_frame_count = 0;
static jau::fraction_timespec gpu_fps_t0;
static jau::fraction_timespec gpu_swap_t0;
static jau::fraction_timespec gpu_swap_t1;

/** Ratio pixel-size / window-size, a DPI derivative per axis*/
static jau::math::Vec2f devicePixelRatio(1, 1);
static jau::cow_darray<gamp::wt::WindowRef> window_list;

//
/**
 * Width of the framebuffer coordinate in pixels.
 *
 * Framebuffer origin 0/0 is top-left corner.
 */
// static jau::math::Recti viewport;

static void reset_gpu_fps(int fps) {
    gpu_fps = float(fps);
    gpu_fdur = jau::fraction_timespec(1.0/double(fps));
    gpu_fps_t0 = getElapsedMonotonicTime();
    gpu_swap_t0 = gpu_fps_t0;
    gpu_swap_t1 = gpu_fps_t0;
    gpu_fps_resync = true;
}

int gamp::monitor_fps() noexcept { return monitor_frames_per_sec; }

int gamp::gpu_forced_fps() noexcept { return gpu_forced_fps_; }

void gamp::set_gpu_forced_fps(int fps) noexcept { gpu_forced_fps_=fps; reset_gpu_fps(fps); }

static void on_window_resized(gamp::wt::Window* win, int wwidth, int wheight, const jau::fraction_timespec& when, bool verbose=false) noexcept {
    if( !win || !win->isValid() ) { return; }
    SDL_Window* sdl_win = reinterpret_cast<SDL_Window*>(win->windowHandle()); // NOLINT

    jau::math::Vec2i window_size = win->getWindowSize();
    {
        int wwidth2 = 0, wheight2 = 0;
        SDL_GetWindowSize(sdl_win, &wwidth2, &wheight2);
        if( verbose ) {
            printf("Win Size %d x %d -> %d x %d (given), %d x %d (query)\n", window_size.x, window_size.y, wwidth, wheight, wwidth2, wheight2);
        }
        if (0 == wwidth || 0 == wheight) {
            wwidth = wwidth2;
            wheight = wheight2;
        }
    }
    window_size.set(wwidth, wheight);

    // printf("SDL: Couldn't fetch renderer (window resize): %s\n", SDL_GetError());
    int fb_width = static_cast<int>(static_cast<float>(wwidth) * devicePixelRatio.x);
    int fb_height = static_cast<int>(static_cast<float>(wheight) * devicePixelRatio.y);
    if( verbose ) {
        printf("DevicePixelRatio Size %f x %f -> %d x %d\n",
               devicePixelRatio.x, devicePixelRatio.y, fb_width, fb_height);
    }

    glViewport(0, 0, fb_width, fb_height);
    jau::math::Vec2i surface_size(fb_width, fb_height);
    if( verbose ) {
        printf("VP Size %s\n", surface_size.toString().c_str());
    }
    {
        SDL_DisplayMode mode;
        const int win_display_idx = SDL_GetWindowDisplayIndex(sdl_win);
        jau::zero_bytes_sec(&mode, sizeof(mode));
        SDL_GetCurrentDisplayMode(win_display_idx, &mode);  // SDL_GetWindowDisplayMode(..) fails on some systems (wrong refresh_rate and logical size
        if( verbose ) {
            printf("WindowDisplayMode: %d x %d @ %d Hz @ display %d\n", mode.w, mode.h, mode.refresh_rate, win_display_idx);
        }
        if( mode.refresh_rate > 0 ) {
            monitor_frames_per_sec = mode.refresh_rate;
        }
    }
    if( verbose ) {
        printf("Window Resized: %s\n", win->toString().c_str());
    }
    win->notifyWindowResize(when, window_size, surface_size);
}

static std::atomic_bool gfx_subsystem_init_called = false;
static std::atomic_bool gfx_subsystem_init = false;
static jau::fraction_timespec init_gfx_t0;

jau::fraction_timespec gamp::getElapsedMonotonicTime() noexcept {
    return jau::getMonotonicTime() - init_gfx_t0;
}

bool gamp::is_gfx_subsystem_initialized() noexcept {
    return gfx_subsystem_init;
}

bool gamp::init_gfx_subsystem(const char* exe_path) {
    bool exp_init_called = false;
    if( !gfx_subsystem_init_called.compare_exchange_strong(exp_init_called, true) ) {
        return gfx_subsystem_init;
    }
    lookup_and_register_asset_dir(exe_path);
    printf("Gamp API %s, lib %s\n", gamp::VERSION_API, gamp::VERSION.toString().c_str());
    printf("%s\n", jau::os::get_platform_info().c_str());

    if (SDL_Init(SDL_INIT_TIMER) != 0) {  // SDL_INIT_EVERYTHING
        printf("SDL: Error initializing TIMER: %s\n", SDL_GetError());
        return false;
    }
    {
        init_gfx_t0 = jau::getMonotonicTime();
        jau::fraction_timespec jau_t1 = getElapsedMonotonicTime();
        const uint32_t sdl_ms = SDL_GetTicks();
        const int64_t s = sdl_ms/1000;
        const int64_t ns = ( sdl_ms - s*1000 ) *1'000'000;
        const jau::fraction_timespec sdl_t1(s, ns);
        // printf("SDL Timer-Sync: jau_t0: %s\n", init_gfx_t0.to_string().c_str());
        // printf("SDL Timer-Sync: jau_t1: %s\n", jau_t1.to_string().c_str());
        // printf("SDL Timer-Sync: sdl_t1: %s\n", sdl_t1.to_string().c_str());
        const jau::fraction_timespec td = jau_t1 - sdl_t1;
        printf("SDL Timer-Sync:     td: %s\n", td.to_string().c_str());
    }
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {  // SDL_INIT_EVERYTHING
        printf("SDL: Error initializing VIDEO/EVENTS: %s\n", SDL_GetError());
        return false;
    }
#if 0
    if ( ( IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG ) != IMG_INIT_PNG ) {
        printf("SDL_image: Error initializing: %s\n", SDL_GetError());
        return false;
    }
    if( 0 != TTF_Init() ) {
        printf("SDL_TTF: Error initializing: %s\n", SDL_GetError());
        return false;
    }
#endif
    gfx_subsystem_init = true;

    return true;
}

gamp::wt::WindowRef gamp::createWindow(const char* title, int wwidth, int wheight, bool enable_vsync) {
    if( !is_gfx_subsystem_initialized() ) {
        return nullptr;
    }
    if (enable_vsync) {
        SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
    }
    // SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, std::to_string(gamp_filter_quality).c_str());

    const Uint32 win_flags = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL;  //  | SDL_WINDOW_SHOWN;

    SDL_Window* sdl_win = nullptr;
    Uint32 sdl_win_id = 0;
    SDL_GLContext sdl_glc = nullptr;

    sdl_win = SDL_CreateWindow(title,
                               SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED,
                               wwidth, wheight,
                               win_flags);
    if (nullptr == sdl_win) {
        printf("SDL: Error initializing window: %s\n", SDL_GetError());
        return nullptr;
    }

    sdl_win_id = SDL_GetWindowID(sdl_win);
    if (0 == sdl_win_id) {
        printf("SDL: Error retrieving window ID: %s\n", SDL_GetError());
        SDL_DestroyWindow(sdl_win);
        return nullptr;
    }

    // Create OpenGL ES 3 or ES 2 context on SDL window
    SDL_GL_SetSwapInterval(enable_vsync ? 1 : 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    std::string_view glp_v = render::gl::GLProfile::GLES3;
    sdl_glc = SDL_GL_CreateContext(sdl_win);
    if (nullptr == sdl_glc) {
        printf("SDL: Error creating GL ES 3 context: %s\n", SDL_GetError());
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        glp_v = render::gl::GLProfile::GLES2;
        sdl_glc = SDL_GL_CreateContext(sdl_win);
        if (nullptr == sdl_glc) {
            glp_v = render::gl::GLProfile::GL_UNDEF;
            printf("SDL: Error creating GL ES 2 context: %s\n", SDL_GetError());
            SDL_DestroyWindow(sdl_win);
            return nullptr;
        }
    }
    if (0 != SDL_GL_MakeCurrent(sdl_win, sdl_glc)) {
        printf("SDL: Error making GL context current: %s\n", SDL_GetError());
        SDL_GL_DeleteContext(sdl_glc);
        SDL_DestroyWindow(sdl_win);
        return nullptr;
    }
    const char* gl_version_cstr = reinterpret_cast<const char*>( glGetString(GL_VERSION) );
    if (nullptr == gl_version_cstr) {
        printf("SDL: Error retrieving GL version: %s\n", SDL_GetError());
        SDL_GL_DeleteContext(sdl_glc);
        SDL_DestroyWindow(sdl_win);
        return nullptr;
    }
    gamp::render::gl::GLVersionNumber gl_version = gamp::render::gl::GLVersionNumber::create(gl_version_cstr);
    render::gl::GLProfile glp(glp_v, gl_version);

    // jau::util::VersionNumber gl_version = jau::util::VersionNumber( gl_version_cstr );
    printf("SDL GL context: %s, %s\n", gl_version.toString().c_str(), glp.toString().c_str());

    gpu_frame_count = 0;
    jau::math::Recti window_bounds(64, 64, wwidth, wheight);
    jau::math::Vec2i surface_size(wwidth, wheight);
    gamp::wt::WindowRef res = gamp::wt::Window::create((handle_t)sdl_win, window_bounds, (handle_t)sdl_win, surface_size, (handle_t)sdl_glc, glp, gl_version);

    on_window_resized(res.get(), wwidth, wheight, getElapsedMonotonicTime(), true);

    reset_gpu_fps(gpu_forced_fps_ > 0 ? gpu_forced_fps_ : monitor_frames_per_sec);
    window_list.push_back(res);
    return res;
}

extern "C" {
    EMSCRIPTEN_KEEPALIVE void set_forced_fps(int v) noexcept { set_gpu_forced_fps(v); }

    EMSCRIPTEN_KEEPALIVE int get_forced_fps() noexcept { return gpu_forced_fps(); }

    EMSCRIPTEN_KEEPALIVE void set_window_size(int ww, int wh, float devPixelRatioX, float devPixelRatioY) noexcept {
        jau::math::Vec2i win_size;
        gamp::wt::WindowRef win = window_list.size() > 0 ? (*window_list.snapshot())[0] : nullptr;
        const bool initial_call = !is_gfx_subsystem_initialized() || !win || !win->isValid();
        if( win && win->isValid() ) {
            win_size = win->getWindowSize();
        }
        static bool warn_once = true;
        if (win_size.x != ww || win_size.y != wh ||
            devicePixelRatio.x != devPixelRatioX || devicePixelRatio.y != devPixelRatioY) {
            if (devPixelRatioX >= 0.5f && devPixelRatioY >= 0.5f) {
                devicePixelRatio.x = devPixelRatioX;
                devicePixelRatio.y = devPixelRatioY;
            }
            if (std::abs(win_size.x - ww) > 1 || std::abs(win_size.y - wh) > 1) {
                if( initial_call || 0 == win_size.x || 0 == win_size.y) {
                    printf("JS Window Initial Size: Win %d x %d -> %d x %d, devPixelRatio %s\n",
                           win_size.x, win_size.y, ww, wh, devicePixelRatio.toString().c_str());
                    win_size.x = ww;
                    win_size.y = wh;
                } else {
                    printf("JS Window Resized: Win %d x %d -> %d x %d, devPixelRatio %s\n",
                           win_size.x, win_size.y, ww, wh, devicePixelRatio.toString().c_str());
                    SDL_SetWindowSize(reinterpret_cast<SDL_Window*>(win->windowHandle()), ww, wh); // NOLINT
                    warn_once = true;
                    on_window_resized(win.get(), ww, wh, getElapsedMonotonicTime(), true);
                }
            } else if (warn_once) {
                warn_once = false;
                printf("JS Window Resize Ignored: Win %d x %d -> %d x %d, devPixelRatio %s\n",
                       win_size.x, win_size.y, ww, wh, devicePixelRatio.toString().c_str());
            }
        } else {
            printf("JS Window Resize Same-Size: Win %d x %d -> %d x %d, devPixelRatio %s\n",
                   win_size.x, win_size.y, ww, wh, devicePixelRatio.toString().c_str());
        }
    }
}

using namespace jau::int_literals;
using namespace jau::fractions_i64_literals;

void gamp::swap_gpu_buffer(int fps) noexcept {
    for(const gamp::wt::WindowRef& win : *window_list.snapshot() ) {
        if( win && win->isValid() ) {
            SDL_GL_SwapWindow(reinterpret_cast<SDL_Window*>(win->windowHandle())); // NOLINT
        }
    }
    gpu_swap_t0 = getElapsedMonotonicTime();
    ++gpu_frame_count;
    constexpr jau::fraction_timespec fps_resync(3, 0); // 3s
    constexpr jau::fraction_timespec fps_avg_period(5, 0); // 5s
    const jau::fraction_timespec td = gpu_swap_t0 - gpu_fps_t0;
    if( gpu_fps_resync && td >= fps_resync ) {
        gpu_fps_t0 = gpu_swap_t0;
        gpu_frame_count = 0;
        gpu_fps_resync = false;
    } else if( td >= fps_avg_period ) {
        gpu_fdur = td / gpu_frame_count;
        gpu_fps = float(double(gpu_frame_count) / ( double(td.to_us()) / 1000000.0 ));
        gpu_fps_t0 = gpu_swap_t0;
        gpu_frame_count = 0;
    }
    if( 0 < fps ) {
        constexpr uint64_t ns_per_ms = 1'000'000UL;
        const jau::fraction_timespec fudge(0, ns_per_ms / 4); // ns granularity
        const jau::fraction_timespec td_per_frame(1.0 / fps);
        const jau::fraction_timespec td_this_frame =  gpu_swap_t0 - gpu_swap_t1;
        const jau::fraction_timespec td_diff = td_per_frame - td_this_frame;
        if( td_diff > fudge ) {
            struct timespec ts;
            ts.tv_sec = static_cast<decltype(ts.tv_sec)>(td_diff.tv_sec); // signed 32- or 64-bit integer
            ts.tv_nsec = td_diff.tv_nsec - fudge.tv_nsec;
            nanosleep( &ts, nullptr );
            // pixel::log_printf("soft-sync [exp %zd > has %zd]ms, delay %" PRIi64 "ms (%lds, %ldns)\n",
            //         ms_per_frame, ms_this_frame, td_ns/pixel::NanoPerMilli, ts.tv_sec, ts.tv_nsec);
        }
        gpu_swap_t1 = getElapsedMonotonicTime();
    } else {
        gpu_swap_t1 = gpu_swap_t0;
    }
}

float gamp::gpu_avg_fps() noexcept {
    return gpu_fps;
}

const jau::fraction_timespec& gamp::gpu_avg_framedur() noexcept {
    return gpu_fdur;
}

using namespace wt::event;

static std::pair<gamp::wt::VKeyCode, InputModifier> to_VKeyCode(SDL_Scancode scancode) {
    if (SDL_SCANCODE_A <= scancode && scancode <= SDL_SCANCODE_Z) {
        return { VKeyCode(*VKeyCode::VK_A + (scancode - SDL_SCANCODE_A)), InputModifier::none};
    }
    if (SDL_SCANCODE_1 <= scancode && scancode <= SDL_SCANCODE_9) {
        return { VKeyCode(*VKeyCode::VK_1 + (scancode - SDL_SCANCODE_1)), InputModifier::none};
    }
    if (SDL_SCANCODE_0 == scancode) {
        return { gamp::wt::VKeyCode::VK_0, InputModifier::none };
    }
    switch (scancode) {
        case SDL_SCANCODE_ESCAPE:
            return { gamp::wt::VKeyCode::VK_ESCAPE, InputModifier::none };
        case SDL_SCANCODE_LSHIFT:
            return { gamp::wt::VKeyCode::VK_SHIFT, InputModifier::lshift};
        case SDL_SCANCODE_RSHIFT:
            return { gamp::wt::VKeyCode::VK_SHIFT, InputModifier::rshift};
        case SDL_SCANCODE_LALT:
            return { gamp::wt::VKeyCode::VK_ALT, InputModifier::lalt};
        case SDL_SCANCODE_RALT:
            return { gamp::wt::VKeyCode::VK_ALT, InputModifier::ralt};
        case SDL_SCANCODE_LCTRL:
            return { gamp::wt::VKeyCode::VK_CONTROL, InputModifier::lctrl};
        case SDL_SCANCODE_RCTRL:
            return { gamp::wt::VKeyCode::VK_CONTROL, InputModifier::rctrl};
        case SDL_SCANCODE_PAUSE:
            return { gamp::wt::VKeyCode::VK_PAUSE, InputModifier::none};
        case SDL_SCANCODE_UP:
            return { gamp::wt::VKeyCode::VK_UP, InputModifier::none};
        case SDL_SCANCODE_LEFT:
            return { gamp::wt::VKeyCode::VK_LEFT, InputModifier::none};
        case SDL_SCANCODE_DOWN:
            return { gamp::wt::VKeyCode::VK_DOWN, InputModifier::none};
        case SDL_SCANCODE_RIGHT:
            return { gamp::wt::VKeyCode::VK_RIGHT, InputModifier::none};
        case SDL_SCANCODE_KP_ENTER:
            [[fallthrough]];
        case SDL_SCANCODE_RETURN:
            return { gamp::wt::VKeyCode::VK_ENTER, InputModifier::none};

        case SDL_SCANCODE_SEMICOLON:
            return { gamp::wt::VKeyCode::VK_SEMICOLON, InputModifier::none};

        case SDL_SCANCODE_MINUS:
            [[fallthrough]];
        case SDL_SCANCODE_KP_MINUS:
            return { gamp::wt::VKeyCode::VK_MINUS, InputModifier::none};

        case SDL_SCANCODE_KP_PLUS:
            return { gamp::wt::VKeyCode::VK_PLUS, InputModifier::none};

        case SDL_SCANCODE_KP_MULTIPLY:
            return { gamp::wt::VKeyCode::VK_MULTIPLY, InputModifier::none};

        case SDL_SCANCODE_SLASH:
            [[fallthrough]];
        case SDL_SCANCODE_KP_DIVIDE:
            return { gamp::wt::VKeyCode::VK_SLASH, InputModifier::none};

        case SDL_SCANCODE_KP_PERCENT:
            return { gamp::wt::VKeyCode::VK_PERCENT, InputModifier::none};

        case SDL_SCANCODE_KP_LEFTPAREN:
            [[fallthrough]];
        case SDL_SCANCODE_KP_LEFTBRACE:
            [[fallthrough]];
        case SDL_SCANCODE_LEFTBRACKET:
            return { gamp::wt::VKeyCode::VK_LEFT_PARENTHESIS, InputModifier::none};

        case SDL_SCANCODE_KP_RIGHTPAREN:
            [[fallthrough]];
        case SDL_SCANCODE_KP_RIGHTBRACE:
            [[fallthrough]];
        case SDL_SCANCODE_RIGHTBRACKET:
            return { gamp::wt::VKeyCode::VK_RIGHT_PARENTHESIS, InputModifier::none};

        case SDL_SCANCODE_COMMA:
            return { gamp::wt::VKeyCode::VK_COMMA, InputModifier::none};

        case SDL_SCANCODE_PERIOD:
            return { gamp::wt::VKeyCode::VK_PERIOD, InputModifier::none};

        case SDL_SCANCODE_SPACE:
            [[fallthrough]];
        case SDL_SCANCODE_TAB:
            return { gamp::wt::VKeyCode::VK_TAB, InputModifier::none};

        case SDL_SCANCODE_BACKSPACE:
            return { gamp::wt::VKeyCode::VK_BACK_SPACE, InputModifier::none};


        default:
            return { gamp::wt::VKeyCode::VK_UNDEFINED, InputModifier::none};
    }
}

static uint16_t to_ascii(SDL_Scancode scancode, const gamp::wt::InputModifier& mods) {
    if (SDL_SCANCODE_A <= scancode && scancode <= SDL_SCANCODE_Z) {
        if( has_any(mods, InputModifier::shift) ) {
            return 'A' + (scancode - SDL_SCANCODE_A);
        } else {
            return 'a' + (scancode - SDL_SCANCODE_A);
        }
    }
    if (SDL_SCANCODE_1 <= scancode && scancode <= SDL_SCANCODE_9) {
        return '1' + (scancode - SDL_SCANCODE_1);
    }
    if (SDL_SCANCODE_0 == scancode) {
        return '0' + (scancode - SDL_SCANCODE_0);
    }
    switch (scancode) {
        case SDL_SCANCODE_SEMICOLON:
            return ';';

        case SDL_SCANCODE_MINUS:
            [[fallthrough]];
        case SDL_SCANCODE_KP_MINUS:
            return '-';

        case SDL_SCANCODE_KP_PLUS:
            return '+';

        case SDL_SCANCODE_KP_MULTIPLY:
            return '*';

        case SDL_SCANCODE_SLASH:
            [[fallthrough]];
        case SDL_SCANCODE_KP_DIVIDE:
            return '/';

        case SDL_SCANCODE_KP_PERCENT:
            return '%';

        case SDL_SCANCODE_KP_LEFTPAREN:
            [[fallthrough]];
        case SDL_SCANCODE_KP_LEFTBRACE:
            [[fallthrough]];
        case SDL_SCANCODE_LEFTBRACKET:
            return '(';

        case SDL_SCANCODE_KP_RIGHTPAREN:
            [[fallthrough]];
        case SDL_SCANCODE_KP_RIGHTBRACE:
            [[fallthrough]];
        case SDL_SCANCODE_RIGHTBRACKET:
            return ')';

        case SDL_SCANCODE_COMMA:
            return ',';

        case SDL_SCANCODE_PERIOD:
            return '.';

        case SDL_SCANCODE_SPACE:
            [[fallthrough]];
        case SDL_SCANCODE_TAB:
            return ' ';

        case SDL_SCANCODE_RETURN:
            [[fallthrough]];
        case SDL_SCANCODE_KP_ENTER:
            return '\n';

        case SDL_SCANCODE_BACKSPACE:
            return 0x08;

        default:
            return 0;
    }
    return 0;
}

static gamp::wt::Window* getWin(Uint32 id) {
    SDL_Window *p = SDL_GetWindowFromID(id);
    if( !p ) {
        return nullptr;
    }
    handle_t h = reinterpret_cast<handle_t>(p);
    for(const gamp::wt::WindowRef& win : *window_list.snapshot() ) {
        if( win && win->windowHandle() == h ) {
            return win.get();
        }
    }
    return nullptr;
}

size_t gamp::handle_events() noexcept {
    if( !is_gfx_subsystem_initialized() ) {
        return 0;
    }
    size_t event_count = 0;
    SDL_Event sdl_event;

    while (SDL_PollEvent(&sdl_event)) {
        ++event_count;
        const int64_t s = sdl_event.common.timestamp/1000;
        const int64_t ns = ( sdl_event.common.timestamp - s*1000 ) *1'000'000;
        const jau::fraction_timespec when(s, ns);
        switch (sdl_event.type) {
            case SDL_QUIT: {
                printf("App Close Requested\n");
                for(const gamp::wt::WindowRef& win : *window_list.snapshot() ) {
                    win->notifyWindowEvent(EVENT_WINDOW_DESTROY_NOTIFY, when);
                }
                window_list.clear(true);
              } break;

            case SDL_WINDOWEVENT: {
                gamp::wt::Window* win = getWin(sdl_event.window.windowID);
                if( win ) {
                    switch (sdl_event.window.event) {
                        case SDL_WINDOWEVENT_CLOSE: {
                            win->notifyWindowEvent(EVENT_WINDOW_DESTROY_NOTIFY, when);
                            try {
                                window_list.erase_if(false,
                                   [win](const gamp::wt::WindowRef& a) noexcept -> bool { return a.get() == win; } );
                            } catch (std::exception &err) {
                                ERR_PRINT("gamp::handle_events: Caught exception %s", err.what());
                            }
                            printf("Window Close Requested: %zu windows\n", (size_t)window_list.size());
                          } break;
                        case SDL_WINDOWEVENT_SHOWN:
                            printf("Window Shown\n");
                            win->notifyWindowEvent(EVENT_WINDOW_VISIBILITY_CHANGED, when, true);
                            break;
                        case SDL_WINDOWEVENT_HIDDEN:
                            printf("Window Hidden\n");
                            win->notifyWindowEvent(EVENT_WINDOW_VISIBILITY_CHANGED, when, false);
                            break;
                        case SDL_WINDOWEVENT_MOVED:
                            printf("Window Moved: %d x %d\n", sdl_event.window.data1, sdl_event.window.data2);
                            win->notifyWindowMoved(when, jau::math::Vec2i(sdl_event.window.data1, sdl_event.window.data2));
                            break;
                        case SDL_WINDOWEVENT_RESIZED:
                            printf("Window Resized: %d x %d\n", sdl_event.window.data1, sdl_event.window.data2);
                            on_window_resized(win, sdl_event.window.data1, sdl_event.window.data2, when);
                            break;
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                            printf("Window SizeChanged: %d x %d\n", sdl_event.window.data1, sdl_event.window.data2);
                            break;

                        default: break;
                    }
                } }
                break;

            case SDL_MOUSEMOTION: {
                gamp::wt::Window* win = getWin(sdl_event.motion.windowID);
                if( win ) {
                    // event.pointer_motion((int)sdl_event.motion.which,
                    //                     (int)sdl_event.motion.x, (int)sdl_event.motion.y);
                } }
                break;
            case SDL_KEYUP: {
                gamp::wt::Window* win = getWin(sdl_event.key.windowID);
                if( win ) {
                    const SDL_Scancode scancode = sdl_event.key.keysym.scancode;
                    const std::pair<gamp::wt::VKeyCode, InputModifier> r = to_VKeyCode(scancode);
                    win->notifyKeyReleased(when, r.first, r.second, to_ascii(scancode, win->keyTracker().modifier()));
                } }
                break;
            case SDL_KEYDOWN: {
                gamp::wt::Window* win = getWin(sdl_event.key.windowID);
                if( win ) {
                    const SDL_Scancode scancode = sdl_event.key.keysym.scancode;
                    const std::pair<gamp::wt::VKeyCode, InputModifier> r = to_VKeyCode(scancode);
                    if( sdl_event.key.repeat ) {
                        win->notifyKeyReleased(when, r.first, r.second | InputModifier::repeat, to_ascii(scancode, win->keyTracker().modifier()));
                        win->notifyKeyPressed (when, r.first, r.second | InputModifier::repeat, to_ascii(scancode, win->keyTracker().modifier()));
                    } else {
                        win->notifyKeyPressed(when, r.first, r.second, to_ascii(scancode, win->keyTracker().modifier()));
                    }
                } }
                break;
            default: break;
        }
    }
    return event_count;
}

void gamp::mainloop_default() noexcept {
    gamp::handle_events();
    if( window_list.size() == 0 ) {
        printf("Exit Application\n");
        #if defined(__EMSCRIPTEN__)
            emscripten_cancel_main_loop();
        #else
            exit(0);
        #endif
    }
    for(const gamp::wt::WindowRef& win : *window_list.snapshot() ) {
        win->display(getElapsedMonotonicTime());
    }
    gamp::swap_gpu_buffer();
}

void gamp::shutdown() noexcept {
    const jau::fraction_timespec when = getElapsedMonotonicTime();
    for(const gamp::wt::WindowRef& win : *window_list.snapshot() ) {
        win->notifyWindowEvent(EVENT_WINDOW_DESTROY_NOTIFY, when);
    }
    window_list.clear(true);
}
