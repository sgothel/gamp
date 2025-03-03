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

#ifndef GAMP_RENDER_RENDERCONTEXT_HPP_
#define GAMP_RENDER_RENDERCONTEXT_HPP_

#include <jau/basic_types.hpp>
#include <jau/int_math_ct.hpp>
#include <jau/int_types.hpp>
#include <jau/float_types.hpp>
#include <jau/string_util.hpp>
#include <jau/enum_util.hpp>
#include <jau/util/VersionNumber.hpp>

#include <gamp/GampTypes.hpp>

namespace gamp::wt {
    class Surface;
    typedef std::shared_ptr<Surface> SurfaceRef;
}

namespace gamp::render {

    /** @defgroup Gamp_Render Gamp Rendering
     *  Managed rendering support, data handling and functionality.
     *
     *  @{
     */
    using namespace jau::enums;

    /** OpenGL context flags. */
    enum class RenderContextFlags : uint32_t {
      none      = 0,
      /** Compatible context. */
      compatible  = 1U << 0,
      /** Debug context. */
      debug    = 1U <<  1,
      /** Robust context. */
      robust   = 1U <<  2,
      /** Software rasterizer context. */
      software = 1U <<  3,
      /** Verbose operations (debugging). */
      verbose  = 1U <<  31
    };
    JAU_MAKE_BITFIELD_ENUM_STRING(RenderContextFlags, compatible, debug, robust, software, verbose);

    class RenderContext;
    typedef std::shared_ptr<RenderContext> RenderContextRef;

    /**
     * Specifies the render profile.
     */
    class RenderProfile {
      public:
        /** The default profile, used for the device default profile map  */
        constexpr static std::string_view RP_UNDEF = "undef";

      private:
        std::string_view m_profile;
        jau::util::VersionNumber m_version;

      protected:
        friend RenderContext;

        virtual void clear() noexcept {
            m_profile = "";
            m_version = jau::util::VersionNumber();
        }
        static const RenderProfile& getUndef() noexcept {
            static RenderProfile a;
            return a;
        }

      public:
        /** Create an undefined instance.*/
        constexpr RenderProfile() noexcept : m_profile(RP_UNDEF), m_version() {}

        /** Create an instance w/ unique name.*/
        constexpr RenderProfile(const std::string_view profile, const jau::util::VersionNumber& version) noexcept
        : m_profile(profile), m_version(version)
        {}

        virtual ~RenderProfile() noexcept = default;

        virtual const jau::type_info& signature() const noexcept { return jau::static_ctti<RenderProfile>(); }

        constexpr const jau::util::VersionNumber& version() const noexcept { return m_version; }
        constexpr const std::string_view& name() const noexcept { return m_profile; }

        constexpr bool operator==(const RenderProfile& rhs) const noexcept {
            return signature() == rhs.signature() && m_profile == rhs.m_profile && m_version == rhs.version();
        }

        virtual std::string toString() const {
            return std::string("RenderProfile[").append(signature().name()).append(", ")
                .append(name()).append(" ").append(m_version.toString()).append("]");
        }
    };
    inline std::ostream& operator<<(std::ostream& out, const RenderProfile& v) {
        return out << v.toString();
    }

    /** Rendering Context */
    class RenderContext {
      private:
        gamp::handle_t m_context;
        jau::util::VersionNumber m_version;
        RenderContextFlags m_flags;
        StringAttachables m_attachables;

      protected:
        struct Private { explicit Private() = default; };
        gamp::wt::SurfaceRef m_surface;

      public:
        /** Private: Create an invalid instance.*/
        RenderContext(Private) noexcept
        : m_context(0), m_version(), m_flags() { }

        /** Private: Create an instance. Given profile tag must be a valid implementation profile. */
        RenderContext(Private, gamp::handle_t context,
           RenderContextFlags contextFlags,
           const jau::util::VersionNumber& version) noexcept
        : m_context(context), m_version(version), m_flags(contextFlags) { }

        virtual ~RenderContext() noexcept = default;

        virtual const jau::type_info& signature() const noexcept { return jau::static_ctti<RenderContext>(); }

        virtual const RenderProfile& renderProfile() const noexcept { return RenderProfile::getUndef(); }

        constexpr bool isValid() const noexcept { return 0 != m_context; }
        constexpr gamp::handle_t context() const noexcept { return m_context; }
        constexpr const jau::util::VersionNumber& version() const { return m_version; }

        constexpr RenderContextFlags contextFlags() const noexcept { return m_flags; }

        virtual void dispose() noexcept {}

        virtual void disposedNotify() {
            m_surface = nullptr;
            m_context = 0;
            m_version = jau::util::VersionNumberString();
            clearAttachedObjects();
        }

        /// Make this context current (used for OpenGL, but a NOP on Vulkan)
        virtual bool makeCurrent(const gamp::wt::SurfaceRef& s) noexcept { m_surface=s; return true; }
        /// Release this context (used for OpenGL, but a NOP on Vulkan)
        virtual void releaseContext() noexcept { m_surface = nullptr; }
        const gamp::wt::SurfaceRef& boundSurface() const noexcept { return m_surface; }

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

        virtual std::string toString() const;
    };
    typedef std::unique_ptr<RenderContext> RenderContextPtr;

    inline std::ostream& operator<<(std::ostream& out, const RenderContext& v) {
        return out << v.toString();
    }
    /**@}*/

} // namespace gamp::render


#endif /* GAMP_RENDER_RENDERCONTEXT_HPP_ */
