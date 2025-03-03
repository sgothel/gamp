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

#include <gamp/Gamp.hpp>

#include <cstdio>
#include <cmath>
#include <memory>

#include <jau/basic_types.hpp>
#include <jau/file_util.hpp>
#include <jau/float_types.hpp>
#include <jau/fraction_type.hpp>

#include <gamp/Gamp.hpp>
#include <gamp/render/RenderContext.hpp>

#include "../demos/RedSquareES2.hpp"
#include "../demos/GLLauncher01.hpp"

using namespace jau::math;
using namespace jau::math::util;

using namespace gamp::wt;
using namespace gamp::wt::event;

int main(int argc, char *argv[]) // NOLINT(bugprone-exception-escape)
{
    return launch("RedSquareES2.hpp",
                  GLLaunchProps{GLProfile(GLProfile::GLES2), gamp::render::RenderContextFlags::verbose},
                  std::make_shared<RedSquareES2>(), argc, argv);
}
