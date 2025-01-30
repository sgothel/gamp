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
#ifndef GAMP_WTKEYEVENT_HPP_
#define GAMP_WTKEYEVENT_HPP_

#include <memory>
#include <string>
#include <jau/bitfield.hpp>
#include <jau/cpp_lang_util.hpp>
#include <jau/enum_util.hpp>
#include <jau/int_math_ct.hpp>
#include <jau/fraction_type.hpp>
#include <jau/string_util.hpp>

#include <gamp/wt/wtevent.hpp>

namespace gamp::wt::event {

    /// Virtual key code following UTF16 specification
    enum class VKeyCode : uint16_t {
        //
        // Unicode: Non printable controls: [0x00 - 0x1F]
        //

        /**
         * This value, {@value}, is used to indicate that the keyCode is unknown.
         */
        VK_UNDEFINED   = 0,

        /// Alias for VK_UNDEFINED
        none           = VK_UNDEFINED,

        VK_FREE01      = 0x01u,

        /** Constant for the HOME function key. ASCII: Start Of Text. */
        VK_HOME           = 0x02u,

        /** Constant for the END function key. ASCII: End Of Text. */
        VK_END            = 0x03u,

        /** Constant for the END function key. ASCII: End Of Transmission. */
        VK_FINAL          = 0x04u,

        /** Constant for the PRINT function key. ASCII: Enquiry. */
        VK_PRINTSCREEN    = 0x05u,

        VK_FREE06      = 0x06u,
        VK_FREE07      = 0x07u,

        /** Constant for the BACK SPACE key "\b"u, matching ASCII. Printable! */
        VK_BACK_SPACE     = 0x08u,

        /** Constant for the HORIZ TAB key "\t"u, matching ASCII. Printable! */
        VK_TAB            = 0x09u,

        /** LINE_FEED "\n"u, matching ASCIIu, n/a on keyboard. */
        VK_LINEFEED    = 0x0Au,

        /** Constant for the PAGE DOWN function key. ASCII: Vertical Tabulation. */
        VK_PAGE_DOWN      = 0x0Bu,

        /** Constant for the CLEAR keyu, i.e. FORM FEEDu, matching ASCII. */
        VK_CLEAR          = 0x0Cu,

        /** Constant for the ENTER keyu, i.e. CARRIAGE RETURNu, matching ASCII. Printable! */
        VK_ENTER          = 0x0Du,

        VK_FREE0E         = 0x0Eu,

        /** Constant for the SHIFT function key. ASCII: shift-in. */
        VK_SHIFT          = 0x0Fu,

        /** Constant for the PAGE UP function key. ASCII: Data Link Escape. */
        VK_PAGE_UP        = 0x10u,

        /** Constant for the CTRL function key. ASCII: device-ctrl-one. */
        VK_CONTROL        = 0x11u,

        /** Constant for the ALT function key. ASCII: device-ctrl-two. */
        VK_ALT            = 0x12u,

        VK_FREE13         = 0x13u,

        /** Constant for the CAPS LOCK function key. ASCII: device-ctrl-four. */
        VK_CAPS_LOCK      = 0x14u,

        VK_FREE15      = 0x15u,

        /** Constant for the PAUSE function key. ASCII: sync-idle. */
        VK_PAUSE          = 0x16u,

        /** <b>scroll lock</b> key. ASCII: End Of Transmission Block. */
        VK_SCROLL_LOCK    = 0x17u,

        /** Constant for the CANCEL function key. ASCII: Cancel. */
        VK_CANCEL         = 0x18u,

        VK_FREE19      = 0x19u,

        /** Constant for the INSERT function key. ASCII: Substitute. */
        VK_INSERT         = 0x1Au,

        /** Constant for the ESCAPE function key. ASCII: Escape. */
        VK_ESCAPE         = 0x1Bu,

        /** Constant for the Convert function keyu, Japanese "henkan". ASCII: File Separator. */
        VK_CONVERT        = 0x1Cu,

        /** Constant for the Don't Convert function keyu, Japanese "muhenkan". ASCII: Group Separator.*/
        VK_NONCONVERT     = 0x1Du,

        /** Constant for the Accept or Commit function keyu, Japanese "kakutei". ASCII: Record Separator.*/
        VK_ACCEPT         = 0x1Eu,

        /** Constant for the Mode Change (?). ASCII: Unit Separator.*/
        VK_MODECHANGE     = 0x1Fu,

        //
        // Unicode: Printable [0x20 - 0x7E]
        // NOTE: Collision of 'a' - 'x' [0x61 .. 0x78]u, used for keyCode/keySym Fn function keys
        //

