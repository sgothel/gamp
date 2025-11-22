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

#ifndef GAMP_GLSLSHADERSTATE_HPP_
#define GAMP_GLSLSHADERSTATE_HPP_

#include <jau/basic_types.hpp>
#include <jau/debug.hpp>
#include <jau/io/file_util.hpp>
#include <jau/io/io_util.hpp>
#include <jau/string_util.hpp>

#include <gamp/Gamp.hpp>
#include <gamp/render/gl/GLTypes.hpp>
#include <gamp/render/gl/data/GLArrayData.hpp>
#include <gamp/render/gl/data/GLUniformData.hpp>
#include <gamp/render/gl/glsl/ShaderProgram.hpp>
#include <gamp/render/gl/glsl/ShaderUtil.hpp>
#include <string_view>

namespace gamp::render::gl::glsl {
    using namespace gamp::render::gl;
    using namespace gamp::render::gl::data;

    /** \addtogroup Gamp_GLSL
     *
     *  @{
     */

    /**
     * ShaderState allows to sharing data between shader programs,
     * while updating the attribute and uniform locations when switching.
     * <p>
     * This allows seamless switching of programs using <i>almost</i> same data
     * but performing different artifacts.
     * </p>
     * <p>
     * A {@link #useProgram(GL, bool) used} ShaderState is attached to the current GL context
     * and can be retrieved via {@link #getShaderState(GL)}.
     * </p>
     */
    class ShaderState {
      public:
        static bool DEBUG_STATE;
        static bool VERBOSE_STATE;

        ShaderState() noexcept = default;

        bool verbose() const noexcept { return DEBUG_STATE || VERBOSE_STATE || m_verbose; }
        bool debug() const noexcept { return DEBUG_STATE; }

        void setVerbose(bool v) noexcept { m_verbose = v; }

        /** Returns the attached user object for the given name. */
        AttachableRef getAttachedObject(std::string_view key) const { return m_attachables.get(key); }

        /** Clears the attachment map. */
        void clearAttachedObjects() { m_attachables.clear(); }

        /**
         * Attaches user object for the given name, overwrites old mapping if exists.
         * @param key persistent std::string_view key, must be valid through the lifecycle of this instance
         * @return previously set object or nullptr.
         */
        AttachableRef attachObject(std::string_view key, const AttachableRef& obj) { return m_attachables.put1(key, obj); }

        /** Removes attached object if exists and returns it, otherwise returns nullptr. */
        AttachableRef detachObject(std::string_view key) { return m_attachables.remove1(key); }

        /**
         * Turns the shader program on or off
         *
         * @throws GLException if no program is attached, linkage or useProgram fails
         *
         * @see com.jogamp.opengl.util.glsl.ShaderState#useProgram(GL, bool)
         */
        void useProgram(GL& gl, bool on) {
            if(!m_shaderProgram) { throw RenderException("No program is attached", E_FILE_LINE); }
            if(on) {
                if(m_shaderProgram->linked()) {
                    m_shaderProgram->useProgram(gl, true);
                    if(m_resetAllShaderData) {
                        resetAllAttributes(gl);
                        resetAllUniforms(gl);
                    }
                } else {
                    if(m_resetAllShaderData) {
                        setAllAttributes(gl);
                    }
                    if(!m_shaderProgram->link(gl, verbose())) {
                        throw RenderException("could not link program: "+m_shaderProgram->toString(), E_FILE_LINE);
                    }
                    m_shaderProgram->useProgram(gl, true);
                    if(m_resetAllShaderData) {
                        resetAllUniforms(gl);
                    }
                }
                m_resetAllShaderData = false;
            } else {
                m_shaderProgram->useProgram(gl, false);
            }
        }

        /** Returns true if the shaderProgram() is linked, see ShaderProgram::linked(). */
        bool linked() const noexcept {
            return m_shaderProgram ? m_shaderProgram->linked() : false;
        }

        /** Returns true if the shaderProgram() is in use, see ShaderProgram::inUse(). */
        bool inUse() const noexcept {
            return m_shaderProgram ? m_shaderProgram->inUse() : false;
        }

