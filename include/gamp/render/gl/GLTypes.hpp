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

#ifndef GAMP_RENDER_GL_GLTYPES_HPP_
#define GAMP_RENDER_GL_GLTYPES_HPP_

#include <jau/basic_types.hpp>
#include <jau/darray.hpp>
#include <jau/int_math_ct.hpp>
#include <jau/int_types.hpp>
#include <jau/float_types.hpp>
#include <jau/string_util.hpp>
#include <jau/type_info.hpp>
#include <jau/util/VersionNumber.hpp>

#include <gamp/GampTypes.hpp>
#include <gamp/render/RenderTypes.hpp>
#include <gamp/render/gl/GLHeader.hpp>
#include <gamp/render/gl/GLLiterals.hpp>
#include <gamp/render/gl/GLVersionNum.hpp>
#include <gamp/render/gl/GLContext.hpp>

namespace gamp::render::gl {

    /** \addtogroup Gamp_GL
     *
     *  @{
     */

    static_assert(sizeof(GLenum)         == sizeof(uint32_t) );
    static_assert(sizeof(GLint)          == sizeof(int32_t) );
    static_assert(sizeof(GLfloat)        == sizeof(float) );
    static_assert(sizeof(jau::float32_t) == sizeof(float) );
    static_assert(sizeof(GLsizeiptr)     == sizeof(ssize_t) );

    template<typename T,
        std::enable_if_t< std::is_same_v<float, T> , bool> = true>
    constexpr GLenum glType() noexcept { return GL_FLOAT; }

    template<typename T,
        std::enable_if_t< std::is_same_v<uint32_t, T> , bool> = true>
    constexpr GLenum glType() noexcept { return GL_UNSIGNED_INT; }

    template<typename T,
        std::enable_if_t< std::is_same_v<uint16_t, T> , bool> = true>
    constexpr GLenum glType() noexcept { return GL_UNSIGNED_SHORT; }

    template<typename T,
        std::enable_if_t< std::is_same_v<uint8_t, T> , bool> = true>
    constexpr GLenum glType() noexcept { return GL_UNSIGNED_BYTE; }

    template<typename T,
        std::enable_if_t< std::is_same_v<int32_t, T> , bool> = true>
    constexpr GLenum glType() noexcept { return GL_INT; }

    template<typename T,
        std::enable_if_t< std::is_same_v<int16_t, T> , bool> = true>
    constexpr GLenum glType() noexcept { return GL_SHORT; }


    /**
     * Runtime GL type retrieval using compile-time-type-info (CTTI) jau::type_info.
     *
     * Returns GL_NONE if type is not supported.
     */
    inline GLenum glType(const jau::type_info& t) noexcept {
        if( jau::float_ctti::f32() == t ) {
            return GL_FLOAT;
        } else if( jau::int_ctti::i32() == t ) {
            return GL_INT;
        } else if( jau::int_ctti::u32() == t ) {
            return GL_UNSIGNED_INT;
        } else if( jau::int_ctti::i16() == t ) {
            return GL_SHORT;
        } else if( jau::int_ctti::u16() == t ) {
            return GL_UNSIGNED_SHORT;
        }
        return GL_NONE;
    }

    /**@}*/

}

#endif /* GAMP_RENDER_GL_GLTYPES_HPP_ */
