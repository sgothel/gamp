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

#include <ctime>
#include <regex>

#include <jau/debug.hpp>
#include <jau/environment.hpp>
#include <jau/string_util.hpp>

#include <gamp/av/PTS.hpp>

//
//
//

static std::regex jau_PTS_time_regex() noexcept {
    try {
        // [[12:]34:]56.789
        // - g1: hour 12 (optional)
        // - g2: minute 34
        // - g3: second 56
        // - g4: second-fraction 789  (optional)
        return
            std::regex( R"(^\s*)"
                        R"((?:(?:(2[0-3]|1\d|0?\d):)?([1-5]\d|0?\d):([1-5]\d|0?\d)(?:\.(\d+))?)?)"
                        //        g1                 g2               g3           g4
                      );

    } catch (...) {
        ERR_PRINT2("Caught unknown exception");
        return std::regex();
    }
}

uint32_t gamp::av::PTS::toMillis(const std::string& timestr) noexcept {
    static std::regex pattern = jau_PTS_time_regex();

    std::smatch match;
    try {
        if( std::regex_search(timestr, match, pattern) ) {
            constexpr bool DBG_OUT = false;
            if constexpr ( DBG_OUT ) {
                std::cout << "XXX: " << timestr << std::endl;
                std::cout << "XXX: match pos " << match.position() << ", len " << match.length() << ", sz " << match.size() << "\n";
                for(size_t i=0; i<match.size(); ++i) {
                    const std::string& ms = match[i];
                    std::cout << "- [" << i << "]: '" << ms << "', len " << ms.length() << "\n";
                }
            }
            uint32_t h=0,m=0,s=0;
            uint64_t ns=0;

            if( match.size() > 1 && match[1].length() > 0 ) {
                h = std::stoi(match[1]);
            }
            if( match.size() > 2 && match[2].length() > 0 ) {
                m = std::stoi(match[2]);
            }
            if( match.size() > 3 && match[3].length() > 0 ) {
                s = std::stoi(match[3]);
                if ( match.size() > 4 && 0 < match[4].length() && match[4].length() <= 9 ) {
                    const size_t ns_sdigits = match[4].length();
                    ns = std::stoul(match[4]) * static_cast<uint64_t>(std::pow(10, 9 - ns_sdigits));
                }
            }
            constexpr uint64_t ns_per_ms  =     1'000'000UL;
            constexpr uint64_t ms_per_sec =         1'000UL;
            constexpr uint64_t ms_per_min = ms_per_sec * 60UL;
            constexpr uint64_t ms_per_h   = ms_per_min * 60UL;

            return h * ms_per_h + m * ms_per_min + s * ms_per_sec +
                   ns / ns_per_ms;
        }
    } catch (...) {
        ERR_PRINT2("Caught unknown exception parsing %s", timestr.c_str());
    }
    return 0; // error
}

