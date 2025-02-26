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
#ifndef GAMP_WTSURFACE_HPP_
#define GAMP_WTSURFACE_HPP_

#include <jau/fraction_type.hpp>
#include <jau/int_types.hpp>
#include <jau/locks.hpp>
#include <gamp/renderer/gl/glprofile.hpp>
#include <memory>
#include <thread>

#include <gamp/gamp_types.hpp>

namespace gamp::wt {
    /** \addtogroup Gamp_WT
     *
     *  @{
     */
    class Window;
    class Surface;
    typedef std::shared_ptr<Surface> SurfaceRef;

    using namespace jau::fractions_i64_literals;
    using namespace jau::math;

    //
    // TODO: GraphicsConfiguration: Screen + Device + Capabilities
    //

    class Surface : public std::enable_shared_from_this<Surface> {
        public:
            static constexpr const jau::fraction_i64 TIMEOUT = 5_s;

            enum class lock_status_t : uint16_t { unlocked,
                                                  not_ready,
                                                  locked_changed,
                                                  locked_same };
            static constexpr bool is_locked(const lock_status_t ls) noexcept {
                return static_cast<uint16_t>(lock_status_t::not_ready) < static_cast<uint16_t>(ls);
            }

        private:
            int m_swapInterval = -1;
            handle_t m_surface_handle;
            /// Surface client-area size in pixel units
            Vec2i m_surface_size;
            gamp::render::gl::GL m_renderContext;

            jau::RecursiveLock m_surface_lock;

            void disposeImpl(handle_t) noexcept {}
            bool setSwapIntervalImpl(int v) noexcept;

        protected:
            struct Private{ explicit Private() = default; };

            template <typename ChildT>
            std::shared_ptr<ChildT> shared_from_base() {
                return std::static_pointer_cast<ChildT>(shared_from_this());
            }

            void setSurfaceSize(const Vec2i& sz) noexcept { m_surface_size = sz; }

        public:
            /** Private ctor for single Surface::create() method w/o public ctor. */
            Surface(Private, handle_t surface_handle, const Vec2i& surface_size)
            : m_surface_handle(surface_handle), m_surface_size(surface_size) { }

            static SurfaceRef create(handle_t surface_handle, const Vec2i& surface_size) {
                return std::make_shared<Surface>(Private(), surface_handle, surface_size);
            }

            Surface(const Surface&) = delete;
            void operator=(const Surface&) = delete;

            /**
             * Releases this instance.
             */
            virtual ~Surface() noexcept {
                if( m_surface_lock.isOwner() ) {
                    nativeSurfaceLock();
                }
            }

            virtual void disposedNotified(const jau::fraction_timespec&) noexcept {
                m_surface_handle = 0;
                m_renderContext.releasedNotify();
            }
            virtual void dispose(const jau::fraction_timespec&) noexcept {
                m_renderContext.dispose();
                if( m_surface_handle ) {
                    disposeImpl(m_surface_handle);
                    m_surface_handle = 0;
                }
            }

            const SurfaceRef shared() { return shared_from_this(); }

            bool createContext(gamp::render::gl::GLProfileMask profileMask, gamp::render::gl::GLContextFlags contextFlags) {
                gamp::render::gl::GL gl = createContext(shared(), profileMask, contextFlags, nullptr);
                if( gl.isValid() ) {
                    ownRenderingContext(std::move(gl));
                    return true;
                }
                return false;
            }
            static gamp::render::gl::GL createContext(const wt::SurfaceRef& surface,
                                                      gamp::render::gl::GLProfileMask profileMask,
                                                      gamp::render::gl::GLContextFlags contextFlags,
                                                      gamp::render::gl::GL* shareWith) noexcept;

            void ownRenderingContext(gamp::render::gl::GL&& gl) noexcept {
                m_renderContext = std::move(gl);
            }
            constexpr const gamp::render::gl::GL& renderContext() const noexcept { return m_renderContext; }
            constexpr gamp::render::gl::GL& renderContext() noexcept { return m_renderContext; }

            /**
             * Returns desired or determined swap interval. Defaults to -1, i.e. adaptive swap interval.
             *
             * Use setSwapInterval() to set the swap interval.
             *
             * *  [1..n] denotes the swap interval
             * *  0 indicates no swap interval, e.g. by failed setSwapInterval()
             * * -1 indicates adaptive swap interval
             */
            constexpr int swapInterval() const noexcept { return m_swapInterval; }

            /** Returns true if swap interval could be set with the native toolkit post createContext(). See swapInterval(). */
            bool setSwapInterval(int v) noexcept {
                if( m_renderContext.isValid() ) {
                    return setSwapIntervalImpl(v);
                }
                m_swapInterval=v;
                return false;
            }

