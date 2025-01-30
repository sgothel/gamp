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

#include <gamp/wt/wtkeyevent.hpp>

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
        if( ( nonPrintableKeys[0].min <= uniChar && uniChar <= nonPrintableKeys[0].max ) ||
            ( nonPrintableKeys[1].min <= uniChar && uniChar <= nonPrintableKeys[1].max ) ||
            ( nonPrintableKeys[2].min <= uniChar && uniChar <= nonPrintableKeys[2].max ) ||
            ( nonPrintableKeys[3].min <= uniChar && uniChar <= nonPrintableKeys[3].max ) ) {
            return false;
        }
    } else {
        if( ( nonPrintableKeys[0].inclKeyChar && nonPrintableKeys[0].min <= uniChar && uniChar <= nonPrintableKeys[0].max ) ||
            ( nonPrintableKeys[1].inclKeyChar && nonPrintableKeys[1].min <= uniChar && uniChar <= nonPrintableKeys[1].max ) ||
            ( nonPrintableKeys[2].inclKeyChar && nonPrintableKeys[2].min <= uniChar && uniChar <= nonPrintableKeys[2].max ) ||
            ( nonPrintableKeys[3].inclKeyChar && nonPrintableKeys[3].min <= uniChar && uniChar <= nonPrintableKeys[3].max ) ) {
            return false;
        }
    }
    return VKeyCode::none != uniCode;
}

