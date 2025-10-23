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

#include <cstdint>

#include <jau/basic_types.hpp>
#include <jau/cow_darray.hpp>
#include <jau/debug.hpp>
#include <jau/enum_util.hpp>
#include <jau/environment.hpp>
#include <jau/fraction_type.hpp>
#include <jau/math/vec2i.hpp>
#include <jau/secmem.hpp>
#include <jau/util/VersionNumber.hpp>

#include <gamp/Gamp.hpp>
#include <gamp/GampTypes.hpp>
#include "gamp/Version.hpp"
#include <gamp/render/gl/GLLiterals.hpp>
#include <gamp/render/gl/GLTypes.hpp>
#include <gamp/render/gl/GLVersionNum.hpp>
#include <gamp/wt/event/Event.hpp>
#include <gamp/wt/Window.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>

#include <SDL2/SDL_scancode.h>
// #include <SDL2/SDL_surface.h>
// #include <SDL2/SDL_ttf.h>
// #include <SDL2/SDL_image.h>

using namespace jau::int_literals;
using namespace jau::fractions_i64_literals;
using namespace jau::enums;

using namespace gamp;
using namespace gamp::wt;
using namespace gamp::wt::event;

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
static jau::cow_darray<WindowRef> window_list;

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

static void on_window_resized(Window* win, int wwidth, int wheight, const jau::fraction_timespec& when, bool verbose=false) noexcept {
    if( !win || !win->isValid() ) { return; }
    SDL_Window* sdl_win = reinterpret_cast<SDL_Window*>(win->windowHandle()); // NOLINT
    if( verbose ) {
        printf("Window::resized:: %d x %d: %s\n", wwidth, wheight, win->toString().c_str());
    }
    jau::math::Vec2i window_size = win->windowSize();
    {
        int wwidth2 = 0, wheight2 = 0;
        SDL_GetWindowSize(sdl_win, &wwidth2, &wheight2);
        if( verbose ) {
            printf("Window::resized: Size %d x %d -> %d x %d (given), %d x %d (query)\n", window_size.x, window_size.y, wwidth, wheight, wwidth2, wheight2);
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
        printf("Window::resized: DevicePixelRatio Size %f x %f -> %d x %d\n",
               devicePixelRatio.x, devicePixelRatio.y, fb_width, fb_height);
    }

    jau::math::Vec2i surface_size(fb_width, fb_height);
    if( verbose ) {
        printf("Window::resized: VP Size %s\n", surface_size.toString().c_str());
    }
    {
        SDL_DisplayMode mode;
        const int win_display_idx = SDL_GetWindowDisplayIndex(sdl_win);
        jau::zero_bytes_sec(&mode, sizeof(mode));
        SDL_GetCurrentDisplayMode(win_display_idx, &mode);  // SDL_GetWindowDisplayMode(..) fails on some systems (wrong refresh_rate and logical size
        if( verbose ) {
            printf("Window::resized: WindowDisplayMode: %d x %d @ %d Hz @ display %d\n", mode.w, mode.h, mode.refresh_rate, win_display_idx);
        }
        if( mode.refresh_rate > 0 ) {
            monitor_frames_per_sec = mode.refresh_rate;
        }
    }
    if( verbose ) {
        printf("Window::resized:: %s\n", win->toString().c_str());
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
        printf("SDL Timer-Sync:     td: %s\n", td.toString().c_str());
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

WindowRef Window::create(const char* title, int wwidth, int wheight, bool verbose) {
    if( !is_gfx_subsystem_initialized() ) {
        return nullptr;
    }
    if( verbose ) {
        printf("Window::create: '%s', %d x %d\n", title, wwidth, wheight);
    }
    // SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, std::to_string(gamp_filter_quality).c_str());

    const Uint32 win_flags = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL;  //  | SDL_WINDOW_SHOWN;

    SDL_Window* sdl_win = nullptr;
    Uint32 sdl_win_id = 0;

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 1);

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

    gpu_frame_count = 0;
    jau::math::Recti window_bounds(64, 64, wwidth, wheight);
    jau::math::Vec2i surface_size(wwidth, wheight);
    WindowRef res = Window::wrapNative((handle_t)sdl_win, window_bounds, (handle_t)sdl_win, surface_size);

    on_window_resized(res.get(), wwidth, wheight, getElapsedMonotonicTime(), verbose);

    reset_gpu_fps(gpu_forced_fps_ > 0 ? gpu_forced_fps_ : monitor_frames_per_sec);
    window_list.push_back(res);
    if( verbose ) {
        printf("Window::create: %zu windows: %s\n", (size_t)window_list.size(), res->toString().c_str());
    }
    return res;
}
void Window::disposeImpl(handle_t handle) noexcept {
    SDL_Window* sdl_win = reinterpret_cast<SDL_Window*>(handle); // NOLINT
    if( nullptr == sdl_win ) {
        return;
    }
    SDL_DestroyWindow(sdl_win);
    try {
        window_list.erase_if(false,
           [this](const WindowRef& a) noexcept -> bool { return a.get() == this; } );
    } catch (std::exception &err) {
        ERR_PRINT("gamp::handle_events: Caught exception %s", err.what());
    }
    printf("Window Closed: Remaining windows: %zu\n", (size_t)window_list.size());
}

extern "C" {
    EMSCRIPTEN_KEEPALIVE void set_forced_fps(int v) noexcept { set_gpu_forced_fps(v); }

    EMSCRIPTEN_KEEPALIVE int get_forced_fps() noexcept { return gpu_forced_fps(); }

    EMSCRIPTEN_KEEPALIVE void set_window_size(int ww, int wh, float devPixelRatioX, float devPixelRatioY) noexcept {
        jau::math::Vec2i win_size;
        WindowRef win = window_list.size() > 0 ? (*window_list.snapshot())[0] : nullptr;
        const bool initial_call = !is_gfx_subsystem_initialized() || !win || !win->isValid();
        if( win && win->isValid() ) {
            win_size = win->windowSize();
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

static bool gamp_show_fps = true;

static void gamp_swap_gpu_buffer(bool swapAllWindows, int fps) noexcept {
    if( swapAllWindows ) {
        for(const WindowRef& win : *window_list.snapshot() ) {
            if( win ) {
                win->surfaceSwap();
            }
        }
    } // else just finish
    gpu_swap_t0 = getElapsedMonotonicTime();
    ++gpu_frame_count;
    constexpr jau::fraction_timespec fps_resync(3, 0); // 3s
    constexpr jau::fraction_timespec fps_avg_period(5, 0); // 5s
    const jau::fraction_timespec td = gpu_swap_t0 - gpu_fps_t0;
    if( gpu_fps_resync && td >= fps_resync ) {
        gpu_fps_t0 = gpu_swap_t0;
        gpu_frame_count = 0;
        gpu_fps_resync = false;
        if( gamp_show_fps ) {
            jau::PLAIN_PRINT(true, "fps resync");
        }
    } else if( td >= fps_avg_period ) {
        gpu_fdur = td / gpu_frame_count;
        gpu_fps = float(double(gpu_frame_count) / ( double(td.to_us()) / 1000000.0 ));
        gpu_fps_t0 = gpu_swap_t0;
        if( gamp_show_fps ) {
            jau::PLAIN_PRINT(true, "fps(5s): fps %3.3f, dur %" PRIu64 "ms over %" PRIi64 " frames", gpu_fps, gpu_fdur.to_ms(), gpu_frame_count);
        }
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

void gamp::swap_gpu_buffer(int fps) noexcept {
    gamp_swap_gpu_buffer(true, fps);
}

float gamp::gpu_avg_fps() noexcept {
    return gpu_fps;
}

const jau::fraction_timespec& gamp::gpu_avg_framedur() noexcept {
    return gpu_fdur;
}

static std::pair<VKeyCode, InputModifier> to_VKeyCode(SDL_Scancode scancode) {
    if (SDL_SCANCODE_A <= scancode && scancode <= SDL_SCANCODE_Z) {
        return { VKeyCode(*VKeyCode::VK_A + (scancode - SDL_SCANCODE_A)), InputModifier::none};
    }
    if (SDL_SCANCODE_1 <= scancode && scancode <= SDL_SCANCODE_9) {
        return { VKeyCode(*VKeyCode::VK_1 + (scancode - SDL_SCANCODE_1)), InputModifier::none};
    }
    if (SDL_SCANCODE_0 == scancode) {
        return { VKeyCode::VK_0, InputModifier::none };
    }
    switch (scancode) {
        case SDL_SCANCODE_ESCAPE:
            return { VKeyCode::VK_ESCAPE, InputModifier::none };
        case SDL_SCANCODE_LSHIFT:
            return { VKeyCode::VK_SHIFT, InputModifier::lshift};
        case SDL_SCANCODE_RSHIFT:
            return { VKeyCode::VK_SHIFT, InputModifier::rshift};
        case SDL_SCANCODE_LALT:
            return { VKeyCode::VK_ALT, InputModifier::lalt};
        case SDL_SCANCODE_RALT:
            return { VKeyCode::VK_ALT, InputModifier::ralt};
        case SDL_SCANCODE_LCTRL:
            return { VKeyCode::VK_CONTROL, InputModifier::lctrl};
        case SDL_SCANCODE_RCTRL:
            return { VKeyCode::VK_CONTROL, InputModifier::rctrl};
        case SDL_SCANCODE_PAUSE:
            return { VKeyCode::VK_PAUSE, InputModifier::none};
        case SDL_SCANCODE_UP:
            return { VKeyCode::VK_UP, InputModifier::none};
        case SDL_SCANCODE_LEFT:
            return { VKeyCode::VK_LEFT, InputModifier::none};
        case SDL_SCANCODE_DOWN:
            return { VKeyCode::VK_DOWN, InputModifier::none};
        case SDL_SCANCODE_RIGHT:
            return { VKeyCode::VK_RIGHT, InputModifier::none};
        case SDL_SCANCODE_KP_ENTER:
            [[fallthrough]];
        case SDL_SCANCODE_RETURN:
            return { VKeyCode::VK_ENTER, InputModifier::none};

        case SDL_SCANCODE_SEMICOLON:
            return { VKeyCode::VK_SEMICOLON, InputModifier::none};

        case SDL_SCANCODE_MINUS:
            [[fallthrough]];
        case SDL_SCANCODE_KP_MINUS:
            return { VKeyCode::VK_MINUS, InputModifier::none};

        case SDL_SCANCODE_KP_PLUS:
            return { VKeyCode::VK_PLUS, InputModifier::none};

        case SDL_SCANCODE_KP_MULTIPLY:
            return { VKeyCode::VK_MULTIPLY, InputModifier::none};

        case SDL_SCANCODE_SLASH:
            [[fallthrough]];
        case SDL_SCANCODE_KP_DIVIDE:
            return { VKeyCode::VK_SLASH, InputModifier::none};

        case SDL_SCANCODE_KP_PERCENT:
            return { VKeyCode::VK_PERCENT, InputModifier::none};

        case SDL_SCANCODE_KP_LEFTPAREN:
            [[fallthrough]];
        case SDL_SCANCODE_KP_LEFTBRACE:
            [[fallthrough]];
        case SDL_SCANCODE_LEFTBRACKET:
            return { VKeyCode::VK_LEFT_PARENTHESIS, InputModifier::none};

        case SDL_SCANCODE_KP_RIGHTPAREN:
            [[fallthrough]];
        case SDL_SCANCODE_KP_RIGHTBRACE:
            [[fallthrough]];
        case SDL_SCANCODE_RIGHTBRACKET:
            return { VKeyCode::VK_RIGHT_PARENTHESIS, InputModifier::none};

        case SDL_SCANCODE_COMMA:
            return { VKeyCode::VK_COMMA, InputModifier::none};

        case SDL_SCANCODE_PERIOD:
            return { VKeyCode::VK_PERIOD, InputModifier::none};

        case SDL_SCANCODE_SPACE:
            [[fallthrough]];
        case SDL_SCANCODE_TAB:
            return { VKeyCode::VK_TAB, InputModifier::none};

        case SDL_SCANCODE_BACKSPACE:
            return { VKeyCode::VK_BACK_SPACE, InputModifier::none};


        default:
            return { VKeyCode::VK_UNDEFINED, InputModifier::none};
    }
}

static uint16_t to_ascii(SDL_Scancode scancode, const InputModifier& mods) {
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

static Window* getWin(Uint32 id) {
    SDL_Window *p = SDL_GetWindowFromID(id);
    if( !p ) {
        return nullptr;
    }
    handle_t h = reinterpret_cast<handle_t>(p);
    for(const WindowRef& win : *window_list.snapshot() ) {
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
                for(const WindowRef& win : *window_list.snapshot() ) {
                    win->dispose(when);
                }
                window_list.clear(true);
              } break;

            case SDL_WINDOWEVENT: {
                Window* win = getWin(sdl_event.window.windowID);
                if( win ) {
                    switch (sdl_event.window.event) {
                        case SDL_WINDOWEVENT_CLOSE: {
                            win->dispose(when);
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
                Window* win = getWin(sdl_event.motion.windowID);
                if( win ) {
                    int ypos = int((float)sdl_event.motion.y*devicePixelRatio.y);
                    if( win->isPointerBLOriented() ) {
                        ypos = win->surfaceSize().y - ypos - 1;
                    }
                    win->notifyPointer(EVENT_POINTER_MOVED, when, PointerType::mouse, (uint16_t)sdl_event.motion.which,
                                       jau::math::Vec2i(int((float)sdl_event.motion.x*devicePixelRatio.x), ypos),
                                       /*clickCount=*/0, InputButton::none,
                                       jau::math::Vec3f(), /*rotationScale=*/0.0f);
                } }
                break;
            case SDL_MOUSEBUTTONDOWN: {
                Window* win = getWin(sdl_event.button.windowID);
                if( win ) {
                    int ypos = int((float)sdl_event.button.y*devicePixelRatio.y);
                    if( win->isPointerBLOriented() ) {
                        ypos = win->surfaceSize().y - ypos - 1;
                    }
                    win->notifyPointer(EVENT_POINTER_PRESSED, when, PointerType::mouse, (uint16_t)0,
                                       jau::math::Vec2i(int((float)sdl_event.button.x*devicePixelRatio.x), ypos),
                                       /*clickCount=*/sdl_event.button.clicks,
                                       static_cast<InputButton>(sdl_event.button.button),
                                       jau::math::Vec3f(), /*rotationScale=*/0.0f);
                } }
                break;
            case SDL_MOUSEBUTTONUP: {
                Window* win = getWin(sdl_event.button.windowID);
                if( win ) {
                    int ypos = int((float)sdl_event.button.y*devicePixelRatio.y);
                    if( win->isPointerBLOriented() ) {
                        ypos = win->surfaceSize().y - ypos - 1;
                    }
                    win->notifyPointer(EVENT_POINTER_RELEASED, when, PointerType::mouse, (uint16_t)0,
                                       jau::math::Vec2i(int((float)sdl_event.button.x*devicePixelRatio.x), ypos),
                                       /*clickCount=*/sdl_event.button.clicks,
                                       static_cast<InputButton>(sdl_event.button.button),
                                       jau::math::Vec3f(), /*rotationScale=*/0.0f);
                } }
                break;
            case SDL_MOUSEWHEEL: {
                Window* win = getWin(sdl_event.wheel.windowID);
                if( win ) {
                    float rotX = sdl_event.wheel.preciseX;
                    float rotY = sdl_event.wheel.preciseY;
                    if( has_any(win->keyTracker().modifier(), InputModifier::shift) ) {
                        std::swap(rotX, rotY);
                    }
                    win->notifyPointer(EVENT_POINTER_WHEEL, when, PointerType::mouse, (uint16_t)sdl_event.wheel.which,
                                       jau::math::Vec2i((int)sdl_event.wheel.mouseX, (int)sdl_event.wheel.mouseY),
                                       /*clickCount=*/0,
                                       InputButton::none,
                                       jau::math::Vec3f(rotX, rotY, 0), /*rotationScale=*/0.0f);
                } }
                break;
            case SDL_KEYUP: {
                Window* win = getWin(sdl_event.key.windowID);
                if( win ) {
                    const SDL_Scancode scancode = sdl_event.key.keysym.scancode;
                    const std::pair<VKeyCode, InputModifier> r = to_VKeyCode(scancode);
                    win->notifyKeyReleased(when, r.first, r.second, to_ascii(scancode, win->keyTracker().modifier()));
                } }
                break;
            case SDL_KEYDOWN: {
                Window* win = getWin(sdl_event.key.windowID);
                if( win ) {
                    const SDL_Scancode scancode = sdl_event.key.keysym.scancode;
                    const std::pair<VKeyCode, InputModifier> r = to_VKeyCode(scancode);
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

bool gamp::mainloop_default() noexcept { // NOLINT
    gamp::handle_events();

    jau::cow_darray<WindowRef>::storage_ref_t window_refs = window_list.snapshot();
    jau::cow_darray<WindowRef>::storage_t windows = *window_refs;
    if( windows.size() == 0 ) {
        return false;
    }
    for(const WindowRef& win : windows ) {
        if( win && win->isValid() ) {
            win->display(getElapsedMonotonicTime());
        }
    }
    for(const WindowRef& win : windows ) {
        if( win ) {
            win->surfaceSwap();
        }
    }
    gamp_swap_gpu_buffer(false, gpu_forced_fps());
    return true;
}
void gamp::mainloop_void() noexcept {
    if( !mainloop_default() ) {
        printf("Exit Application\n");
        #if defined(__EMSCRIPTEN__)
            emscripten_cancel_main_loop();
        #else
            exit(0);
        #endif
    }
}

void gamp::shutdown() noexcept {
    const jau::fraction_timespec when = getElapsedMonotonicTime();
    for(const WindowRef& win : *window_list.snapshot() ) {
        win->dispose(when);
        // win->notifyWindowEvent(EVENT_WINDOW_DESTROY_NOTIFY, when);
    }
    window_list.clear(true);
}