        /**
         * Attach or switch a shader program
         *
         * <p>Attaching a shader program the first time,
         * as well as switching to another program on the fly,
         * while managing all attribute and uniform data.</p>
         *
         * <p>[Re]sets all data and use program in case of a program switch.</p>
         *
         * <p>Use program, {@link #useProgram(GL, bool)},
         * if <code>enable</code> is <code>true</code>.</p>
         *
         * @return true if shader program was attached, otherwise false (already attached)
         *
         * @throws GLException if program was not linked and linking fails
         */
        bool attachShaderProgram(GL& gl, const ShaderProgramRef& prog, bool enable) {
            if(debug()) {
                const size_t curId = m_shaderProgram ? m_shaderProgram->id() : 0;
                const size_t newId = prog ? prog->id() : 0;
                jau::INFO_PRINT("ShaderState: attachShaderProgram: %zu -> %zu (enable: %d)\n\t%s\n\t%s",
                    curId, newId, enable,
                    (m_shaderProgram ? m_shaderProgram->toString().c_str() : "null"),
                    (prog ? prog->toString().c_str() : "null"));
            }
            if(m_shaderProgram) {
                if(m_shaderProgram == prog) {
                    if(enable) {
                        useProgram(gl, true);
                    }
                    // nothing else to do ..
                    if(verbose()) {
                        jau::INFO_PRINT("ShaderState: attachShaderProgram: No switch, equal id: %zu , enabling %d",
                         m_shaderProgram->id(), enable);
                    }
                    return false;
                }
                if(m_shaderProgram->inUse()) {
                    if(prog && enable) {
                        m_shaderProgram->notifyNotInUse();
                    } else {
                        // no new 'enabled' program - disable
                        useProgram(gl, false);
                    }
                }
                m_resetAllShaderData = true;
            }

            // register new one
            m_shaderProgram = prog;

            if(m_shaderProgram) {
                // [re]set all data and use program if switching program,
                // or  use program if program is linked
                if(m_resetAllShaderData || enable) {
                    useProgram(gl, true); // may reset all data
                    if(!enable) {
                        useProgram(gl, false);
                    }
                }
            }
            if(debug()) {
                jau::INFO_PRINT("Info: attachShaderProgram: END");
            }
            return true;
        }

        /** Returns the attachedShaderProgram() or nullptr. */
        const ShaderProgramRef& shaderProgram() const noexcept { return m_shaderProgram; }

        /**
         * Calls {@link #release(GL, bool, bool, bool) release(gl, false, true, true)}
         *
         * @see #releaseAllAttributes
         * @see #releaseAllUniforms
         * @see #release(GL, bool, bool, bool)
         */
        void destroyShaderProgram(GL& gl) {
            release(gl, false, true, true);
        }

        /**
         * Calls {@link #release(GL, bool, bool, bool) release(gl, true, false, false)}
         *
         * @see #releaseAllAttributes
         * @see #releaseAllUniforms
         * @see #release(GL, bool, bool, bool)
         */
        void destroyAllData(GL& gl) {
            release(gl, true, false, false);
        }

        /**
         * Calls {@link #release(GL, bool, bool, bool) release(gl, true, true, true)}
         *
         * @see #releaseAllAttributes
         * @see #releaseAllUniforms
         * @see #release(GL, bool, bool, bool)
         */
        void destroy(GL& gl) {
            release(gl, true, true, true);
            clearAttachedObjects();
        }

        /**
         * Calls release(GL, bool, bool, bool) release(gl, false, false, false)}
         *
         * @see #releaseAllAttributes
         * @see #releaseAllUniforms
         * @see #release(GL, bool, bool, bool)
         */
        void releaseAllData(GL& gl) {
            release(gl, false, false, false);
        }

        /**
         * @see #glReleaseAllVertexAttributes
         * @see #glReleaseAllUniforms
         * @see ShaderProgram#release(GL, bool)
         */
        void release(GL& gl, bool destroyBoundAttributes, bool destroyShaderProgram, bool destroyShaderCode) {
            if(m_shaderProgram && m_shaderProgram->linked() ) {
                m_shaderProgram->useProgram(gl, false);
            }
            if(destroyBoundAttributes) {
                for(GLArrayDataRef& iter : m_managedAttributes) {
                    iter->destroy(gl);
                }
            }
            releaseAllAttributes(gl);
            releaseAllUniforms();
            if(m_shaderProgram && destroyShaderProgram) {
                m_shaderProgram->release(gl, destroyShaderCode);
            }
        }

        //
        // Shader attribute handling
        //

        /**
         * Gets the cached location of a shader attribute.
         *
         * @return -1 if there is no such attribute available,
         *         otherwise >= 0
         *
         * @see #bindAttribLocation(GL, int, String)
         * @see #bindAttribLocation(GL, int, GLArrayData)
         * @see #getAttribLocation(GL, String)
         * @see glGetAttribLocation(int, String)
         */
        GLint getCachedAttribLocation(const stringview_t name) const {
            return m_activeAttribLocationMap.get(name);
        }

        /**
         * Get the previous cached vertex attribute data.
         *
         * @return the GLArrayData object, null if not previously set.
         *
         * @see #ownAttribute(GLArrayData, bool)
         *
         * @see #glEnableVertexAttribArray
         * @see #glDisableVertexAttribArray
         * @see #glVertexAttribPointer
         * @see #getVertexAttribPointer
         * @see #glReleaseAllVertexAttributes
         * @see #glResetAllVertexAttributes
         * @see ShaderProgram#glReplaceShader
         */
        GLArrayDataRef getAttribute(const stringview_t name) const {
            return m_activeAttribMap.get(name);
        }

