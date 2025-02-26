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

#ifndef GAMP_GLPROFILE_HPP_
#define GAMP_GLPROFILE_HPP_

#include <jau/basic_types.hpp>
#include <jau/int_math_ct.hpp>
#include <jau/int_types.hpp>
#include <jau/float_types.hpp>
#include <jau/string_util.hpp>
#include <jau/util/VersionNumber.hpp>

#include <gamp/gamp_types.hpp>
#include <gamp/renderer/gl/glheader.hpp>
#include <gamp/renderer/gl/glliterals.hpp>
#include <gamp/renderer/gl/glversionnum.hpp>
#include <gamp/renderer/gl/glmisc.hpp>

namespace gamp::render::gl {

    /** @defgroup Gamp_GL Gamp GL Rendering 
     *  OpenGL managed rendering support, data handling and GLSL functionality.
     *
     *  @{
     */
     
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

        /** The default profile, used for the device default profile map  */
        constexpr static std::string_view GL_UNDEF = "undef";

      private:
        /** The default profile, used for the device default profile map  */
        constexpr static std::string_view GL_DEFAULT = "GL_DEFAULT";
        /** The default profile, used for the device default profile map  */
        constexpr static std::string_view GL_GL      = "GL";

        constexpr static std::string_view mapProfile2Tag(const jau::util::VersionNumber& version, GLProfileMask mask) noexcept {
            if( 1 != jau::ct_bit_count( number(mask) ) ) {
                ERR_PRINT("GLProfileMask %s invalid, should have exactly 1 bit set", to_string(mask).c_str());                
            }
            if ( has_any(mask, GLProfileMask::es) ) {
                if( version.major() < 2 ) {
                    return GLES1;
                } else if( version.major() < 3 ) {
                    return GLES2;
                } else {
                    return GLES3;
                }
            }
            if( version.major() < 3 || ( version.major() == 3 && version.minor() < 1 ) ) {
                return GL2;
            } else if( version.major() < 4 ) {
                if ( has_any(mask, GLProfileMask::compat) ) {
                    return GL3bc;
                } else { // if ( has_any(mask, GLProfileMask::core) )
                    return GL3;
                }
            } 
            // else if( version.major() < 4 )
            if ( has_any(mask, GLProfileMask::compat) ) {
                return GL4bc;
            }
            // else if ( has_any(mask, GLProfileMask::core) )
            return GL4;
        }
        GLProfileMask m_profileMask;
        GLContextFlags m_contextFlags;
        std::string_view m_profile;
        jau::util::VersionNumber m_version, m_glslVersion;

      protected:
        void clear() noexcept {
            m_profileMask = GLProfileMask::none;
            m_contextFlags = GLContextFlags::none;
            m_profile = "";
            m_version = jau::util::VersionNumber();
            m_glslVersion = jau::util::VersionNumber();
        }

      public:
        constexpr GLProfile() noexcept : m_profileMask(GLProfileMask::none), m_profile(GL_UNDEF), m_version() {}

        /** Create a new instance.*/
        constexpr GLProfile(const jau::util::VersionNumber& version, GLProfileMask profileMask, GLContextFlags contextFlags) noexcept
        : m_profileMask(profileMask), m_contextFlags(contextFlags), 
          m_profile(mapProfile2Tag(version, profileMask)), m_version(version), 
          m_glslVersion(getGLSLVersionNumber(version, profileMask)) 
        {}

        constexpr const jau::util::VersionNumber& version() const noexcept { return m_version; }
        constexpr const jau::util::VersionNumber& glslVersion() const noexcept { return m_glslVersion; }
        constexpr GLProfileMask profileMask() const noexcept { return m_profileMask; }
        constexpr GLContextFlags contextFlags() const noexcept { return m_contextFlags; }
        constexpr const std::string_view& name() const noexcept { return m_profile; }

