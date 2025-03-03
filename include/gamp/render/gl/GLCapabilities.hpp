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
#ifndef GAMP_RENDER_GL_GLCAPABILITIES_HPP_
#define GAMP_RENDER_GL_GLCAPABILITIES_HPP_

#include <gamp/GampTypes.hpp>
#include <gamp/wt/Capabilities.hpp>

#include <jau/string_util.hpp>

namespace gamp::render::gl {
    /** \addtogroup Gamp_GL
     *
     *  @{
     */

    using namespace gamp::wt;

    /** Specifies a set of OpenGL capabilities.<br>
        At creation time of a {@link GLDrawable} using {@link GLDrawableFactory},
        an instance of this class is passed,
        describing the desired capabilities that a rendering context
        must support, such as the OpenGL profile, color depth and whether stereo is enabled.<br>

        The actual capabilites of created {@link GLDrawable}s are then reflected by their own
        GLCapabilites instance, which can be queried with {@link GLDrawable#getChosenGLCapabilities()}.
        <br>

        It currently contains the minimal number of routines which allow
        configuration on all supported window systems. */
    class GLCapabilities : public Capabilities {
      private:
        bool m_isFBO               = false;
        bool m_doubleBuffered      = true;
        bool m_stereo              = false;
        bool m_hardwareAccelerated = true;
        int  m_depthBits           = 16;
        int  m_stencilBits         = 0;
        int  m_accumRedBits        = 0;
        int  m_accumGreenBits      = 0;
        int  m_accumBlueBits       = 0;
        int  m_accumAlphaBits      = 0;

        // Support for full-scene antialiasing (FSAA)
        // std::string  sampleExtension = DEFAULT_SAMPLE_EXTENSION;
        bool m_hasSamples   = false;
        int  m_samplesCount = 2;

      public:
        constexpr GLCapabilities() noexcept                       = default;
        constexpr GLCapabilities(const GLCapabilities&) noexcept  = default;
        constexpr GLCapabilities(GLCapabilities&&) noexcept       = default;
        GLCapabilities& operator=(const GLCapabilities&) noexcept = default;

        const jau::type_info& signature() const noexcept override { return jau::static_ctti<GLCapabilities>(); }

        CapabilitiesPtr clone() const noexcept override { return std::make_unique<GLCapabilities>(*this); }

        std::size_t hash_code() const noexcept override {
            // 31 * x == (x << 5) - x
            size_t hash = Capabilities::hash_code();
            hash        = ((hash << 5) - hash) + (m_hardwareAccelerated ? 1 : 0);
            hash        = ((hash << 5) - hash) + (m_stereo ? 1 : 0);
            hash        = ((hash << 5) - hash) + (m_isFBO ? 1 : 0);
            // hash     = ((hash << 5) - hash) + (isPBuffer ? 1 : 0);
            hash        = ((hash << 5) - hash) + (m_hasSamples ? 1 : 0);
            hash        = ((hash << 5) - hash) + samplesCount();
            // hash     = ((hash << 5) - hash) + sampleExtension.hashCode(); // FIXME
            hash        = ((hash << 5) - hash) + m_depthBits;
            hash        = ((hash << 5) - hash) + m_stencilBits;
            hash        = ((hash << 5) - hash) + m_accumRedBits;
            hash        = ((hash << 5) - hash) + m_accumGreenBits;
            hash        = ((hash << 5) - hash) + m_accumBlueBits;
            hash        = ((hash << 5) - hash) + m_accumAlphaBits;
            return hash;
        }

        bool operator==(const Capabilities& rhs0) const noexcept override {
            if( signature() != rhs0.signature() ) {
                return false;
            }
            const GLCapabilities& rhs = static_cast<const GLCapabilities&>(rhs0);
            if( this == &rhs ) {
                return true;
            }
            bool res = Capabilities::operator==(rhs) &&
                       // rhs.isPBuffer() == isPBuffer &&
                       rhs.isFBO() == m_isFBO &&
                       rhs.doubleBuffered() == m_doubleBuffered &&
                       rhs.stereo() == m_stereo &&
                       rhs.hardwareAccelerated() == m_hardwareAccelerated &&
                       rhs.depthBits() == m_depthBits &&
                       rhs.stencilBits() == m_stencilBits &&
                       rhs.accumRedBits() == m_accumRedBits &&
                       rhs.accumGreenBits() == m_accumGreenBits &&
                       rhs.accumBlueBits() == m_accumBlueBits &&
                       rhs.accumAlphaBits() == m_accumAlphaBits &&
                       rhs.hasSamples() == m_hasSamples;
            if( res && m_hasSamples ) {
                res = rhs.samplesCount() == samplesCount()
                // && rhs.getSampleExtension().equals(sampleExtension)
                ;
            }
            return res;
        }

