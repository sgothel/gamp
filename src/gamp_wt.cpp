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

#include <gamp/GampTypes.hpp>
#include <gamp/wt/Surface.hpp>
#include <gamp/wt/event/KeyEvent.hpp>
#include <gamp/wt/Window.hpp>
#include <gamp/render/gl/GLHeader.hpp>
#include <stdexcept>
#include <jau/basic_types.hpp>
#include <jau/string_util.hpp>

using namespace jau;
using namespace gamp::wt::event;

class NonPrintableRange {
  public:
    /** min. unicode value, inclusive */
    uint16_t min;
    /** max. unicode value, inclusive */
    uint16_t max;
    /** true if valid for keyChar values as well, otherwise only valid for keyCode and keySym due to collision. */
    bool inclKeyChar;

    constexpr NonPrintableRange(uint16_t min_, uint16_t max_, bool inclKeyChar_) noexcept
    : min(min_), max(max_), inclKeyChar(inclKeyChar_) {}
};

/**
 * Non printable key ranges, currently fixed to an array of size 4.
 * <p>
 * Not included, queried upfront:
 * <ul>
 *  <li>{@link #BACK_SPACE}</li>
 *  <li>{@link #TAB}</li>
 *  <li>{@link #ENTER}</li>
 * </ul>
 * </p>
 */
static constexpr NonPrintableRange nonPrintableKeys[] = {
    { 0x0000, 0x001F, true },  // Unicode: Non printable controls: [0x00 - 0x1F], see exclusion above
    { 0x0061, 0x0078, false},  // Small 'a' thru 'z' (0x61 - 0x7a) - Not used for keyCode / keySym - Re-used for Fn (collision)
    { 0x008F, 0x009F, true },  // Unicode: Non printable controls: [0x7F - 0x9F], Numpad keys [0x7F - 0x8E] are printable!
    { 0xE000, 0xF8FF, true }   // Unicode: Private 0xE000 - 0xF8FF (Marked Non-Printable)
};


bool gamp::wt::event::isPrintableKey(uint16_t uniChar, bool isKeyChar) noexcept {
    const VKeyCode uniCode = VKeyCode(uniChar);
    if ( VKeyCode::VK_BACK_SPACE == uniCode || VKeyCode::VK_TAB == uniCode || VKeyCode::VK_ENTER == uniCode ) {
        return true;
    }
    if( !isKeyChar ) {
        if( ( /*nonPrintableKeys[0].min <= uniChar && */ uniChar <= nonPrintableKeys[0].max ) ||
            ( nonPrintableKeys[1].min <= uniChar && uniChar <= nonPrintableKeys[1].max ) ||
            ( nonPrintableKeys[2].min <= uniChar && uniChar <= nonPrintableKeys[2].max ) ||
            ( nonPrintableKeys[3].min <= uniChar && uniChar <= nonPrintableKeys[3].max ) ) {
            return false;
        }
    } else {
        if( ( /* nonPrintableKeys[0].inclKeyChar && nonPrintableKeys[0].min <= uniChar && */ uniChar <= nonPrintableKeys[0].max ) ||
            ( nonPrintableKeys[1].inclKeyChar && nonPrintableKeys[1].min <= uniChar && uniChar <= nonPrintableKeys[1].max ) ||
            ( nonPrintableKeys[2].inclKeyChar && nonPrintableKeys[2].min <= uniChar && uniChar <= nonPrintableKeys[2].max ) ||
            ( nonPrintableKeys[3].inclKeyChar && nonPrintableKeys[3].min <= uniChar && uniChar <= nonPrintableKeys[3].max ) ) {
            return false;
        }
    }
    return VKeyCode::none != uniCode;
}

void gamp::wt::Window::display(const jau::fraction_timespec& when) noexcept {
    if( !is_set(m_state, WindowState::visible) ) {
        return;
    }
    const jau::math::Recti viewport(0, 0, surfaceSize().x, surfaceSize().y);
    const WindowRef& self = shared();
    for(const RenderListenerRef& l : *m_render_listener.snapshot()) {
        std::exception_ptr eptr;
        try {
            const gamp::render::RenderContext* ctx = renderContext();
            bool initOK = ctx && ctx->isValid();
            if( initOK ) {
                if( is_set(l->pendingActions(), RenderActions::init) ) {
                    if( l->init(self, when) ) {
                        write(l->pendingActions(), RenderActions::init, false);
                    } else {
                        initOK = false;
                    }
                }
            }
            if( initOK ) {
                if( is_set(l->pendingActions(), RenderActions::reshape) ) {
                    ::glViewport(viewport.x(), viewport.y(), viewport.width(), viewport.height());
                    l->reshape(self, viewport, when);
                    write(l->pendingActions(), RenderActions::reshape, false);
                }
                l->display(self, when);
            }
        } catch (const std::exception &e) {
            eptr = std::current_exception();
            ERR_PRINT2("Caught exception %s", e.what());
        } catch (...) {
            eptr = std::current_exception();
            ERR_PRINT2("Caught unknown exception");
        }
        if( eptr ) {
            try {
                RenderListenerRef l2 = l;
                m_render_listener.erase_if(false,
                   [l](const RenderListenerRef& a) noexcept -> bool { return a.get() == l.get(); } );
                l2->dispose(self, when);
            } catch (const std::exception &e) {
                ERR_PRINT2("Caught exception %s", e.what());
            } catch (...) {
                ERR_PRINT2("Caught unknown exception");
            }
            WARN_PRINT("Removed listener (sz %zu)", m_render_listener.size());
        }
    }
}

void gamp::wt::Window::disposeRenderListener(bool clearRenderListener, const jau::fraction_timespec& when) noexcept {
    const WindowRef& self = shared();
    for(const RenderListenerRef& l : *m_render_listener.snapshot()) {
        try {
            l->dispose(self, when);
            write(l->pendingActions(), RenderActions::init, true);
            write(l->pendingActions(), RenderActions::reshape, true);
        } catch (std::exception &err) {
            ERR_PRINT("Window::display: %s: Caught exception %s", toString().c_str(), err.what());
        }
    }
    if( clearRenderListener ) {
        m_render_listener.clear(true);
    }
}

std::string gamp::wt::Window::toString() const noexcept {
    std::string res = "Window[";
    res.append(to_string(m_state))
       .append(", handle ").append(jau::toHexString(m_window_handle))
       .append(", bounds ").append(m_window_bounds.toString())
       .append(", listener[render ").append(std::to_string(m_render_listener.size()))
       .append(", window ").append(std::to_string(windowListenerCount()))
       .append(", key ").append(std::to_string(keyListenerCount()))
       .append("], ").append(Surface::toString())
       .append("]");
    return res;
}