        /** Constant for the SPACE function key. ASCII: SPACE. */
        VK_SPACE          = 0x20u,

        /** Constant for the "!" key. */
        VK_EXCLAMATION_MARK = 0x21u,

        /** Constant for the """ key. */
        VK_QUOTEDBL       = 0x22u,

        /** Constant for the "#" key. */
        VK_NUMBER_SIGN    = 0x23u,

        /** Constant for the "$" key. */
        VK_DOLLAR         = 0x24u,

        /** Constant for the "%" key. */
        VK_PERCENT        = 0x25u,

        /** Constant for the "&" key. */
        VK_AMPERSAND      = 0x26u,

        /** Constant for the "'" key. */
        VK_QUOTE          = 0x27u,

        /** Constant for the "(" key. */
        VK_LEFT_PARENTHESIS  = 0x28u,

        /** Constant for the ")" key. */
        VK_RIGHT_PARENTHESIS = 0x29u,

        /** Constant for the "*" key */
        VK_ASTERISK       = 0x2Au,

        /** Constant for the "+" key. */
        VK_PLUS           = 0x2Bu,

        /** Constant for the comma keyu, "u," */
        VK_COMMA          = 0x2Cu,

        /** Constant for the minus keyu, "-" */
        VK_MINUS          = 0x2Du,

        /** Constant for the period keyu, "." */
        VK_PERIOD         = 0x2Eu,

        /** Constant for the forward slash keyu, "/" */
        VK_SLASH          = 0x2Fu,

        /** VK_0 thru VK_9 are the same as UTF16/ASCII '0' thru '9' [0x30 - 0x39] */
        VK_0           = 0x30u,
        /** See {@link #VK_0}. */
        VK_1           = 0x31u,
        /** See {@link #VK_0}. */
        VK_2           = 0x32u,
        /** See {@link #VK_0}. */
        VK_3           = 0x33u,
        /** See {@link #VK_0}. */
        VK_4           = 0x34u,
        /** See {@link #VK_0}. */
        VK_5           = 0x35u,
        /** See {@link #VK_0}. */
        VK_6           = 0x36u,
        /** See {@link #VK_0}. */
        VK_7           = 0x37u,
        /** See {@link #VK_0}. */
        VK_8           = 0x38u,
        /** See {@link #VK_0}. */
        VK_9           = 0x39u,

        /** Constant for the ":" key. */
        VK_COLON          = 0x3Au,

        /** Constant for the semicolon keyu, "u," */
        VK_SEMICOLON      = 0x3Bu,

        /** Constant for the equals keyu, "<" */
        VK_LESS           = 0x3Cu,

        /** Constant for the equals keyu, "=" */
        VK_EQUALS         = 0x3Du,

        /** Constant for the equals keyu, ">" */
        VK_GREATER        = 0x3Eu,

        /** Constant for the equals keyu, "?" */
        VK_QUESTIONMARK   = 0x3Fu,

        /** Constant for the equals keyu, "@" */
        VK_AT             = 0x40u,

        /** VK_A thru VK_Z are the same as Capital UTF16/ASCII 'A' thru 'Z' (0x41 - 0x5A) */
        VK_A              = 0x41u,
        /** See {@link #VK_A}. */
        VK_B              = 0x42u,
        /** See {@link #VK_A}. */
        VK_C              = 0x43u,
        /** See {@link #VK_A}. */
        VK_D              = 0x44u,
        /** See {@link #VK_A}. */
        VK_E              = 0x45u,
        /** See {@link #VK_A}. */
        VK_F              = 0x46u,
        /** See {@link #VK_A}. */
        VK_G              = 0x47u,
        /** See {@link #VK_A}. */
        VK_H              = 0x48u,
        /** See {@link #VK_A}. */
        VK_I              = 0x49u,
        /** See {@link #VK_A}. */
        VK_J              = 0x4Au,
        /** See {@link #VK_A}. */
        VK_K              = 0x4Bu,
        /** See {@link #VK_A}. */
        VK_L              = 0x4Cu,
        /** See {@link #VK_A}. */
        VK_M              = 0x4Du,
        /** See {@link #VK_A}. */
        VK_N              = 0x4Eu,
        /** See {@link #VK_A}. */
        VK_O              = 0x4Fu,
        /** See {@link #VK_A}. */
        VK_P              = 0x50u,
        /** See {@link #VK_A}. */
        VK_Q              = 0x51u,
        /** See {@link #VK_A}. */
        VK_R              = 0x52u,
        /** See {@link #VK_A}. */
        VK_S              = 0x53u,
        /** See {@link #VK_A}. */
        VK_T              = 0x54u,
        /** See {@link #VK_A}. */
        VK_U              = 0x55u,
        /** See {@link #VK_A}. */
        VK_V              = 0x56u,
        /** See {@link #VK_A}. */
        VK_W              = 0x57u,
        /** See {@link #VK_A}. */
        VK_X              = 0x58u,
        /** See {@link #VK_A}. */
        VK_Y              = 0x59u,
        /** See {@link #VK_A}. */
        VK_Z              = 0x5Au,