        bool isActive(const GLArrayDataRef& attribute) const {
            return attribute == m_activeAttribMap.get(attribute->name());
        }

        /**
         * Binds or unbinds the {@link GLArrayData} lifecycle to this ShaderState.
         *
         * If an attribute location is cached (ie {@link #bindAttribLocation(GL, int, String)})
         * it is promoted to the {@link GLArrayData} instance.</p>
         *
         * The attribute will be destroyed with {@link #destroy(GL)}
         * and it's location will be reset when switching shader with {@link #attachShaderProgram(GL, ShaderProgram)}.
         *
         * The data will not be transfered to the GPU, use {@link #vertexAttribPointer(GL, GLArrayData)} additionally.
         *
         * The data will also be {@link GLArrayData#associate(Object, bool) associated} with this ShaderState.
         *
         * Always issue ownAttribute() before GLArrayDataClient::seal() or GLArrayDataClient::enableBuffer(),
         * allowing it to fetch the attribute location via this ShaderState instance to render it functional.
         *
         * @param attribute the {@link GLArrayData} which lifecycle shall be managed
         * @param own true if <i>owning</i> shall be performs, false if <i>disowning</i>.
         *
         * @see #bindAttribLocation(GL, int, String)
         * @see #getAttribute(String)
         * @see GLArrayData#associate(Object, bool)
         */
        void manage(const GLArrayDataRef& attr, bool enable = true) {
            if(enable) {
                const GLint location = getCachedAttribLocation(attr->name());
                if(0<=location) {
                    attr->setLocation(location);
                }
                m_managedAttributes.push_back(attr);
            } else {
                std::erase(m_managedAttributes, attr);
            }
            attr->associate(*this, enable);
        }

        /// Returns true if given attribute is managed via manage()
        bool isManaged(const GLArrayDataRef& attribute) const {
            return m_managedAttributes.end() != std::find(m_managedAttributes.begin(), m_managedAttributes.end(), attribute); // NOLINT(modernize-use-ranges)
        }

        /**
         * Binds a shader attribute to a location.
         * Multiple names can be bound to one location.
         * The value will be cached and can be retrieved via {@link #getCachedAttribLocation(String)}
         * before or after linking.
         *
         * @param name Persistent attribute name, must be valid through the lifecycle of this instance
         * @throws GLException if no program is attached or program is already linked
         *
         * @see glBindAttribLocation(int, int, String)
         * @see #getAttribLocation(GL, String)
         * @see #getCachedAttribLocation(String)
         */
        void bindAttribLocation(const GL&, GLint location, const stringview_t name) {
            if(!m_shaderProgram) throw RenderException("No program is attached", E_FILE_LINE);
            if(m_shaderProgram->linked()) throw RenderException("Program is already linked", E_FILE_LINE);
            m_activeAttribLocationMap.put1(name, location);
            ::glBindAttribLocation(m_shaderProgram->program(), location, string_t(name).c_str());
        }

        /**
         * Binds a shader {@link GLArrayData} attribute to a location.
         * Multiple names can be bound to one location.
         * The value will be cached and can be retrieved via {@link #getCachedAttribLocation(String)}
         * and {@link #getAttribute(String)}before or after linking.
         * The {@link GLArrayData}'s location will be set as well.
         *
         * @throws GLException if no program is attached or program is already linked
         *
         * @see glBindAttribLocation(int, int, String)
         * @see #getAttribLocation(GL, String)
         * @see #getCachedAttribLocation(String)
         * @see #getAttribute(String)
         */
        void bindAttribLocation(const GL& gl, GLint location, const GLArrayDataRef& attr) {
            if(!m_shaderProgram) throw RenderException("No program is attached", E_FILE_LINE);
            if(m_shaderProgram->linked()) throw RenderException("Program is already linked", E_FILE_LINE);
            const stringview_t name = attr->name();
            m_activeAttribLocationMap.put1(name, location);
            attr->setLocation(gl, m_shaderProgram->program(), location);
            m_activeAttribMap.put1(name, attr);
            ::glBindAttribLocation(m_shaderProgram->program(), location, string_t(name).c_str());
        }

