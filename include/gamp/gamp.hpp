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
#ifndef JAU_GAMP_HPP_
#define JAU_GAMP_HPP_

#include <gamp/gamp_types.hpp>
#include <gamp/version.hpp>

/**
 * Basic computer graphics math and utilities helping with the framebuffer and I/O tooling.
 */
namespace gamp {
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

    /** Display frames per seconds */
    extern int display_frames_per_sec;
    /** Optional custom forced frames per seconds, pass to swap_gpu_buffer() by default. Defaults to -1, i.e. automatic fps. */
    extern int forced_fps;
    extern int font_height;

    //
    // gfx toolkit dependent API
    //

    /** GFX Toolkit: Initialize a window of given size with a usable framebuffer. */
    bool init_gfx_subsystem(const char* title, int window_width, int window_height, bool enable_vsync = true);
    /** GFX Toolkit: Swap GPU back to front framebuffer using given fps, maintaining vertical monitor synchronization if possible. fps <= 0 implies automatic fps. */
    void swap_gpu_buffer(int fps) noexcept;
    /** GFX Toolkit: Swap GPU back to front framebuffer using forced_fps, maintaining vertical monitor synchronization if possible. */
    inline void swap_gpu_buffer() noexcept { swap_gpu_buffer(forced_fps); }

    /** Returns frames per seconds, averaged over get_gpu_stat_period(). */
    float get_gpu_stats_fps() noexcept;
    /** Returns rendering costs per frame in seconds, averaged over get_gpu_stat_period(). */
    double get_gpu_stats_frame_costs() noexcept;
    /** Returns active sleeping period per frame in seconds, averaged over get_gpu_stat_period(). Only reasonable if swap_gpu_buffer() has been called with fps > 0. */
    double get_gpu_stats_frame_sleep() noexcept;
    /** Sets the period length to average get_gpu_fps(), get_gpu_frame_costs(), get_gpu_frame_sleep() statistics. Defaults to 5s.*/
    void set_gpu_stats_period(int64_t milliseconds) noexcept;
    /** Returns the current period length for statistics in milliseconds, see set_gpu_stat_period(). Defaults is 5s. */
    int64_t get_gpu_stats_period() noexcept;
    /** Print statistics on the console to stdout after get_gpu_stat_period(). */
    void set_gpu_stats_show(bool enable) noexcept;
    /** Returns whether statistics are printed on the console, see set_show_gpu_stats(). */
    bool get_gpu_stats_show() noexcept;

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
        P1_UP,  // 5
        P1_DOWN,
        P1_RIGHT,
        P1_LEFT,
        P1_ACTION1,
        P1_ACTION2,
        P1_ACTION3,
        PAUSE,  // 11
        P2_UP,
        P2_DOWN,
        P2_RIGHT,
        P2_LEFT,
        P2_ACTION1,
        P2_ACTION2,
        P2_ACTION3,
        RESET,  // 18
        /** Request to close window, which then should be closed by caller */
        WINDOW_CLOSE_REQ,
        WINDOW_RESIZED,  // 20
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
            uint32_t m_pressed;  // [P1_UP..RESET]
            uint32_t m_lifted;   // [P1_UP..RESET]
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
            void set(input_event_type_t e, uint16_t key_code = 0) noexcept {
                const int bit = bitno(e);
                if (0 <= bit && bit <= 31) {
                    const uint32_t m = bitmask(bit);
                    m_lifted &= ~m;
                    m_pressed |= m;
                }
                this->last = e;
                this->last_key_code = key_code;
                if (this->text.length() > 0 && '\n' == this->text[this->text.length() - 1]) {
                    this->text.clear();
                }
                if (0 != key_code && is_ascii_code(key_code)) {
                    if (0x08 == key_code) {
                        if (this->text.length() > 0) {
                            this->text.pop_back();
                        }
                    } else {
                        this->text.push_back((char)key_code);
                    }
                }
            }
            void clear(input_event_type_t e, uint16_t key_code = 0) noexcept {
                (void)key_code;
                const int bit = bitno(e);
                if (0 <= bit && bit <= 31) {
                    const uint32_t m = bitmask(bit);
                    m_lifted |= m_pressed & m;
                    m_pressed &= ~m;
                    this->last_key_code = 0;
                }
                if (input_event_type_t::PAUSE == e) {
                    m_paused = !m_paused;
                }
            }
            bool paused() const noexcept { return m_paused; }
            bool pressed(input_event_type_t e) const noexcept {
                const int bit = bitno(e);
                if (0 <= bit && bit <= 31) {
                    return 0 != (m_pressed & bitmask(bit));
                } else {
                    return false;
                }
            }
            bool pressed_and_clr(input_event_type_t e) noexcept {
                if (pressed(e)) {
                    clear(e);
                    return true;
                } else {
                    return false;
                }
            }
            bool released_and_clr(input_event_type_t e) noexcept {
                const int bit = bitno(e);
                if (0 <= bit && bit <= 31) {
                    const uint32_t m = bitmask(bit);
                    if (0 != (m_lifted & m)) {
                        m_lifted &= ~m;
                        return true;
                    }
                }
                return false;
            }
            bool has_any_p1() const noexcept {
                return 0 != ((m_pressed | m_lifted) & p1_mask);
            }
            bool has_any_p2() const noexcept {
                return 0 != ((m_pressed | m_lifted) & p2_mask);
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
        while (gamp::handle_one_event(event)) {
            one = true;
            // std::cout << "Input " << to_string(event) << std::endl;
        }
        return one;
    }
}  // namespace gamp

#endif /*  JAU_GAMP_HPP_ */