        constexpr bool operator==(const GLProfile& rhs) const noexcept {
            return m_profile == rhs.m_profile;
        }

        /** Indicates whether this profile is capable of GL4bc.  <p>Includes [ GL4bc ].</p> */
        constexpr bool isGL4bc() const noexcept {
            return GL4bc == m_profile;
        }

        /** Indicates whether this profile is capable of GL4.    <p>Includes [ GL4bc, GL4 ].</p> */
        constexpr bool isGL4() const noexcept {
            return isGL4bc() || GL4 == m_profile;
        }

        /** Indicates whether this profile is capable of GL3bc.  <p>Includes [ GL4bc, GL3bc ].</p> */
        constexpr bool isGL3bc() const noexcept {
            return isGL4bc() || GL3bc == m_profile;
        }

        /** Indicates whether this profile is capable of GL3.    <p>Includes [ GL4bc, GL4, GL3bc, GL3 ].</p> */
        constexpr bool isGL3() const noexcept {
            return isGL4() || isGL3bc() || GL3 == m_profile;
        }

        /** Indicates whether this profile is capable of GL2 .   <p>Includes [ GL4bc, GL3bc, GL2 ].</p> */
        constexpr bool isGL2() const noexcept {
            return isGL3bc() || GL2 == m_profile;
        }

        /** Indicates whether this profile is capable of GLES1.  <p>Includes [ GLES1 ].</p> */
        constexpr bool isGLES1() const noexcept {
            return GLES1 == m_profile;
        }

        /** Indicates whether this profile is capable of GLES2.  <p>Includes [ GLES2, GLES3 ].</p> */
        constexpr bool isGLES2() const noexcept {
            return isGLES3() || GLES2 == m_profile;
        }

        /** Indicates whether this profile is capable of GLES3.  <p>Includes [ GLES3 ].</p> */
        constexpr bool isGLES3() const noexcept {
            return GLES3 == m_profile;
        }

        /** Indicates whether this profile is capable of GLES.  <p>Includes [ GLES1, GLES2, GLES3 ].</p> */
        constexpr bool isGLES() const noexcept {
            return GLES3 == m_profile || GLES2 == m_profile || GLES1 == m_profile;
        }

        /** 
         * Indicates whether this profile is capable of GL2ES1. 
         *
         * Includes [ GL4bc, GL3bc, GL2, GLES1, GL2ES1 ].
         * 
         * GL2ES1 is the intersection of the desktop GL2 and embedded ES1 profile.
         */
        constexpr bool isGL2ES1() const noexcept {
            return isGLES1() || isGL2();
        }

        /** 
         * Indicates whether this profile is capable of GL2GL3. 
         *
         * Includes [ GL4bc, GL4, GL3bc, GL3, GL2, GL2GL3 ].
         *
         * GL2GL3 is the intersection of the desktop GL2 and GL3 profile.
         */
        constexpr bool isGL2GL3() const noexcept {
            return isGL3() || isGL2();
        }

        /** 
         * Indicates whether this profile is capable of GL2ES2.
         * 
         * Includes [ GL4bc, GL4, GL3bc, GL3, GLES3, GL2, GL2GL3, GL2ES2, GLES2 ].
         *
         * GL2ES2 is the intersection of the desktop GL2 and embedded ES2 profile.
         */         
        constexpr bool isGL2ES2() const noexcept {
            return isGLES2() || isGL2GL3();
        }

        /**
         * Indicates whether this profile is capable of GL2ES3.
         *
         * Includes [ GL4bc, GL4, GL3bc, GL3, GLES3, GL3ES3, GL2, GL2GL3 ].
         *
         * GL2ES3 is the intersection of the desktop GL2 and embedded ES3 profile.
         * @see #isGL3ES3()
         * @see #isGL2GL3()
         */
        constexpr bool isGL2ES3() const noexcept {
            return isGL3ES3() || isGL2GL3();
        }

