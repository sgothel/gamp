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
#ifndef GAMP_WT_CAPABILITIES_HPP_
#define GAMP_WT_CAPABILITIES_HPP_

#include <gamp/GampTypes.hpp>
#include <memory>

#include <jau/string_util.hpp>
#include <jau/type_info.hpp>

namespace gamp::wt {
    /** \addtogroup Gamp_WT
     *
     *  @{
     */

    class Capabilities;
    typedef std::unique_ptr<Capabilities> CapabilitiesPtr;

    /** Specifies a set of capabilities that a window's rendering context
        must support, such as color depth per channel. It currently
        contains the minimal number of routines which allow configuration
        on all supported window systems. */
    class Capabilities {
      protected:
        constexpr static std::string_view na_str = "----";

      private:
        int m_vid       = 0;
        int m_redBits   = 8;
        int m_greenBits = 8;
        int m_blueBits  = 8;
        int m_alphaBits = 0;

        // Support for transparent windows containing OpenGL content
        bool m_backgroundOpaque = true;

        int m_xparentValueRed   = 0;
        int m_xparentValueGreen = 0;
        int m_xparentValueBlue  = 0;
        int m_xparentValueAlpha = 0;

        // Switch for on- or offscreen
        bool m_onscreen = true;

        // offscreen bitmap mode
        bool m_isBitmap = false;

      public:
        /** Creates a Capabilities object. All attributes are in a default
            state.
          */
        constexpr Capabilities() noexcept = default;
        virtual ~Capabilities() noexcept  = default;

        constexpr Capabilities(const Capabilities&) noexcept  = default;
        constexpr Capabilities(Capabilities&&) noexcept       = default;
        Capabilities& operator=(const Capabilities&) noexcept = default;

        virtual const jau::type_info& signature() const noexcept { return jau::static_ctti<Capabilities>(); }

        virtual CapabilitiesPtr clone() const noexcept { return std::make_unique<Capabilities>(*this); }

        virtual std::size_t hash_code() const noexcept {
            // 31 * x == (x << 5) - x
            size_t hash = 31U + m_redBits;
            hash        = ((hash << 5) - hash) + (m_onscreen ? 1U : 0U);
            hash        = ((hash << 5) - hash) + (m_isBitmap ? 1U : 0U);
            hash        = ((hash << 5) - hash) + m_greenBits;
            hash        = ((hash << 5) - hash) + m_blueBits;
            hash        = ((hash << 5) - hash) + m_alphaBits;
            hash        = ((hash << 5) - hash) + (m_backgroundOpaque ? 1U : 0U);
            hash        = ((hash << 5) - hash) + m_xparentValueRed;
            hash        = ((hash << 5) - hash) + m_xparentValueGreen;
            hash        = ((hash << 5) - hash) + m_xparentValueBlue;
            hash        = ((hash << 5) - hash) + m_xparentValueAlpha;
            return hash;
        }

        virtual bool operator==(const Capabilities& rhs) const noexcept {
            if( this == &rhs ) {
                return true;
            }
            if( signature() != rhs.signature() ) {
                return false;
            }
            {
                // first check whether the VID is compatible
                const int id_t = visualID();
                const int id_o = rhs.visualID();
                if( id_t != id_o ) {
                    return false;
                }
            }
            bool res = rhs.redBits() == m_redBits &&
                       rhs.greenBits() == m_greenBits &&
                       rhs.blueBits() == m_blueBits &&
                       rhs.alphaBits() == m_alphaBits &&
                       rhs.isBackgroundOpaque() == m_backgroundOpaque &&
                       rhs.isOnscreen() == m_onscreen &&
                       rhs.isBitmap() == m_isBitmap;
            if( res && !m_backgroundOpaque ) {
                res = rhs.transparentRedValue() == m_xparentValueRed &&
                      rhs.transparentGreenValue() == m_xparentValueGreen &&
                      rhs.transparentBlueValue() == m_xparentValueBlue &&
                      rhs.transparentAlphaValue() == m_xparentValueAlpha;
            }

            return res;
        }

        /**
         * Comparing RGBA values only
         *
         * Returns
         * - -1 if this < other
         * -  0 if this == other
         * -  1 if this > other
         */
        virtual int compare(const Capabilities& rhs) const noexcept {
            if( this == &rhs ) {
                return 0;
            }
            if( signature() != rhs.signature() ) {
                return 1;
            }
            const int rgba  = m_redBits * m_greenBits * m_blueBits * (m_alphaBits + 1);
            const int xrgba = rhs.redBits() * rhs.greenBits() * rhs.blueBits() * (rhs.alphaBits() + 1);
            if( rgba > xrgba ) {
                return 1;
            } else if( rgba < xrgba ) {
                return -1;
            }
            return 0;  // equal RGBA
        }

        std::strong_ordering operator<=>(const Capabilities& rhs) const noexcept {
            const int r = compare(rhs);
            return 0 == r ? std::strong_ordering::equal : (0 > r ? std::strong_ordering::less : std::strong_ordering::greater);
        }

