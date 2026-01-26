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
#ifndef GAMP_WTEVENT_HPP_
#define GAMP_WTEVENT_HPP_

#include <string>
#include <vector>

#include <jau/enum_util.hpp>
#include <jau/int_math_ct.hpp>
#include <jau/fraction_type.hpp>

namespace gamp::wt {
    class Window;
    typedef std::shared_ptr<Window> WindowSRef;
    typedef std::weak_ptr<Window> WindowWeakPtr;
}

namespace gamp::wt::event {
    /** \addtogroup Gamp_WT
     *
     *  @{
     */

    using namespace jau::enums;

    inline static constexpr uint16_t EVENT_WINDOW_RESIZED = 100;
    inline static constexpr uint16_t EVENT_WINDOW_MOVED   = 101;
    inline static constexpr uint16_t EVENT_WINDOW_DESTROY_NOTIFY = 102;
    inline static constexpr uint16_t EVENT_WINDOW_FOCUS_CHANGED = 103;
    inline static constexpr uint16_t EVENT_WINDOW_REPAINT = 104;
    inline static constexpr uint16_t EVENT_WINDOW_DESTROYED = 105;
    inline static constexpr uint16_t EVENT_WINDOW_VISIBILITY_CHANGED = 106;

    /** A key has been pressed, excluding {@link #isAutoRepeat() auto-repeat}-{@link #isModifierKey() modifier} keys. */
    inline static constexpr uint16_t EVENT_KEY_PRESSED      = 300;
    /** A key has been released, excluding {@link #isAutoRepeat() auto-repeat}-{@link #isModifierKey() modifier} keys. */
    inline static constexpr uint16_t EVENT_KEY_RELEASED     = 301;

    inline static constexpr short EVENT_POINTER_CLICKED       = 200;
    /** Only generated for PointerType::mouse */
    inline static constexpr short EVENT_POINTER_ENTERED       = 201;
    /** Only generated for PointerType::mouse */
    inline static constexpr short EVENT_POINTER_EXITED        = 202;
    inline static constexpr short EVENT_POINTER_PRESSED       = 203;
    inline static constexpr short EVENT_POINTER_RELEASED      = 204;
    inline static constexpr short EVENT_POINTER_MOVED         = 205;
    inline static constexpr short EVENT_POINTER_DRAGGED       = 206;
    inline static constexpr short EVENT_POINTER_WHEEL         = 207;

    class WTEvent {
      private:
        uint16_t m_type;
        jau::fraction_timespec m_when;
        WindowWeakPtr m_source;
        bool m_consumed;

      public:
        WTEvent(uint16_t type, const jau::fraction_timespec& when, const WindowSRef& source) noexcept
        : m_type(type), m_when(when), m_source(source), m_consumed(false) {}

        constexpr uint16_t type() const noexcept { return m_type; }
        constexpr const jau::fraction_timespec& when() const noexcept { return m_when; }
        constexpr const WindowWeakPtr& source() const noexcept { return m_source; }

        /** Consumed events will stop traversing through listener. */
        constexpr bool consumed() const noexcept { return m_consumed; }
        /** Consumed events will stop traversing through listener. */
        constexpr void setConsumed(bool v) noexcept { m_consumed = v; }
        std::string toString() const noexcept {
            std::string s = "WTEvent[consumed ";
            s.append(m_consumed ? "true" : "false");
            s.append(", when ").append(m_when.toString());
            s.append("]");
            return s;
        }
    };
    inline std::ostream& operator<<(std::ostream& out, const WTEvent& v) {
        return out << v.toString();
    }

    enum class InputButton : uint16_t {
        none       = 0,
        button1    = 1,
        button2    = 2,
        button3    = 3,
        button4    = 4,
        button5    = 5,
        button6    = 6,
        button7    = 7,
        button8    = 8,
        button9    = 9,
        button10   = 10,
        button11   = 11,
        button12   = 12,
        button13   = 13,
        button14   = 14,
        button15   = 15,
        button16   = 16,

        /// button property description, no actual bitmask value
        button_count = 16
    };
    JAU_MAKE_ENUM_INFO(InputButton,
                       button1, button2, button3, button4, button5, button6, button7, button8,
                       button9, button10, button11, button12, button13, button14, button15, button16);
    JAU_MAKE_ENUM_STRING(InputButton,
                         button1, button2, button3, button4, button5, button6, button7, button8,
                         button9, button10, button11, button12, button13, button14, button15, button16);

    enum class InputModifier : uint32_t {
        none       = 0,
        lshift     = 1U << 0,
        rshift     = 1U << 1,
        lctrl      = 1U << 2,
        rctrl      = 1U << 3,
        lalt       = 1U << 4,
        ralt       = 1U << 5,
        lmeta      = 1U << 6,
        rmeta      = 1U << 7,

        shift      = lshift | rshift,
        ctrl       = lctrl | rctrl,
        alt        = lalt | ralt,
        meta       = lmeta | rmeta,
        modifier   = shift | ctrl | alt | meta,

        /// button property description, no actual bitmask value
        button1_bit  =  8,

        button1    = 1U <<  button1_bit,
        button2    = 1U <<  9,
        button3    = 1U << 10,
        button4    = 1U << 11,
        button5    = 1U << 12,
        button6    = 1U << 13,
        button7    = 1U << 14,
        button8    = 1U << 15,
        button9    = 1U << 16,
        button10    = 1U << 17,
        button11    = 1U << 18,
        button12    = 1U << 19,
        button13    = 1U << 20,
        button14    = 1U << 21,
        button15    = 1U << 22,
        button16    = 1U << 23,

