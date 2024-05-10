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
#ifndef GAMP_HPP_
#define GAMP_HPP_

#include <cinttypes>
#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>
#include <iostream>
#include <cctype>

#include <jau/math/vec2f.hpp>
#include <jau/math/vec3f.hpp>
#include <jau/math/vec4f.hpp>
#include <jau/math/mat4f.hpp>
#include <jau/math/recti.hpp>

#include <jau/math/util/pmvmat4f.hpp>

#if defined(__EMSCRIPTEN__)
    #include <emscripten.h>
#else
    #define EMSCRIPTEN_KEEPALIVE
#endif

/**
 * Basic computer graphics math and utilities helping with the framebuffer and I/O tooling.
 */
namespace gamp {
    inline constexpr float epsilon() noexcept {
        float a = 1.0f;
        float b;
        do {
            b = a;
            a = a / 2.0f;
        } while(1.0f + a != 1.0f);
        return b;
    }

    /** Returns true of the given float is less than float epsilon. */
    inline constexpr bool is_zero(const float v) noexcept {
        return std::abs(v) < std::numeric_limits<float>::epsilon();
    }
    /** Returns true of the given double  is less than double epsilon. */
    inline constexpr bool is_zero(const double v) noexcept {
        return std::abs(v) < std::numeric_limits<double>::epsilon();
    }

    /**
     * Return true if both values are equal, i.e. their absolute delta is less than float epsilon,
     * false otherwise.
     */
    inline constexpr bool equals(const float a, const float b) noexcept {
        return std::abs(a - b) < std::numeric_limits<float>::epsilon();
    }

    /**
     * Return zero if both values are equal, i.e. their absolute delta is less than float epsilon,
     * -1 if a < b and 1 otherwise.
     */
    inline constexpr int compare(const float a, const float b) noexcept {
        if( std::abs(a - b) < std::numeric_limits<float>::epsilon() ) {
            return 0;
        } else if (a < b) {
            return -1;
        } else {
            return 1;
        }
    }

    /** Returns the rounded float value cast to int. */
    inline constexpr int round_to_int(const float v) noexcept {
        return (int)std::round(v);
    }
    /** Returns the rounded double value cast to int. */
    inline constexpr int round_to_int(const double v) noexcept {
        return (int)std::round(v);
    }

    /** Converts arc-degree to radians */
    inline constexpr float adeg_to_rad(const float arc_degree) noexcept {
        return arc_degree * (float)M_PI / 180.0f;
    }

    /** Converts radians to arc-degree */
    inline constexpr float rad_to_adeg(const float rad) noexcept {
        return rad * 180.0f / (float)M_PI;
    }

    enum class orientation_t {
        /** Collinear **/
        COL,
        /** Clockwise **/
        CLW,
        /** Counter-Clockwise **/
        CCW
    };

    /** Width of the window, coordinate in window units. */
    extern int win_width;
    /** Height of the window, coordinate in window units. */
    extern int win_height;
    /** Ratio pixel-size / window-size, a DPI derivative per axis*/
    extern float devicePixelRatio[2];
    
    /**
     * Width of the framebuffer coordinate in pixels.
     *
     * Framebuffer origin 0/0 is top-left corner.
     */
    extern jau::math::Recti viewport;
    
    typedef std::vector<uint32_t> pixel_buffer_t; // 32-bit pixel
    extern pixel_buffer_t fb_pixels;
    /** Display frames per seconds */
    extern int display_frames_per_sec;
    /** Optional custom forced frames per seconds, pass to swap_gpu_buffer() by default. */
    extern int forced_fps;
    extern int font_height;

    //
    // gfx toolkit dependent API
    //

    /** GFX Toolkit: Initialize a window of given size with a usable framebuffer. */
    bool init_gfx_subsystem(const char* title, int window_width, int window_height, bool enable_vsync=true);
    /** GFX Toolkit: Swap GPU back to front framebuffer while maintaining vertical monitor synchronization if possible. */
    void swap_gpu_buffer(int fps=forced_fps) noexcept;
    float get_gpu_fps() noexcept;

    //
    // input
    //