        constexpr int visualID() const noexcept { return m_vid; }
        /**
         * Returns the number of bits for the color buffer's red
         * component. On some systems only the color depth, which is the sum of the
         * red, green, and blue bits, is considered.
         */
        constexpr int redBits() const noexcept { return m_redBits; }
        /**
         * Returns the number of bits for the color buffer's green
         * component. On some systems only the color depth, which is the sum of the
         * red, green, and blue bits, is considered.
         */
        constexpr int greenBits() const noexcept { return m_greenBits; }
        /**
         * Returns the number of bits for the color buffer's blue
         * component. On some systems only the color depth, which is the sum of the
         * red, green, and blue bits, is considered.
         */
        constexpr int blueBits() const noexcept { return m_blueBits; }
        /**
         * Returns the number of bits for the color buffer's alpha
         * component. On some systems only the color depth, which is the sum of the
         * red, green, and blue bits, is considered.
         */
        constexpr int alphaBits() const noexcept { return m_alphaBits; }

        constexpr int& visualID() noexcept { return m_vid; }

        /** Allows setting the number of bits requested for the color buffer's red
            component. On some systems only the color depth, which is the
            sum of the red, green, and blue bits, is considered. */
        constexpr int& redBits() noexcept { return m_redBits; }

        /** Allows setting the number of bits requested for the color buffer's green
            component. On some systems only the color depth, which is the
            sum of the red, green, and blue bits, is considered. */
        constexpr int& greenBits() noexcept { return m_greenBits; }

        /** Allows setting the number of bits requested for the color buffer's blue
            component. On some systems only the color depth, which is the
            sum of the red, green, and blue bits, is considered. */
        constexpr int& blueBits() noexcept { return m_blueBits; }

        /**
         * Allows setting the number of bits requested for the color buffer's alpha
         * component. On some systems only the color depth, which is the
         * sum of the red, green, and blue bits, is considered.
         * <p>
         * <b>Note:</b> If alpha bits are <code>zero</code>, they are set to <code>one</code>
         * by {@link #setBackgroundOpaque(boolean)} and it's OpenGL specialization <code>GLCapabilities::setSampleBuffers(boolean)</code>.<br/>
         * Ensure to call this method after the above to ensure a <code>zero</code> value.</br>
         * The above automated settings takes into account, that the user calls this method to <i>request</i> alpha bits,
         * not to <i>reflect</i> a current state. Nevertheless if this is the case - call it at last.
         * </p>
         */
        constexpr int& alphaBits() noexcept { return m_alphaBits; }

        /**
         * Sets whether the surface shall be opaque or translucent.
         * <p>
         * Platform implementations may need an alpha component in the surface (eg. Windows),
         * or expect pre-multiplied alpha values (eg. X11/XRender).<br>
         * To unify the experience, this method also invokes {@link #setAlphaBits(int) setAlphaBits(1)}
         * if {@link #getAlphaBits()} == 0.<br>
         * Please note that in case alpha is required on the platform the
         * clear color shall have an alpha lower than 1.0 to allow anything shining through.
         * </p>
         * <p>
         * Mind that translucency may cause a performance penalty
         * due to the composite work required by the window manager.
         * </p>
         */
        constexpr void setBackgroundOpaque(bool opaque) noexcept {
            m_backgroundOpaque = opaque;
            if( !opaque && alphaBits() == 0 ) {
                alphaBits() = 1;
            }
        }

        /**
         * Returns whether an opaque or translucent surface is requested, supported or chosen.
         * <p>
         * Default is true, i.e. opaque.
         * </p>
         */
        constexpr bool isBackgroundOpaque() const noexcept { return m_backgroundOpaque; }

        /**
         * Sets whether the surface shall be on- or offscreen.
         * <p>
         * Defaults to true.
         * </p>
         * <p>
         * If requesting an offscreen surface without further selection of it's mode,
         * e.g. FBO, Pbuffer or {@link #setBitmap(boolean) bitmap},
         * the implementation will choose the best available offscreen mode.
         * </p>
         * @param onscreen
         */
        constexpr void setOnscreen(bool v) noexcept { m_onscreen = v; }

        /**
         * Returns whether an on- or offscreen surface is requested, available or chosen.
         * <p>
         * Default is true, i.e. onscreen.
         * </p>
         * <p>
         * Mind that an capabilities intance w/ <i>available</i> semantics
         * may show onscreen, but also the offscreen modes FBO or {@link #setBitmap(boolean) bitmap}.
         * This is valid, since one native configuration maybe used for either functionality.
         * </p>
         */
        constexpr bool isOnscreen() const noexcept { return m_onscreen; }

        /**
         * Requesting offscreen bitmap mode.
         * <p>
         * If enabled this method also invokes {@link #setOnscreen(int) setOnscreen(false)}.
         * </p>
         * <p>
         * Defaults to false.
         * </p>
         * <p>
         * Requesting offscreen bitmap mode disables the offscreen auto selection.
         * </p>
         */
        constexpr void setBitmap(bool enable) noexcept {
            if( enable ) {
                setOnscreen(false);
            }
            m_isBitmap = enable;
        }

