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

#include <jau/basic_types.hpp>
#include <jau/string_util.hpp>
#include <jau/util/VersionNumber.hpp>

#include <string_view>

#include <gamp/renderer/gl/glheader.hpp>
#include <gamp/renderer/gl/glliterals.hpp>
#include <gamp/renderer/gl/glversionnum.hpp>

namespace gamp::render::gl {

    class GLException : public jau::RuntimeException {
      public:
        GLException(std::string const& m, const char* file, int line) noexcept
        : RuntimeException("GLException", m, file, line) {}
    };

    /**
     * Specifies the OpenGL profile.
     */
    class GLProfile {
      public:
        /** The desktop OpenGL compatibility profile 4.x, with x >= 0, ie GL2 plus GL4.<br>
            <code>bc</code> stands for backward compatibility. */
        constexpr static std::string_view GL4bc = "GL4bc";

        /** The desktop OpenGL core profile 4.x, with x >= 0 */
        constexpr static std::string_view GL4 = "GL4";

        /** The desktop OpenGL compatibility profile 3.x, with x >= 1, ie GL2 plus GL3.<br>
            <code>bc</code> stands for backward compatibility. */
        constexpr static std::string_view GL3bc = "GL3bc";

        /** The desktop OpenGL core profile 3.x, with x >= 1 */
        constexpr static std::string_view GL3 = "GL3";

        /** The desktop OpenGL profile 1.x up to 3.0 */
        constexpr static std::string_view GL2 = "GL2";

        /** The embedded OpenGL profile ES 1.x, with x >= 0 */
        constexpr static std::string_view GLES1 = "GLES1";

        /** The embedded OpenGL profile ES 2.x, with x >= 0 */
        constexpr static std::string_view GLES2 = "GLES2";

        /** The embedded OpenGL profile ES 3.x, with x >= 0 */
        constexpr static std::string_view GLES3 = "GLES3";

        /** The intersection of the desktop GL2 and embedded ES1 profile */
        constexpr static std::string_view GL2ES1 = "GL2ES1";

        /** The intersection of the desktop GL3, GL2 and embedded ES2 profile */
        constexpr static std::string_view GL2ES2 = "GL2ES2";

        /** The intersection of the desktop GL3 and GL2 profile */
        constexpr static std::string_view GL2GL3 = "GL2GL3";

        /** The intersection of the desktop GL4 and ES3 profile, available only if either ES3 or GL4 w/ <code>GL_ARB_ES3_compatibility</code> is available. */
        constexpr static std::string_view GL4ES3 = "GL4ES3";

        /** The default profile, used for the device default profile map  */
        constexpr static std::string_view GL_UNDEF    = "undef";

      private:
        /** The default profile, used for the device default profile map  */
        constexpr static std::string_view GL_DEFAULT = "GL_DEFAULT";
        /** The default profile, used for the device default profile map  */
        constexpr static std::string_view GL_GL      = "GL";

        std::string_view m_profile, m_profileImpl;
        jau::util::VersionNumber m_version;

      public:
        constexpr GLProfile() noexcept : m_profile(GL_UNDEF), m_profileImpl(GL_UNDEF), m_version() {}

        /** Create a new instance. Given profile tag must be one of this class' constant `GL` profiles. */
        constexpr GLProfile(std::string_view tag, const jau::util::VersionNumber& version) noexcept
        : m_profile(tag), m_profileImpl(tag), m_version(version) {}

        /** Create a new instance. Given profile tag must be one of this class' constant `GL` profiles. */
        constexpr GLProfile(std::string_view tag, std::string_view tagImpl, const jau::util::VersionNumber& version) noexcept
        : m_profile(tag), m_profileImpl(tagImpl), m_version(version) {}

        constexpr const jau::util::VersionNumber& getVersion() const { return m_version; }
        constexpr const std::string_view& getName() const { return m_profile; }
        constexpr const std::string_view& getImplName() const { return m_profileImpl; }

        constexpr bool operator==(const GLProfile& rhs) const noexcept {
            return m_profile.data() == rhs.m_profile.data() && m_profileImpl.data() == rhs.m_profileImpl.data();
        }