        /// button property description, no actual bitmask value
        button_last = button16,
        /// mask for all 16 buttons, see InputButton::button_count
        button_all  = 0xffffU << 8,

        /** Event is caused by auto-repeat. */
        repeat = 1U << 29,

        /** Pointer is confined, see {@link Window#confinePointer(boolean)}. */
        confined   = 1U << 30,

        /** Pointer is invisible, see {@link Window#setPointerVisible(boolean)}. */
        invisible  = 1U << 31
    };
    JAU_MAKE_BITFIELD_ENUM_STRING(InputModifier, lshift, rshift, lctrl, rctrl, lalt, ralt, lmeta, rmeta,
                                  button1, button2, button3, button4, button5, button6, button7, button8,
                                  button9, button10, button11, button12, button13, button14, button15, button16,
                                  repeat, confined, invisible);

    class InputEvent : public WTEvent {
      private:
        InputModifier m_mods;
      public:
        /**
          * Returns the corresponding button mask for the given button.
          * <p>
          * In case the given button lies outside
          * of the valid range [InputButton::button1 .. InputButton::button16],
          * InputModifier::none is returned.
          * </p>
          */
         constexpr static InputModifier buttonMask(InputButton button) noexcept {
             if( InputButton::none < button && button <= InputButton::button_count ) {
                 return InputModifier(1U << ( *InputModifier::button1_bit - 1 + *button ));
             }
             return InputModifier::none;
         }

        InputEvent(uint16_t type, const jau::fraction_timespec& when, const WindowSRef& source, InputModifier mods) noexcept
        : WTEvent(type, when, source), m_mods(mods) {}

        constexpr InputModifier modifier() const noexcept { return m_mods; }

        /** Use with single bits, e.g. InputModifier::lshift etc */
        constexpr bool is_set(InputModifier bits) const noexcept { return gamp::wt::event::is_set(m_mods, bits); }
        /** Use with groups of bits / mask, e.g. InputModifier::shift etc */
        constexpr bool has_any(InputModifier bits) const noexcept { return gamp::wt::event::has_any(m_mods, bits); }

        /**
         * See also {@link MouseEvent}'s section about <i>Multiple-Pointer Events</i>.
         * @param button the button to test
         * @return true if the given button is down
         */
        constexpr bool isButtonDown(InputButton button) const noexcept {
           return ( m_mods & buttonMask(button) ) != InputModifier::none;
        }

        /**
         * Returns the number of pressed buttons by counting the set bits:
         * <pre>
         *     jau::ct_bit_count(modifier() & InputModifier::button_all);
         * </pre>
         * <p>
         * See also {@link MouseEvent}'s section about <i>Multiple-Pointer Events</i>.
         * </p>
         * @see InputModifier::button_all
         */
        constexpr int buttonDownCount() const noexcept {
            return (int) jau::ct_bit_count(*m_mods & *InputModifier::button_all);
        }

        /**
         * Returns true if at least one button is pressed, otherwise false:
         * <pre>
         *     0 != ( modifier() & InputModifier::button_all )
         * </pre>
         * <p>
         * See also {@link MouseEvent}'s section about <i>Multiple-Pointer Events</i>.
         * </p>
         * @see InputModifier::button_all
         */
        constexpr bool isAnyButtonDown() const noexcept {
            return InputModifier::none != ( m_mods & InputModifier::button_all );
        }
        /**
         * See also {@link MouseEvent}'s section about <i>Multiple-Pointer Events</i>.
         * @return List of pressed mouse buttons  [InputButton::button1 .. InputButton::button16].
         *         If none is down, the resulting list is of length 0.
         */
        std::vector<InputButton> buttonsDown() const {
            const int len = buttonDownCount();
            std::vector<InputButton> res;
            res.reserve(len);

            const InputButton_info_t& ei = InputButton_info_t::get();
            InputButton_info_t::iterator end = ei.end();
            for(typename InputButton_info_t::iterator iter = ei.begin(); iter != end; ++iter) {
                const InputButton ev = *iter;
                if( isButtonDown(ev) ) { res.push_back( ev ); }
            }
            return res;
        }

        /** Returns true if modifier() contains InputModifier::alt */
        constexpr bool isAltDown() const noexcept {
            return has_any(InputModifier::alt);
        }
        /** Returns true if modifier() contains InputModifier::ctrl */
        constexpr bool isControlDown() const noexcept {
            return has_any(InputModifier::ctrl);
        }
        /** Returns true if modifier() contains InputModifier::meta */
        constexpr bool isMetaDown() const noexcept {
            return has_any(InputModifier::meta);
        }
        /** Returns true if modifier() contains InputModifier::shift */
        constexpr bool isShiftDown() const noexcept {
            return has_any(InputModifier::shift);
        }
        /** Returns true if modifier() contains InputModifier::autorepeat */
        constexpr bool isAutorepeat() const noexcept {
            return is_set(InputModifier::repeat);
        }

        /** Returns true if modifier() contains InputModifier::confined */
        constexpr bool isConfined() const noexcept {
            return is_set(InputModifier::confined);
        }
        /** Returns true if modifier() contains InputModifier::invisible */
        constexpr bool isInvisible() const noexcept {
            return is_set(InputModifier::invisible);
        }

        std::string toString() const noexcept {
            std::string s = "InputEvent[modifier";
            s.append(to_string(m_mods));
            s.append(", ").append(WTEvent::toString());
            s.append("]");
            return s;
        }
    };
    inline std::ostream& operator<<(std::ostream& out, const InputEvent& v) {
        return out << v.toString();
    }

    /**@}*/
}

#endif /* GAMP_WTEVENT_HPP_ */
