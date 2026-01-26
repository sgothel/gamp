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
#ifndef GAMP_WTWINMNGR_HPP_
#define GAMP_WTWINMNGR_HPP_

#include <jau/bitfield.hpp>
#include <jau/int_types.hpp>
#include <jau/enum_util.hpp>
#include <jau/int_math_ct.hpp>
#include <jau/fraction_type.hpp>
#include <jau/cow_darray.hpp>

#include <gamp/wt/event/Event.hpp>
#include <gamp/wt/event/WinEvent.hpp>

namespace gamp::wt::event {
    /** \addtogroup Gamp_WT
     *
     *  @{
     */

    class WindowEventManager {
      private:
        jau::cow_darray<WindowListenerSRef> m_windowListener;

      public:
        void dispatch(uint16_t type, const jau::fraction_timespec& when, const WindowSRef& source, bool value=true) noexcept {
            WindowEvent evt(type, when, source);
            for(const WindowListenerSRef& kl : *m_windowListener.snapshot()) {
                try {
                    switch(evt.type()) {
                        case EVENT_WINDOW_DESTROY_NOTIFY: kl->windowDestroyNotify(evt); break;
                        case EVENT_WINDOW_DESTROYED:      kl->windowDestroyed(evt); break;
                        case EVENT_WINDOW_FOCUS_CHANGED:  kl->windowFocusChanged(evt, value); break;
                        case EVENT_WINDOW_REPAINT:        kl->windowRepaint(evt); break;
                        case EVENT_WINDOW_VISIBILITY_CHANGED: kl->windowVisibilityChanged(evt, value); break;
                        default: break;
                    }
                } catch (std::exception &err) {
                    ERR_PRINT("WindowManager::dispatch: %s: Caught exception %s", evt.toString().c_str(), err.what());
                }
                if( evt.consumed() ) { break; }
            }
        }
        void dispatchResize(const jau::fraction_timespec& when, const WindowSRef& source,
                            const jau::math::Vec2i& winSize, const jau::math::Vec2i& surfSize) noexcept {
            WindowEvent evt(EVENT_WINDOW_RESIZED, when, source);
            for(const WindowListenerSRef& kl : *m_windowListener.snapshot()) {
                try {
                    kl->windowResized(evt, winSize, surfSize); break;
                } catch (std::exception &err) {
                    ERR_PRINT("WindowManager::dispatch: %s: Caught exception %s", evt.toString().c_str(), err.what());
                }
                if( evt.consumed() ) { break; }
            }
        }
        void dispatchMoved(const jau::fraction_timespec& when, const WindowSRef& source,
                           const jau::math::Vec2i& winPos) noexcept {
            WindowEvent evt(EVENT_WINDOW_MOVED, when, source);
            for(const WindowListenerSRef& kl : *m_windowListener.snapshot()) {
                try {
                    kl->windowMoved(evt, winPos); break;
                } catch (std::exception &err) {
                    ERR_PRINT("WindowManager::dispatch: %s: Caught exception %s", evt.toString().c_str(), err.what());
                }
                if( evt.consumed() ) { break; }
            }
        }

        void addListener(const WindowListenerSRef& l) { m_windowListener.push_back(l); }

        size_t removeListener(const WindowListenerSRef& l) {
            return m_windowListener.erase_matching(l, true,
                [](const WindowListenerSRef& a, const WindowListenerSRef& b) noexcept -> bool { return a.get() == b.get(); } );
        }

        size_t removeAllListener() {
            const size_t count = m_windowListener.size();
            m_windowListener.clear(true);
            return count;
        }

        size_t listenerCount() const noexcept { return m_windowListener.size(); }
    };

    /**@}*/
}

#endif /* GAMP_WTWINMNGR_HPP_ */