        /** Indicates whether this profile is capable of GL4bc.  <p>Includes [ GL4bc ].</p> */
        constexpr bool isGL4bc() const noexcept {
            return GL4bc.data() == m_profile.data();
        }

        /** Indicates whether this profile is capable of GL4.    <p>Includes [ GL4bc, GL4 ].</p> */
        constexpr bool isGL4() const noexcept {
            return isGL4bc() || GL4.data() == m_profile.data();
        }

        /** Indicates whether this profile is capable of GL3bc.  <p>Includes [ GL4bc, GL3bc ].</p> */
        constexpr bool isGL3bc() const noexcept {
            return isGL4bc() || GL3bc.data() == m_profile.data();
        }

        /** Indicates whether this profile is capable of GL3.    <p>Includes [ GL4bc, GL4, GL3bc, GL3 ].</p> */
        constexpr bool isGL3() const noexcept {
            return isGL4() || isGL3bc() || GL3.data() == m_profile.data();
        }

        /** Indicates whether this profile is capable of GL2 .   <p>Includes [ GL4bc, GL3bc, GL2 ].</p> */
        constexpr bool isGL2() const noexcept {
            return isGL3bc() || GL2.data() == m_profile.data();
        }

        /** Indicates whether this profile is capable of GLES1.  <p>Includes [ GLES1 ].</p> */
        constexpr bool isGLES1() const noexcept {
            return GLES1.data() == m_profile.data();
        }

        /** Indicates whether this profile is capable of GLES2.  <p>Includes [ GLES2, GLES3 ].</p> */
        constexpr bool isGLES2() const noexcept {
            return isGLES3() || GLES2.data() == m_profile.data();
        }

        /** Indicates whether this profile is capable of GLES3.  <p>Includes [ GLES3 ].</p> */
        constexpr bool isGLES3() const noexcept {
            return GLES3.data() == m_profile.data();
        }

        /** Indicates whether this profile is capable of GLES.  <p>Includes [ GLES1, GLES2, GLES3 ].</p> */
        constexpr bool isGLES() const noexcept {
            return GLES3.data() == m_profile.data() || GLES2.data() == m_profile.data() || GLES1.data() == m_profile.data();
        }

        /** Indicates whether this profile is capable of GL2ES1. <p>Includes [ GL4bc, GL3bc, GL2, GLES1, GL2ES1 ].</p> */
        constexpr bool isGL2ES1() const noexcept {
            return GL2ES1.data() == m_profile.data() || isGLES1() || isGL2();
        }

        /** Indicates whether this profile is capable of GL2GL3. <p>Includes [ GL4bc, GL4, GL3bc, GL3, GL2, GL2GL3 ].</p> */
        constexpr bool isGL2GL3() const noexcept {
            return GL2GL3.data() == m_profile.data() || isGL3() || isGL2();
        }

        /** Indicates whether this profile is capable of GL2ES2. <p>Includes [ GL4bc, GL4, GL3bc, GL3, GLES3, GL2, GL2GL3, GL2ES2, GLES2 ].</p> */
        constexpr bool isGL2ES2() const noexcept {
            return GL2ES2.data() == m_profile.data() || isGLES2() || isGL2GL3();
        }

        /**
         * Indicates whether this profile is capable of GL2ES3. <p>Includes [ GL4bc, GL4, GL3bc, GL3, GLES3, GL3ES3, GL2, GL2GL3 ].</p>
         * @see #isGL3ES3()
         * @see #isGL2GL3()
         */
        constexpr bool isGL2ES3() const noexcept {
            return isGL3ES3() || isGL2GL3();
        }

        /** Indicates whether this profile is capable of GL3ES3. <p>Includes [ GL4bc, GL4, GL3bc, GL3, GLES3 ].</p> */
        constexpr bool isGL3ES3() const noexcept {
            return isGL4ES3() || isGL3();
        }

        /** Indicates whether this profile is capable of GL4ES3. <p>Includes [ GL4bc, GL4, GLES3 ].</p> */
        constexpr bool isGL4ES3() const noexcept {
            return GL4ES3.data() == m_profile.data() || isGLES3() || isGL4();
        }

        /** Indicates whether this profile supports GLSL, i.e. {@link #isGL2ES2()}. */
        constexpr bool hasGLSL() const noexcept {
            return isGL2ES2() ;
        }