        /** Constant for the open bracket keyu, "[" */
        VK_OPEN_BRACKET   = 0x5Bu,

        /**Constant for the back slash keyu, "\" */
        VK_BACK_SLASH     = 0x5Cu,

        /** Constant for the close bracket keyu, "]" */
        VK_CLOSE_BRACKET  = 0x5Du,

        /** Constant for the "^" key. */
        VK_CIRCUMFLEX     = 0x5Eu,

        /** Constant for the "_" key */
        VK_UNDERSCORE     = 0x5Fu,

        /** Constant for the "`" key */
        VK_BACK_QUOTE     = 0x60u,

        /** Small UTF/ASCII 'a' thru 'z' (0x61 - 0x7a) - Not used for keyCode / keySym. */

        /**
         * Constant for the F<i>n</i> function keys.
         * <p>
         * F1..F24u, i.e. F<i>n</i>u, are mapped from on <code>0x60+n</code> -> <code>[0x61 .. 0x78]</code>.
         * </p>
         * <p>
         * <b>Warning:</b> The F<i>n</i> function keys <b>do collide</b> with unicode characters small 'a' thru 'x'!<br/>
         * See <a href="#unicodeCollision">Unicode Collision</a> for details.
         * </p>
         */
        VK_F1             = ( 0x60U + 1U ),

        /** Constant for the F2 function key. See {@link #VK_F1}. */
        VK_F2             = ( 0x60U + 2U ),

        /** Constant for the F3 function key. See {@link #VK_F1}. */
        VK_F3             = ( 0x60U + 3U ),

        /** Constant for the F4 function key. See {@link #VK_F1}. */
        VK_F4             = ( 0x60U + 4U ),

        /** Constant for the F5 function key. See {@link #VK_F1}. */
        VK_F5             = ( 0x60U + 5U ),

        /** Constant for the F6 function key. See {@link #VK_F1}. */
        VK_F6             = ( 0x60U + 6U ),

        /** Constant for the F7 function key. See {@link #VK_F1}. */
        VK_F7             = ( 0x60U + 7U ),

        /** Constant for the F8 function key. See {@link #VK_F1}. */
        VK_F8             = ( 0x60U + 8U ),

        /** Constant for the F9 function key. See {@link #VK_F1}. */
        VK_F9             = ( 0x60U + 9U ),

        /** Constant for the F11 function key. See {@link #VK_F1}. */
        VK_F10            = ( 0x60U +10U ),

        /** Constant for the F11 function key. See {@link #VK_F1}. */
        VK_F11            = ( 0x60U +11U ),

        /** Constant for the F12 function key. See {@link #VK_F1}.*/
        VK_F12            = ( 0x60U +12U ),

        /** Constant for the F13 function key. See {@link #VK_F1}. */
        VK_F13            = ( 0x60U +13U ),

        /** Constant for the F14 function key. See {@link #VK_F1}. */
        VK_F14            = ( 0x60U +14U ),

        /** Constant for the F15 function key. See {@link #VK_F1}. */
        VK_F15            = ( 0x60U +15U ),

        /** Constant for the F16 function key. See {@link #VK_F1}. */
        VK_F16            = ( 0x60U +16U ),

        /** Constant for the F17 function key. See {@link #VK_F1}. */
        VK_F17            = ( 0x60U +17U ),

        /** Constant for the F18 function key. See {@link #VK_F1}. */
        VK_F18            = ( 0x60U +18U ),

        /** Constant for the F19 function key. See {@link #VK_F1}. */
        VK_F19            = ( 0x60U +19U ),

        /** Constant for the F20 function key. See {@link #VK_F1}. */
        VK_F20            = ( 0x60U +20U ),

        /** Constant for the F21 function key. See {@link #VK_F1}. */
        VK_F21            = ( 0x60U +21U ),

        /** Constant for the F22 function key. See {@link #VK_F1}. */
        VK_F22            = ( 0x60U +22U ),

