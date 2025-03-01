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
#ifndef GAMP_WTKEYBOARD_HPP_
#define GAMP_WTKEYBOARD_HPP_

#include <jau/bitfield.hpp>
#include <jau/int_types.hpp>
#include <jau/enum_util.hpp>
#include <jau/int_math_ct.hpp>
#include <jau/fraction_type.hpp>
#include <jau/cow_darray.hpp>

#include <gamp/wt/event/Event.hpp>
#include <gamp/wt/event/KeyEvent.hpp>

namespace gamp::wt::event {
    /** \addtogroup Gamp_WT
     *
     *  @{
     */

    class KeyEventManager : public KeyboardTracker {
      private:
        jau::cow_darray<KeyListenerRef> m_keyListener;
        PressedKeyCodes m_keybuffer;
        InputModifier m_modifiers;

        bool isTracked(VKeyCode keyCode) {
            return ( 0xFFFF & *keyCode ) < m_keybuffer.bit_size;
        }

        bool setPressed(VKeyCode keyCode, bool pressed) {
            const size_t v = 0xFFFFU & *keyCode;
            if( v < m_keybuffer.bit_size ) {
                m_keybuffer.put(v, pressed);
                return true;
            }
            return false;
        }

      public:
        KeyEventManager() noexcept
        : m_modifiers(InputModifier::none) { }

        InputModifier modifier() const noexcept override { return m_modifiers; }

        bool isPressed(VKeyCode keyCode) const noexcept override {
            const size_t v = 0xFFFFU & *keyCode;
            if( v < m_keybuffer.bit_size ) {
                return m_keybuffer.get(v);
            }
            return false;
        }

        const PressedKeyCodes& pressedKeyCodes() const noexcept override { return m_keybuffer; }

        void dispatchPressed(const jau::fraction_timespec& when, const WindowRef& source,
                             VKeyCode keySym, InputModifier keySymMods, uint16_t keyChar) noexcept
        {
            const InputModifier clrRepeatMod =  is_set(m_modifiers, InputModifier::repeat) &&
                                               !is_set(keySymMods, InputModifier::repeat) ? InputModifier::repeat : InputModifier::none;
            m_modifiers |= keySymMods;
            m_modifiers &= ~clrRepeatMod;
            KeyEvent ke(EVENT_KEY_PRESSED, when, source, m_modifiers, keySym, keySymMods, keyChar);
            setPressed(keySym, true);
            for(const KeyListenerRef& kl : *m_keyListener.snapshot()) {
                try {
                    kl->keyPressed(ke, *this);
                } catch (std::exception &err) {
                    ERR_PRINT("KeyboardManager::dispatch (p): %s: Caught exception %s", ke.toString().c_str(), err.what());
                }
            }
        }
        void dispatchReleased(const jau::fraction_timespec& when, const WindowRef& source,
                              VKeyCode keySym, InputModifier keySymMods, uint16_t keyChar) noexcept
        {
            const InputModifier clrRepeatMod =  is_set(m_modifiers, InputModifier::repeat) &&
                                               !is_set(keySymMods, InputModifier::repeat) ? InputModifier::repeat : InputModifier::none;
            m_modifiers &= ~clrRepeatMod;
            KeyEvent ke(EVENT_KEY_RELEASED, when, source, m_modifiers, keySym, keySymMods, keyChar);
            m_modifiers &= ~keySymMods;
            setPressed(keySym, false);
            for(const KeyListenerRef& kl : *m_keyListener.snapshot()) {
                try {
                    kl->keyReleased(ke, *this);
                } catch (std::exception &err) {
                    ERR_PRINT("KeyboardManager::dispatch (r): %s: Caught exception %s", ke.toString().c_str(), err.what());
                }
            }
        }

        void addListener(const KeyListenerRef& l) { m_keyListener.push_back(l); }

        size_t removeListener(const KeyListenerRef& l) {
            return m_keyListener.erase_matching(l, true,
                [](const KeyListenerRef& a, const KeyListenerRef& b) noexcept -> bool { return a.get() == b.get(); } );
        }

        size_t removeAllListener() {
            const size_t count = m_keyListener.size();
            m_keyListener.clear(true);
            return count;
        }

        size_t listenerCount() const noexcept { return m_keyListener.size(); }
    };

    /**@}*/
}

#endif /* GAMP_WTKEYBOARD_HPP_ */
