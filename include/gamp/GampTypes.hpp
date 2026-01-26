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
#include <optional>

#include <string_view>
#include <string>

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
#include <jau/math/util/pmvmat4.hpp>
#include <jau/math/util/float_util.hpp>
#include <jau/string_util.hpp>
#include <jau/basic_collections.hpp>

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

    class GampException : public jau::RuntimeException {
      protected:
        GampException(std::string &&type, std::string const& m, const char* file, int line) noexcept
        : RuntimeException(std::move(type), m, file, line) {}

      public:
        GampException(std::string const& m, const char* file, int line) noexcept
        : RuntimeException("GampException", m, file, line) {}
    };

    template<typename T, typename U>
    inline bool checkOverflow(T has) {
        return has <= std::numeric_limits<U>::max();
    }
    template<typename T, typename U>
    inline void throwOnOverflow(T has) {
        if( has > std::numeric_limits<U>::max() ) {
            throw GampException("Value "+std::to_string(has)+" > "+std::to_string(std::numeric_limits<U>::max()), E_FILE_LINE);
        }
    }
    template<typename T, typename U>
    inline U castOrThrow(T has) {
        if( has > std::numeric_limits<U>::max() ) {
            throw GampException("Value "+std::to_string(has)+" > "+std::to_string(std::numeric_limits<U>::max()), E_FILE_LINE);
        }
        return static_cast<U>(has);
    }

    /** An attachable object */
    class Attachable {
      public:
        virtual ~Attachable() noexcept = default;
        virtual const jau::type_info& signature() const noexcept { return jau::static_ctti<Attachable>(); }
    };
    typedef std::shared_ptr<Attachable> AttachableSRef;

    using StringAttachables = jau::StringHashMapWrap<AttachableSRef, std::nullptr_t, nullptr>;
    using StringViewAttachables = jau::StringViewHashMapWrap<AttachableSRef, std::nullptr_t, nullptr>;

    /**@}*/
}  // namespace gamp

#endif /*  JAU_GAMP_TYPES_HPP_ */
