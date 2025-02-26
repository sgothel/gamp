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
