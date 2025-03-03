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

#ifndef GAMP_GLPROFILE_HPP_
#define GAMP_GLPROFILE_HPP_

#include <jau/basic_types.hpp>
#include <jau/int_math_ct.hpp>
#include <jau/int_types.hpp>
#include <jau/float_types.hpp>
#include <jau/string_util.hpp>
#include <jau/util/VersionNumber.hpp>

#include <gamp/GampTypes.hpp>
#include <gamp/render/RenderContext.hpp>
#include <gamp/render/gl/GLHeader.hpp>
#include <gamp/render/gl/GLLiterals.hpp>
#include <gamp/render/gl/GLVersionNum.hpp>
#include <gamp/wt/Surface.hpp>

namespace gamp::render::gl {
    using namespace gamp::render;

    /** @defgroup Gamp_GL Gamp GL Rendering
     *  OpenGL managed rendering support, data handling and GLSL functionality.
     *
     *  @{
     */

    /**
     * Specifies the OpenGL profile.
     */
    class GLProfile : public RenderProfile {
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
        constexpr jau::util::VersionNumber mapTag2Version(std::string_view tag) noexcept {
            if( GL4 == tag || GL4bc == tag ) {
                return Version4_0;
            } else if( GL3 == tag || GL3bc == tag ) {
                return Version3_1;
            } else if( GL2 == tag ) {
                return Version2_0;
            } else if( GLES3 == tag ) {
                return Version3_0;
            } else if( GLES2 == tag ) {
                return Version2_0;
            } else if( GLES1 == tag ) {
                return Version1_0;
            }
            return Version0_0;
        }
        constexpr GLProfileMask mapTag2Mask(std::string_view tag) noexcept {
            if( GL4 == tag ) {
                return GLProfileMask::core;
            } else if( GL4bc == tag ) {
                return GLProfileMask::compat;
            } else if( GL3 == tag ) {
                return GLProfileMask::core;
            } else if( GL3bc == tag ) {
                return GLProfileMask::compat;
            } else if( GL2 == tag ) {
                return GLProfileMask::compat;
            } else if( GLES3 == tag ) {
                return GLProfileMask::es;
            } else if( GLES2 == tag ) {
                return GLProfileMask::es;
            } else if( GLES1 == tag ) {
                return GLProfileMask::es;
            }
            return GLProfileMask::none;
        }
        GLProfileMask m_profileMask;
        jau::util::VersionNumber m_glslVersion;

      protected:
        void clear() noexcept override {
            RenderProfile::clear();
            m_profileMask = GLProfileMask::none;
            m_glslVersion = jau::util::VersionNumber();
        }

      public:
        constexpr GLProfile() noexcept : RenderProfile(), m_profileMask(GLProfileMask::none), m_glslVersion() {}

        /** Create a new instance.*/
        constexpr GLProfile(const jau::util::VersionNumber& version, GLProfileMask profileMask) noexcept
        : RenderProfile(mapProfile2Tag(version, profileMask), version),
          m_profileMask(profileMask),
          m_glslVersion(getGLSLVersionNumber(version, profileMask))
        {}

        /** Create a new instance, merely deriving from given tag.*/
        constexpr GLProfile(std::string_view tag) noexcept
        : RenderProfile(tag, mapTag2Version(tag)),
          m_profileMask(mapTag2Mask(tag)),
          m_glslVersion(getGLSLVersionNumber(RenderProfile::version(), m_profileMask))
        {}

        const jau::type_info& signature() const noexcept override { return jau::static_ctti<GLProfile>(); }

        constexpr const jau::util::VersionNumber& glslVersion() const noexcept { return m_glslVersion; }
        constexpr GLProfileMask profileMask() const noexcept { return m_profileMask; }

        /** Indicates whether this profile is capable of GL4bc.  <p>Includes [ GL4bc ].</p> */
        constexpr bool isGL4bc() const noexcept {
            return GL4bc == name();
        }