        /** comparing hw/sw, stereo, multisample, stencil, RGBA and depth only */
        int compare(const Capabilities& rhs0) const noexcept override {
            if( signature() != rhs0.signature() ) {
                return 1;
            }
            const GLCapabilities& rhs = static_cast<const GLCapabilities&>(rhs0);
            if( this == &rhs ) {
                return 0;
            }
            if( m_hardwareAccelerated && !rhs.hardwareAccelerated() ) {
                return 1;
            } else if( !m_hardwareAccelerated && rhs.hardwareAccelerated() ) {
                return -1;
            }

            if( m_stereo && !rhs.stereo() ) {
                return 1;
            } else if( !m_stereo && rhs.stereo() ) {
                return -1;
            }

            if( m_doubleBuffered && !rhs.doubleBuffered() ) {
                return 1;
            } else if( !m_doubleBuffered && rhs.doubleBuffered() ) {
                return -1;
            }

            const int ms  = samplesCount();
            const int xms = rhs.samplesCount();

            if( ms > xms ) {
                return 1;
            } else if( ms < xms ) {
                return -1;
            }
            // ignore the sample extension

            if( m_stencilBits > rhs.stencilBits() ) {
                return 1;
            } else if( m_stencilBits < rhs.stencilBits() ) {
                return -1;
            }

            const int sc = Capabilities::compare(rhs0);  // RGBA
            if( 0 != sc ) {
                return sc;
            }

            if( m_depthBits > rhs.depthBits() ) {
                return 1;
            } else if( m_depthBits < rhs.depthBits() ) {
                return -1;
            }

            return 0;  // they are equal: hw/sw, stereo, multisample, stencil, RGBA and depth
        }

        constexpr bool isFBO() const noexcept { return m_isFBO; }

        /**
         * Requesting offscreen FBO mode.
         * <p>
         * If enabled this method also invokes {@link #setOnscreen(boolean) setOnscreen(false)}.
         * </p>
         * <p>
         * Defaults to false.
         * </p>
         * <p>
         * Requesting offscreen FBO mode disables the offscreen auto selection.
         * </p>
         */
        constexpr void setFBO(bool enable) noexcept {
            if( enable ) {
                setOnscreen(false);
            }
            m_isFBO = enable;
        }

        constexpr bool doubleBuffered() const noexcept { return m_doubleBuffered; }

        /** Enables or disables double buffering. */
        constexpr void setDoubleBuffered(bool enable) noexcept { m_doubleBuffered = enable; }

        constexpr bool stereo() const noexcept { return m_stereo; }

        /** Enables or disables stereo viewing. */
        constexpr void setStereo(bool enable) noexcept { m_stereo = enable; }

        constexpr bool hardwareAccelerated() const noexcept { return m_hardwareAccelerated; }

        /** Enables or disables hardware acceleration. */
        constexpr void setHardwareAccelerated(bool enable) noexcept { m_hardwareAccelerated = enable; }

        constexpr int depthBits() const noexcept { return m_depthBits; }

        /** Sets the number of bits requested for the depth buffer. */
        constexpr int& depthBits() noexcept { return m_depthBits; }

        constexpr int stencilBits() const noexcept { return m_stencilBits; }

        /** Sets the number of bits requested for the stencil buffer. */
        constexpr int& stencilBits() noexcept { return m_stencilBits; }

        constexpr int accumRedBits() const noexcept { return m_accumRedBits; }

        /** Sets the number of bits requested for the accumulation buffer's
            red component. On some systems only the accumulation buffer
            depth, which is the sum of the red, green, and blue bits, is
            considered. */
        constexpr int& accumRedBits() noexcept { return m_accumRedBits; }

        constexpr int accumGreenBits() const noexcept { return m_accumGreenBits; }

        /** Sets the number of bits requested for the accumulation buffer's
            green component. On some systems only the accumulation buffer
            depth, which is the sum of the red, green, and blue bits, is
            considered. */
        constexpr int& accumGreenBits() noexcept { return m_accumGreenBits; }