        /**
         * Indicates whether the native OpenGL ES1 profile is in use.
         * This requires an EGL interface.
         */
        constexpr bool usesNativeGLES1() const noexcept {
            return GLES1.data() == m_profileImpl.data();
        }

        /**
         * Indicates whether the native OpenGL ES3 or ES2 profile is in use.
         * This requires an EGL, ES3 or ES2 compatible interface.
         */
        constexpr bool usesNativeGLES2() const noexcept {
            return GLES3.data() == m_profileImpl.data() || GLES2.data() == m_profileImpl.data();
        }

        /**
         * Indicates whether the native OpenGL ES2 profile is in use.
         * This requires an EGL, ES3 compatible interface.
         */
        constexpr bool usesNativeGLES3() const noexcept {
            return GLES3.data() == m_profileImpl.data();
        }

        /** Indicates whether either of the native OpenGL ES profiles are in use. */
        constexpr bool usesNativeGLES() const noexcept {
            return usesNativeGLES2() || usesNativeGLES1();
        }

        /**
         * General validation if type is a valid GL data type for the current profile.
         * <p>
         * Disclaimer: The validation might not satisfy updated OpenGL specifications.
         * </p>
         */
        constexpr bool isValidDataType(GLenum type, bool throwException) const {
            switch(type) {
                case GL_UNSIGNED_BYTE:
                case GL_BYTE:
                case GL_UNSIGNED_SHORT:
                case GL_SHORT:
                case GL_FLOAT:
                case GL_FIXED:
                    return true;
                case GL_INT:
                case GL_UNSIGNED_INT:
                    if( isGL2ES2() ) {
                        return true;
                    }
                case GL_DOUBLE:
                    if( isGL3() ) {
                        return true;
                    }
                case GL_2_BYTES:
                case GL_3_BYTES:
                case GL_4_BYTES:
                    if( isGL2() ) {
                        return true;
                    }
                default: break;
            }
            if(throwException) {
                throw GLException("Illegal data type on profile "+toString()+": "+jau::to_hexstring(type), E_FILE_LINE);
            }
            return false;
        }

