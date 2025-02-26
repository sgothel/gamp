/*
 * Author: Sven Gothel <sgothel@jausoft.com> and Svenson Han Gothel
 * Copyright (c) 2022-2025 Gothel Software e.K.
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

#include <gamp/wt/event/event.hpp>
#include <gamp/wt/event/keyevent.hpp>
#include <gamp/wt/event/winevent.hpp>
#include <gamp/wt/window.hpp>

/**
 * Gamp: Graphics, Audio, Multimedia and Processing Framework (Native C++, WebAssembly, ...)
 *
 * *Gamp* addresses native hardware accelerated graphics, audio, multimedia and processing.
 * It is implemented in C++, supports WebAssembly and perhaps interfacing w/ other languages/systems.
 */
namespace gamp {
    /** @defgroup Gamp Gamp Root
     *  Graphics, Audio, Multimedia and Processing Framework
     *
     *  @{
     */

    std::string lookup_and_register_asset_dir(const char* exe_path, const char* asset_file="fonts/freefont/FreeSansBold.ttf", const char* asset_install_subdir="gamp") noexcept;
    std::string asset_dir() noexcept;
    std::string resolve_asset(const std::string &asset_file, bool lookup_direct=false) noexcept;
    //
    // gfx toolkit dependent API
    //

    /** Returns the elapsed monotonic time since `init_gfx_subsystem`, synchronized with the gfx subsystem timer. */
    jau::fraction_timespec getElapsedMonotonicTime() noexcept;

    /** Monitor frames per seconds */
    int monitor_fps() noexcept;

    /**
     * Returns optional forced frames per seconds or -1 if unset, set via set_gpu_forced_fps().
     * Passed to swap_gpu_buffer() by default.
     */
    int gpu_forced_fps() noexcept;

    /** Optional forced frames per seconds, pass to swap_gpu_buffer() by default. */
    void set_gpu_forced_fps(int fps) noexcept;

    /** Returns expected fps, either gpu_forced_fps() if set, otherwise monitor_fps(). */
    inline int expected_fps() noexcept { int v=gpu_forced_fps(); return v>0?v:monitor_fps(); }
    /** Returns the expected frame duration in [s], i.e. 1/expected_fps() */
    inline jau::fraction_timespec expected_framedur() noexcept {
        return jau::fraction_timespec(1.0/double(expected_fps()));
    }

    bool is_gfx_subsystem_initialized() noexcept;
    /** GFX Toolkit: Initialize the subsystem once. */
    bool init_gfx_subsystem(const char* exe_path);
    /**
     * GFX Toolkit: Swap GPU back to front framebuffer of all windows using given fps, maintaining vertical monitor synchronization if possible. fps <= 0 implies automatic fps.
     *
     * Should not be called by user, instead use mainloop_default().
     *
     * @param fps use gpu_forced_fps()
     *
     * @see mainloop_default()
     */
    void swap_gpu_buffer(int fps) noexcept;

    /** Returns the measured gpu fps each 5s, starting with monitor_fps() */
    float gpu_avg_fps() noexcept;
    /** Returns the measured gpu frame duration in [s] each 5s, starting with 1/gpu_avg_fps() */
    const jau::fraction_timespec& gpu_avg_framedur() noexcept;

    /**
     * Performs the whole tasks for all created gamp::wt::Window instances
     * - handle events and propagates them to registered listener at each window
     * - renders all gamp::wt::Window first
     *   - calling each gamp::wt::Window::display() method, serving gamp::wt::RenderListener
     * - swaps front/back buffer of all gamp::wt::Window instances
     * - adjust timers for whole set
     *
     * @see swap_gpu_buffer()
     */
    void mainloop_default() noexcept;
    void shutdown() noexcept;

    //
    // input
    //

    /**
     * GFX Toolkit: Handle windowing and keyboard events.
     *
     * Should be called until function returns false
     * to process all buffered events.
     *
     * @return number of events received
     */
    size_t handle_events() noexcept;

    /**@}*/
}  // namespace gamp

#endif /*  JAU_GAMP_HPP_ */
