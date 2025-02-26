/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2012-2024 Gothel Software e.K.
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