        constexpr int accumBlueBits() const noexcept { return m_accumBlueBits; }

        /** Sets the number of bits requested for the accumulation buffer's
            blue component. On some systems only the accumulation buffer
            depth, which is the sum of the red, green, and blue bits, is
            considered. */
        constexpr int& accumBlueBits() noexcept { return m_accumBlueBits; }

        constexpr int accumAlphaBits() const noexcept { return m_accumAlphaBits; }

        /** Sets number of bits requested for accumulation buffer's alpha
            component. On some systems only the accumulation buffer depth,
            which is the sum of the red, green, and blue bits, is
            considered. */
        constexpr int& accumAlphaBits() noexcept { return m_accumAlphaBits; }

        /**
         * Sets the desired extension for full-scene antialiasing
         * (FSAA), default is {@link #DEFAULT_SAMPLE_EXTENSION}.
         */
        // constexpr void setSampleExtension(final String se) { sampleExtension = se; }
        // const std::string& getSampleExtension() { return sampleExtension; }

        constexpr bool hasSamples() const noexcept { return m_hasSamples; }

        /**
         * Defaults to false.<br>
         * Indicates whether sample buffers for full-scene antialiasing
         * (FSAA) should be allocated for this drawable.<br>
         * Mind that this requires the alpha component.<br>
         * If enabled this method also invokes {@link #setAlphaBits(int) setAlphaBits(1)}
         * if {@link #getAlphaBits()} == 0.<br>
         */
        constexpr void setHasSamples(bool enable) noexcept {
            m_hasSamples = enable;
            if( m_hasSamples && alphaBits() == 0 ) {
                alphaBits() = 1;
            }
        }

        constexpr int samplesCount() const noexcept { return m_hasSamples ? m_samplesCount : 0; }

        /**
         * If sample buffers are enabled, indicates the number of buffers
         * to be allocated. Defaults to 2.
         * @see #getNumSamples()
         */
        constexpr int& samplesCount() noexcept { return m_samplesCount; }

        /** Returns a textual representation of this GLCapabilities
            object. */
        std::string toString() const override {
            std::string msg("GLCaps[");
            toString(msg);
            msg.append("]");
            return msg;
        }

      protected:
        std::string toString(std::string& sink) const {
            const int samples = m_hasSamples ? m_samplesCount : 0;

            Capabilities::toString(sink, false);

            sink.append(", accum-rgba ").append(std::to_string(m_accumRedBits)).append(ESEP).append(std::to_string(m_accumGreenBits)).append(ESEP)
                .append(std::to_string(m_accumBlueBits)).append(ESEP).append(std::to_string(m_accumAlphaBits));
            sink.append(", dp/st/ms ").append(std::to_string(m_depthBits)).append(ESEP)
                .append(std::to_string(m_stencilBits)).append(ESEP).append(std::to_string(samples));
            // if(samples>0) { sink.append(", sample-ext ").append(m_sampleExtension); }
            if( m_doubleBuffered ) {
                sink.append(", dbl");
            } else {
                sink.append(", one");
            }
            if( m_stereo ) {
                sink.append(", stereo");
            } else {
                sink.append(", mono  ");
            }
            if( m_hardwareAccelerated ) {
                sink.append(", hw, ");
            } else {
                sink.append(", sw");
            }
            // sink.append(", ").append(glProfile);
            if( isOnscreen() ) {
                sink.append(", on-scr[");
            } else {
                sink.append(", offscr[");
            }
            bool ns = false;
            if( isFBO() ) {
                sink.append("fbo");
                ns = true;
            }
            if( isBitmap() ) {
                if( ns ) { sink.append(CSEP); }
                sink.append("bitmap");
                ns = true;
            }
            if( !ns ) {  // !FBO !Bitmap
                if( isOnscreen() ) {
                    sink.append(".");  // no additional off-screen modes besides on-screen
                } else {
                    sink.append("auto-cfg");  // auto-config off-screen mode
                }
            }
            sink.append("]");

            return sink;
        }
    };
    typedef std::unique_ptr<GLCapabilities> GLCapabilitiesPtr;

    /**@}*/

}  // namespace gamp::render::gl

#endif /* GAMP_RENDER_GL_GLCAPABILITIES_HPP_ */