        /**
         * Gets the location of a shader attribute with given <code>name</code>.<br>
         * Uses either the cached value {@link #getCachedAttribLocation(String)} if valid,
         * or the GLSL queried via {@link glGetAttribLocation(int, String)}.<br>
         * The location will be cached.
         *
         * @param name Persistent attribute name, must be valid through the lifecycle of this instance
         * @return -1 if there is no such attribute available,
         *         otherwise >= 0
         * @throws GLException if no program is attached
         * @throws GLException if the program is not linked and no location was cached.
         *
         * @see #getCachedAttribLocation(String)
         * @see #bindAttribLocation(GL, int, GLArrayData)
         * @see #bindAttribLocation(GL, int, String)
         * @see glGetAttribLocation(int, String)
         */
        GLint getAttribLocation(const GL&, const stringview_t name) {
            if(!m_shaderProgram) throw RenderException("No program is attached", E_FILE_LINE);
            GLint location = getCachedAttribLocation(name);
            if(0>location) {
                if(!m_shaderProgram->linked()) throw RenderException("Program is not linked", E_FILE_LINE);
                location = ::glGetAttribLocation(m_shaderProgram->program(), string_t(name).c_str());
                if(0<=location) {
                    m_activeAttribLocationMap.put1(name, location);
                    if(debug()) {
                        jau::INFO_PRINT("ShaderState: glGetAttribLocation: %s, loc: %d", string_t(name).c_str(), location);
                    }
                } else if(verbose()) {
                    jau::INFO_PRINT("ShaderState: glGetAttribLocation failed, no location for: %s, loc: %d", string_t(name).c_str(), location);
                }
            }
            return location;
        }

        /**
         * Validates and returns the location of a shader attribute.<br>
         * Uses either the cached value {@link #getCachedAttribLocation(String)} if valid,
         * or the GLSL queried via {@link glGetAttribLocation(int, String)}.<br>
         * The location will be cached and set in the
         * {@link GLArrayData} object.
         *
         * @return -1 if there is no such attribute available,
         *         otherwise >= 0
         *
         * @throws GLException if no program is attached
         * @throws GLException if the program is not linked and no location was cached.
         *
         * @see #getCachedAttribLocation(String)
         * @see #bindAttribLocation(GL, int, GLArrayData)
         * @see #bindAttribLocation(GL, int, String)
         * @see glGetAttribLocation(int, String)
         * @see #getAttribute(String)
         */
        GLint getAttribLocation(const GL& gl, const GLArrayDataRef& data) {
            if(!m_shaderProgram) throw RenderException("No program is attached", E_FILE_LINE);
            const stringview_t name = data->name();
            GLint location = getCachedAttribLocation(name);
            if(0<=location) {
                data->setLocation(location);
            } else {
                if(!m_shaderProgram->linked()) throw RenderException("Program is not linked", E_FILE_LINE);
                location = data->setLocation(gl, m_shaderProgram->program());
                if(0<=location) {
                    m_activeAttribLocationMap.put1(name, location);
                    if(debug()) {
                        jau::INFO_PRINT("ShaderState: glGetAttribLocation: %s, loc: %d", string_t(data->name()).c_str(), location);
                    }
                } else if(verbose()) {
                    jau::INFO_PRINT("ShaderState: glGetAttribLocation failed, no location for: %s", string_t(data->name()).c_str());
                }
            }
            m_activeAttribMap.put1(data->name(), data);
            return location;
        }

        //
        // Enabled Vertex Arrays and its data
        //

        /**
         * @return true if the named attribute is enable
         */
        bool isVertexAttribArrayEnabled(const stringview_t name) const {
            return m_enabledAttribDataMap.get(name);
        }

        /**
         * @return true if the {@link GLArrayData} attribute is enable
         */
        bool isVertexAttribArrayEnabled(const GLArrayDataRef& data) const {
            return isVertexAttribArrayEnabled(data->name());
        }

      private:
        /// @param name Persistent attribute name, must be valid through the lifecycle of this instance
        bool enableVertexAttribArray(const GL& gl, const stringview_t name, GLint location) {
            m_enabledAttribDataMap.put1(name, true);
            if(0>location) {
                location = getAttribLocation(gl, name);
                if(0>location) {
                    if(verbose()) {
                        jau::INFO_PRINT("ShaderState: glEnableVertexAttribArray failed, no location for: %s", string_t(name).c_str());
                    }
                    return false;
                }
            }
            if(debug()) {
                jau::INFO_PRINT("ShaderState: glEnableVertexAttribArray: %s, loc: %d", string_t(name).c_str(), location);
            }
            ::glEnableVertexAttribArray(location);
            return true;
        }

      public:
        /**
         * Enables a vertex attribute array.
         *
         * This method retrieves the the location via {@link #getAttribLocation(GL, GLArrayData)}
         * hence {@link #enableVertexAttribArray(GL, GLArrayData)} shall be preferred.
         *
         * Even if the attribute is not found in the current shader,
         * it is marked enabled in this state.
         *
         * @param name Persistent attribute name, must be valid through the lifecycle of this instance
         * @return false, if the name is not found, otherwise true
         *
         * @throws GLException if the program is not linked and no location was cached.
         *
         * @see #glEnableVertexAttribArray
         * @see #glDisableVertexAttribArray
         * @see #glVertexAttribPointer
         * @see #getVertexAttribPointer
         */
        bool enableVertexAttribArray(const GL& gl, const stringview_t name) {
            return enableVertexAttribArray(gl, name, -1);
        }


