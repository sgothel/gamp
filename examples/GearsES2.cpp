/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2022-2024 Gothel Software e.K.
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

#include <gamp/gamp.hpp>

#include <cstdio>
#include <cmath>
#include <memory>

#include <jau/basic_types.hpp>
#include <jau/file_util.hpp>
#include <jau/float_types.hpp>
#include <jau/fraction_type.hpp>

#include <gamp/gamp.hpp>
#include "../demos/GearsES2.hpp"
#include "../demos/Launcher01.hpp"

using namespace jau::math;
using namespace jau::math::util;

using namespace gamp::wt;
using namespace gamp::wt::event;

int main(int argc, char *argv[]) // NOLINT(bugprone-exception-escape)
{
    return launch("GearsES2.hpp", LaunchProps{GLProfileMask::none, GLContextFlags::verbose},
                  std::make_shared<GearsES2>(), argc, argv);
}