        /**
         * Returns whether bitmap offscreen mode is requested, available or chosen.
         * <p>
         * Default is false.
         * </p>
         * <p>
         * For chosen capabilities, only the selected offscreen surface is set to <code>true</code>.
         * </p>
         */
        constexpr bool isBitmap() const noexcept { return m_isBitmap; }

        /**
         * Gets the transparent red value for the frame buffer configuration. This
         * value is undefined if; equals true.
         */
        constexpr int transparentRedValue() const noexcept { return m_xparentValueRed; }
        /**
         * Gets the transparent green value for the frame buffer configuration. This
         * value is undefined if; equals true.
         */
        constexpr int transparentGreenValue() const noexcept { return m_xparentValueGreen; }
        /**
         * Gets the transparent blue value for the frame buffer configuration. This
         * value is undefined if; equals true.
         */
        constexpr int transparentBlueValue() const noexcept { return m_xparentValueBlue; }
        /**
         * Gets the transparent alpha value for the frame buffer configuration. This
         * value is undefined if; equals true.
         */
        constexpr int transparentAlphaValue() const noexcept { return m_xparentValueAlpha; }

        /** Allows setting the transparent red value for the frame buffer configuration,
            ranging from 0 to the maximum frame buffer value for red.
            This value is ignored if {@link #isBackgroundOpaque()} equals true.<br>
            It defaults to half of the frambuffer value for red. <br>
            A value of -1 is interpreted as any value. */
        constexpr int& transparentRedValue() noexcept { return m_xparentValueRed; }

        /** Allows setting the transparent green value for the frame buffer configuration,
            ranging from 0 to the maximum frame buffer value for green.
            This value is ignored if {@link #isBackgroundOpaque()} equals true.<br>
            It defaults to half of the frambuffer value for green.<br>
            A value of -1 is interpreted as any value. */
        constexpr int& transparentGreenValue() noexcept { return m_xparentValueGreen; }

        /** Allows setting the transparent blue value for the frame buffer configuration,
            ranging from 0 to the maximum frame buffer value for blue.
            This value is ignored if {@link #isBackgroundOpaque()} equals true.<br>
            It defaults to half of the frambuffer value for blue.<br>
            A value of -1 is interpreted as any value. */
        constexpr int& transparentBlueValue() noexcept { return m_xparentValueBlue; }

        /** Allows setting the transparent alpha value for the frame buffer configuration,
            ranging from 0 to the maximum frame buffer value for alpha.
            This value is ignored if {@link #isBackgroundOpaque()} equals true.<br>
            It defaults to half of the frambuffer value for alpha.<br>
            A value of -1 is interpreted as any value. */
        constexpr int& transparentAlphaValue() noexcept { return m_xparentValueAlpha; }

        /** Returns a textual representation of this Capabilities
            object. */
        virtual std::string toString() const {
            std::string msg("Caps[");
            toString(msg, true);
            return msg.append("]");
        }

      protected:
        /** Element separator */
        constexpr static std::string_view ESEP = "/";
        /** Component separator */
        constexpr static std::string_view CSEP = ", ";

        void toString(std::string& sink, bool withOnOffScreen) const {
            sink.append("rgba ").append(std::to_string(m_redBits)).append(ESEP).append(std::to_string(m_greenBits)).append(ESEP)
                .append(std::to_string(m_blueBits)).append(ESEP).append(std::to_string(m_alphaBits));
            if (m_backgroundOpaque) {
                sink.append(", opaque");
            } else {
                sink.append(", trans-rgba 0x").append(jau::toHexString(m_xparentValueRed)).append(ESEP).append(jau::toHexString(m_xparentValueGreen))
                    .append(ESEP).append(jau::toHexString(m_xparentValueBlue)).append(ESEP).append(jau::toHexString(m_xparentValueAlpha));
            }
            if (withOnOffScreen) {
                sink.append(CSEP);
                if (m_onscreen) {
                    sink.append("on-scr");
                } else {
                    sink.append("offscr[");
                }
                if (m_isBitmap) {
                    sink.append("bitmap");
                } else if (m_onscreen) {
                    sink.append(".");  // no additional off-screen modes besides on-screen
                } else {
                    sink.append("auto-cfg");  // auto-config off-screen mode
                }
                sink.append("]");
            }
        }
    };

    /**@}*/

}  // namespace gamp::wt

// injecting specialization of std::hash to namespace std of our types above
namespace std {
    /** \addtogroup Gamp_WT
     *
     */

    template <>
    struct hash<gamp::wt::Capabilities> {
        std::size_t operator()(gamp::wt::Capabilities const& a) const noexcept {
            return a.hash_code();
        }
    };

    /**@}*/
}  // namespace std

#endif /* GAMP_WT_CAPABILITIES_HPP_ */
