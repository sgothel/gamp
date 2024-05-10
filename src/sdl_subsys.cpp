/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2022 Gothel Software e.K.
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
#include "gamp/gamp.hpp"

#include <thread>

#include <GLES2/gl2.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengles2.h>
#include <SDL2/SDL_video.h>

using namespace gamp;

static SDL_Window* sdl_win = nullptr;
static Uint32 sdl_win_id = 0;
static SDL_GLContext sdl_glc = nullptr;
static SDL_Renderer* sdl_rend = nullptr;

static float gpu_fps = 0.0f;
static int gpu_frame_count = 0;
static uint64_t gpu_fps_t0 = 0;
static uint64_t gpu_swap_t0 = 0;
static uint64_t gpu_swap_t1 = 0;

jau::math::Recti gamp::viewport;

static void on_window_resized(int wwidth, int wheight) noexcept {
    int wwidth2=0, wheight2=0;
    SDL_GetWindowSize(sdl_win, &wwidth2, &wheight2);
   
    printf("Win Size %d x %d -> %d x %d (given), %d x %d (query)\n", win_width, win_height, wwidth, wheight, wwidth2, wheight2);
    if( 0 == wwidth || 0 == wheight ) {
        wwidth = wwidth2;
        wheight = wheight2;
    }
    win_width = wwidth;
    win_height = wheight;

    int fb_width = 0;
    int fb_height = 0;
    SDL_RendererInfo sdi;
    if( nullptr != sdl_rend ) {
        SDL_GetRendererOutputSize(sdl_rend, &fb_width, &fb_height);
        SDL_GetRendererInfo(sdl_rend, &sdi);
        printf("SDL Renderer %s\n", sdi.name);        
        printf("SDL Renderer Size %d x %d\n", fb_width, fb_height);
    } else {
        printf("SDL Renderer null\n");
        fb_width = static_cast<int>(static_cast<float>(wwidth) * devicePixelRatio[0]);  
        fb_height = static_cast<int>(static_cast<float>(wheight) * devicePixelRatio[1]);  
        printf("DevicePixelRatio Size %f x %f -> %d x %d\n", 
            devicePixelRatio[0], devicePixelRatio[1], fb_width, fb_height);
    }
    
    glViewport(0, 0, fb_width, fb_height);
    viewport.setWidth(fb_width); viewport.setHeight(fb_height);
    printf("VP Size %s\n", viewport.toString().c_str());    
    {
        SDL_DisplayMode mode;
        const int win_display_idx = SDL_GetWindowDisplayIndex(sdl_win);
        bzero(&mode, sizeof(mode));
        SDL_GetCurrentDisplayMode(win_display_idx, &mode); // SDL_GetWindowDisplayMode(..) fails on some systems (wrong refresh_rate and logical size
        printf("WindowDisplayMode: %d x %d @ %d Hz @ display %d\n", mode.w, mode.h, mode.refresh_rate, win_display_idx);
        display_frames_per_sec = mode.refresh_rate;
    }    
}