        /**
         * Enables a vertex attribute array, usually invoked by {@link GLArrayDataEditable#enableBuffer(GL, bool)}.
         *
         * This method uses the {@link GLArrayData}'s location if set
         * and is the preferred alternative to {@link #enableVertexAttribArray(GL, String)}.
         * If data location is unset it will be retrieved via {@link #getAttribLocation(GL, GLArrayData)} set
         * and cached in this state.
         *
         * Even if the attribute is not found in the current shader,
         * it is marked enabled in this state.
         *
         * @return false, if the name is not found, otherwise true
         *
         * @throws GLException if the program is not linked and no location was cached.
         *
         * @see #glEnableVertexAttribArray
         * @see #glDisableVertexAttribArray
         * @see #glVertexAttribPointer
         * @see #getVertexAttribPointer
         * @see GLArrayDataEditable#enableBuffer(GL, bool)
         */
        bool enableVertexAttribArray(const GL& gl, const GLArrayDataRef& data) {
            if(0 > data->location()) {
                getAttribLocation(gl, data);
            } else {
                // ensure data is the current bound one
                m_activeAttribMap.put1(data->name(), data);
            }
            return enableVertexAttribArray(gl, data->name(), data->location());
        }

      private:
        /// @param name Persistent attribute name, must be valid through the lifecycle of this instance
        bool disableVertexAttribArray(const GL& gl, const stringview_t name, GLint location) {
            m_enabledAttribDataMap.put1(name, false);
            if(0>location) {
                location = getAttribLocation(gl, name);
                if(0>location) {
                    if(verbose()) {
                        jau::INFO_PRINT("ShaderState: glDisableVertexAttribArray failed, no location for: %s", string_t(name).c_str());
                    }
                    return false;
                }
            }
            if(debug()) {
                jau::INFO_PRINT("ShaderState: glDisableVertexAttribArray: %s, %d", string_t(name).c_str(), location);
            }
            ::glDisableVertexAttribArray(location);
            return true;
        }

      public:
        /**
         * Disables a vertex attribute array
         *
         * This method retrieves the the location via {@link #getAttribLocation(GL, GLArrayData)}
         * hence {@link #disableVertexAttribArray(GL, GLArrayData)} shall be preferred.
         *
         * Even if the attribute is not found in the current shader,
         * it is removed from this state enabled list.
         *
         * @return false, if the name is not found, otherwise true
         *
         * @throws GLException if no program is attached
         * @throws GLException if the program is not linked and no location was cached.
         *
         * @see #glEnableVertexAttribArray
         * @see #glDisableVertexAttribArray
         * @see #glVertexAttribPointer
         * @see #getVertexAttribPointer
         */
        bool disableVertexAttribArray(const GL& gl, const string_t& name) {
            return disableVertexAttribArray(gl, name, -1);
        }

        /**
         * Disables a vertex attribute array
         *
         * This method uses the {@link GLArrayData}'s location if set
         * and is the preferred alternative to {@link #disableVertexAttribArray(GL, String)}.
         * If data location is unset it will be retrieved via {@link #getAttribLocation(GL, GLArrayData)} set
         * and cached in this state.
         *
         * Even if the attribute is not found in the current shader,
         * it is removed from this state enabled list.
         *
         * @return false, if the name is not found, otherwise true
         *
         * @throws GLException if no program is attached
         * @throws GLException if the program is not linked and no location was cached.
         *
         * @see #glEnableVertexAttribArray
         * @see #glDisableVertexAttribArray
         * @see #glVertexAttribPointer
         * @see #getVertexAttribPointer
         */
        bool disableVertexAttribArray(const GL& gl, const GLArrayDataRef& data) {
            if(0 > data->location()) {
                getAttribLocation(gl, data);
            }
            return disableVertexAttribArray(gl, data->name(), data->location());
        }

        /**
         * Set the {@link GLArrayData} vertex attribute data, if it's location is valid, i.e. &ge; 0.
         * <p>
         * This method uses the {@link GLArrayData}'s location if valid, i.e. &ge; 0.<br/>
         * If data's location is invalid, it will be retrieved via {@link #getAttribLocation(GL, GLArrayData)},
         * set and cached in this state.
         * </p>
         *
         * @return false, if the location could not be determined, otherwise true
         *
         * @throws GLException if no program is attached
         * @throws GLException if the program is not linked and no location was cached.
         *
         * @see #glEnableVertexAttribArray
         * @see #glDisableVertexAttribArray
         * @see #glVertexAttribPointer
         * @see #getVertexAttribPointer
         */
        bool vertexAttribPointer(const GL& gl, const GLArrayDataRef& data) {
            GLint location = data->location();
            if(0 > location) {
                location = getAttribLocation(gl, data);
            }
            if(0>location) {
                if(verbose()) {
                    jau::INFO_PRINT("ShaderState: glVertexAttribPointer failed, no location for: %s", string_t(data->name()).c_str());
                }
                return false;
            }
            // only pass the data, if the attribute exists in the current shader
            if(debug()) {
                jau::INFO_PRINT("ShaderState: glVertexAttribPointer: %s, location %d", string_t(data->name()).c_str(), location);
            }
            data->glVertexAttribPointer(gl);
            return true;
        }

