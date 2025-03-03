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

#ifndef GAMP_GLSLSHADERPROGRAM_HPP_
#define GAMP_GLSLSHADERPROGRAM_HPP_

#include <unordered_set>

#include <jau/basic_types.hpp>
#include <jau/debug.hpp>
#include <jau/file_util.hpp>
#include <jau/io_util.hpp>
#include <jau/string_util.hpp>

#include <gamp/Gamp.hpp>
#include <gamp/render/gl/glsl/ShaderCode.hpp>

namespace gamp::render::gl::glsl {
    using namespace gamp::render::gl;

    /** \addtogroup Gamp_GLSL
     *
     *  @{
     */

    class ShaderProgram;
    typedef std::shared_ptr<ShaderProgram> ShaderProgramRef;

    class ShaderProgram {
      private:
        struct Private{ explicit Private() = default; };

      public:
        /** Private ctor for `ShaderProgramRef create(...). */
        ShaderProgram(Private) noexcept
        : m_programLinked(false), m_programInUse(false), m_shaderProgram(0), m_id(0) {
            m_id = nextID();
        }
        static ShaderProgramRef create() noexcept {
            return std::make_shared<ShaderProgram>(Private());
        }

        constexpr bool linked() const noexcept { return m_programLinked; }

        constexpr bool inUse() const noexcept { return m_programInUse; }

        /** Returns the shader program name, which is non zero if valid. */
        constexpr GLuint program() const noexcept { return m_shaderProgram; }

        /** Returns the unique program id for successfully created instances, zero if instance creation failed. */
        constexpr size_t id() const noexcept { return m_id; }

        /**
         * Detaches all shader codes and deletes the program.
         * Destroys the shader codes as well.
         * Calls release(gl, true)
         *
         * @see #release(GL2ES2, boolean)
         */
        void destroy(GL& gl) noexcept {
            release(gl, true);
        }

        /**
         * Detaches all shader codes and deletes the program,
         * but leaves the shader code intact.
         * Calls release(gl, false)
         *
         * @see #release(GL2ES2, boolean)
         */
        void release(GL& gl) noexcept {
            release(gl, false);
        }

        /**
         * Detaches all shader codes and deletes the program.
         * If <code>destroyShaderCode</code> is true it destroys the shader codes as well.
         */
        void release(GL& gl, bool destroyShaderCode) noexcept {
            if( m_programLinked ) {
                unUseProgram(gl);
            }
            for(const auto & shaderCode : m_allShaderCode) {
                if( 1 == m_attachedShaderCode.erase(shaderCode) ) {
                    ShaderUtil::detachShader(gl, m_shaderProgram, shaderCode->shader());
                }
                if(destroyShaderCode) {
                    shaderCode->destroy(gl);
                }
            }
            m_allShaderCode.clear();
            m_attachedShaderCode.clear();
            if( 0 != m_shaderProgram ) {
                glDeleteProgram(m_shaderProgram);
                m_shaderProgram=0;
            }
            m_programLinked=false;
        }

        //
        // ShaderCode handling
        //

        /**
         * Adds a new shader to this program.
         *
         * <p>This command does not compile and attach the shader,
         * use {@link #add(GL2ES2, ShaderCode)} for this purpose.</p>
         *
         * @return true if the shader was successfully added, otherwise false (duplicate)
         */
        bool add(const ShaderCodeRef& shaderCode) noexcept {
            auto res = m_allShaderCode.insert(shaderCode);
            return res.second;
        }

        bool contains(const ShaderCodeRef& shaderCode) const noexcept{
            return m_allShaderCode.contains(shaderCode);
        }

        /**
         * Warning slow O(n) operation ..
         * @param id
         * @return
         */
        ShaderCodeRef getShader(size_t id) noexcept {
            for(const auto & shaderCode : m_allShaderCode) {
                 if(shaderCode->id() == id) {
                    return shaderCode;
                }
            }
            return nullptr;
        }

