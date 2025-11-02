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

#include <cassert>
#include <cstring>

#include <jau/test/catch2_ext.hpp>

#include <jau/basic_types.hpp>

using namespace jau;
using namespace jau::fractions_i64_literals;
using namespace jau::int_literals;

#include <gamp/av/PTS.hpp>

using namespace gamp::av;

TEST_CASE( "PTS toMillis Test 01.01", "[PTS][time]" ) {
    {
        REQUIRE(0 == PTS::toMillis("00:00:00") );
        REQUIRE(0 == PTS::toMillis("00:00") );
        REQUIRE(   0 == PTS::toMillis("00:00.0001") );
        REQUIRE(   1 == PTS::toMillis("00:00.001") );
        REQUIRE(  10 == PTS::toMillis("00:00.010") );
        REQUIRE(  10 == PTS::toMillis("00:00.01" ) );
        REQUIRE( 100 == PTS::toMillis("00:00.100") );
        REQUIRE( 100 == PTS::toMillis("00:00.1"  ) );
        REQUIRE( 111 == PTS::toMillis("00:00.111") );
        REQUIRE(1000 == PTS::toMillis("00:01.000") );
        REQUIRE(1111 == PTS::toMillis("00:01.111") );
        REQUIRE(1111 == PTS::toMillis("00:00:01.111") );
    }
    {
        uint64_t t00a = fraction_timespec::from("1970-01-01T12:34:56.789Z").to_ms();
        uint64_t t00b = PTS::toMillis("12:34:56.789");

        REQUIRE(t00a == t00b);
    }
}
