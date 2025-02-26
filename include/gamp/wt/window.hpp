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
#ifndef GAMP_WTWINDOW_HPP_
#define GAMP_WTWINDOW_HPP_

#include <jau/basic_types.hpp>
#include <jau/debug.hpp>
#include <jau/fraction_type.hpp>
#include <jau/math/vec2i.hpp>
#include <jau/string_util.hpp>

#include <gamp/wt/event/event.hpp>
#include <gamp/wt/surface.hpp>
#include <gamp/wt/event/winevent.hpp>
#include <gamp/wt/event/keyeventmngr.hpp>
#include <gamp/wt/event/wineventmngr.hpp>
#include <gamp/wt/event/pointereventmngr.hpp>

#include <gamp/renderer/gl/gltypes.hpp>

namespace gamp::wt {

    /** @defgroup Gamp_WT Gamp Windowing Toolkit
     *  Windowing toolkit functionality.
     *
     *  @{
     */

    class Window;
    typedef std::shared_ptr<Window> WindowRef;

    using namespace jau::fractions_i64_literals;
    using namespace jau::math;
    using namespace jau::enums;
    using namespace gamp::wt::event;

    enum class RenderActions : uint16_t {
        none    = 0,
        init    = 1,
        reshape  = 2
    };
    JAU_MAKE_ENUM_INFO(RenderActions, init, reshape);
    JAU_MAKE_BITFIELD_ENUM_STRING(RenderActions, init, reshape);

    class RenderListener : public std::enable_shared_from_this<RenderListener> {
      private:
        RenderActions m_pending_actions = RenderActions::init | RenderActions::reshape;

      protected:
        struct Private{ explicit Private() = default; };

      public:
        /** Private ctor for shared_ptr<RenderListener> instance method w/o public ctor. */
        constexpr RenderListener(Private) noexcept {}

        virtual ~RenderListener() noexcept = default;

        RenderActions pendingActions() const noexcept { return m_pending_actions; }
        RenderActions& pendingActions() noexcept { return m_pending_actions; }
        void setPendingActions(RenderActions v) noexcept { m_pending_actions=v; }

        /** Called by the drawable immediately after the render context is
            initialized. Can be used to perform one-time renderer
            initialization per context, such as setup of lights and display lists.<p>

            Note that this method may be called more than once if the underlying
            render context for the drawable is destroyed and
            recreated.
        *
        * @return true if initialization is completed, false if not initialized yet and init shall be called again
        */
        virtual bool init(const WindowRef&, const jau::fraction_timespec& /*when*/) = 0;

        /** Notifies the listener to perform the release of all renderer
            resources per context, such as memory buffers and shader programs.<P>

            Called by the drawable before the render context is
            destroyed by an external event.<P>

            Note that this event does not imply the end of life of the application.
        */
        virtual void dispose(const WindowRef&, const jau::fraction_timespec& /*when*/) = 0;

        /** Called by the drawable to initiate rendering by the
            client. After all listener have been notified of a
            display event, the drawable will swap its buffers. */
        virtual void display(const WindowRef&, const jau::fraction_timespec& /*when*/) = 0;

        /**
         * Called by the drawable during the first repaint after the
         * component has been resized.
         *
         * The client can update it's viewport associated data
         * and view volume of the window/surface appropriately.
         *
         * For efficiency the renderer viewport has already been updated,
         * e.g. via <code>glViewport(x, y, width, height)</code> when this method is called.
         *
         * @param drawable the triggering {@link GLAutoDrawable}
         * @param viewport the viewport in pixel units
         */
        virtual void reshape(const WindowRef&, const jau::math::Recti& /*viewport*/, const jau::fraction_timespec& /*when*/) = 0;

        virtual std::string toStringImpl() const noexcept { return "none"; }

        std::string toString() const noexcept {
            std::string res = "RenderListener[";
            res.append("pending ").append(to_string(m_pending_actions))
               .append(", this ").append(jau::to_hexstring(this))
               .append(", ").append(toStringImpl())
               .append("]");
            return res;
        }

    };
    typedef std::shared_ptr<RenderListener> RenderListenerRef;

    enum class WindowState : uint16_t {
        none       = 0,
        focused    = 1,
        visible    = 2,
    };
    JAU_MAKE_ENUM_INFO(WindowState, focused, visible);
    JAU_MAKE_BITFIELD_ENUM_STRING(WindowState, focused, visible);

    class Window : public Surface {
        private:
            handle_t m_window_handle;

            /// Window client-area top-left position and size in window units
            Recti m_window_bounds;

            WindowState m_state;

            WindowEventManager  m_win_evt_mngr;
            KeyEventManager     m_key_evt_mngr;
            PointerEventManager m_ptr_evt_mngr;

