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

#ifndef GAMP_GLLITERALS_HPP_
#define GAMP_GLLITERALS_HPP_

#include <gamp/renderer/gl/glheader.hpp>
#include <gamp/renderer/gl/glversionnum.hpp>

#include <jau/cpp_lang_util.hpp>
#include <jau/enum_util.hpp>

namespace gamp::render::gl {
    using namespace jau::enums;

    /** \addtogroup Gamp_GL
     *
     *  @{
     */

    inline static constexpr GLenum GL_NVIDIA_PLATFORM_BINARY_NV = 0x890B;

    /** Version 1.00, i.e. GLSL 1.00 for ES 2.0. */
    inline constexpr static jau::util::VersionNumber Version1_0  = jau::util::VersionNumber(1, 0, 0);
    /** Version 1.10, i.e. GLSL 1.10 for GL 2.0. */
    inline constexpr static jau::util::VersionNumber Version1_10 = jau::util::VersionNumber(1, 10, 0);
    /** Version 1.20, i.e. GLSL 1.20 for GL 2.1. */
    inline constexpr static jau::util::VersionNumber Version1_20 = jau::util::VersionNumber(1, 20, 0);
    /** Version 1.30, i.e. GLSL 1.30 for GL 3.0. */
    inline constexpr static jau::util::VersionNumber Version1_30 = jau::util::VersionNumber(1, 30, 0);
    /** Version 1.40, i.e. GLSL 1.40 for GL 3.1. */
    inline constexpr static jau::util::VersionNumber Version1_40 = jau::util::VersionNumber(1, 40, 0);
    /** Version 1.50, i.e. GLSL 1.50 for GL 3.2. */
    inline constexpr static jau::util::VersionNumber Version1_50 = jau::util::VersionNumber(1, 50, 0);

    /** Version 1.1, i.e. GL 1.1 */
    inline constexpr static jau::util::VersionNumber Version1_1 = jau::util::VersionNumber(1, 1, 0);

    /** Version 1.2, i.e. GL 1.2 */
    inline constexpr static jau::util::VersionNumber Version1_2 = jau::util::VersionNumber(1, 2, 0);

    /** Version 1.4, i.e. GL 1.4 */
    inline constexpr static jau::util::VersionNumber Version1_4 = jau::util::VersionNumber(1, 4, 0);

    /** Version 1.5, i.e. GL 1.5 */
    inline constexpr static jau::util::VersionNumber Version1_5 = jau::util::VersionNumber(1, 5, 0);

    /** Version 3.0. As an OpenGL version, it qualifies for desktop {@link #isGL2()} only, or ES 3.0. Or GLSL 3.00 for ES 3.0. */
    inline constexpr static jau::util::VersionNumber Version3_0 = jau::util::VersionNumber(3, 0, 0);

    /** Version 3.1. As an OpenGL version, it qualifies for {@link #isGL3core()}, {@link #isGL3bc()} and {@link #isGL3()} */
    inline constexpr static jau::util::VersionNumber Version3_1 = jau::util::VersionNumber(3, 1, 0);

    /** Version 3.2. As an OpenGL version, it qualifies for geometry shader */
    inline constexpr static jau::util::VersionNumber Version3_2 = jau::util::VersionNumber(3, 2, 0);

    /** Version 4.3. As an OpenGL version, it qualifies for <code>GL_ARB_ES3_compatibility</code> */
    inline constexpr static jau::util::VersionNumber Version4_3 = jau::util::VersionNumber(4, 3, 0);

    /** OpenGL profile-mask bits. */
    enum class GLProfileMask : uint32_t {
      none    = 0,
      /** Desktop compatibility profile. */
      compat  = 1U <<  0,
      /** Desktop core profile. */
      core    = 1U <<  1,
      /** ES profile */
      es      = 1U <<  2
    };
    JAU_MAKE_BITFIELD_ENUM_STRING(GLProfileMask, compat, core, es);

    /** OpenGL context flags. */
    enum class GLContextFlags : uint32_t {
      none      = 0,
      /** Forward compatible context. */
      forward  = 1U << 0,
      /** Debug context. */
      debug    = 1U <<  1,
      /** Robust context. */
      robust   = 1U <<  2,
      /** Software rasterizer context. */
      software = 1U <<  3,
      /** Verbose operations (debugging). */
      verbose  = 1U <<  31
    };
    JAU_MAKE_BITFIELD_ENUM_STRING(GLContextFlags, forward, debug, robust, software, verbose);

    constexpr std::string_view getGLTypeName(GLenum type) noexcept {
        switch( type ) {
            case GL_UNSIGNED_BYTE:
                return "GL_UNSIGNED_BYTE";
            case GL_BYTE:
                return "GL_BYTE";
            case GL_UNSIGNED_SHORT:
                return "GL_UNSIGNED_SHORT";
            case GL_SHORT:
                return "GL_SHORT";
            case GL_FLOAT:
                return "GL_FLOAT";
            case GL_FIXED:
                return "GL_FIXED";
            case GL_INT:  // GL2ES2
                return "GL_INT";
            case GL_UNSIGNED_INT:
                return "GL_UNSIGNED_INT";
            case GL_DOUBLE:  // GL2GL3
                return "GL_DOUBLE";
            case GL_2_BYTES:  // GL2
                return "GL_2_BYTES";
            case GL_3_BYTES:  // GL2
                return "GL_3_BYTES";
            case GL_4_BYTES:  // GL2
                return "GL_4_BYTES";
            default: return "";
        }
    }