        /** Constant for the F23 function key. See {@link #VK_F1}. */
        VK_F23            = ( 0x60U +23U ),

        /** Constant for the F24 function key. See {@link #VK_F1}. */
        VK_F24            = ( 0x60U +24U ),


        /** Constant for the "{" key */
        VK_LEFT_BRACE     = 0x7Bu,
        /** Constant for the "|" key */
        VK_PIPE           = 0x7Cu,
        /** Constant for the "}" key */
        VK_RIGHT_BRACE    = 0x7Du,

        /** Constant for the "~" keyu, matching ASCII */
        VK_TILDE          = 0x7Eu,

        //
        // Unicode: Non printable controls: [0x7F - 0x9F]
        //
        // Numpad keys [0x7F - 0x8E] are printable
        //

        /** Numeric keypad <b>decimal separator</b> key. Non printable UTF control. */
        VK_SEPARATOR      = 0x7Fu,

        /** Numeric keypad VK_NUMPAD0 thru VK_NUMPAD9 are mapped to UTF control (0x80 - 0x89). Non printable UTF control. */
        VK_NUMPAD0        = 0x80u,
        /** See {@link #VK_NUMPAD0}. */
        VK_NUMPAD1        = 0x81u,
        /** See {@link #VK_NUMPAD0}. */
        VK_NUMPAD2        = 0x82u,
        /** See {@link #VK_NUMPAD0}. */
        VK_NUMPAD3        = 0x83u,
        /** See {@link #VK_NUMPAD0}. */
        VK_NUMPAD4        = 0x84u,
        /** See {@link #VK_NUMPAD0}. */
        VK_NUMPAD5        = 0x85u,
        /** See {@link #VK_NUMPAD0}. */
        VK_NUMPAD6        = 0x86u,
        /** See {@link #VK_NUMPAD0}. */
        VK_NUMPAD7        = 0x87u,
        /** See {@link #VK_NUMPAD0}. */
        VK_NUMPAD8        = 0x88u,
        /** See {@link #VK_NUMPAD0}. */
        VK_NUMPAD9        = 0x89u,

        /** Numeric keypad <b>decimal separator</b> key. Non printable UTF control. */
        VK_DECIMAL        = 0x8Au,

        /** Numeric keypad <b>add</b> key. Non printable UTF control. */
        VK_ADD            = 0x8Bu,

        /** Numeric keypad <b>subtract</b> key. Non printable UTF control. */
        VK_SUBTRACT       = 0x8Cu,

        /** Numeric keypad <b>multiply</b> key. Non printable UTF control. */
        VK_MULTIPLY       = 0x8Du,

        /** Numeric keypad <b>divide</b> key. Non printable UTF control. */
        VK_DIVIDE         = 0x8Eu,

        /** Constant for the DEL keyu, matching ASCII. Non printable UTF control. */
        VK_DELETE         = 0x93u,

        /** Numeric keypad <b>num lock</b> key. Non printable UTF control. */
        VK_NUM_LOCK       = 0x94u,

        /** Constant for the cursor- or numerical-pad <b>left</b> arrow key. Non printable UTF control. */
        VK_LEFT           = 0x95u,

        /** Constant for the cursor- or numerical-pad <b>up</b> arrow key. Non printable UTF control. */
        VK_UP             = 0x96u,

        /** Constant for the cursor- or numerical-pad <b>right</b> arrow key. Non printable UTF control. */
        VK_RIGHT          = 0x97u,

        /** Constant for the cursor- or numerical pad <b>down</b> arrow key. Non printable UTF control. */
        VK_DOWN           = 0x98u,

        /** Constant for the Context Menu key. Non printable UTF control. */
        VK_CONTEXT_MENU   = 0x99u,

        /**
         * Constant for the MS "Windows" function key.
         * It is used for both the left and right version of the key.
         */
        VK_WINDOWS        = 0x9Au,

        /** Constant for the Meta function key. */
        VK_META           = 0x9Bu,

        /** Constant for the Help function key. */
        VK_HELP           = 0x9Cu,

        /** Constant for the Compose function key. */
        VK_COMPOSE        = 0x9Du,

        /** Constant for the Begin function key. */
        VK_BEGIN          = 0x9Eu,

        /** Constant for the Stop function key. */
        VK_STOP           = 0x9Fu,

        //
        // Unicode: Printable [0x00A0 - 0xDFFF]
        //

        /** Constant for the inverted exclamation mark key. */
        VK_INVERTED_EXCLAMATION_MARK = 0xA1u,