        /**
         * Releases all mapped vertex attribute data,
         * disables all enabled attributes and loses all indices
         *
         * @see #glEnableVertexAttribArray
         * @see #glDisableVertexAttribArray
         * @see #glVertexAttribPointer
         * @see #getVertexAttribPointer
         * @see #glReleaseAllVertexAttributes
         * @see #glResetAllVertexAttributes
         * @see #glResetAllVertexAttributes
         * @see ShaderProgram#glReplaceShader
         */
        void releaseAllAttributes(const GL& gl) {
            if(m_shaderProgram) {
                for (const std::pair<const std::string_view, GLArrayDataRef>& n : m_activeAttribMap.map()) {
                    disableVertexAttribArray(gl, n.second);
                }
            }
            m_activeAttribMap.clear();
            m_enabledAttribDataMap.clear();
            m_activeAttribLocationMap.clear();
            m_managedAttributes.clear();
        }

        /**
         * Disables all vertex attribute arrays.
         *
         * Their enabled stated will be removed from this state only
         * if 'removeFromState' is true.
         *
         * This method purpose is more for debugging.
         *
         * @see #glEnableVertexAttribArray
         * @see #glDisableVertexAttribArray
         * @see #glVertexAttribPointer
         * @see #getVertexAttribPointer
         * @see #glReleaseAllVertexAttributes
         * @see #glResetAllVertexAttributes
         * @see #glResetAllVertexAttributes
         * @see ShaderProgram#glReplaceShader
         */
        void disableAllVertexAttributeArrays(const GL& gl, bool removeFromState) {
            for (const std::pair<const std::string_view, bool>& n : m_enabledAttribDataMap.map()) {
                const stringview_t name = n.first;
                if(removeFromState) {
                    m_enabledAttribDataMap.remove1(name);
                }
                const GLint index = getAttribLocation(gl, name);
                if(0<=index) {
                    ::glDisableVertexAttribArray(index);
                }
            }
        }

      private:
        void relocateAttribute(const GL& gl, GLArrayData& attribute) {
            // get new location .. note: 'activeAttribLocationMap' is cleared before
            const stringview_t name = attribute.name();
            const GLint loc = attribute.setLocation(gl, m_shaderProgram->program());
            if(0<=loc) {
                m_activeAttribLocationMap.put1(name, loc);
                if(debug()) {
                    jau::INFO_PRINT("ShaderState: relocateAttribute: %s, loc: %d", attribute.toString().c_str(), loc);
                }
                if(isVertexAttribArrayEnabled(name)) {
                    // enable attrib, VBO and pass location/data
                    ::glEnableVertexAttribArray(loc);
                }

                if( attribute.isVBO() ) {
                    ::glBindBuffer(GL_ARRAY_BUFFER, attribute.vboName());
                    attribute.glVertexAttribPointer(gl);
                    ::glBindBuffer(GL_ARRAY_BUFFER, 0);
                } else {
                    attribute.glVertexAttribPointer(gl);
                }
            }
        }

        /**
         * Reset all previously enabled mapped vertex attribute data.
         *
         * <p>
         * Attribute data is bound to the GL state, i.e. VBO data itself will not be updated.
         * </p>
         *
         * <p>
         * Attribute location and it's data assignment is bound to the program,
         * hence both are updated.
         * </p>
         *
         * <p>
         * Note: Such update could only be prevented,
         * if tracking am attribute/program dirty flag.
         * </p>
         *
         * @throws GLException is the program is not linked
         *
         * @see #attachShaderProgram(GL, ShaderProgram)
         */
        void resetAllAttributes(const GL& gl) {
            if(!m_shaderProgram->linked()) throw RenderException("Program is not linked", E_FILE_LINE);
            m_activeAttribLocationMap.clear();

            for(GLArrayDataRef& ad : m_managedAttributes) {
                ad->setLocation(-1);
            }
            for (const std::pair<const std::string_view, GLArrayDataRef>& n : m_activeAttribMap.map()) {
                relocateAttribute(gl, *n.second);
            }
        }

        void setAttribute(const GL& gl, const GLArrayData& attribute) {
            // get new location ..
            const stringview_t name = attribute.name();
            const GLint loc = attribute.location();

            if(0<=loc) {
                bindAttribLocation(gl, loc, name);

                if(isVertexAttribArrayEnabled(name)) {
                    // enable attrib, VBO and pass location/data
                    ::glEnableVertexAttribArray(loc);
                }

                if( attribute.isVBO() ) {
                    ::glBindBuffer(GL_ARRAY_BUFFER, attribute.vboName());
                    attribute.glVertexAttribPointer(gl);
                    ::glBindBuffer(GL_ARRAY_BUFFER, 0);
                } else {
                    attribute.glVertexAttribPointer(gl);
                }
            }
        }