    constexpr std::string_view getGLArrayName(GLenum array) noexcept {
        switch( array ) {
            case GL_VERTEX_ARRAY:
                return "GL_VERTEX_ARRAY";
            case GL_NORMAL_ARRAY:
                return "GL_NORMAL_ARRAY";
            case GL_COLOR_ARRAY:
                return "GL_COLOR_ARRAY";
            case GL_TEXTURE_COORD_ARRAY:
                return "GL_TEXTURE_COORD_ARRAY";
            default: return "";
        }
    }

    constexpr bool isValidShaderType(GLenum type) noexcept {
        switch( type ) {
            case GL_VERTEX_SHADER:           // GL2ES2
            case GL_FRAGMENT_SHADER:         // GL2ES2
            case GL_GEOMETRY_SHADER:         // GL3ES3
            case GL_TESS_CONTROL_SHADER:     // GL3ES3
            case GL_TESS_EVALUATION_SHADER:  // GL3ES3
            case GL_COMPUTE_SHADER:          // GL3ES3
                return true;
            default:
                return false;
        }
    }

    constexpr std::string_view shaderTypeString(GLenum type) noexcept {
        switch( type ) {
            case GL_VERTEX_SHADER:  // GL2ES2
                return "VERTEX_SHADER";
            case GL_FRAGMENT_SHADER:  // GL2ES2
                return "FRAGMENT_SHADER";
            case GL_GEOMETRY_SHADER:  // GL3ES3
                return "GEOMETRY_SHADER";
            case GL_TESS_CONTROL_SHADER:  // GL3ES3
                return "TESS_CONTROL_SHADER";
            case GL_TESS_EVALUATION_SHADER:  // GL3ES3
                return "TESS_EVALUATION_SHADER";
            case GL_COMPUTE_SHADER:  // GL3ES3
                return "COMPUTE_SHADER";
            default: return "UNKNOWN_SHADER";
        }
    }

    constexpr jau::util::VersionNumber getGLSLVersionNumber(const jau::util::VersionNumber& glVersion, GLProfileMask mask) {
        if( has_any(mask, GLProfileMask::es) ) {
            if( 3 == glVersion.major() ) {
                return Version3_0;  // ES 3.0  ->  GLSL 3.00
            } else if( 2 == glVersion.major() ) {
                return Version1_0;  // ES 2.0  ->  GLSL 1.00
            }
        } else if( 1 == glVersion.major() ) {
            return Version1_10;  // GL 1.x  ->  GLSL 1.10
        } else if( 2 == glVersion.major() ) {
            switch( glVersion.minor() ) {
                case 0:  return Version1_10;  // GL 2.0  ->  GLSL 1.10
                default: return Version1_20;  // GL 2.1  ->  GLSL 1.20
            }
        } else if( 3 == glVersion.major() && 2 >= glVersion.minor() ) {
            switch( glVersion.minor() ) {
                case 0:  return Version1_30;  // GL 3.0  ->  GLSL 1.30
                case 1:  return Version1_40;  // GL 3.1  ->  GLSL 1.40
                default: return Version1_50;  // GL 3.2  ->  GLSL 1.50
            }
        }
        // The new default: GL >= 3.3, ES >= 3.0
        return jau::util::VersionNumber(glVersion.major(), glVersion.minor() * 10, 0);  // GL M.N  ->  GLSL M.N
    }

    inline static constexpr std::string_view mgl_Vertex          = "mgl_Vertex";
    inline static constexpr std::string_view mgl_Normal          = "mgl_Normal";
    inline static constexpr std::string_view mgl_Color           = "mgl_Color";
    inline static constexpr std::string_view mgl_MultiTexCoord   = "mgl_MultiTexCoord";
    inline static constexpr std::string_view mgl_InterleaveArray = "mgl_InterleaveArray";  // magic name for interleaved arrays w/ sub-arrays

    /**
     * @param glArrayIndex the fixed function array index
     * @param multiTexCoordIndex index for multiTexCoordIndex
     * @return default fixed function array name
     */
    inline std::string getPredefinedArrayIndexName(GLenum glArrayIndex, GLint multiTexCoordIndex) {
        switch( glArrayIndex ) {
            case GL_VERTEX_ARRAY:
                return std::string(mgl_Vertex);
            case GL_NORMAL_ARRAY:
                return std::string(mgl_Normal);
            case GL_COLOR_ARRAY:
                return std::string(mgl_Color);
            case GL_TEXTURE_COORD_ARRAY:
                if( 0 <= multiTexCoordIndex ) {
                    return std::string(mgl_MultiTexCoord).append(std::to_string(multiTexCoordIndex));
                } else {
                    return std::string(mgl_MultiTexCoord).append(std::to_string(multiTexCoordIndex));
                }
            default: return "";
        }
    }
    /**
     * @param glArrayIndex the fixed function array index
     * @return default fixed function array name
     */
    inline std::string getPredefinedArrayIndexName(GLenum glArrayIndex) {
        return getPredefinedArrayIndexName(glArrayIndex, -1);
    }

    /**@}*/

}  // namespace gamp::render::gl

#endif /* GAMP_GLLITERALS_HPP_ */