        //
        // ShaderCode / Program handling
        //

        /**
         * Creates the empty GL program object using {@link GL2ES2#glCreateProgram()},
         * if not already created.
         *
         * @param gl
         * @return true if shader program is valid, i.e. not zero
         */
        bool init(const GL&) noexcept {
            if( 0 == m_shaderProgram ) {
                m_shaderProgram = glCreateProgram();
            }
            return 0 != m_shaderProgram;
        }

        /**
         * Adds a new shader to a this non running program.
         *
         * <p>Compiles and attaches the shader, if not done yet.</p>
         *
         * @return true if the shader was successfully added, false if duplicate or compilation failed.
         */
        bool add(GL& gl, const ShaderCodeRef& shaderCode, bool verbose=false) {
            if( !init(gl) ) { return false; }
            if( m_allShaderCode.insert(shaderCode).second ) {
                if( !shaderCode->compile(gl, verbose) ) {
                    return false;
                }
                if( m_attachedShaderCode.insert(shaderCode).second ) {
                    ShaderUtil::attachShader(gl, m_shaderProgram, shaderCode->shader());
                }
            }
            return true;
        }

        /**
         * Replace a shader in a program and re-links the program.
         *
         * @param gl
         * @param oldShader   the to be replace Shader
         * @param newShader   the new ShaderCode
         * @param verboseOut  the optional verbose output stream
         *
         * @return true if all steps are valid, shader compilation, attachment and linking; otherwise false.
         *
         * @see ShaderState#glEnableVertexAttribArray
         * @see ShaderState#glDisableVertexAttribArray
         * @see ShaderState#glVertexAttribPointer
         * @see ShaderState#getVertexAttribPointer
         * @see ShaderState#glReleaseAllVertexAttributes
         * @see ShaderState#glResetAllVertexAttributes
         * @see ShaderState#glResetAllVertexAttributes
         * @see ShaderState#glResetAllVertexAttributes
         */
        bool replaceShader(GL& gl, const ShaderCodeRef& oldShader, const ShaderCodeRef& newShader, bool verbose=false) {
            if(!init(gl) || !newShader->compile(gl, verbose)) {
                return false;
            }

            const bool shaderWasInUse = inUse();
            if(shaderWasInUse) {
                unUseProgram(gl);
            }

            if( 1 == m_allShaderCode.erase(oldShader) && 1 == m_attachedShaderCode.erase(oldShader) ) {
                ShaderUtil::detachShader(gl, m_shaderProgram, oldShader->shader());
            }

            add(newShader);
            if( m_attachedShaderCode.insert(newShader).second ) {
                ShaderUtil::attachShader(gl, m_shaderProgram, newShader->shader());
            }

            glLinkProgram(m_shaderProgram);

            m_programLinked = ShaderUtil::isProgramLinkStatusValid(gl, m_shaderProgram, verbose);
            if ( m_programLinked && shaderWasInUse )  {
                useProgram(gl, true);
            }
            return m_programLinked;
        }

        /**
         * Links the shader code to the program.
         *
         * <p>Compiles and attaches the shader code to the program if not done by yet</p>
         *
         * <p>Within this process, all GL resources (shader and program objects) are created if necessary.</p>
         *
         * @param gl
         * @param verboseOut
         * @return true if program was successfully linked and is valid, otherwise false
         *
         * @see #init(GL2ES2)
         */
        bool link(GL& gl, bool verbose=false) {
            if( !init(gl) ) {
                m_programLinked = false; // mark unlinked due to user attempt to [re]link
                return false;
            }

            for(const auto & shaderCode : m_allShaderCode) {
                if(!shaderCode->compile(gl, verbose)) {
                    m_programLinked = false; // mark unlinked due to user attempt to [re]link
                    return false;
                }
                if( m_attachedShaderCode.insert(shaderCode).second ) {
                    ShaderUtil::attachShader(gl, m_shaderProgram, shaderCode->shader());
                }
            }

            // Link the program
            glLinkProgram(m_shaderProgram);

            m_programLinked = ShaderUtil::isProgramLinkStatusValid(gl, m_shaderProgram, verbose);

            return m_programLinked;
        }