        /** Indicates whether this profile is capable of GL4.    <p>Includes [ GL4bc, GL4 ].</p> */
        constexpr bool isGL4() const noexcept {
            return isGL4bc() || GL4 == name();
        }

        /** Indicates whether this profile is capable of GL3bc.  <p>Includes [ GL4bc, GL3bc ].</p> */
        constexpr bool isGL3bc() const noexcept {
            return isGL4bc() || GL3bc == name();
        }

        /** Indicates whether this profile is capable of GL3.    <p>Includes [ GL4bc, GL4, GL3bc, GL3 ].</p> */
        constexpr bool isGL3() const noexcept {
            return isGL4() || isGL3bc() || GL3 == name();
        }

        /** Indicates whether this profile is capable of GL2 .   <p>Includes [ GL4bc, GL3bc, GL2 ].</p> */
        constexpr bool isGL2() const noexcept {
            return isGL3bc() || GL2 == name();
        }

        /** Indicates whether this profile is capable of GLES1.  <p>Includes [ GLES1 ].</p> */
        constexpr bool isGLES1() const noexcept {
            return GLES1 == name();
        }

        /** Indicates whether this profile is capable of GLES2.  <p>Includes [ GLES2, GLES3 ].</p> */
        constexpr bool isGLES2() const noexcept {
            return isGLES3() || GLES2 == name();
        }

        /** Indicates whether this profile is capable of GLES3.  <p>Includes [ GLES3 ].</p> */
        constexpr bool isGLES3() const noexcept {
            return GLES3 == name();
        }

        /** Indicates whether this profile is capable of GLES.  <p>Includes [ GLES1, GLES2, GLES3 ].</p> */
        constexpr bool isGLES() const noexcept {
            return GLES3 == name() || GLES2 == name() || GLES1 == name();
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
                 ( !nativeGLES() && version().major()>1 );
        }

        /**
         * Indicates whether the native OpenGL ES1 profile is in use.
         * This requires an EGL interface.
         */
        constexpr bool nativeGLES1() const noexcept {
            return nativeGLES() && version().major() < 2;
        }

        /**
         * Indicates whether the native OpenGL ES3 or ES2 profile is in use.
         * This requires an EGL, ES3 or ES2 compatible interface.
         */
        constexpr bool nativeGLES2() const noexcept {
            return nativeGLES() && version().major() > 1;
        }

        /**
         * Indicates whether the native OpenGL ES2 profile is in use.
         * This requires an EGL, ES3 compatible interface.
         */
        constexpr bool nativeGLES3() const noexcept {
            return nativeGLES() && version().major() > 2;
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

        std::string toString() const override {
            return std::string("GLProfile[")
                   .append(name()).append(" ").append(version().toString()).append(", ")
                   .append(to_string(profileMask())).append(", glsl ").append(m_glslVersion.toString()).append("]");
        }
    };

    /** OpenGL Rendering Context */
    class GLContext : public RenderContext {
      protected:
        struct Private { explicit Private() = default; };

      private:
        static thread_local GLContext* m_current;
        GLProfile m_glprofile;
        GLVersionNumber m_glversion;

        static bool makeCurrentImpl(const gamp::wt::SurfaceRef& s, gamp::handle_t context) noexcept;
        static void releaseContextImpl(const gamp::wt::SurfaceRef& s) noexcept;

      public:
        /** Private: Create an invalid instance.*/
        GLContext(Private) noexcept : RenderContext(RenderContext::Private()) { }

        /** Private: Create a new instance of a non-current context. Given profile tag must be one of this class' constant `GL` profiles. */
        GLContext(Private, gamp::handle_t context, GLProfile profile,
                  RenderContextFlags contextFlags, GLVersionNumber glVersion) noexcept
        : RenderContext(RenderContext::Private(), context, contextFlags, glVersion),
          m_glprofile(std::move(profile)), m_glversion(std::move(glVersion)) {}