        /**
         * General validation if index, comps and type are valid for the current profile.
         * <p>
         * Disclaimer: The validation might not satisfy updated OpenGL specifications.
         * </p>
         */
        constexpr bool isValidArrayDataType(GLenum index, size_t comps, GLenum type,
                                            bool isVertexAttribPointer, bool throwException) const {
            if( isGLES1() ) {
                if(isVertexAttribPointer) {
                    if(throwException) {
                        throw GLException("Illegal array type for "+GLLiterals::getGLArrayName(index)+" on profile GLES1: VertexAttribPointer", E_FILE_LINE);
                    }
                    return false;
                }
                switch(index) {
                    case GL_VERTEX_ARRAY:
                    case GL_TEXTURE_COORD_ARRAY:
                        switch(type) {
                            case GL_BYTE:
                            case GL_SHORT:
                            case GL_FIXED:
                            case GL_FLOAT:
                                break;
                            default:
                                if(throwException) {
                                    throw GLException("Illegal data type for "+GLLiterals::getGLArrayName(index)+" on profile GLES1: "+jau::to_hexstring(type), E_FILE_LINE);
                                }
                                return false;
                        }
                        switch(comps) {
                            case 0:
                            case 2:
                            case 3:
                            case 4:
                                break;
                            default:
                                if(throwException) {
                                    throw GLException("Illegal component number for "+GLLiterals::getGLArrayName(index)+" on profile GLES1: "+std::to_string(comps), E_FILE_LINE);
                                }
                                return false;
                        }
                        break;
                    case GL_NORMAL_ARRAY:
                        switch(type) {
                            case GL_BYTE:
                            case GL_SHORT:
                            case GL_FIXED:
                            case GL_FLOAT:
                                break;
                            default:
                                if(throwException) {
                                    throw GLException("Illegal data type for "+GLLiterals::getGLArrayName(index)+" on profile GLES1: "+jau::to_hexstring(type), E_FILE_LINE);
                                }
                                return false;
                        }
                        switch(comps) {
                            case 0:
                            case 3:
                                break;
                            default:
                                if(throwException) {
                                    throw GLException("Illegal component number for "+GLLiterals::getGLArrayName(index)+" on profile GLES1: "+std::to_string(comps), E_FILE_LINE);
                                }
                                return false;
                        }
                        break;
                    case GL_COLOR_ARRAY:
                        switch(type) {
                            case GL_UNSIGNED_BYTE:
                            case GL_FIXED:
                            case GL_FLOAT:
                                break;
                            default:
                                if(throwException) {
                                    throw new GLException("Illegal data type for "+GLLiterals::getGLArrayName(index)+" on profile GLES1: "+jau::to_hexstring(type), E_FILE_LINE);
                                }
                                return false;
                        }
                        switch(comps) {
                            case 0:
                            case 4:
                                break;
                            default:
                                if(throwException) {
                                    throw GLException("Illegal component number for "+GLLiterals::getGLArrayName(index)+" on profile GLES1: "+std::to_string(comps), E_FILE_LINE);
                                }
                                return false;
                        }
                        break;
                    default: break;
                }
            } else if( isGLES3() ) {
                // simply ignore !isVertexAttribPointer case, since it is simulated anyway ..
                switch(type) {
                    case GL_UNSIGNED_BYTE:
                    case GL_BYTE:
                    case GL_UNSIGNED_SHORT:
                    case GL_SHORT:
                    case GL_INT: // GL2ES2
                    case GL_UNSIGNED_INT:
                    case GL_HALF_FLOAT:
                    case GL_FLOAT:
                    case GL_FIXED:
                    case GL_INT_2_10_10_10_REV: // GL3ES3
                    case GL_UNSIGNED_INT_2_10_10_10_REV: // GL3ES2
                    case GL_UNSIGNED_INT_10F_11F_11F_REV:
                        break;
                    default:
                        if(throwException) {
                            throw GLException("Illegal data type on profile GLES3: "+jau::to_hexstring(type), E_FILE_LINE);
                        }
                        return false;
                }
                /** unable to validate .. could be any valid type/component combination
                switch(comps) {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                        break;
                    default:
                        if(throwException) {
                            throw new GLException("Illegal component number on profile GLES3: "+std::to_string(comps));
                        }
                        return false;
                } */
            } else if( isGLES2() ) {
                // simply ignore !isVertexAttribPointer case, since it is simulated anyway ..
                switch(type) {
                    case GL_UNSIGNED_BYTE:
                    case GL_BYTE:
                    case GL_UNSIGNED_SHORT:
                    case GL_SHORT:
                    case GL_FLOAT:
                    case GL_FIXED:
                        break;
                    default:
                        if(throwException) {
                            throw GLException("Illegal data type on profile GLES2: "+jau::to_hexstring(type), E_FILE_LINE);
                        }
                        return false;
                }
                /** unable to validate .. could be any valid type/component combination
                switch(comps) {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                        break;
                    default:
                        if(throwException) {
                            throw GLException("Illegal component number on profile GLES2: "+std::to_string(comps), E_FILE_LINE);
                        }
                        return false;
                } */
            } else if( isGL2ES2() ) {
                if(isVertexAttribPointer) {
                    switch(type) {
                        case GL_UNSIGNED_BYTE:
                        case GL_BYTE:
                        case GL_UNSIGNED_SHORT:
                        case GL_SHORT:
                        case GL_INT: // GL3ES2
                        case GL_UNSIGNED_INT:
                        case GL_HALF_FLOAT:
                        case GL_FLOAT:
                        case GL_FIXED:
                        case GL_INT_2_10_10_10_REV: // GL3ES3
                        case GL_UNSIGNED_INT_2_10_10_10_REV: // GL3ES2
                        case GL_DOUBLE: // GL2GL3
                        case GL_UNSIGNED_INT_10F_11F_11F_REV:
                            break;
                        default:
                            if(throwException) {
                                throw GLException("Illegal data type on profile GL2: "+jau::to_hexstring(type), E_FILE_LINE);
                            }
                            return false;
                    }
                    switch(comps) {
                        case 0:
                        case 1:
                        case 2:
                        case 3:
                        case 4:
                            break;
                        default:
                            if(throwException) {
                                throw GLException("Illegal component number on profile GL2: "+std::to_string(comps), E_FILE_LINE);
                            }
                            return false;
                    }
                } else {
                    switch(index) {
                        case GL_VERTEX_ARRAY:
                            switch(type) {
                                case GL_SHORT:
                                case GL_FLOAT:
                                case GL_INT: // GL2ES2
                                case GL_DOUBLE: // GL2GL3
                                    break;
                                default:
                                    if(throwException) {
                                        throw GLException("Illegal data type for "+GLLiterals::getGLArrayName(index)+" on profile GL2: "+jau::to_hexstring(type), E_FILE_LINE);
                                    }
                                    return false;
                            }
                            switch(comps) {
                                case 0:
                                case 2:
                                case 3:
                                case 4:
                                    break;
                                default:
                                    if(throwException) {
                                        throw GLException("Illegal component number for "+GLLiterals::getGLArrayName(index)+" on profile GL2: "+std::to_string(comps), E_FILE_LINE);
                                    }
                                    return false;
                            }
                            break;
                        case GL_NORMAL_ARRAY:
                            switch(type) {
                                case GL_BYTE:
                                case GL_SHORT:
                                case GL_FLOAT:
                                case GL_INT: // GL2ES2
                                case GL_DOUBLE: // GL2GL3
                                    break;
                                default:
                                    if(throwException) {
                                        throw GLException("Illegal data type for "+GLLiterals::getGLArrayName(index)+" on profile GL2: "+jau::to_hexstring(type), E_FILE_LINE);
                                    }
                                    return false;
                            }
                            switch(comps) {
                                case 0:
                                case 3:
                                    break;
                                default:
                                    if(throwException) {
                                        throw GLException("Illegal component number for "+GLLiterals::getGLArrayName(index)+" on profile GLES1: "+std::to_string(comps), E_FILE_LINE);
                                    }
                                    return false;
                            }
                            break;
                        case GL_COLOR_ARRAY:
                            switch(type) {
                                case GL_UNSIGNED_BYTE:
                                case GL_BYTE:
                                case GL_UNSIGNED_SHORT:
                                case GL_SHORT:
                                case GL_FLOAT:
                                case GL_INT: // GL2ES2
                                case GL_UNSIGNED_INT:
                                case GL_DOUBLE: // GL2GL3
                                    break;
                                default:
                                    if(throwException) {
                                        throw GLException("Illegal data type for "+GLLiterals::getGLArrayName(index)+" on profile GL2: "+jau::to_hexstring(type), E_FILE_LINE);
                                    }
                                    return false;
                            }
                            switch(comps) {
                                case 0:
                                case 3:
                                case 4:
                                    break;
                                default:
                                    if(throwException) {
                                        throw GLException("Illegal component number for "+GLLiterals::getGLArrayName(index)+" on profile GL2: "+std::to_string(comps), E_FILE_LINE);
                                    }
                                    return false;
                            }
                            break;
                        case GL_TEXTURE_COORD_ARRAY:
                            switch(type) {
                                case GL_SHORT:
                                case GL_FLOAT:
                                case GL_INT: // GL2ES2
                                case GL_DOUBLE: // GL2GL3
                                    break;
                                default:
                                    if(throwException) {
                                        throw GLException("Illegal data type for "+GLLiterals::getGLArrayName(index)+" on profile GL2: "+jau::to_hexstring(type), E_FILE_LINE);
                                    }
                                    return false;
                            }
                            switch(comps) {
                                case 0:
                                case 1:
                                case 2:
                                case 3:
                                case 4:
                                    break;
                                default:
                                    if(throwException) {
                                        throw GLException("Illegal component number for "+GLLiterals::getGLArrayName(index)+" on profile GL2: "+std::to_string(comps), E_FILE_LINE);
                                    }
                                    return false;
                            }
                            break;
                        default: break;
                    }
                }
            }
            return true;
        }

        std::string toString() const {
            std::string r("GLProfile[");
            r.append(getName()).append("/").append(getImplName()).append(", ").append(m_version.toString()).append("]");
            return r;
        }
    };
    inline std::ostream& operator<<(std::ostream& out, const GLProfile& v) {
        return out << v.toString();
    }

} // namespace gamp::render::gl


#endif /* GAMP_GLTYPES_HPP_ */
