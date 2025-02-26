/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2025 Gothel Software e.K.
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
#ifndef GAMP_WTWINEVENT_HPP_
#define GAMP_WTWINEVENT_HPP_

#include <jau/bitfield.hpp>
#include <jau/int_types.hpp>
#include <jau/enum_util.hpp>
#include <jau/int_math_ct.hpp>
#include <jau/fraction_type.hpp>
#include <jau/cow_darray.hpp>
#include <jau/math/recti.hpp>

#include <gamp/wt/event/event.hpp>

namespace gamp::wt::event {
    /** \addtogroup Gamp_WT
     *
     *  @{
     */
    class WindowEvent : public WTEvent {
      private:

        std::string getEventTypeString() const noexcept {
            switch(type()) {
                case EVENT_WINDOW_RESIZED: return "RESIZED";
                case EVENT_WINDOW_MOVED: return "MOVED";
                case EVENT_WINDOW_DESTROY_NOTIFY: return "DESTROY_NOTIFY";
                case EVENT_WINDOW_FOCUS_CHANGED: return "FOCUS";
                case EVENT_WINDOW_REPAINT: return "REPAINT";
                case EVENT_WINDOW_DESTROYED: return "DESTROYED";
                case EVENT_WINDOW_VISIBILITY_CHANGED: return "VISIBILITY";
                default: return "unknown (" + std::to_string(type()) + ")";
            }
        }

      public:
        WindowEvent(uint16_t type, const jau::fraction_timespec& when, const WindowRef& source) noexcept
        : WTEvent(type, when, source)
        { }

        std::string toString() const noexcept {
            std::string res = "WindowEvent[";
            res.append(getEventTypeString()).append(", ").append(WTEvent::toString()).append("]");
            return res;
        }
    };
    inline std::ostream& operator<<(std::ostream& out, const WindowEvent& v) {
        return out << v.toString();
    }

    /**
     * Listener for multiple WindowEvent.
     *
     * @see WindowEvent
     */
    class WindowListener
    {
      public:
        virtual ~WindowListener() noexcept = default;

        /** Window is resized, your application shall respect the new window-size in window units and surface-size in pixel. A repaint is recommended. */
        virtual void windowResized(const WindowEvent&, const jau::math::Vec2i& /*winSize*/, const jau::math::Vec2i& /*surfSize*/) {}

        /** Window has been moved to given windows-position in window units. */
        virtual void windowMoved(const WindowEvent&, const jau::math::Vec2i& /*winPos*/) {}

        /**
         * Window destruction has been requested.
         * <p>
         * Depending on the {@link WindowClosingProtocol#getDefaultCloseOperation() default close operation},
         * the window maybe destroyed or not.
         * </p>
         * In case the window will be destroyed (see above), release of resources is recommended.
         **/
        virtual void windowDestroyNotify(const WindowEvent&) {}

        /**
         * Window has been destroyed.
         */
        virtual void windowDestroyed(const WindowEvent&) {}

        /** Window gained or lost focus. */
        virtual void windowFocusChanged(const WindowEvent&, bool /*focused*/) {}

        /** Window area shall be repainted. */
        virtual void windowRepaint(const WindowEvent&) {} // FIXME: WindowUpdateEvent

        /** Window visibility changed. */
        virtual void windowVisibilityChanged(const WindowEvent&, bool /*visible*/) {}
    };
    typedef std::shared_ptr<WindowListener> WindowListenerRef;

    /**@}*/
}

#endif /* GAMP_WTWINEVENT_HPP_ */
