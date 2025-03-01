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
#include <iostream>

#include <jau/test/catch2_ext.hpp>

#include <gamp/version.hpp>
#include <jau/util/VersionNumber.hpp>

using namespace jau::util;

TEST_CASE( "VersionNumber Test 00", "[version][util]" ) {
    std::cout << "gamp version: " << gamp::VERSION << std::endl;
    REQUIRE(true == gamp::VERSION.hasMajor());
    REQUIRE(true == gamp::VERSION.hasMinor());
    REQUIRE(true == gamp::VERSION.hasSub());
    REQUIRE(true == gamp::VERSION.hasString());
}