            jau::cow_darray<RenderListenerRef> m_render_listener;

        protected:
            struct Private{ explicit Private() = default; };

            void setWindowBounds(const Recti& r) noexcept { m_window_bounds = r; }
            void setWindowPos(const Vec2i& sz) noexcept { m_window_bounds.setPosition(sz); }
            void setWindowSize(const Vec2i& sz) noexcept { m_window_bounds.setSize(sz); }
            void setFocused(bool v) noexcept { write(m_state, WindowState::focused, v); }
            void setVisible(bool v) noexcept { write(m_state, WindowState::visible, v); }

        private:
            class SelfWinListener : public WindowListener {
              private:
                Window* m_self;
              public:
                SelfWinListener(Window* self) noexcept : m_self(self) {}
                void windowResized(const WindowEvent&, const Vec2i& winSize, const jau::math::Vec2i& surfSize) override {
                    m_self->setWindowSize(winSize);
                    m_self->setSurfaceSize(surfSize);
                    for(const RenderListenerRef& l : *m_self->m_render_listener.snapshot()) {
                        write(l->pendingActions(), RenderActions::reshape, true);
                    }
                }
                void windowMoved(const WindowEvent&, const Vec2i& winPos) override { m_self->setWindowPos(winPos); }
                void windowDestroyNotify(const WindowEvent&) override {}
                void windowDestroyed(const WindowEvent&) override {}
                void windowFocusChanged(const WindowEvent&, bool focused) override { m_self->setFocused(focused); }
                void windowRepaint(const WindowEvent&) override {}
                void windowVisibilityChanged(const WindowEvent&, bool visible) override { m_self->setVisible(visible); }
            };
            WindowListenerRef m_win_selflistener;

            void disposeImpl(handle_t handle) noexcept;

        public:
            /** Private ctor for single Window::create() method w/o public ctor. */
            Window(Private, handle_t window_handle, const Recti& window_bounds,
                   handle_t surface_handle, const Vec2i& surface_size)
            : Surface(Surface::Private(), surface_handle, surface_size),
              m_window_handle(window_handle),
              m_window_bounds(window_bounds), m_state(WindowState::none),
              m_win_selflistener(std::make_shared<SelfWinListener>(this))
            {
                write(m_state, WindowState::visible, 0 != window_handle && !window_bounds.is_zero());
                addWindowListener(m_win_selflistener);
            }

            /**
             * Create an new instance, wrapping the native windowing toolkit's handle/resources.
             *
             * Must be driven by a native toolkit implementation, see create() below.
             * @see create()
             */
            static WindowRef wrapNative(handle_t window_handle, const Recti& window_bounds,
                                        handle_t surface_handle, const Vec2i& surface_size) {
                return std::make_shared<Window>(Private(), window_handle, window_bounds,
                                                surface_handle, surface_size);
            }

            /**
             * Create an new instance using a native windowing toolkit.
             *
             * @see wrapNative()
             */
            static WindowRef create(const char* title, int wwidth, int wheight, bool verbose=false);

            Window(const Window&) = delete;
            void operator=(const Window&) = delete;

            /**
             * Releases this instance.
             */
            ~Window() noexcept override = default;

            /**
             * Returns the associated {@link Surface} of this {@link SurfaceHolder}.
             * <p>
             * Returns this instance, which <i>is-a</i> {@link Surface}.
             * </p>
             */
            SurfaceRef nativeSurface() { return shared_from_this(); }
            const WindowRef shared() { return Surface::shared_from_base<Window>(); }

            /** Returns the window top-lect position of client-area in window units */
            constexpr Vec2i windowPos() const noexcept { return m_window_bounds.getPosition(); }

            /** Returns the window size of the client area excluding insets (window decorations) in window units. */
            constexpr Vec2i windowSize() const noexcept { return m_window_bounds.getSize(); }

            /** Returns the window client-area top-left position and size excluding insets (window decorations) in window units. */
            constexpr const Recti& windowBounds() const noexcept { return m_window_bounds; }

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
            constexpr handle_t windowHandle() const noexcept { return m_window_handle; }
            constexpr bool isValid() const noexcept { return 0 != m_window_handle; }

            constexpr WindowState state() const noexcept { return m_state; }
            constexpr bool hasFocus() const noexcept { return is_set(m_state, WindowState::focused); }
            constexpr bool isVisible() const noexcept { return is_set(m_state, WindowState::visible); }

