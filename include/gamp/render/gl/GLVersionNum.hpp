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

#ifndef GAMP_GLVERSIONNUMBER_HPP_
#define GAMP_GLVERSIONNUMBER_HPP_

#include <jau/util/VersionNumber.hpp>

namespace gamp::render::gl {

    /** \addtogroup Gamp_GL
     *
     *  @{
     */

    /**
     * A class for storing and comparing OpenGL version numbers.
     * This only works for desktop OpenGL at the moment.
     */
    class GLVersionNumber : public jau::util::VersionNumberString {
      private:

        bool m_valid;

        GLVersionNumber(int val[], ssize_t strEnd, uint16_t state, const std::string& versionString, bool valid)
        : VersionNumberString(val[0], val[1], val[2],
                              0, 0, false, // git
                              strEnd, state, versionString), m_valid(valid)
        { }

        static const std::regex& getUnderscorePattern() noexcept { // NOLINT(bugprone-exception-escape)
            static std::regex pattern = getPattern("_");
            return pattern;
        }

      public:
        GLVersionNumber() noexcept : VersionNumberString() {}

        static GLVersionNumber create(const std::string& versionString) noexcept {
            int val[] = { 0, 0, 0 };
            ssize_t strEnd = 0;
            uint16_t state = 0;
            bool valid = false;
            if (versionString.length() > 0) {
                std::regex versionPattern;
                if (versionString.starts_with("GL_VERSION_")) {
                    versionPattern = getUnderscorePattern();
                } else {
                    versionPattern = VersionNumberString::getDefaultPattern();
                }
                VersionNumberString version(versionString, versionPattern);
                strEnd = version.endOfStringMatch();
                val[0] = version.major();
                val[1] = version.minor();
                state = (uint16_t) ( ( version.hasMajor() ? VersionNumber::HAS_MAJOR : (uint16_t)0 ) |
                                     ( version.hasMinor() ? VersionNumber::HAS_MINOR : (uint16_t)0 ) );
                valid = version.hasMajor() && version.hasMinor(); // Requires at least a defined major and minor version component!
            }
            return GLVersionNumber(val, strEnd, state, versionString, valid);
        }

        constexpr bool isValid() const noexcept {
            return m_valid;
        }

        /**
         * Returns the optional vendor version at the end of the
         * <code>GL_VERSION</code> string if exists, otherwise the {@link VersionNumberString#zeroVersion zero version} instance.
         * <pre>
         *   2.1 Mesa 7.0.3-rc2 -> 7.0.3 (7.0.3-rc2)
         *   2.1 Mesa 7.12-devel (git-d6c318e) -> 7.12.0 (7.12-devel)
         *   4.2.12171 Compatibility Profile Context 9.01.8 -> 9.1.8 (9.01.8)
         *   4.2.12198 Compatibility Profile Context 12.102.3.0 -> 12.102.3 (12.102.3.0)
         *   4.3.0 NVIDIA 310.32 -> 310.32 (310.32)
         * </pre>
         */
        VersionNumber createVendorVersion(const std::string& versionString) noexcept {
            if (versionString.length() <= 0) {
                return VersionNumber();
            }

            // Skip the 1st GL version
            std::string str;
            {
                GLVersionNumber glv = create(versionString);
                str = jau::trim( versionString.substr( glv.endOfStringMatch() ) );
            }

            while ( str.length() > 0 ) {
                VersionNumberString version(str, getDefaultPattern());
                ssize_t eosm = version.endOfStringMatch();
                if( 0 < eosm ) {
                    if( version.hasMajor() && version.hasMinor() ) { // Requires at least a defined major and minor version component!
                        return version;
                    }
                    str = jau::trim( str.substr( eosm ) );
                } else {
                    break; // no match
                }
            }
            return VersionNumber();
        }
    };

    /**@}*/

} // namespace gamp::render::gl


#endif /* GAMP_GLVERSIONNUMBER_HPP_ */