        /** Constant for the Euro currency sign key. */
        VK_EURO_SIGN                = 0x20ACu,

        //
        // Unicode: Private 0xE000 - 0xF8FF (Marked Non-Printable)
        //

        /* for Sun keyboards */
        VK_CUT            = 0xF879u,
        VK_COPY           = 0xF87Au,
        VK_PASTE          = 0xF87Bu,
        VK_UNDO           = 0xF87Cu,
        VK_AGAIN          = 0xF87Du,
        VK_FIND           = 0xF87Eu,
        VK_PROPS          = 0xF87Fu,

        /* for input method support on Asian Keyboards */

        /**
         * Constant for the input method on/off key.
         */
        /* Japanese PC 106 keyboard: kanji. Japanese Solaris keyboard: nihongo */
        VK_INPUT_METHOD_ON_OFF = 0xF890u,

        /**
         * Constant for the Code Input function key.
         */
        /* Japanese PC 106 keyboard - VK_ALPHANUMERIC + ALT: kanji bangou */
        VK_CODE_INPUT = 0xF891u,

        /**
         * Constant for the Roman Characters function key.
         */
        /* Japanese PC 106 keyboard: roumaji */
        VK_ROMAN_CHARACTERS = 0xF892u,

        /**
         * Constant for the All Candidates function key.
         */
        /* Japanese PC 106 keyboard - VK_CONVERT + ALT: zenkouho */
        VK_ALL_CANDIDATES = 0xF893u,

        /**
         * Constant for the Previous Candidate function key.
         */
        /* Japanese PC 106 keyboard - VK_CONVERT + SHIFT: maekouho */
        VK_PREVIOUS_CANDIDATE = 0xF894u,

        /**
         * Constant for the Alphanumeric function key.
         */
        /* Japanese PC 106 keyboard: eisuu */
        VK_ALPHANUMERIC   = 0xF895u,

        /**
         * Constant for the Katakana function key.
         */
        /* Japanese PC 106 keyboard: katakana */
        VK_KATAKANA       = 0xF896u,

        /**
         * Constant for the Hiragana function key.
         */
        /* Japanese PC 106 keyboard: hiragana */
        VK_HIRAGANA       = 0xF897u,

        /**
         * Constant for the Full-Width Characters function key.
         */
        /* Japanese PC 106 keyboard: zenkaku */
        VK_FULL_WIDTH     = 0xF898u,

        /**
         * Constant for the Half-Width Characters function key.
         */
        /* Japanese PC 106 keyboard: hankaku */
        VK_HALF_WIDTH     = 0xF89Au,

        /**
         * Constant for the Japanese-Katakana function key.
         * This key switches to a Japanese input method and selects its Katakana input mode.
         */
        /* Japanese Macintosh keyboard - VK_JAPANESE_HIRAGANA + SHIFT */
        VK_JAPANESE_KATAKANA = 0xF89Bu,

        /**
         * Constant for the Japanese-Hiragana function key.
         * This key switches to a Japanese input method and selects its Hiragana input mode.
         */
        /* Japanese Macintosh keyboard */
        VK_JAPANESE_HIRAGANA = 0xF89Cu,

        /**
         * Constant for the Japanese-Roman function key.
         * This key switches to a Japanese input method and selects its Roman-Direct input mode.
         */
        /* Japanese Macintosh keyboard */
        VK_JAPANESE_ROMAN = 0xF89Du,

        /**
         * Constant for the locking Kana function key.
         * This key locks the keyboard into a Kana layout.
         */
        /* Japanese PC 106 keyboard with special Windows driver - eisuu + Controlu, Japanese Solaris keyboard: kana */
        VK_KANA_LOCK = 0xF89Fu,

        /**
         * Constant for Keyboard became invisible, e.g. Android's soft keyboard Back button hit while keyboard is visible.
         */
        VK_KEYBOARD_INVISIBLE = 0xF8FFu

    };
    constexpr uint16_t operator*(const VKeyCode v) noexcept { return static_cast<uint16_t>(v); }

    constexpr bool operator==(const VKeyCode lhs, const VKeyCode rhs) noexcept {
        return *lhs == *rhs;
    }

    /**
     * Returns <code>true</code> if given <code>uniChar</code> represents a printable character,
     * i.e. a value other than VKeyCode::VK_UNDEFINED and not a control or non-printable private code.
     * <p>
     * A printable character is neither a {@link #isModifierKey(short) modifier key}, nor an {@link #isActionKey(short) action key}.
     * </p>
     * <p>
     * Otherwise returns <code>false</code>.
     * </p>
     * <p>
     * Distinction of key character and virtual key code is made due to <a href="#unicodeCollision">unicode collision</a>.
     * </p>
     *
     * @param uniChar the UTF-16 unicode value, which maybe a virtual key code or key character.
     * @param isKeyChar true if <code>uniChar</code> is a key character, otherwise a virtual key code
     */
    bool isPrintableKey(uint16_t uniChar, bool isKeyChar) noexcept;