    /** Fixed input action enumerator, useful to denote typical game actions e.g. cursor keys. */
    enum class input_event_type_t : int {
        NONE,
        POINTER_BUTTON,
        POINTER_MOTION,
        ANY_KEY_UP,
        ANY_KEY_DOWN,
        P1_UP, // 5
        P1_DOWN,
        P1_RIGHT,
        P1_LEFT,
        P1_ACTION1,
        P1_ACTION2,
        P1_ACTION3,
        PAUSE, // 11
        P2_UP,
        P2_DOWN,
        P2_RIGHT,
        P2_LEFT,
        P2_ACTION1,
        P2_ACTION2,
        P2_ACTION3,
        RESET, // 18
        /** Request to close window, which then should be closed by caller */
        WINDOW_CLOSE_REQ,
        WINDOW_RESIZED, // 20
    };
    constexpr int bitno(const input_event_type_t e) noexcept {
        return static_cast<int>(e) - static_cast<int>(input_event_type_t::P1_UP);
    }
    constexpr uint32_t bitmask(const input_event_type_t e) noexcept {
        return 1U << bitno(e);
    }
    constexpr uint32_t bitmask(const int bit) noexcept {
        return 1U << bit;
    }

    inline bool is_ascii_code(int c) noexcept {
        return 0 != std::iscntrl(c) || 0 != std::isprint(c);
    }

    class input_event_t {
        private:
            constexpr static const uint32_t p1_mask =
                    bitmask(input_event_type_t::P1_UP) |
                    bitmask(input_event_type_t::P1_DOWN) |
                    bitmask(input_event_type_t::P1_RIGHT) |
                    bitmask(input_event_type_t::P1_LEFT) |
                    bitmask(input_event_type_t::P1_ACTION1) |
                    bitmask(input_event_type_t::P1_ACTION2) |
                    bitmask(input_event_type_t::P1_ACTION3);

            constexpr static const uint32_t p2_mask =
                    bitmask(input_event_type_t::P2_UP) |
                    bitmask(input_event_type_t::P2_DOWN) |
                    bitmask(input_event_type_t::P2_RIGHT) |
                    bitmask(input_event_type_t::P2_LEFT) |
                    bitmask(input_event_type_t::P2_ACTION1) |
                    bitmask(input_event_type_t::P2_ACTION2) |
                    bitmask(input_event_type_t::P2_ACTION3);
            uint32_t m_pressed; // [P1_UP..RESET]
            uint32_t m_lifted; // [P1_UP..RESET]
            bool m_paused;
        public:
            input_event_type_t last;
            /** ASCII code, ANY_KEY_UP, ANY_KEY_DOWN key code */
            uint16_t last_key_code;
            std::string text;
            int pointer_id;
            int pointer_x;
            int pointer_y;

            input_event_t() noexcept { clear(); }
            void clear() noexcept {
                m_pressed = 0;
                m_lifted = 0;
                m_paused = false;
                last = input_event_type_t::NONE;
                pointer_id = -1;
                pointer_x = -1;
                pointer_y = -1;
            }
            void pointer_motion(int id, int x, int y) noexcept {
                set(input_event_type_t::POINTER_MOTION);
                pointer_id = id;
                pointer_x = x;
                pointer_y = y;
            }
            void set(input_event_type_t e, uint16_t key_code=0) noexcept {
                const int bit = bitno(e);
                if( 0 <= bit && bit <= 31 ) {
                    const uint32_t m = bitmask(bit);
                    m_lifted &= ~m;
                    m_pressed |= m;
                }
                this->last = e;
                this->last_key_code = key_code;
                if( this->text.length() > 0 && '\n' == this->text[this->text.length()-1] ) {
                    this->text.clear();
                }
                if( 0 != key_code && is_ascii_code(key_code) ) {
                    if( 0x08 == key_code ) {
                        if( this->text.length() > 0 ) {
                            this->text.pop_back();
                        }
                    } else {
                        this->text.push_back( (char)key_code );
                    }
                }
            }
            void clear(input_event_type_t e, uint16_t key_code=0) noexcept {
                (void)key_code;
                const int bit = bitno(e);
                if( 0 <= bit && bit <= 31 ) {
                    const uint32_t m = bitmask(bit);
                    m_lifted |= m_pressed & m;
                    m_pressed &= ~m;
                    this->last_key_code = 0;
                }
                if( input_event_type_t::PAUSE == e ) {
                    m_paused = !m_paused;
                }
            }
            bool paused() const noexcept { return m_paused; }
            bool pressed(input_event_type_t e) const noexcept {
                const int bit = bitno(e);
                if( 0 <= bit && bit <= 31 ) {
                    return 0 != ( m_pressed & bitmask(bit) );
                } else {
                    return false;
                }
            }
            bool pressed_and_clr(input_event_type_t e) noexcept {
                if( pressed(e) ) {
                    clear(e);
                    return true;
                } else {
                    return false;
                }
            }
            bool released_and_clr(input_event_type_t e) noexcept {
                const int bit = bitno(e);
                if( 0 <= bit && bit <= 31 ) {
                    const uint32_t m = bitmask(bit);
                    if( 0 != ( m_lifted & m ) ) {
                        m_lifted &= ~m;
                        return true;
                    }
                }
                return false;
            }
            bool has_any_p1() const noexcept {
                return 0 != ( ( m_pressed | m_lifted ) & p1_mask );
            }
            bool has_any_p2() const noexcept {
                return 0 != ( ( m_pressed | m_lifted ) & p2_mask );
            }
            std::string to_string() const noexcept;
    };
    inline std::string to_string(const input_event_t& e) noexcept { return e.to_string(); }

