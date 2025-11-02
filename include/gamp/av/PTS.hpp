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
#ifndef GAMP_AV_PTS_HPP_
#define GAMP_AV_PTS_HPP_

#include <jau/basic_types.hpp>
#include <jau/fraction_type.hpp>
#include <jau/ordered_atomic.hpp>
#include <jau/string_util.hpp>

#include <chrono>
#include <cstdint>
#include <gamp/GampTypes.hpp>
#include <gamp/av/TimeFrameI.hpp>

namespace gamp::av {

    /** @defgroup Gamp_AV Gamp Audio Video
     *  Gamp audio video support utilities.
     *
     *  @{
     */

    /**
     * Presentation Timestamp (PTS) with added System Clock Reference (SCR) via
     * {@link #set(long, int)} and its interpolation via {@link #get(long)}, as well as giving raw access via {@link #getLast()}.
     * <p>
     * The relative millisecond PTS since start of the presentation stored in uint32_t
     * covers a time span of 4'294'967'293 ms
     * or 4'294'967 seconds or 49 days.
     * </p>
     */
    class PTS {
      public:
        /** An external float value getter */
        typedef jau::function<float()> FloatValue;

      private:
        FloatValue m_speed;
        /** System Clock Reference (SCR) of last PTS update. */
        jau::relaxed_atomic_uint64 m_scr;
        /** Last updated PTS value */
        jau::relaxed_atomic_uint32 m_pts;

      public:
        /**
         * Create new instance, initializing pts with {@link TimeFrameI#INVALID_PTS} and system-clock timestamp with zero.
         * @param speed external {@link FloatValue} getter for playback speed.
         * @see #set(long, int)
         */
        PTS(FloatValue speed) noexcept
        : m_speed(std::move(speed)), m_scr(0), m_pts(TimeFrameI::INVALID_PTS)
        { }

        /**
         * Create new instance.
         * @param speed external {@link FloatValue} getter for playback speed.
         * @param scr System Clock Reference (SCR) in milliseconds of taken pts value, i.e. {@link Clock#currentMillis()}.
         * @param pts the presentation timestamp (PTS) in milliseconds
         * @see #set(long, int)
         */
        PTS(FloatValue speed, uint64_t scr, uint32_t pts) noexcept
        : m_speed(std::move(speed)), m_scr(scr), m_pts(pts)
        { }

        /** Returns true if {@link #getLast()} is unequal to {@link TimeFrameI#INVALID_PTS}. */
        constexpr_atomic bool isValid() const noexcept  { return TimeFrameI::INVALID_PTS != m_pts; }

        /** Returns true if {@link #getLast()} equals to {@link TimeFrameI#END_OF_STREAM_PTS}, indicating end of stream (EOS). */
        constexpr_atomic bool isEOS() const noexcept  { return TimeFrameI::END_OF_STREAM_PTS == m_pts; }

        /** Returns the System Clock Reference (SCR) in milliseconds of last PTS update via {@link #set(long, int)}. */
        constexpr_atomic uint64_t scr() const noexcept { return m_scr; }

        /** Returns scr() as time string representation via {@link #toTimeStr(long, boolean)}. */
        std::string getSCRTimeStr(bool addFractions) const noexcept {
            return toTimeStr(scr(), addFractions);
        }

        /** Returns the last updated PTS value via {@link #set(long, int)} w/o System Clock Reference (SCR) interpolation. */
        constexpr_atomic uint32_t last() const noexcept { return m_pts; }

        /** Returns {@link #getLast()} as time string representation via {@link #toTimeStr(long, boolean)}. */
        std::string getLastTimeStr(bool addFractions) const noexcept {
            return toTimeStr(last(), addFractions);
        }

        /** Returns the external playback speed. */
        float getSpeed() const noexcept { return m_speed(); }

        /**
         * Updates the PTS value with given System Clock Reference (SCR) in milliseconds.
         * @param scr System Clock Reference (SCR) in milliseconds of taken PTS value, i.e. {@link Clock#currentMillis()}.
         * @param pts the presentation timestamp (PTS) in milliseconds
         */
        void set(uint64_t scr, uint32_t pts) noexcept {
            m_scr = scr;
            m_pts = pts;
        }
        /** Sets the PTS value, see {@link #set(long, int)}. */
        void setPTS(uint32_t pts) noexcept { m_pts = pts; }
        /** Sets the System Clock Reference (SCR) in milliseconds of last PTS update, see {@link #set(long, int)}. */
        void setSCR(uint64_t currentMillis) noexcept { m_scr = currentMillis; }

