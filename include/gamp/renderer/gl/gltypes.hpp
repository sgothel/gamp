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
