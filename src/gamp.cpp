/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2022 Gothel Software e.K.
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
#include "gamp/gamp.hpp"

#include <cstdint>
#include <ctime>

int gamp::win_width=0;
int gamp::win_height=0;
float gamp::devicePixelRatio[] = { 1, 1 };
int gamp::display_frames_per_sec=60;
int gamp::forced_fps = -1;

/**
 * See <http://man7.org/linux/man-pages/man2/clock_gettime.2.html>
 * <p>
 * Regarding avoiding kernel via VDSO,
 * see <http://man7.org/linux/man-pages/man7/vdso.7.html>,
 * clock_gettime seems to be well supported at least on kernel >= 4.4.
 * Only bfin and sh are missing, while ia64 seems to be complicated.
 */
uint64_t gamp::getCurrentMilliseconds() noexcept {
    struct timespec t;
    ::clock_gettime(CLOCK_MONOTONIC, &t);
    return static_cast<uint64_t>( t.tv_sec ) * (uint64_t)MilliPerOne +
           static_cast<uint64_t>( t.tv_nsec ) / (uint64_t)NanoPerMilli;
}

static uint64_t _exe_start_time = gamp::getCurrentMilliseconds();

uint64_t gamp::getElapsedMillisecond() noexcept {
    return getCurrentMilliseconds() - _exe_start_time;
}

void gamp::milli_sleep(uint64_t td_ms) noexcept {
    const int64_t td_ns_0 = (int64_t)( (td_ms * (uint64_t)NanoPerMilli) % (uint64_t)NanoPerOne );
    struct timespec ts;
    ts.tv_sec = static_cast<decltype(ts.tv_sec)>(td_ms/(uint64_t)MilliPerOne); // signed 32- or 64-bit integer
    ts.tv_nsec = td_ns_0;
    ::nanosleep( &ts, nullptr );
}

void gamp::log_printf(const uint64_t elapsed_ms, const char * format, ...) noexcept {
    fprintf(stderr, "[%s] ", to_decstring(elapsed_ms, ',', 9).c_str());
    va_list args;
    va_start (args, format);
    vfprintf(stderr, format, args);
    va_end (args);
}
void gamp::log_printf(const char * format, ...) noexcept {
    fprintf(stderr, "[%s] ", to_decstring(getElapsedMillisecond(), ',', 9).c_str());
    va_list args;
    va_start (args, format);
    vfprintf(stderr, format, args);
    va_end (args);
}

//
//
//
std::string gamp::input_event_t::to_string() const noexcept {
    return "event[p1 "+std::to_string(has_any_p1())+
            ", pressed "+std::to_string(m_pressed)+", p1_mask "+std::to_string(p1_mask)+
            ", p2 "+std::to_string(has_any_p2())+
            ", paused "+std::to_string(paused())+
            ", close "+std::to_string(pressed( gamp::input_event_type_t::WINDOW_CLOSE_REQ ))+
            ", last "+std::to_string((int)last)+", key "+std::to_string(last_key_code)+
            ", text "+text+
            ", ptr["+std::to_string(pointer_id)+" "+std::to_string(pointer_x)+"/"+
            std::to_string(pointer_y)+"]]"
            ;
}

