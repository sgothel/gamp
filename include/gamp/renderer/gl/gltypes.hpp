/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2022-2025 Gothel Software e.K.
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

#ifndef GAMP_GLTYPES_HPP_
#define GAMP_GLTYPES_HPP_

#include <GL/gl.h>
#include <GLES2/gl2.h>
#include <jau/basic_types.hpp>
#include <jau/darray.hpp>
#include <jau/int_math_ct.hpp>
#include <jau/int_types.hpp>
#include <jau/float_types.hpp>
#include <jau/string_util.hpp>
#include <jau/type_info.hpp>
#include <jau/util/VersionNumber.hpp>

#include <gamp/gamp_types.hpp>
#include <gamp/renderer/gl/glheader.hpp>
#include <gamp/renderer/gl/glliterals.hpp>
#include <gamp/renderer/gl/glversionnum.hpp>
#include <gamp/renderer/gl/glmisc.hpp>
#include <gamp/renderer/gl/glprofile.hpp>

static_assert(sizeof(GLenum)         == sizeof(uint32_t) );
static_assert(sizeof(GLint)          == sizeof(int32_t) );
static_assert(sizeof(GLfloat)        == sizeof(float) );
static_assert(sizeof(jau::float32_t) == sizeof(float) );
static_assert(sizeof(GLsizeiptr)     == sizeof(ssize_t) );

template<typename T,
    std::enable_if_t< std::is_same_v<float, T> , bool> = true>
constexpr GLenum glType() noexcept { return GL_FLOAT; }

template<typename T,
    std::enable_if_t< std::is_same_v<int32_t, T> , bool> = true>
constexpr GLenum glType() noexcept { return GL_INT; }

template<typename T,
    std::enable_if_t< std::is_same_v<uint32_t, T> , bool> = true>
constexpr GLenum glType() noexcept { return GL_UNSIGNED_INT; }

template<typename T,
    std::enable_if_t< std::is_same_v<int16_t, T> , bool> = true>
constexpr GLenum glType() noexcept { return GL_SHORT; }

template<typename T,
    std::enable_if_t< std::is_same_v<uint16_t, T> , bool> = true>
constexpr GLenum glType() noexcept { return GL_UNSIGNED_SHORT; }

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

#endif /* GAMP_GLTYPES_HPP_ */