bool gamp::init_gfx_subsystem(const char* title, int wwidth, int wheight, bool enable_vsync) {
    if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) { // SDL_INIT_EVERYTHING
        printf("SDL: Error initializing: %s\n", SDL_GetError());
        return false;
    }
    if( enable_vsync ) {
        SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
    }
    // SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, std::to_string(gamp_filter_quality).c_str());

    const Uint32 win_flags = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL; //  | SDL_WINDOW_SHOWN;

    if( 0 != win_width && 0 != win_height ) {
        // override using pre-set default, i.e. set_window_size(..)
        wwidth = win_width; wheight = win_height;
    }
    sdl_win = SDL_CreateWindow(title,
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            wwidth, wheight,
            win_flags);
    if( nullptr == sdl_win ) {
        printf("SDL: Error initializing window: %s\n", SDL_GetError());
        return false;
    }
    
    sdl_win_id = SDL_GetWindowID(sdl_win);
    if( 0 == sdl_win_id ) {
        printf("SDL: Error retrieving window ID: %s\n", SDL_GetError());
        SDL_DestroyWindow(sdl_win);
        return false;
    }
    
    // Create OpenGLES 2 context on SDL window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetSwapInterval( enable_vsync ? 1 : 0 );
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    sdl_glc = SDL_GL_CreateContext(sdl_win);    
    if( nullptr == sdl_glc ) {
        printf("SDL: Error creating GL context: %s\n", SDL_GetError());
        SDL_DestroyWindow(sdl_win);
        return false;
    }    
    if( false ) {
    if( 0 != SDL_GL_MakeCurrent(sdl_win, sdl_glc) ) {
        printf("SDL: Error making GL context current: %s\n", SDL_GetError());
        SDL_GL_DeleteContext(sdl_glc);
        SDL_DestroyWindow(sdl_win);
        return false;        
    }
    }
    const GLubyte* gl_version = glGetString(GL_VERSION);
    if( nullptr == gl_version ) {
        printf("SDL: Error retrieving GL version: %s\n", SDL_GetError());
        SDL_GL_DeleteContext(sdl_glc);
        SDL_DestroyWindow(sdl_win);
        return false;                
    }
    printf("SDL GL context: %s\n", gl_version);
    
    // const Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
    sdl_rend = SDL_GetRenderer(sdl_win); // SDL_CreateRenderer(sdl_win, -1, render_flags);
    
    gpu_fps = 0.0f;
    gpu_fps_t0 = getCurrentMilliseconds();
    gpu_swap_t0 = gpu_fps_t0;
    gpu_swap_t1 = gpu_fps_t0;
    gpu_frame_count = 0;

    on_window_resized(wwidth, wheight);
    return true;
}

extern "C" {
    EMSCRIPTEN_KEEPALIVE void set_forced_fps(int v) noexcept { forced_fps = v; }

    EMSCRIPTEN_KEEPALIVE int get_forced_fps() noexcept { return forced_fps; }

    EMSCRIPTEN_KEEPALIVE void set_window_size(int ww, int wh, float devPixelRatioX, float devPixelRatioY) noexcept {
        static bool warn_once = true;
        if( win_width != ww || win_height != wh || 
            devicePixelRatio[0] != devPixelRatioX || devicePixelRatio[1] != devPixelRatioY) 
        {
            if( devPixelRatioX >= 0.5f && devPixelRatioY >= 0.5f ) {
                devicePixelRatio[0] = devPixelRatioX;
                devicePixelRatio[1] = devPixelRatioY;
            }
            if( std::abs(win_width - ww) > 1 || std::abs(win_height - wh) > 1 ) {
                if( 0 == win_width || 0 == win_height ) {
                    printf("JS Window Initial Size: Win %d x %d -> %d x %d, devPixelRatio %f / %f\n", 
                        win_width, win_height, ww, wh, devicePixelRatio[0], devicePixelRatio[1]);
                    win_width = ww;
                    win_height = wh;
                } else {
                    printf("JS Window Resized: Win %d x %d -> %d x %d, devPixelRatio %f / %f\n", 
                        win_width, win_height, ww, wh, devicePixelRatio[0], devicePixelRatio[1]);
                    SDL_SetWindowSize( sdl_win, ww, wh );
                    warn_once = true;
                    on_window_resized(ww, wh);
                }
            } else if( warn_once ) {
                warn_once = false;
                printf("JS Window Resize Ignored: Win %d x %d -> %d x %d, devPixelRatio %f / %f\n", 
                    win_width, win_height, ww, wh, devicePixelRatio[0], devicePixelRatio[1]);
            }
        } else {
            printf("JS Window Resize Same-Size: Win %d x %d -> %d x %d, devPixelRatio %f / %f\n", 
                 win_width, win_height, ww, wh, devicePixelRatio[0], devicePixelRatio[1]);
        }
    }
}

