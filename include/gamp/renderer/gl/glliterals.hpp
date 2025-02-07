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

namespace gamp::render::gl {

    class GLLiterals {
      public:
        constexpr static std::string getGLTypeName(GLenum type) noexcept {
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

        constexpr static std::string getGLArrayName(GLenum array) noexcept {
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

        static constexpr const char* mgl_Vertex = "mgl_Vertex";
        static constexpr const char* mgl_Normal = "mgl_Normal";
        static constexpr const char* mgl_Color = "mgl_Color";
        static constexpr const char* mgl_MultiTexCoord = "mgl_MultiTexCoord" ;
        static constexpr const char* mgl_InterleaveArray = "mgl_InterleaveArray" ; // magic name for interleaved arrays w/ sub-arrays

        /**
         * @param glArrayIndex the fixed function array index
         * @return default fixed function array name
         */
        static std::string getPredefinedArrayIndexName(GLenum glArrayIndex) {
            return getPredefinedArrayIndexName(glArrayIndex, -1);
        }

        /**
         * @param glArrayIndex the fixed function array index
         * @param multiTexCoordIndex index for multiTexCoordIndex
         * @return default fixed function array name
         */
        static std::string getPredefinedArrayIndexName(GLenum glArrayIndex, GLint multiTexCoordIndex) {
            switch(glArrayIndex) {
                case GL_VERTEX_ARRAY:
                    return mgl_Vertex;
                case GL_NORMAL_ARRAY:
                    return mgl_Normal;
                case GL_COLOR_ARRAY:
                    return mgl_Color;
                case GL_TEXTURE_COORD_ARRAY:
                    if(0<=multiTexCoordIndex) {
                        return mgl_MultiTexCoord+std::to_string(multiTexCoordIndex);
                    } else {
                        return mgl_MultiTexCoord+std::to_string(multiTexCoordIndex);
                    }
                default: return "";
            }
        }
        /** Version 1.00, i.e. GLSL 1.00 for ES 2.0. */
        constexpr static jau::util::VersionNumber Version1_0  = jau::util::VersionNumber(1, 0, 0);
        /** Version 1.10, i.e. GLSL 1.10 for GL 2.0. */
        constexpr static jau::util::VersionNumber Version1_10 = jau::util::VersionNumber(1, 10, 0);
        /** Version 1.20, i.e. GLSL 1.20 for GL 2.1. */
        constexpr static jau::util::VersionNumber Version1_20 = jau::util::VersionNumber(1, 20, 0);
        /** Version 1.30, i.e. GLSL 1.30 for GL 3.0. */
        constexpr static jau::util::VersionNumber Version1_30 = jau::util::VersionNumber(1, 30, 0);
        /** Version 1.40, i.e. GLSL 1.40 for GL 3.1. */
        constexpr static jau::util::VersionNumber Version1_40 = jau::util::VersionNumber(1, 40, 0);
        /** Version 1.50, i.e. GLSL 1.50 for GL 3.2. */
        constexpr static jau::util::VersionNumber Version1_50 = jau::util::VersionNumber(1, 50, 0);

        /** Version 1.1, i.e. GL 1.1 */
        constexpr static jau::util::VersionNumber Version1_1 = jau::util::VersionNumber(1, 1, 0);

        /** Version 1.2, i.e. GL 1.2 */
        constexpr static jau::util::VersionNumber Version1_2 = jau::util::VersionNumber(1, 2, 0);

        /** Version 1.4, i.e. GL 1.4 */
        constexpr static jau::util::VersionNumber Version1_4 = jau::util::VersionNumber(1, 4, 0);

        /** Version 1.5, i.e. GL 1.5 */
        constexpr static jau::util::VersionNumber Version1_5 = jau::util::VersionNumber(1, 5, 0);

        /** Version 3.0. As an OpenGL version, it qualifies for desktop {@link #isGL2()} only, or ES 3.0. Or GLSL 3.00 for ES 3.0. */
        constexpr static jau::util::VersionNumber Version3_0 = jau::util::VersionNumber(3, 0, 0);

        /** Version 3.1. As an OpenGL version, it qualifies for {@link #isGL3core()}, {@link #isGL3bc()} and {@link #isGL3()} */
        constexpr static jau::util::VersionNumber Version3_1 = jau::util::VersionNumber(3, 1, 0);

        /** Version 3.2. As an OpenGL version, it qualifies for geometry shader */
        constexpr static jau::util::VersionNumber Version3_2 = jau::util::VersionNumber(3, 2, 0);

        /** Version 4.3. As an OpenGL version, it qualifies for <code>GL_ARB_ES3_compatibility</code> */
        constexpr static jau::util::VersionNumber Version4_3 = jau::util::VersionNumber(4, 3, 0);
    };

} // namespace gamp::render::gl


#endif /* GAMP_GLLITERALS_HPP_ */