    /**
     * Returns <code>true</code> if given <code>uniCode</code> represents a printable character,
     * i.e. a value other than VKeyCode::VK_UNDEFINED and not a control or non-printable private code.
     * <p>
     * A printable character is neither a KeyEvent::isModifierKey(), nor an KeyEvent::isActionKey().
     * </p>
     * <p>
     * Otherwise returns <code>false</code>.
     * </p>
     * @param uniChar the UTF-16 VKeyCode unicode value.
     */
    inline bool isPrintableKey(VKeyCode vKey, bool isKeyChar) noexcept { return isPrintableKey(*vKey, isKeyChar); }

    /**
     * @param keyChar UTF16 value to map. It is expected that the incoming keyChar value is unshifted and unmodified,
     *        however, lower case a-z is mapped to {@link KeyEvent#VK_A} - {@link KeyEvent#VK_Z}.
     * @return {@link KeyEvent} virtual key (VK) value.
     */
    constexpr VKeyCode toVKeyCode(uint16_t keyChar) noexcept {
        if( 'a' <= keyChar && keyChar <= 'z' ) {
            return VKeyCode( ( keyChar - uint16_t('a') ) + *VKeyCode::VK_A );
        }
        return VKeyCode(keyChar);
    }

    /**
     * Returns <code>true</code> if the given <code>virtualKey</code> represents a modifier key, otherwise <code>false</code>.
     * <p>
     * A modifier key is one of VKeyCode::VK_SHIFT, VKeyCode::VK_CONTROL, VKeyCode::VK_ALT, VKeyCode::VK_META.
     * </p>
     */
    constexpr bool isModifierKey(VKeyCode vKey) {
        switch (vKey) {
            case VKeyCode::VK_SHIFT:
            case VKeyCode::VK_CONTROL:
            case VKeyCode::VK_ALT:
            case VKeyCode::VK_META:
                return true;
            default:
                return false;
        }
    }