void gamp::swap_gpu_buffer(int fps) noexcept {
    SDL_GL_SwapWindow(sdl_win);
    gpu_swap_t0 = getCurrentMilliseconds();
    ++gpu_frame_count;
    const uint64_t td = gpu_swap_t0 - gpu_fps_t0;
    if( td >= 5000 ) {
        gpu_fps = (float)gpu_frame_count / ( (float)td / 1000.0f );
        gpu_fps_t0 = gpu_swap_t0;
        gpu_frame_count = 0;
    }
    if( 0 < fps ) {
        const int64_t fudge_ns = gamp::NanoPerMilli / 4;
        const uint64_t ms_per_frame = (uint64_t)std::round(1000.0 / fps);
        const uint64_t ms_this_frame =  gpu_swap_t0 - gpu_swap_t1;
        int64_t td_ns = int64_t( ms_per_frame - ms_this_frame ) * gamp::NanoPerMilli;
        if( td_ns > fudge_ns ) {
            const int64_t td_ns_0 = td_ns%gamp::NanoPerOne;
            struct timespec ts;
            ts.tv_sec = static_cast<decltype(ts.tv_sec)>(td_ns/gamp::NanoPerOne); // signed 32- or 64-bit integer
            ts.tv_nsec = td_ns_0 - fudge_ns;
            nanosleep( &ts, nullptr );
            // gamp::log_printf("soft-sync [exp %zd > has %zd]ms, delay %" PRIi64 "ms (%lds, %ldns)\n",
            //         ms_per_frame, ms_this_frame, td_ns/gamp::NanoPerMilli, ts.tv_sec, ts.tv_nsec);
        }
        gpu_swap_t1 = gamp::getCurrentMilliseconds();
    } else {
        gpu_swap_t1 = gpu_swap_t0;
    }
}

float gamp::get_gpu_fps() noexcept {
    return gpu_fps;
}

static input_event_type_t to_event_type(SDL_Scancode scancode) {
    switch ( scancode ) {
        case SDL_SCANCODE_ESCAPE:
            return input_event_type_t::WINDOW_CLOSE_REQ;
        case SDL_SCANCODE_P:
            return input_event_type_t::PAUSE;
        case SDL_SCANCODE_UP:
            return input_event_type_t::P1_UP;
        case SDL_SCANCODE_LEFT:
            return input_event_type_t::P1_LEFT;
        case SDL_SCANCODE_DOWN:
            return input_event_type_t::P1_DOWN;
        case SDL_SCANCODE_RIGHT:
            return input_event_type_t::P1_RIGHT;
        case SDL_SCANCODE_RSHIFT:
            return input_event_type_t::P1_ACTION1;
        case SDL_SCANCODE_RETURN:
            return input_event_type_t::P1_ACTION2;
        case SDL_SCANCODE_RALT:
            return input_event_type_t::P1_ACTION3;
            /**
        case SDL_SCANCODE_RGUI:
            return input_event_type_t::P1_ACTION4; */
        case SDL_SCANCODE_W:
            return input_event_type_t::P2_UP;
        case SDL_SCANCODE_A:
            return input_event_type_t::P2_LEFT;
        case SDL_SCANCODE_S:
            return input_event_type_t::P2_DOWN;
        case SDL_SCANCODE_D:
            return input_event_type_t::P2_RIGHT;
        case SDL_SCANCODE_LSHIFT:
            return input_event_type_t::P2_ACTION1;
        case SDL_SCANCODE_LCTRL:
            return input_event_type_t::P2_ACTION2;
        case SDL_SCANCODE_LALT:
            return input_event_type_t::P2_ACTION3;
            /**
        case SDL_SCANCODE_LGUI:
            return input_event_type_t::P2_ACTION4; */
        case SDL_SCANCODE_R:
            return input_event_type_t::RESET;
        default:
            return input_event_type_t::NONE;
    }
}
static uint16_t to_ascii(SDL_Scancode scancode) {
    if(SDL_SCANCODE_A <= scancode && scancode <= SDL_SCANCODE_Z ) {
        return 'a' + ( scancode - SDL_SCANCODE_A );
    }
    if(SDL_SCANCODE_1 <= scancode && scancode <= SDL_SCANCODE_9 ) {
        return '1' + ( scancode - SDL_SCANCODE_1 );
    }
    if(SDL_SCANCODE_0 == scancode ) {
        return '0' + ( scancode - SDL_SCANCODE_0 );
    }
    switch( scancode ) {
        case SDL_SCANCODE_SEMICOLON: return ';';

        case SDL_SCANCODE_MINUS:
            [[fallthrough]];
        case SDL_SCANCODE_KP_MINUS: return '-';

        case SDL_SCANCODE_KP_PLUS: return '+';

        case SDL_SCANCODE_KP_MULTIPLY: return '*';

        case SDL_SCANCODE_SLASH:
            [[fallthrough]];
        case SDL_SCANCODE_KP_DIVIDE: return '/';

        case SDL_SCANCODE_KP_PERCENT: return '%';

        case SDL_SCANCODE_KP_LEFTPAREN:
            [[fallthrough]];
        case SDL_SCANCODE_KP_LEFTBRACE:
            [[fallthrough]];
        case SDL_SCANCODE_LEFTBRACKET: return '(';

        case SDL_SCANCODE_KP_RIGHTPAREN:
            [[fallthrough]];
        case SDL_SCANCODE_KP_RIGHTBRACE:
            [[fallthrough]];
        case SDL_SCANCODE_RIGHTBRACKET: return ')';

        case SDL_SCANCODE_COMMA: return ',';

        case SDL_SCANCODE_PERIOD: return '.';

        case SDL_SCANCODE_SPACE:
            [[fallthrough]];
        case SDL_SCANCODE_TAB: return ' ';

        case SDL_SCANCODE_RETURN:
            [[fallthrough]];
        case SDL_SCANCODE_KP_ENTER: return '\n';

        case SDL_SCANCODE_BACKSPACE: return 0x08;

        default:
            return 0;
    }
    return 0;
}