        /**
         * preserves the attribute location .. (program not linked)
         */
        void setAllAttributes(const GL& gl) {
            for (const std::pair<const std::string_view, GLArrayDataRef>& n : m_activeAttribMap.map()) {
                setAttribute(gl, *n.second);
            }
        }

      public:
        //
        // Shader Uniform handling
        //

        /**
         * Bind the {@link GLUniform} lifecycle to this ShaderState.
         *
         * The uniform will be destroyed with {@link #destroy(GL)}
         * and it's location will be reset when switching shader with {@link #attachShaderProgram(GL, ShaderProgram)}.
         *
         * The data will not be transfered to the GPU, use pushUniform() additionally.
         *
         * @param uniform the {@link GLUniformData} which lifecycle shall be managed
         */
        void manage(GLUniformData& data, bool enable=true) {
            if(enable) {
                m_managedUniforms.push_back(&data);
                m_activeUniformMap.put1(data.name(), &data);
            } else {
                m_activeUniformMap.remove1(data.name());
                std::erase(m_managedUniforms, &data);
            }
        }

        /// Returns true if given uniform is managed via manage()
        bool isManaged(const GLUniformData& uniform) const {
            return m_managedUniforms.end() != std::find(m_managedUniforms.begin(), m_managedUniforms.end(), &uniform); // NOLINT(modernize-use-ranges)
        }

        /**
         * Validates the uniform location or buffer-index.
         *
         * Queries GLSL in case !GLUniformData::hasLocation().
         *
         * The current shader program (attachShaderProgram(GL, ShaderProgram))
         * must be in use ({@link #useProgram(GL, bool) }) !
         *
         * @return true if successful, otherwise false
         * @see ShaderProgram#glReplaceShader
         */
        bool resolveUniform(const GL &gl, GLUniformData &data) {
            if (!data.hasLocation()) {
                if (!m_shaderProgram->inUse()) {
                    if (verbose()) {
                        jau::INFO_PRINT("ShaderState: program not in use: %s", m_shaderProgram->toString().c_str());
                    }
                    return false;
                }
                if (!data.resolveLocation(gl, m_shaderProgram->program())) {
                    if (verbose()) {
                        jau::INFO_PRINT("ShaderState: resolving uniform failed, no location/index for: %s", string_t(data.name()).c_str());
                    }
                    return false;
                }
            }
            m_activeUniformMap.put1(data.name(), &data);
            return true;
        }

        /**
         * Set the uniform data, if it's location is valid, i.e. >= 0.
         *
         * This method uses the {@link GLUniformData}'s location if valid, i.e. >= 0.
         * If data's location is invalid, it will be retrieved via getUniformLocation(GL, GLUniformData),
         * set and cached in this state.
         *
         * @return false, if the location could not be determined, otherwise true
         *
         * @see #glGetUniformLocation
         * @see glGetUniformLocation
         * @see glUniform
         * @see #getUniformLocation
         * @see ShaderProgram#glReplaceShader
         */
        bool pushUniform(const GL& gl, GLUniformData& data) noexcept {
            if (!m_shaderProgram->inUse()) {
                if (verbose()) {
                    jau::INFO_PRINT("ShaderState: program not in use: %s", m_shaderProgram->toString().c_str());
                }
                return false;
            }
            if (!data.hasLocation() && !resolveUniform(gl, data)) {
                m_activeUniformMap.remove1(data.name());
                return false;
            }
            data.send(gl);
            return true;
        }

        /** Same as pushUniform(), but for all active uniforms. */
        bool pushAllUniforms(const GL& gl) noexcept {
            if(!m_shaderProgram->inUse()) {
                if (verbose()) {
                    jau::INFO_PRINT("ShaderState: program not in use: %s", m_shaderProgram->toString().c_str());
                }
                return false;
            }
            for (const std::pair<const std::string_view, GLUniformData*>& n : m_activeUniformMap.map()) {
                GLUniformData &data = *n.second;
                if (data.hasLocation() || resolveUniform(gl, data)) {
                    data.send(gl);
                }
            }
            return true;
        }

        /// Returns true if given uniform data is active, i.e. previously resolved/pushed and used in current program.
        bool isActive(const GLUniformData& uniform) const {
            return &uniform == m_activeUniformMap.get(uniform.name());
        }

        /**
         * Get the active uniform data, previously resolved/pushed and used in current program.
         *
         * @param name uniform name
         * @return the GLUniformData object, nullptr if not previously resolved.
         */
        GLUniformData* getActiveUniform(const stringview_t name) {
            GLUniformData** v = m_activeUniformMap.get(name);
            return v ? *v : nullptr;
        }