            /**
             * Returns <code>true</code> if the drawable is rendered in
             * OpenGL's coordinate system, <i>origin at bottom left</i>.
             * Otherwise returns <code>false</code>, i.e. <i>origin at top left</i>.
             *
             * Default impl. is <code>true</code>, i.e. OpenGL coordinate system.
             *
             * Currently only MS-Windows bitmap offscreen drawable uses a non OpenGL orientation and hence returns <code>false</code>.<br/>
             * This removes the need of a vertical flip when used in AWT or Windows applications.
             */
            constexpr bool isGLOriented() const noexcept { return true; }

            /** Returns the surface size of the client area excluding insets (window decorations) in pixel units. */
            constexpr const Vec2i& surfaceSize() const noexcept { return m_surface_size; }

            /**
             * Returns the handle to the surface for this NativeSurface. <P>
             *
             * The surface handle should be set/update by {@link #lockSurface()},
             * where {@link #unlockSurface()} is not allowed to modify it.
             * After {@link #unlockSurface()} it is no more guaranteed
             * that the surface handle is still valid.
             *
             * The surface handle shall reflect the platform one
             * for all drawable surface operations, e.g. opengl, swap-buffer. <P>
             *
             * On X11 this returns an entity of type Window,
             * since there is no differentiation of surface and window there. <BR>
             * On Microsoft Windows this returns an entity of type HDC.
             */
            constexpr handle_t surfaceHandle() const noexcept { return m_surface_handle; }

            /**
             * Provide a mechanism to utilize custom (pre-) swap surface
             * code. This method is called before the render toolkit (e.g. JOGL)
             * swaps the buffer/surface if double buffering is enabled.
             *
             * The implementation may itself apply the swapping,
             * in which case true shall be returned.
             *
             * @return true if this method completed swapping the surface,
             *         otherwise false, in which case eg the GLDrawable
             *         implementation has to swap the code.
             */
            virtual bool surfaceSwap() noexcept { return true; }

            /**
             * Lock the surface of this native window.
             *
             * The surface handle shall be valid after a successfull call,
             * i.e. return a value other than lock_status_t::unlocked and lock_status_t::not_ready.
             *
             * The caller may need to take care of the result lock_status_t::locked_changed,
             * where the surface handle is valid but has changed.
             *
             * This call is blocking until the surface has been locked
             * or a timeout is reached. The latter will throw a runtime exception.
             *
             * This call allows recursion from the same thread.
             *
             * The implementation may want to aquire the
             * application level {@link com.jogamp.common.util.locks.RecursiveLock}
             * first before proceeding with a native surface lock.
             *
             * The implementation shall also invoke {@link AbstractGraphicsDevice#lock()}
             * for the initial lock (recursive count zero).
             *
             * @return Surface::lock_status_t
             *
             * @throws RuntimeException after timeout when waiting for the surface lock
             * @throws NativeWindowException if native locking failed, maybe platform related
             */
            lock_status_t lockSurface() {
                if( !m_surface_lock.tryLock(TIMEOUT) ) {
                    throw jau::RuntimeException("Waited "+TIMEOUT.to_string()+"s for: "+toString()+" - "+jau::threadName(std::this_thread::get_id()), E_FILE_LINE);
                }
                if (1 == m_surface_lock.holdCount()) {
                    const lock_status_t res = nativeSurfaceLock();
                    if( !is_locked(res) ) {
                        m_surface_lock.unlock();
                    }
                    return res;
                } else {
                    return lock_status_t::locked_same;
                }
            }

            /**
             * Unlock the surface of this native window
             *
             * Shall not modify the surface handle, see lockSurface().
             *
             * The implementation shall also invoke GraphicsDevice::unlock()
             * for the final unlock (recursive count zero).
             *
             * The implementation shall be fail safe, i.e. tolerant in case the native resources
             * are already released / unlocked. In this case the implementation shall simply ignore the call.
             *
             * @see lockSurface()
             */
            void unlockSurface() {
                m_surface_lock.validateLocked();
                if (1 == m_surface_lock.holdCount()) {
                    nativeSurfaceLock();
                }
                m_surface_lock.unlock();
            }

        protected:
            virtual lock_status_t nativeSurfaceLock() noexcept { return lock_status_t::locked_same; }
            virtual void nativeSurfaceUnlock() noexcept { }

        public:
            std::string toString() const noexcept {
                std::string res = "Surface[";
                res.append("handle ").append(jau::to_hexstring(m_surface_handle))
                   .append(", ").append(m_renderContext.toString())
                   .append(", size ").append(m_surface_size.toString())
                   .append("]");
                return res;
            }
    };
    /**@}*/
}  // namespace gamp::wt

#endif /*  GAMP_WTSURFACE_HPP_ */