        constexpr bool operator==(const ShaderProgram& rhs) const noexcept {
            if(this==&rhs) { return true; }
            return m_id == rhs.m_id;
        }
        constexpr std::size_t hash_code() const noexcept { return m_id; }

        string_t toString() const {
            string_t sb("ShaderCode[id=");
            sb.append(std::to_string(m_id))
              .append(", linked=").append(std::to_string(m_programLinked))
              .append(", inUse=").append(std::to_string(m_programInUse))
              .append(", program: ").append(std::to_string(m_shaderProgram)).append(", ")
              .append(std::to_string(m_allShaderCode.size())).append(" code: ");
            if( 0 < m_allShaderCode.size() ) {
                for(const auto & iter : m_allShaderCode) {
                    sb.append("\n").append("   ").append(iter->toString());
                }
            } else {
                sb.append("none");
            }
            sb.append("]");
            return sb;
        }

        /**
         * Performs {@link GL2ES2#glValidateProgram(int)} via {@link ShaderUtil#isProgramExecStatusValid(GL, int, PrintStream)}.
         * @return true on success, otherwise false
         * @see ShaderUtil#isProgramExecStatusValid(GL, int, PrintStream)
         **/
        bool validateProgram(GL& gl, bool verbose=false) noexcept {
            return ShaderUtil::isProgramExecStatusValid(gl, m_shaderProgram, verbose);
        }

        /**
         * Enables or disabled the shader program.
         *
         * @throws GLException if on==true and program was not linked
         */
        void useProgram(const GL&, bool on) {
            if(on && !m_programLinked) {
                ERR_PRINT("Not linked (on = %d): %s", on, toString().c_str());
                throw RenderException("Program is not linked", E_FILE_LINE);
            }
            if(m_programInUse==on) { return; }
            if( 0 == m_shaderProgram ) {
                on = false;
            }
            glUseProgram( on ? m_shaderProgram : 0 );
            m_programInUse = on;
        }
        /**
         * Disabled the shader program.
         */
        void unUseProgram(const GL&) noexcept {
            if(!m_programInUse) { return; }
            glUseProgram( 0 );
            m_programInUse = false;
        }
        void notifyNotInUse() noexcept {
            m_programInUse = false;
        }

        void dumpSource() {
            jau::PLAIN_PRINT(true, "");
            jau::PLAIN_PRINT(true, "%s", toString().c_str());
            for(const auto & iter : m_allShaderCode) {
                iter->dumpSource();
            }
            jau::PLAIN_PRINT(true, "");
        }

      private:
        bool m_programLinked;
        bool m_programInUse;
        GLuint m_shaderProgram; // non zero is valid!
        size_t m_id;
        std::unordered_set<ShaderCodeRef> m_allShaderCode, m_attachedShaderCode;

        static size_t nextID() { return m_nextID++; }
        static std::atomic<size_t> m_nextID;
    };

    inline std::ostream& operator<<(std::ostream& out, const ShaderProgram& v) {
        return out << v.toString();
    }

    /**@}*/
}

// injecting specialization of std::hash to namespace std of our types above
namespace std
{
    /** \addtogroup Gamp_GLSL
     *
     */

    template<> struct hash<gamp::render::gl::glsl::ShaderProgram> {
        std::size_t operator()(gamp::render::gl::glsl::ShaderProgram const& a) const noexcept {
            return a.hash_code();
        }
    };
    template<> struct hash<gamp::render::gl::glsl::ShaderProgramRef> {
        std::size_t operator()(gamp::render::gl::glsl::ShaderProgramRef const& a) const noexcept {
            return a->hash_code();
        }
    };

    /**@}*/
}

#endif // GAMP_GLSLSHADERPROGRAM_HPP_