        /**
         * Get the managed uniform data, previously owned.
         *
         * @param name uniform name
         * @return the GLUniformData object, nullptr if not previously owned.
         */
        GLUniformData* getManagedUniform(const stringview_t name) {
            {
                GLUniformData* r = getActiveUniform(name);
                if (r) {
                    return r;
                }
            }
            auto r = std::find_if(m_managedUniforms.begin(), m_managedUniforms.end(), [&name](GLUniformData *e) -> bool { return name == e->name(); }); // NOLINT(modernize-use-ranges)
            if( m_managedUniforms.end() != r ) {
                return *r;
            }
            return nullptr;
        }

        /**
         * Releases all mapped uniform data
         * and loses all indices
         */
        void releaseAllUniforms() {
            m_activeUniformMap.clear();
            m_managedUniforms.clear();
        }

      private:
        /**
         * Reset all previously mapped uniform data
         * <p>
         * Uniform data and location is bound to the program,
         * hence both are updated.
         * </p>
         * <p>
         * Note: Such update could only be prevented,
         * if tracking a uniform/program dirty flag.
         * </p>
         *
         * @throws GLException is the program is not in use
         *
         * @see #attachShaderProgram(GL, ShaderProgram)
         */
        bool resetAllUniforms(const GL& gl) noexcept {
            if (!m_shaderProgram->inUse()) {
                if (verbose()) {
                    jau::INFO_PRINT("ShaderState: program not in use: %s", m_shaderProgram->toString().c_str());
                }
                return false;
            }
            for (GLUniformData *u : m_managedUniforms) {
                u->clearLocation();
            }
            for (const std::pair<const std::string_view, GLUniformData*> &n : m_activeUniformMap.map()) {
                GLUniformData *data = n.second;
                if (data->resolveLocation(gl, m_shaderProgram->program())) {
                    if (debug()) {
                        jau::INFO_PRINT("ShaderState: resetAllUniforms: %s", string_t(data->name()).c_str());
                    }
                    if (!data->isBuffer()) {  // only send plain uniforms again
                        data->send(gl);
                    }
                }
            }
            return true;
        }

      public:
        string_t toString(bool alsoUnlocated=DEBUG_STATE) const {
            string_t sb;
            sb.append("ShaderState[\n ");
            if(m_shaderProgram) {
                sb.append(m_shaderProgram->toString());
            } else {
                sb.append("ShaderProgram: null");
            }
            sb.append("\n").append(" enabledAttributes [");
            for (const std::pair<const std::string_view, bool>& n : m_enabledAttribDataMap.map()) {
                sb.append("\n  ").append(n.first).append(": ").append(n.second?"enabled":"disabled");
            }
            sb.append("\n ],").append(" activeAttributes [");
            for (const std::pair<const std::string_view, GLArrayDataRef>& n : m_activeAttribMap.map()) {
                if( alsoUnlocated || 0 <= n.second->location() ) {
                    sb.append("\n  ").append(n.second->toString());
                }
            }
            sb.append("\n ],").append(" managedAttributes [");
            for(const GLArrayDataRef& ad : m_managedAttributes) {
                sb.append("\n  ").append(ad->toString());
            }
            sb.append("\n ],").append(" activeUniforms [");
            for (const std::pair<const std::string_view, GLUniformData*>& n : m_activeUniformMap.map()) {
                const GLUniformData *ud = n.second;
                if( alsoUnlocated || ud->hasLocation() ) {
                    sb.append("\n  ").append(ud->toString());
                }
            }
            sb.append("\n ],").append(" managedUniforms [");
            for(const GLUniformData *ud : m_managedUniforms) {
                sb.append("\n  ").append(ud->toString());
            }
            sb.append("\n ]").append("\n]");
            return sb;
        }

      private:
        bool             m_verbose            = false;
        ShaderProgramRef m_shaderProgram      = nullptr;
        bool             m_resetAllShaderData = false;

        jau::StringViewHashMapWrap<bool, bool, false> m_enabledAttribDataMap;
        jau::StringViewHashMapWrap<GLint, GLint, -1>  m_activeAttribLocationMap;
        jau::StringViewHashMapWrap<GLArrayDataRef, std::nullptr_t, nullptr> m_activeAttribMap;
        std::vector<GLArrayDataRef>              m_managedAttributes;

        jau::StringViewHashMapWrap<GLUniformData*, std::nullptr_t, nullptr> m_activeUniformMap;
        std::vector<GLUniformData*>              m_managedUniforms;

        StringViewAttachables m_attachables;
    };

    inline std::ostream& operator<<(std::ostream& out, const ShaderState& v) {
        return out << v.toString();
    }

    /**@}*/
}

#endif // GAMP_GLSLSHADERSTATE_HPP_
