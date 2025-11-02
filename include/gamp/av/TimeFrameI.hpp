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
#ifndef GAMP_AV_TIMEFRAMEI_HPP_
#define GAMP_AV_TIMEFRAMEI_HPP_

#include <cstdint>
#include <limits>

#include <jau/string_util.hpp>

#include <gamp/GampTypes.hpp>

namespace gamp::av {

    /** \addtogroup Gamp_AV
     *
     *  @{
     */

    /**
     * Integer time frame in milliseconds, maybe specialized for texture/video, audio, .. animated content.
     * <p>
     * Type and value range has been chosen to suit embedded CPUs
     * and characteristics of audio / video streaming and animations.
     * Milliseconds of type integer with a maximum value of MAX_UINT32 - 2 or 4'294'967'293 ms
     * will allow tracking time up 4'294'967.293 seconds or
     * 49 days 17 hours 2 minutes and 43 seconds, see pts() and duration().
     * </p>
     * <p>
     * Milliseconds granularity is also more than enough to deal with A-V synchronization,
     * where the threshold usually lies within 22ms.
     * </p>
     * <p>
     * Milliseconds granularity for displaying video frames might seem inaccurate
     * for each single frame, i.e. 60Hz != 16ms, however, accumulated values diminish
     * this error and vertical sync is achieved by build-in V-Sync of the video drivers.
     * </p>
     */
    class TimeFrameI {
      public:
        /** Constant marking an invalid (or undefined) PTS, i.e. MAX_UINT32 == 0xffffffff. Sync w/ native code. */
        constexpr static uint32_t INVALID_PTS = std::numeric_limits<uint32_t>::max();

        /** Constant marking the end of the stream PTS, i.e. MAX_UINT32 == 0xfffffffe. Sync w/ native code. */
        constexpr static uint32_t END_OF_STREAM_PTS = std::numeric_limits<uint32_t>::max() - 1;

      protected:
        uint32_t m_pts;
        uint32_t m_duration;

      public:
        /**
         * Ctor w/ zero duration and ::INVALID_PTS.
         */
        constexpr TimeFrameI() noexcept
        : m_pts(INVALID_PTS), m_duration(0)
        { }

        /**
         * Create a new instance
         * @param pts frame pts in milliseconds, see pts()
         * @param duration frame duration in milliseconds, see duration()
         * @see pts()
         * @see duration()
         */
        constexpr  TimeFrameI(uint32_t pts, uint32_t duration) noexcept
        : m_pts(pts), m_duration(duration)
        { }

        /**
         * Returns this frame's presentation timestamp (PTS) in milliseconds.
         * <p>
         * The relative millisecond PTS since start of the presentation stored in integer
         * covers a time span of 4'294'967'293 ms
         * or 4'294'967 seconds or 49 days.
         * </p>
         */
        constexpr uint32_t pts() const noexcept { return m_pts; }

        /**
         * Set this frame's presentation timestamp (PTS) in milliseconds.
         * @see pts()
         */
        void setPTS(uint32_t pts) noexcept { m_pts = pts; }

        /**
         * Get this frame's duration in milliseconds.
         * <p>
         * The duration stored in integer covers 4'294'967'293 ms
         * or 4'294'967 seconds or 49 days.
         * </p>
         */
        constexpr uint32_t duration() const noexcept { return m_duration; }

        /**
         * Set this frame's duration in milliseconds.
         * @see duration()
         */
        void setDuration(uint32_t duration) noexcept { m_duration = duration; }

        std::string toString() noexcept {
            std::string r("TimeFrame[pts ");
            if( m_pts < END_OF_STREAM_PTS ) {
                r.append(std::to_string(m_pts)).append(" ms");
            } else if( m_pts == END_OF_STREAM_PTS ) {
                r.append("eos");
            } else {
                r.append("undef");
            }
            return r.append(", l ").append(std::to_string(m_duration)).append(" ms]");
        }
    };

    /**@}*/

}  // namespace gamp::av

#endif /* GAMP_AV_TIMEFRAMEI_HPP_ */