            //
            //
            //
            void notifyWindowEvent(uint16_t type, const jau::fraction_timespec& when, bool value=true) noexcept {
                m_win_evt_mngr.dispatch(type, when, shared(), value);
            }
            void notifyWindowResize(const jau::fraction_timespec& when,
                                    const jau::math::Vec2i& winSize, const jau::math::Vec2i& surfSize) noexcept {
                if( m_window_bounds.getSize() != winSize || Surface::surfaceSize() != surfSize ) {
                    m_win_evt_mngr.dispatchResize(when, shared(), winSize, surfSize);
                }
            }
            void notifyWindowMoved(const jau::fraction_timespec& when,
                                   const jau::math::Vec2i& winPos) noexcept {
                if( m_window_bounds.getPosition() != winPos ) {
                    m_win_evt_mngr.dispatchMoved(when, shared(), winPos);
                }
            }

            void addWindowListener(const WindowListenerRef& l) { m_win_evt_mngr.addListener(l); }
            size_t removeWindowListener(const WindowListenerRef& l) { return m_win_evt_mngr.removeListener(l); }
            size_t removeAllWindowListener() { size_t r = m_win_evt_mngr.removeAllListener() - 1; addWindowListener(m_win_selflistener); return r; }
            size_t windowListenerCount() const noexcept { return m_win_evt_mngr.listenerCount() - 1; }

            //
            //
            //
            const KeyboardTracker& keyTracker() const noexcept { return m_key_evt_mngr; }

            void notifyKeyPressed(const jau::fraction_timespec& when, VKeyCode keySym, InputModifier keySymMods, uint16_t keyChar) noexcept {
                m_key_evt_mngr.dispatchPressed(when, shared(), keySym, keySymMods, keyChar);
            }
            void notifyKeyReleased(const jau::fraction_timespec& when, VKeyCode keySym, InputModifier keySymMods, uint16_t keyChar) noexcept {
                m_key_evt_mngr.dispatchReleased(when, shared(), keySym, keySymMods, keyChar);
            }
            void addKeyListener(const KeyListenerRef& l) { m_key_evt_mngr.addListener(l); }
            size_t removeKeyListener(const KeyListenerRef& l) { return m_key_evt_mngr.removeListener(l); }
            size_t removeAllKeyListener() { return m_key_evt_mngr.removeAllListener(); }
            size_t keyListenerCount() const noexcept { return m_key_evt_mngr.listenerCount(); }

            //
            //
            //
            void notifyPointer(uint16_t type, const jau::fraction_timespec& when,
                               PointerType ptype, uint16_t id,
                               jau::math::Vec2i pos, uint16_t clickCount, InputButton button,
                               jau::math::Vec3f rotation, float rotationScale) noexcept {
                m_ptr_evt_mngr.dispatch(type, when, shared(), keyTracker().modifier(), ptype, id, pos, clickCount, button, rotation, rotationScale);
            }
            void addPointerListener(const PointerListenerRef& l) { m_ptr_evt_mngr.addListener(l); }
            size_t removePointerListener(const PointerListenerRef& l) { return m_ptr_evt_mngr.removeListener(l); }
            size_t removeAllPointerListener() { return m_ptr_evt_mngr.removeAllListener(); }
            size_t pointerListenerCount() const noexcept { return m_ptr_evt_mngr.listenerCount(); }

            //
            //
            //

            void addRenderListener(const RenderListenerRef& l) { m_render_listener.push_back(l); }

            size_t removeRenderListener(const RenderListenerRef& l) {
                return m_render_listener.erase_matching(l, true,
                    [](const RenderListenerRef& a, const RenderListenerRef& b) noexcept -> bool { return a.get() == b.get(); } );
            }

            size_t removeAllRenderListener() {
                const size_t count = m_render_listener.size();
                m_render_listener.clear(true);
                return count;
            }

            size_t renderListenerCount() const noexcept { return m_render_listener.size(); }

            void display(const jau::fraction_timespec& when) noexcept;
            void disposeRenderListener(bool clearRenderListener, const jau::fraction_timespec& when) noexcept;

            void disposedNotified(const jau::fraction_timespec& when) noexcept override {
                notifyWindowEvent(EVENT_WINDOW_DESTROYED, when);
                Surface::disposedNotified(when);
                m_window_handle = 0;
            }
            void dispose(const jau::fraction_timespec& when) noexcept override {
                notifyWindowEvent(EVENT_WINDOW_DESTROY_NOTIFY, when);
                disposeRenderListener(true, when);
                Surface::dispose(when);
                if( m_window_handle ) {
                    disposeImpl(m_window_handle);
                    m_window_handle = 0;
                }
            }

            //
            //
            //

            std::string toString() const noexcept;
    };

    /**@}*/

}  // namespace gamp::wt

#endif /*  GAMP_WTWINDOW_HPP_ */
