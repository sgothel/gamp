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
#ifndef GAMP_WTPOINTERMNNGR_HPP_
#define GAMP_WTPOINTERMNNGR_HPP_

#include <jau/bitfield.hpp>
#include <jau/int_types.hpp>
#include <jau/enum_util.hpp>
#include <jau/int_math_ct.hpp>
#include <jau/fraction_type.hpp>
#include <jau/cow_darray.hpp>

#include <gamp/wt/event/event.hpp>
#include <gamp/wt/event/keyevent.hpp>
#include <gamp/wt/event/keyeventmngr.hpp>
#include <gamp/wt/event/pointerevent.hpp>
#include <vector>

namespace gamp::wt::event {
    /** \addtogroup Gamp_WT
     *
     *  @{
     */

    class PointerEventManager {
      private:
        jau::cow_darray<PointerListenerRef> m_pointerListener;
        std::vector<InputButton> m_buttonDown;

        // TODO
        // - CLICKED
        // - ENTERED/EXITED

      public:
        void dispatch(uint16_t type, const jau::fraction_timespec& when, const WindowRef& source, InputModifier mods,
                      PointerType ptype, uint16_t id,
                      jau::math::Vec2i pos, uint16_t clickCount, InputButton button,
                      jau::math::Vec3f rotation, float rotationScale) noexcept {
            if( InputButton::none != button ) {
                if( EVENT_POINTER_PRESSED == type ) {
                    m_buttonDown.push_back(button);
                } else if( EVENT_POINTER_RELEASED == type ) {
                    std::erase(m_buttonDown, button);
                }
            }
            if( EVENT_POINTER_MOVED == type && m_buttonDown.size() > 0 ) {
                type = EVENT_POINTER_DRAGGED;
            }
            const PointerEvent evt(type, when, source, mods, ptype, id, pos, clickCount, button, rotation, rotationScale);
            for(const PointerListenerRef& kl : *m_pointerListener.snapshot()) {
                try {
                    switch(type) {
                        case EVENT_POINTER_CLICKED:
                            kl->pointerClicked(evt); break;
                        case EVENT_POINTER_ENTERED:
                            kl->pointerEntered(evt); break;
                        case EVENT_POINTER_EXITED:
                            kl->pointerExited(evt); break;
                        case EVENT_POINTER_PRESSED:
                            kl->pointerPressed(evt); break;
                        case EVENT_POINTER_RELEASED:
                            kl->pointerReleased(evt); break;
                        case EVENT_POINTER_MOVED:
                            kl->pointerMoved(evt); break;
                        case EVENT_POINTER_DRAGGED:
                            kl->pointerDragged(evt); break;
                        case EVENT_POINTER_WHEEL:
                            kl->pointerWheelMoved(evt); break;
                        default: break;
                    }
                } catch (std::exception &err) {
                    ERR_PRINT("PointerManager::dispatch: %s: Caught exception %s", evt.toString().c_str(), err.what());
                }
            }
        }

        void addListener(const PointerListenerRef& l) { m_pointerListener.push_back(l); }

        size_t removeListener(const PointerListenerRef& l) {
            return m_pointerListener.erase_matching(l, true,
                [](const PointerListenerRef& a, const PointerListenerRef& b) noexcept -> bool { return a.get() == b.get(); } );
        }

        size_t removeAllListener() {
            const size_t count = m_pointerListener.size();
            m_pointerListener.clear(true);
            return count;
        }

        size_t listenerCount() const noexcept { return m_pointerListener.size(); }
    };

    /**@}*/
}

#endif /* GAMP_WTPOINTERMNNGR_HPP_ */
