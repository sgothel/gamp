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

#ifndef JAU_GAMP_HPP_
#define JAU_GAMP_HPP_

#include <gamp/GampTypes.hpp>
#include <gamp/Version.hpp>

#include <gamp/wt/event/Event.hpp>
#include <gamp/wt/event/KeyEvent.hpp>
#include <gamp/wt/event/WinEvent.hpp>
#include <gamp/wt/Window.hpp>

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
     * @return true signaling continuation, false to end the mainloop
     * @see swap_gpu_buffer()
     */
    bool mainloop_default() noexcept;

    /// Calls mainloop_default(), but exits application if returning false
    void mainloop_void() noexcept;

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

    /**
     * Singleton runtime environment properties
     *
     * Also see {@link jau::environment::getExplodingProperties(const std::string & prefixDomain)}.
     *
     * Note that all environment variables listed below can be set using `_` instead of `.`.
     */
    class GampEnv : public jau::root_environment {
        private:
            GampEnv() noexcept; // NOLINT(modernize-use-equals-delete)

        public:
            /** Global Debug flag, retrieved first to triggers GampEnv initialization. */
            const bool DEBUG_GLOBAL;

        private:
            [[maybe_unused]] const bool exploding; // just to trigger exploding properties

        public:
            /**
             * Debug all WT event communication
             *
             * Environment variable is 'gamp.debug.wt.event'.
             *
             */
            const bool DEBUG_WT_EVENT;

            /**
             * Debug shader code, i.e. ShaderCode
             *
             * Environment variable is 'gamp.debug.render.gl.glsl.code'.
             *
             */
            const bool DEBUG_RENDER_GL_GLSL_CODE;

            /**
             * Debug shader state, i.e. ShaderState
             *
             * Environment variable is 'gamp.debug.render.gl.glsl.state'.
             *
             */
            const bool DEBUG_RENDER_GL_GLSL_STATE;

            /**
             * Debug Graph Renderer
             *
             * Environment variable is 'gamp.debug.graph.render'.
             *
             */
            const bool DEBUG_GRAPH_RENDER;

        public:
            static GampEnv& get() noexcept {
                static GampEnv e;
                return e;
            }
    };


    /**@}*/
}  // namespace gamp

/** \example RedSquareES2.cpp
 * This C++ example stub showcases the simple Launcher01.hpp mainloop framework integration. It creates a window, provides
 * keyboard interaction and uses RedSquareES2 gamp::wt::RenderListener for rendering.
 */

/** \example RedSquareES2.hpp
 * This C++ demo showcases a simple gamp::wt::RenderListener, implementing rendering using
 * gamp::render::gl::glsl::ShaderState setup with attributes, uniforms and gamp::render::gl::glsl::ShaderCode.
 */

/** \example GearsES2.cpp
 * This C++ example uses the simple Launcher01.hpp mainloop framework integration. It creates a window, provides
 * keyboard and pointer interaction including picking, rotate & drag. It
 * uses GearsES2 gamp::wt::RenderListener for rendering and uses per-pixel-lighting.
 */

/** \example GearsES2.hpp
 * This C++ demo showcases a simple gamp::wt::RenderListener exposing three shapes, implementing rendering using
 * gamp::render::gl::glsl::ShaderState setup with attributes, uniforms and gamp::render::gl::glsl::ShaderCode.
 */

/** \example Primitives02.cpp
 * This C++ example demonstrates simple polylines using the the GLU tesselator and per-pixel-lighting.
 */

/** \example GraphShapes01.cpp
 * This is the first C++ example demonstrating Graph resolution independent GPU curve rendering.
 * Multiple complex shapes are rotated with front- and back-face as well as per-pixel-lighting.
 */

 /** \example SolInSpace.cpp
  * Simple solar fragment-shader w/ core- and halo-radius.
  */

#endif /*  JAU_GAMP_HPP_ */