    /**
     * <a name="eventDelivery"><h5>KeyEvent Delivery</h5></a>
     *
     * Key events are delivered in the following order:
     * <p>
     * <table border="0">
     *   <tr><th>#</th><th>Event Type</th>      <th>Constraints</th>  <th>Notes</th></tr>
     *   <tr><td>1</td><td>{@link #EVENT_KEY_PRESSED}  </td><td> <i> excluding {@link #isAutoRepeat() auto-repeat}-{@link #isModifierKey() modifier} keys</i></td><td></td></tr>
     *   <tr><td>2</td><td>{@link #EVENT_KEY_RELEASED} </td><td> <i> excluding {@link #isAutoRepeat() auto-repeat}-{@link #isModifierKey() modifier} keys</i></td><td></td></tr>
     * </table>
     * </p>
     * In case the native platform does not
     * deliver keyboard events in the above order or skip events,
     * the NEWT driver will reorder and inject synthetic events if required.
     * <p>
     * Besides regular modifiers like InputModifier::shift etc.,
     * the InputModifier::autorepeat bit is added if repetition is detected, following above constraints.
     * </p>
     * <p>
     * Auto-Repeat shall behave as follow:
     * <pre>
        P = pressed, R = released
        0 = normal, 1 = auto-repeat

        P(0), [ R(1), P(1), R(1), ..], R(0)
     * </pre>
     * The idea is if you mask out auto-repeat in your event listener
     * you just get one long pressed P/R tuple for {@link #isPrintableKey() printable} and {@link #isActionKey() Action} keys.
     * </p>
     * <p>
     * {@link #isActionKey() Action} keys will produce {@link #EVENT_KEY_PRESSED pressed}
     * and {@link #EVENT_KEY_RELEASED released} events including {@link #isAutoRepeat() auto-repeat}.
     * </p>
     * <p>
     * {@link #isPrintableKey() Printable} keys will produce {@link #EVENT_KEY_PRESSED pressed} and {@link #EVENT_KEY_RELEASED released} events.
     * </p>
     * <p>
     * {@link #isModifierKey() Modifier} keys will produce {@link #EVENT_KEY_PRESSED pressed} and {@link #EVENT_KEY_RELEASED released} events
     * excluding {@link #isAutoRepeat() auto-repeat}.
     * They will also influence subsequent event's {@link #getModifiers() modifier} bits while pressed.
     * </p>
     *
     * <a name="unicodeMapping"><h5>Unicode Mapping</h5></a>
     * <p>
     * {@link #getKeyChar() Key-chars}, as well as
     * {@link #isPrintableKey() printable} {@link #getKeyCode() key-codes} and {@link #getKeySymbol() key-symbols}
     * use the UTF-16 unicode space w/o collision.
     *
     * </p>
     * <p>
     * Non-{@link #isPrintableKey() printable} {@link #getKeyCode() key-codes} and {@link #getKeySymbol() key-symbols},
     * i.e. {@link #isModifierKey() modifier-} and {@link #isActionKey() action-}keys,
     * are mapped to unicode's control and private range and do not collide w/ {@link #isPrintableKey() printable} unicode values
     * with the following exception.
     * </p>
     *
     * <a name="unicodeCollision"><h5>Unicode Collision</h5></a>
     * <p>
     * The following {@link #getKeyCode() Key-code}s and {@link #getKeySymbol() key-symbol}s collide w/ unicode space:<br/>
     * <table border="1">
     *   <tr><th>unicode range</th>    <th>virtual key code</th>                            <th>unicode character</th></tr>
     *   <tr><td>[0x61 .. 0x78]</td>   <td>[{@link #VK_F1}..{@link #VK_F24}]</td>           <td>['a'..'x']</td></tr>
     * </table>
     * </p>
     * <p>
     * Collision was chosen for {@link #getKeyCode() Key-code} and {@link #getKeySymbol() key-symbol} mapping
     * to allow a minimal code range, i.e. <code>[0..255]</code>.
     * The reduced code range in turn allows the implementation to utilize fast and small lookup tables,
     * e.g. to implement a key-press state tracker.
     * </p>
     * <pre>
     * http://www.utf8-chartable.de/unicode-utf8-table.pl
     * http://www.unicode.org/Public/5.1.0/ucd/PropList.txt
     * https://en.wikipedia.org/wiki/Mapping_of_Unicode_characters
     * https://en.wikipedia.org/wiki/Unicode_control_characters
     * https://en.wikipedia.org/wiki/Private_Use_%28Unicode%29#Private_Use_Areas
     * </pre>
     * </p>
     */
    class KeyEvent : public InputEvent {
      private:
        VKeyCode m_keySym;
        uint16_t m_keyChar;
        uint8_t m_flags;
        static constexpr uint8_t F_MODIFIER_MASK   = 1 << 0;
        static constexpr uint8_t F_ACTION_MASK     = 1 << 1;
        static constexpr uint8_t F_PRINTABLE_MASK  = 1 << 2;

        std::string getEventTypeString() const noexcept {
            switch(type()) {
                case EVENT_KEY_PRESSED: return  "PRESSED";
                case EVENT_KEY_RELEASED: return "RELEASD";
                default: return "unknown (" + std::to_string(type()) + ")";
            }
        }
      public:
        KeyEvent(uint16_t type, const jau::fraction_timespec& when, const WindowRef& source, InputModifier globalMods,
                 VKeyCode keySym, InputModifier keySymMods, uint16_t keyChar) noexcept
        : InputEvent(type, when, source, globalMods), m_keySym(keySym), m_keyChar(keyChar), m_flags(0)
        {
            // cache modifier and action flags
            if( ::gamp::wt::event::isPrintableKey(keySym, false) && ::gamp::wt::event::isPrintableKey(keyChar, true) ) {
                m_flags |= F_PRINTABLE_MASK;
            } else if( ::gamp::wt::event::has_any(keySymMods, InputModifier::modifier) ) {
                m_flags |= F_MODIFIER_MASK;
            } else {
                // A = U - ( P + M )
                m_flags |= F_ACTION_MASK;
            }
        }

        /**
         * Returns the virtual <i>key symbol</i> reflecting the current <i>keyboard layout</i>.
         * <p>
         * For {@link #isPrintableKey() printable keys}, the <i>key symbol</i> is the {@link #isModifierKey() unmodified}
         * representation of the UTF-16 {@link #getKeyChar() key char}.<br/>
         * E.g. symbol [{@link #VK_A}, 'A'] for char 'a'.
         * </p>
         * @see #isPrintableKey()
         * @see #getKeyChar()
         * @see #getKeyCode()
         */
        constexpr VKeyCode keySym() const noexcept { return m_keySym; }