        /** 
         * Indicates whether this profile is capable of GL3ES3. 
         *
         * Includes [ GL4bc, GL4, GL3bc, GL3, GLES3 ].
         *
         * GL3ES3 is the intersection of the desktop GL3 and embedded ES3 profile.
         */         
        constexpr bool isGL3ES3() const noexcept {
            return isGL4ES3() || isGL3();
        }

        /** Indicates whether this profile is capable of GL4ES3. <p>Includes [ GL4bc, GL4, GLES3 ].</p> */
        constexpr bool isGL4ES3() const noexcept {
            return isGLES3() || isGL4();
        }

        /** Indicates whether this profile supports GLSL, i.e. `nativeGLES2() || ( !nativeGLES() && m_version.major()>1 )`. */
        constexpr bool hasGLSL() const noexcept {
          return nativeGLES2() ||
                 ( !nativeGLES() && m_version.major()>1 );
        }

        /**
         * Indicates whether the native OpenGL ES1 profile is in use.
         * This requires an EGL interface.
         */
        constexpr bool nativeGLES1() const noexcept {
            return nativeGLES() && m_version.major() < 2;
        }

        /**
         * Indicates whether the native OpenGL ES3 or ES2 profile is in use.
         * This requires an EGL, ES3 or ES2 compatible interface.
         */
        constexpr bool nativeGLES2() const noexcept {
            return nativeGLES() && m_version.major() > 1;
        }

        /**
         * Indicates whether the native OpenGL ES2 profile is in use.
         * This requires an EGL, ES3 compatible interface.
         */
        constexpr bool nativeGLES3() const noexcept {
            return nativeGLES() && m_version.major() > 2;
        }

        /** Indicates whether either of the native OpenGL ES profiles are in use. */
        constexpr bool nativeGLES() const noexcept {
            return has_any(profileMask(), GLProfileMask::es);
        }

        /** Indicates whether either of the native OpenGL core profiles are in use. */
        constexpr bool nativeGLCore() const noexcept {
            return has_any(profileMask(), GLProfileMask::core);
        }
        
        /** Indicates whether either of the native OpenGL compatibility profiles are in use. */
        constexpr bool nativeGLCompat() const noexcept {
            return has_any(profileMask(), GLProfileMask::compat);
        }
        
        /**
         * General validation if type is a valid GL data type for the current profile.
         * <p>
         * Disclaimer: The validation might not satisfy updated OpenGL specifications.
         * </p>
         */
        constexpr bool isValidDataType(GLenum type) const noexcept {
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
            return false;
        }

        /**
         * General validation if index, comps and type are valid for the current profile.
         * <p>
         * Disclaimer: The validation might not satisfy updated OpenGL specifications.
         * </p>
         */
        bool isValidArrayDataType(GLenum index, GLsizei comps, GLenum type,
                                  bool isVertexAttribPointer, bool throwException) const;

        /**
         * Returns the GLSL version string as to be used in a shader program, including a terminating newline '\n',
         * i.e. for desktop
         * <pre>
         *    #version 110
         *    ..
         *    #version 150 core
         *    #version 330 compatibility
         *    ...
         * </pre>
         * And for ES:
         * <pre>
         *    #version 100
         *    #version 300 es
         *    ..
         * </pre>
         * <p>
         * If context has not been made current yet, a string of zero length is returned.
         * </p>
         * @see #getGLSLVersionNumber()
         */
        std::string getGLSLVersionString() const {
            
            if( m_glslVersion.isZero() ) {
                return "";
            }
            const int minor = m_glslVersion.minor();
            std::string profileOpt;
            if( has_any(m_profileMask, GLProfileMask::es) ) {
                profileOpt = m_glslVersion >= Version3_0 ? " es" : "";
            } else if( has_any(m_profileMask, GLProfileMask::core) ) {
                profileOpt = m_glslVersion >= Version1_50 ? " core" : "";
            } else if( has_any(m_profileMask, GLProfileMask::compat) ) {
                profileOpt = m_glslVersion >= Version1_50 ? " compatibility" : "";
            } else {
                profileOpt = ""; // unreachable, pre-validate
            }
            return "#version " + std::to_string(m_glslVersion.major()) + (minor < 10 ? "0" + std::to_string(minor) : std::to_string(minor)) + profileOpt + "\n";
        }