    /**
     * GFX Toolkit: Handle windowing and keyboard events.
     *
     * Should be called until function returns false
     * to process all buffered events.
     *
     * @param event
     * @return true if event received, false otherwise
     */
    bool handle_one_event(input_event_t& event) noexcept;

    inline bool handle_events(input_event_t& event) noexcept {
        bool one = false;
        while( gamp::handle_one_event(event) ) {
            one = true;
            // std::cout << "Input " << to_string(event) << std::endl;
        }
        return one;
    }
    
    //
    // Misc
    //

    inline constexpr const int64_t NanoPerMilli = 1000000L;
    inline constexpr const int64_t MilliPerOne = 1000L;
    inline constexpr const int64_t NanoPerOne = NanoPerMilli*MilliPerOne;

    /** Return current milliseconds, since Unix epoch. */
    uint64_t getCurrentMilliseconds() noexcept;
    /** Return current milliseconds, since program launch. */
    uint64_t getElapsedMillisecond() noexcept;
    /** Sleep for the givn milliseconds. */
    void milli_sleep(uint64_t td) noexcept;

    void log_printf(const uint64_t elapsed_ms, const char * format, ...) noexcept;
    void log_printf(const char * format, ...) noexcept;

    //
    // Cut from jaulib
    //

    template <typename T>
    constexpr ssize_t sign(const T x) noexcept
    {
        return (T(0) < x) - (x < T(0));
    }

    template <typename T>
    constexpr T invert_sign(const T x) noexcept
    {
        return std::numeric_limits<T>::min() == x ? std::numeric_limits<T>::max() : -x;
    }

    template<typename T>
    constexpr size_t digits10(const T x, const ssize_t x_sign, const bool sign_is_digit=true) noexcept
    {
        if( x_sign == 0 ) {
            return 1;
        }
        if( x_sign < 0 ) {
            return 1 + static_cast<size_t>( std::log10<T>( invert_sign<T>( x ) ) ) + ( sign_is_digit ? 1 : 0 );
        } else {
            return 1 + static_cast<size_t>( std::log10<T>(                 x   ) );
        }
    }

    template< class value_type,
              std::enable_if_t< std::is_integral_v<value_type>,
                                bool> = true>
    std::string to_decstring(const value_type& v, const char separator=',', const size_t width=0) noexcept {
        const ssize_t v_sign = sign<value_type>(v);
        const size_t digit10_count1 = digits10<value_type>(v, v_sign, true /* sign_is_digit */);
        const size_t digit10_count2 = v_sign < 0 ? digit10_count1 - 1 : digit10_count1; // less sign

        const size_t comma_count = 0 == separator ? 0 : ( digit10_count1 - 1 ) / 3;
        const size_t net_chars = digit10_count1 + comma_count;
        const size_t total_chars = std::max<size_t>(width, net_chars);
        std::string res(total_chars, ' ');

        value_type n = v;
        size_t char_iter = 0;

        for(size_t digit10_iter = 0; digit10_iter < digit10_count2 /* && char_iter < total_chars */; digit10_iter++ ) {
            const int digit = v_sign < 0 ? invert_sign( n % 10 ) : n % 10;
            n /= 10;
            if( 0 < digit10_iter && 0 == digit10_iter % 3 ) {
                res[total_chars-1-(char_iter++)] = separator;
            }
            res[total_chars-1-(char_iter++)] = '0' + digit;
        }
        if( v_sign < 0 /* && char_iter < total_chars */ ) {
            res[total_chars-1-(char_iter++)] = '-';
        }
        return res;
    }
}

#endif /*  GAMP_HPP_ */