        /**
         * Returns the <i>UTF-16</i> character reflecting the {@link #getKeySymbol() key symbol}
         * incl. active {@link #isModifierKey() modifiers}.
         * @see #getKeySymbol()
         * @see #getKeyCode()
         */
        constexpr uint16_t keyChar() const noexcept { return m_keyChar; }

        /**
         * Returns <code>true</code> if {@link #getKeySymbol() key symbol} and {@link #getKeyChar() key char}
         * represents a printable character, i.e. a value other than {@link #VK_UNDEFINED}
         * and not a control or non-printable private code.
         * <p>
         * A printable character is neither a {@link #isModifierKey(short) modifier key}, nor an {@link #isActionKey(short) action key}.
         * </p>
         * <p>
         * Otherwise returns <code>false</code>.
         * </p>
         */
        constexpr bool isPrintableKey() const noexcept { return 0 != ( F_PRINTABLE_MASK & m_flags ); }

        /**
         * Returns <code>true</code> if {@link #getKeySymbol() key symbol} represents a modifier key,
         * otherwise <code>false</code>.
         * <p>
         * See {@link #isModifierKey(short)} for details.
         * </p>
         * <p>
         * Note: Implementation uses a cached value.
         * </p>
         */
        constexpr bool isModifierKey() const noexcept { return 0 != ( F_MODIFIER_MASK & m_flags ); }

        /**
         * Returns <code>true</code> if {@link #getKeySymbol() key symbol} represents a non-printable and
         * non-{@link #isModifierKey(short) modifier} action key, otherwise <code>false</code>.
         * <p>
         * Hence it is the set A of all keys U w/o printable P and w/o modifiers M:
         * <code> A = U - ( P + M ) </code>
         * </p>
         * @see #isPrintableKey()
         * @see #isModifierKey()
         */
        constexpr bool isActionKey() const noexcept { return 0 != ( F_ACTION_MASK & m_flags ); }

        /**
         * Returns true if event matches original key typed semantics, i.e.:
         * <pre>
             isPrintableKey() && !is_set(InputModifier::autorepeat)
         * </pre>
         */
        constexpr bool isTyped() const noexcept {
            return isPrintableKey() && !is_set(InputModifier::repeat);
        }

        std::string toString() const noexcept {
            std::string res = "KeyEvent[";
            res.append(getEventTypeString()).append(", sym ").append(jau::to_hexstring(*m_keySym))
               .append(", char ").append(jau::to_hexstring(m_keyChar));
            if( isPrintableKey() ) {
                res.append(" `").append(std::string(1, (char)m_keyChar)).append("`");
            }
            res.append("), ");
            if( isPrintableKey() ) {
                res.append("printable");
            } else if( isModifierKey() ) {
                res.append("modifier");
            } else if( isActionKey() ) {
                res.append("actionkey");
            }
            res.append(", ").append(InputEvent::toString()).append("]");
            return res;
        }
    };
    inline std::ostream& operator<<(std::ostream& out, const KeyEvent& v) {
        return out << v.toString();
    }

    class KeyboardTracker {
      public:
        typedef jau::bitfield<256> PressedKeyCodes;

        virtual ~KeyboardTracker() noexcept = default;
        virtual InputModifier modifier() const noexcept = 0;
        virtual bool isPressed(VKeyCode) const noexcept = 0;
        virtual const PressedKeyCodes& getPressedKeyCodes() const noexcept = 0;
    };

    /**
     * Listener for multiple KeyEvent.
     *
     * @see KeyEvent
     */
    class KeyListener
    {
      public:
        virtual ~KeyListener() noexcept = default;

        /** A key has been {@link KeyEvent#EVENT_KEY_PRESSED pressed}, excluding {@link #isAutoRepeat() auto-repeat} {@link #isModifierKey() modifier} keys. See {@link KeyEvent}. */
        virtual void keyPressed(const KeyEvent& /*e*/, const KeyboardTracker&) {}

        /**
         * A key has been {@link KeyEvent#EVENT_KEY_RELEASED released}, excluding {@link #isAutoRepeat() auto-repeat} {@link #isModifierKey() modifier} keys. See {@link KeyEvent}.
         * <p>
         * To simulated the removed <code>keyTyped(KeyEvent e)</code> semantics, see KeyEvent::isTyped().
         * </p>
         */
        virtual void keyReleased(const KeyEvent& /*e*/, const KeyboardTracker&) {}
    };
    typedef std::shared_ptr<KeyListener> KeyListenerRef;


}

#endif /* GAMP_WTKEYEVENT_HPP_ */
