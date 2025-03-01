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

#ifndef JAU_GAMP_TYPES_HPP_
#define JAU_GAMP_TYPES_HPP_

#include <cmath>
#include <cstdarg>
#include <cstdint>

#include <jau/cpp_lang_util.hpp>
#include <jau/environment.hpp>
#include <jau/os/os_support.hpp>

#include <jau/float_math.hpp>
#include <jau/math/vec2f.hpp>
#include <jau/math/vec2i.hpp>
#include <jau/math/vec3f.hpp>
#include <jau/math/vec4f.hpp>
#include <jau/math/mat4f.hpp>
#include <jau/math/recti.hpp>
#include <jau/math/util/pmvmat4f.hpp>
#include <jau/math/util/float_util.hpp>

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

namespace gamp {
    /** \addtogroup Gamp
     *
     *  @{
     */

    /** A native handle type, big enough to store a pointer. */
    typedef uintptr_t handle_t;

    /**@}*/
}  // namespace gamp

#endif /*  JAU_GAMP_TYPES_HPP_ */