bool gamp::handle_one_event(input_event_t& event) noexcept {
    SDL_Event sdl_event;

    if( SDL_PollEvent(&sdl_event) ) {
        switch (sdl_event.type) {
            case SDL_QUIT:
                event.set( input_event_type_t::WINDOW_CLOSE_REQ );
                printf("Window Close Requested\n");
                break;

            case SDL_WINDOWEVENT:
                switch (sdl_event.window.event) {
                    case SDL_WINDOWEVENT_SHOWN:
                        // log_printf("Window Shown\n");
                        break;
                    case SDL_WINDOWEVENT_HIDDEN:
                        // log_printf("Window Hidden\n");
                        break;
                    case SDL_WINDOWEVENT_RESIZED:
                        printf("Window Resized: %d x %d\n", sdl_event.window.data1, sdl_event.window.data2);
                        event.set(input_event_type_t::WINDOW_RESIZED);
                        on_window_resized(sdl_event.window.data1, sdl_event.window.data2);
                        break;
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        printf("Window SizeChanged: %d x %d\n", sdl_event.window.data1, sdl_event.window.data2);
                        break;
                }
                break;

                case SDL_MOUSEMOTION:
                    event.pointer_motion((int)sdl_event.motion.which,
                                         (int)sdl_event.motion.x, (int)sdl_event.motion.y);
                    break;
                case SDL_KEYUP: {
                    const SDL_Scancode scancode = sdl_event.key.keysym.scancode;
                    event.clear(to_event_type(scancode), to_ascii(scancode));
                  }
                  break;

                case SDL_KEYDOWN: {
                    const SDL_Scancode scancode = sdl_event.key.keysym.scancode;
                    event.set(to_event_type(scancode), to_ascii(scancode));
             //       printf("%d", scancode);
                  }
                  break;
        }
        return true;
    } else {
        return false;
    }
}