        /**
         * Updates the PTS value with values from other {@link PTS} instance.
         * @param other source {@link PTS} values
         * @see #get(long)
         */
        void set(const PTS& other) noexcept {
            m_scr = other.scr();
            m_pts = other.last();
        }

        /**
         * Returns the last() updated PTS, interpolated by scr() System Clock Reference (SCR) delta to given `currentMillis` and playback getSpeed().
         * <pre>
         *      last_pts + (uint32_t) ( ( currentMillis - SCR ) * speed + 0.5f )
         * </pre>
         * @param currentMillis current system clock in milliseconds, i.e. {@link Clock#currentMillis()}.
         * @see #set(long, int)
         */
        constexpr_atomic uint32_t get(uint64_t currentMillis) const noexcept {
            const float dpts = static_cast<float>( currentMillis - m_scr ) * m_speed() + 0.5f;
            return m_pts + static_cast<uint32_t>( dpts );
        }
        /** Returns {@link #get(long)} passing {@link Clock#currentMillis()}. */
        uint32_t getCurrent() const noexcept { return get( jau::getCurrentMilliseconds() ); }

        /** Returns {@link #get(long)} as time string representation via {@link #toTimeStr(long, boolean)}. */
        std::string getTimeStr(uint64_t currentMillis, bool addFractions) {
            return toTimeStr(get(currentMillis), addFractions);
        }

        /** Returns {@link #getLast()} - rhs.{@link #getLast()}. */
        constexpr_atomic uint32_t diffLast(const PTS& rhs) const noexcept {
            return m_pts - rhs.last();
        }

        /** Returns {@link #get(long)} - rhs.{@link #get(long)}. */
        constexpr_atomic uint32_t diff(uint64_t currentMillis, const PTS& rhs) const noexcept {
            return get(currentMillis) - rhs.get(currentMillis);
        }

        std::string toString() noexcept { return std::to_string(m_pts); }

        std::string toString(uint64_t currentMillis) noexcept { return "last "+std::to_string(m_pts)+" ms, current "+std::to_string(get(currentMillis))+" ms"; }

        /**
         * Returns a time string representation '[HH:]mm:ss[.SSS]', dropping unused hour quantities and fractions of seconds optionally.
         * @param millis complete time in milliseconds
         * @param addFractions toggle for fractions of seconds
         * @see #toTimeStr(long)
         */
        std::string toTimeStr(uint64_t millis, bool addFractions) const noexcept {
            using namespace std::chrono;
            milliseconds ms(millis);
            seconds sec = duration_cast<seconds>(ms);
            ms -= duration_cast<milliseconds>(sec);
            minutes m = duration_cast<minutes>(sec);
            sec -= duration_cast<seconds>(m);
            hours h = duration_cast<hours>(m);
            m -= duration_cast<minutes>(h);

            if( addFractions ) {
                if( 0 < h.count() ) {
                    return jau::unsafe::format_string_n(12, "%02d:%02d:%02d.%03d", h, m, sec, millis);
                } else {
                    return jau::unsafe::format_string_n( 9, "%02d:%02d.%03d", m, sec, millis);
                }
            } else {
                if( 0 < h.count() ) {
                    return jau::unsafe::format_string_n(12, "%02d:%02d:%02d", h, m, sec);
                } else {
                    return jau::unsafe::format_string_n( 5, "%02d:%02d", m, sec);
                }
            }
        }

        /**
         * Returns a full time string representation 'HH:mm:ss.SSS'.
         * @param millis complete time in milliseconds
         * @see #toTimeStr(long, boolean)
         */
        std::string toTimeStr(uint64_t millis) {
            using namespace std::chrono;
            milliseconds ms(millis);
            seconds sec = duration_cast<seconds>(ms);
            ms -= duration_cast<milliseconds>(sec);
            minutes m = duration_cast<minutes>(sec);
            sec -= duration_cast<seconds>(m);
            hours h = duration_cast<hours>(m);
            m -= duration_cast<minutes>(h);
            return jau::unsafe::format_string_n(12, "%02d:%02d:%02d.%03d", h, m, sec, millis);
        }

        /**
         * Returns milliseconds from given string representation in '[H[H]:]m[m]:s[s][.S*]'
         * @param v the timestamp string to parse.
         */
        static uint32_t toMillis(const std::string& datestr) noexcept;

        /** Returns milliseconds from given string representation in '[H[H]:]m[m]:s[s][.S*]' or {@code -1} for parsing error. */
        // int toMillis(const std::string& datestr v) { return toMillis(v); }
    };

    /**@}*/

}  // namespace gamp::av

#endif /* GAMP_AV_PTS_HPP_ */