        /** Private: Create a new instance of a current context. Given profile tag must be one of this class' constant `GL` profiles. */
        GLContext(Private, const wt::SurfaceRef& surface, gamp::handle_t context, GLProfile profile,
                  RenderContextFlags contextFlags, GLVersionNumber glVersion) noexcept
        : RenderContext(RenderContext::Private(), context, contextFlags, glVersion),
          m_glprofile(std::move(profile)), m_glversion(std::move(glVersion))
        {
            RenderContext::makeCurrent(surface); // -> m_surface
            m_current = this;
        }

        ~GLContext() noexcept override {
            if( isCurrent() ) {
                m_current = nullptr;
            }
        }

        /** Create a new instance of a non-current context. Given profile tag must be one of this class' constant `GL` profiles. */
        static RenderContextPtr create(gamp::handle_t context, GLProfile profile,
                                       RenderContextFlags contextFlags, const char* gl_version_cstr) noexcept
        {
            return std::make_unique<GLContext>(Private(), context, std::move(profile), contextFlags, GLVersionNumber::create(gl_version_cstr));
        }
        /** Create a new instance of a current. Given profile tag must be one of this class' constant `GL` profiles. */
        static RenderContextPtr create(const wt::SurfaceRef& surface, gamp::handle_t context, GLProfile profile,
                                       RenderContextFlags contextFlags, const char* gl_version_cstr) noexcept
        {
            return std::make_unique<GLContext>(Private(), surface, context, std::move(profile), contextFlags, GLVersionNumber::create(gl_version_cstr));
        }

        const jau::type_info& signature() const noexcept override { return GLSignature(); }
        static const jau::type_info& GLSignature() noexcept { return jau::static_ctti<GLContext>(); }

        constexpr const RenderProfile& renderProfile() const noexcept override { return m_glprofile; }
        constexpr const GLProfile& glProfile() const noexcept { return m_glprofile; }

        constexpr const GLVersionNumber& glVersion() const { return m_glversion; }

        /// Returns an invalid GLContext reference
        static GLContext& getInvalid() noexcept {
            static GLContext a( (Private()) );
            return a;
        }
        static GLContext& cast(RenderContext* rc) {
            if( rc ) {
                if( rc->signature() == GLSignature() ) {
                    return static_cast<GLContext&>(*rc);
                }
                throw jau::IllegalArgumentError("Not a GLContext: "+rc->toString(), E_FILE_LINE);
            }
            throw jau::IllegalArgumentError("Null context", E_FILE_LINE);
        }

        /// Return thread local GLContext or an invalid GLContext, see isValid()
        static GLContext& getCurrent() noexcept {
            if( nullptr != m_current ) {
                return *m_current;
            }
            return getInvalid();
        }
        bool makeCurrent(const gamp::wt::SurfaceRef& s) noexcept override {
            if( makeCurrentImpl(s, context()) ) {
                RenderContext::makeCurrent(s); // -> m_surface
                m_current = this;
                return true;
            }
            return false;
        }
        void releaseContext() noexcept override {
            releaseContextImpl(m_surface);
            RenderContext::releaseContext(); // -> m_surface
            m_current = nullptr;
        }
        bool isCurrent() const noexcept { return this == m_current; }

        bool isExtensionAvailable(GLenum name) const noexcept {
            (void)name; // FIXME
            return false;
        }
        bool isExtensionAvailable(const char* name) const noexcept {
            (void)name; // FIXME
            return false;
        }

        void dispose() noexcept override;

        void disposedNotify() override {
            RenderContext::disposedNotify();
            if( isCurrent() ) {
                m_current = nullptr;
            }
        }

        std::string toString() const override {
            return std::string("GL[")
               .append(jau::to_hexstring(context())).append(", ")
               .append(to_string(contextFlags())).append(", ")
               .append(glProfile().toString()).append(", ")
               .append(glVersion().toString()).append(" -> surface ")
               .append(m_surface?jau::to_hexstring(m_surface->surfaceHandle()):"nil").append("]");
        }

    };
    typedef GLContext GL;

    /**@}*/

} // namespace gamp::render::gl


#endif /* GAMP_GLPROFILE_HPP_ */