        std::string toString() const {
            return std::string("GLProfile[").append(m_version.toString()).append(" ")
                               .append(to_string(profileMask()))
                               .append(to_string(contextFlags()))
                               .append(", glsl ").append(m_glslVersion.toString())
                               .append(", ").append(name()).append("]");
        }
    };
    inline std::ostream& operator<<(std::ostream& out, const GLProfile& v) {
        return out << v.toString();
    }
    
    /** OpenGL Rendering Context */
    class GL : public GLProfile {
      private:
        gamp::handle_t m_context;
        gamp::render::gl::GLVersionNumber m_glversion;
        StringAttachables m_attachables;

      public:
        GL() noexcept : GLProfile(), m_context(0), m_glversion() { }
        
        /** Create a new instance. Given profile tag must be one of this class' constant `GL` profiles. */
        GL(gamp::handle_t context, const jau::util::VersionNumber& version, 
           GLProfileMask profileMask, GLContextFlags contextFlags, 
           const char* gl_version_cstr) noexcept
        : GLProfile(version, profileMask, contextFlags), m_context(context), m_glversion(gamp::render::gl::GLVersionNumber::create(gl_version_cstr)) { }
        
        constexpr bool isValid() const noexcept { return 0 != m_context; }
        constexpr gamp::handle_t context() const noexcept { return m_context; }        
        constexpr const gamp::render::gl::GLVersionNumber& glVersion() const { return m_glversion; }
        bool isExtensionAvailable(GLenum name) const noexcept {
            (void)name; // FIXME 
            return false; 
        }
        bool isExtensionAvailable(const char* name) const noexcept {
            (void)name; // FIXME 
            return false; 
        }
        
        void dispose() noexcept;
        
        void releasedNotify() { 
            m_context = 0;
            m_glversion=gamp::render::gl::GLVersionNumber(); 
            clearAttachedObjects();
            clear(); 
        }
                
        /** Returns the attached user object for the given name. */
        AttachableRef getAttachedObject(std::string_view key) const { return m_attachables.get(key); }

        /** Clears the attachment map. */
        void clearAttachedObjects() { m_attachables.clear(); }
        
        /**
         * Attaches user object for the given name, overwrites old mapping if exists.
         * @return previously set object or nullptr.
         */
        AttachableRef attachObject(std::string_view key, const AttachableRef& obj) { return m_attachables.put(key, obj); }

        /** Removes attached object if exists and returns it, otherwise returns nullptr. */
        AttachableRef detachObject(std::string_view key) { return m_attachables.remove(key); }
        
        std::string toLongString() const {
            return std::string("GL[").append(jau::to_hexstring(m_context)).append(", ")
                                     .append(version().toString()).append(" ").append(to_string(profileMask()))
                                     .append(", ").append(to_string(contextFlags()))
                                     .append(", glsl ").append(glslVersion().toString())
                                     .append(", ").append(name()).append(", ").append(glVersion().toString()).append("]");                                     
        }
        std::string toString() const {
            return std::string("GL[").append(jau::to_hexstring(m_context)).append(", ")
                                     .append(version().toString()).append(" ").append(to_string(profileMask()))
                                     .append(", ").append(to_string(contextFlags()))
                                     .append(", glsl ").append(glslVersion().toString())
                                     .append(", ").append(name()).append("]");                                     
        }
    };

    /**@}*/
    
} // namespace gamp::render::gl


#endif /* GAMP_GLPROFILE_HPP_ */
