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

#include <functional>
#include <string_view>

namespace gamp::render::gl::glsl {
    using namespace gamp::render::gl;
    using namespace gamp::render::gl::data;

    /** \addtogroup Gamp_GLSL
     *
     *  @{
     */

    namespace impl {
        struct DataLocNone {
        };
        struct DataLoc {
          private:
            stringview_t m_name;
            GLArrayDataSRef m_data;
            GLint m_location;
            bool m_enabled;

          public:
            constexpr DataLoc(DataLocNone) noexcept
            : m_name(), m_data(nullptr), m_location(-1), m_enabled(false)
            { }

            constexpr DataLoc(stringview_t name, GLint loc) noexcept
            : m_name(name), m_data(nullptr), m_location(loc), m_enabled(false)
            { }

            DataLoc(const GLArrayDataSRef& d) noexcept
            : m_name(d->name()), m_data(d), m_location(d->location()), m_enabled(false)
            { }

            constexpr bool valid() const noexcept { return !m_name.empty(); }
            constexpr operator bool() const noexcept { return valid(); }

            constexpr stringview_t name() const noexcept { return m_name; }
            constexpr const GLArrayDataSRef& data() const noexcept { return m_data; }
            constexpr GLArrayDataSRef& data() noexcept { return m_data; }
            constexpr GLint location() const noexcept { return m_location; }
            constexpr bool enabled() const noexcept { return m_enabled; }

            void setData(const GLArrayDataSRef &d) noexcept { if( valid() ) { m_data = d; } }
            void setLocation(GLint l) noexcept { if( valid() ) { m_location = l; } }
            void setEnabled(bool v) noexcept {
                if( valid() ) {
                    m_enabled = v;
                }
            }
            std::string toString() const noexcept {
                return "name: "+std::string(m_name)+", valid "+jau::to_string(valid())+
                       ", loc "+std::to_string(m_location)+", enabled "+jau::to_string(m_enabled)+
                       ", addr "+jau::to_string(this);
            }
        };
        typedef std::reference_wrapper<DataLoc> DataLocRef;
    }

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
        AttachableSRef getAttachedObject(std::string_view key) const { return m_attachables.get(key); }

        /** Clears the attachment map. */
        void clearAttachedObjects() { m_attachables.clear(); }

        /**
         * Attaches user object for the given name, overwrites old mapping if exists.
         * @param key persistent std::string_view key, must be valid through the lifecycle of this instance
         * @return previously set object or nullptr.
         */
        AttachableSRef attachObject(std::string_view key, const AttachableSRef& obj) { return m_attachables.put3(key, obj); }

        /** Removes attached object if exists and returns it, otherwise returns nullptr. */
        AttachableSRef detachObject(std::string_view key) { return m_attachables.remove2(key); }

        /**
         * Turns the shader program on or off
         *
         * @throws RenderException if no program is attached, linkage or useProgram fails
         *
         * @see useProgram(GL, bool)
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
         * @throws RenderException if program was not linked and linking fails
         */
        bool attachShaderProgram(GL& gl, const ShaderProgramSRef& prog, bool enable) {
            if(debug()) {
                const size_t curId = m_shaderProgram ? m_shaderProgram->id() : 0;
                const size_t newId = prog ? prog->id() : 0;
                jau_INFO_PRINT("ShaderState: attachShaderProgram: %zu -> %zu (enable: %d)\n\t%s\n\t%s",
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
                        jau_INFO_PRINT("ShaderState: attachShaderProgram: No switch, equal id: %zu , enabling %d",
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
                jau_INFO_PRINT("Info: attachShaderProgram: END");
            }
            return true;
        }

        /** Returns the attachedShaderProgram() or nullptr. */
        const ShaderProgramSRef& shaderProgram() const noexcept { return m_shaderProgram; }

        /**
         * Calls {@link #release(GL, bool, bool, bool) release(gl, false, true, true)}
         *
         * @see releaseAllAttributes
         * @see releaseAllUniforms
         * @see release(GL, bool, bool, bool)
         */
        void destroyShaderProgram(GL& gl) {
            release(gl, false, true, true);
        }

        /**
         * Calls {@link #release(GL, bool, bool, bool) release(gl, true, false, false)}
         *
         * @see releaseAllAttributes
         * @see releaseAllUniforms
         * @see release(GL, bool, bool, bool)
         */
        void destroyAllData(GL& gl) {
            release(gl, true, false, false);
        }

        /**
         * Calls {@link #release(GL, bool, bool, bool) release(gl, true, true, true)}
         *
         * @see releaseAllAttributes
         * @see releaseAllUniforms
         * @see release(GL, bool, bool, bool)
         */
        void destroy(GL& gl) {
            release(gl, true, true, true);
            clearAttachedObjects();
        }

        /**
         * Calls release(GL, bool, bool, bool) release(gl, false, false, false)}
         *
         * @see releaseAllAttributes
         * @see releaseAllUniforms
         * @see release(GL, bool, bool, bool)
         */
        void releaseAllData(GL& gl) {
            release(gl, false, false, false);
        }

        /**
         * @see releaseAllAttributes
         * @see releaseAllUniforms
         * @see ShaderProgram#release(GL, bool)
         */
        void release(GL& gl, bool destroyBoundAttributes, bool destroyShaderProgram, bool destroyShaderCode) {
            if(m_shaderProgram && m_shaderProgram->linked() ) {
                m_shaderProgram->useProgram(gl, false);
            }
            if(destroyBoundAttributes) {
                for(GLArrayDataSRef& iter : m_managedAttributes) {
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
         * Get the previous cached vertex attribute data.
         *
         * @return the GLArrayData object, null if not previously set.
         *
         * @see ownAttribute(GLArrayData, bool)
         *
         * @see enableAttribute
         * @see disableAttribute
         * @see vertexAttribPointer
         * @see releaseAllAttributes
         * @see resetAllAttributes
         * @see ShaderProgram#glReplaceShader
         */
        GLArrayDataSRef getAttribute(const stringview_t name) const {
            return m_activeAttribMap.get(name).data();
        }

        /**
         * Get the previous cached vertex attribute location.
         *
         * @return the location or -1 if not previously set.
         *
         * @see manage(GLArrayData, boolean)
         * @see enableVertexAttribArray
         * @see disableVertexAttribArray
         * @see vertexAttribPointer
         * @see releaseAllVertexAttributes
         * @see resetAllAttributes
         * @see ShaderProgram#replaceShader(GL2ES2, ShaderCode, ShaderCode, java.io.PrintStream)
         */
        GLint getAttributeLocation(stringview_t name) {
            return m_activeAttribMap.get(name).location();
        }

      private:
        impl::DataLoc& updateAttributeCache(const GLArrayDataSRef &data) {
            return updateDataLoc(m_activeAttribMap.get(data->name()), data, true);
        }
        impl::DataLoc& updateDataLoc(impl::DataLoc &dl, const GLArrayDataSRef &data, bool mapNewInstance) {
            if( dl.valid() ) {
                if( !dl.data() || dl.data() != data ) {
                    // no or different previous data object
                    dl.setData(data);
                }
                dl.setLocation(data->location());
                return dl;
            } else {
                // new instance
                if( mapNewInstance) {
                    return m_activeAttribMap.put2(data->name(), impl::DataLoc(data));
                }
                return m_activeAttribMap.novalue();
            }
        }
        void updateDataLoc(impl::DataLoc &dl, GLint location) {
            if( dl.data() ) {
                dl.data()->setLocation(location);
            }
            dl.setLocation(location);
        }

      public:
        bool isActive(const GLArrayDataSRef& attribute) const {
            return attribute == m_activeAttribMap.get(attribute->name()).data();
        }

        /**
         * Binds or unbinds the {@link GLArrayData} lifecycle to this ShaderState.
         *
         * If an attribute location is cached, ie bindAttributeLocation(),
         * it is promoted to the {@link GLArrayData} instance.</p>
         *
         * The attribute will be destroyed with {@link #destroy(GL)}
         * and it's location will be reset when switching shader with {@link #attachShaderProgram(GL, ShaderProgram)}.
         *
         * The data will not be transfered to the GPU, use {@link #vertexAttribPointer(GL, GLArrayData)} additionally.
         *
         * The data will also be {@link GLArrayData#associate(Object, bool) associated} with this ShaderState.
         *
         * Always issue manage() before GLArrayDataClient::seal() or GLArrayDataClient::enableBuffer(),
         * allowing it to fetch the attribute location via this ShaderState instance to render it functional.
         *
         * @param attribute the {@link GLArrayData} which lifecycle shall be managed
         * @param own true if <i>owning</i> shall be performs, false if <i>disowning</i>.
         *
         * @see bindAttributeLocation
         * @see getAttribute(String)
         * @see GLArrayData#associate(Object, bool)
         */
        void manage(const GLArrayDataSRef& attr, bool enable = true) {
            if(enable) {
                m_managedAttributes.push_back(attr);
            } else {
                std::erase(m_managedAttributes, attr);
            }
            attr->associate(*this, enable);
        }

        /// Returns true if given attribute is managed via manage()
        bool isManaged(const GLArrayDataSRef& attribute) const {
            return m_managedAttributes.end() != std::find(m_managedAttributes.begin(), m_managedAttributes.end(), attribute); // NOLINT(modernize-use-ranges)
        }

        /**
         * Binds a shader {@link GLArrayData} attribute to a location.
         * Multiple names can be bound to one location.
         * The value will be cached and can be retrieved via {@link #getCachedAttribLocation(String)}
         * and {@link #getAttribute(String)}before or after linking.
         * The {@link GLArrayData}'s location will be set as well.
         *
         * @throws RenderException if no program is attached or program is already linked
         *
         * @see bindAttributeLocation
         * @see getAttribute(GL, String)
         * @see getAttributeLocation(String)
         */
        void bindAttributeLocation(const GL& gl, GLint location, const GLArrayDataSRef& attr) {
            if(!m_shaderProgram) throw RenderException("No program is attached", E_FILE_LINE);
            if(m_shaderProgram->linked()) throw RenderException("Program is already linked", E_FILE_LINE);
            const stringview_t name = attr->name();
            attr->setLocation(gl, m_shaderProgram->program(), location);
            updateAttributeCache(attr);
            ::glBindAttribLocation(m_shaderProgram->program(), location, string_t(name).c_str());
        }

        /**
         * Validates the location of a shader attribute.
         *
         * If the cashed {@link #getAttributeLocation(String)} is invalid,
         * it is queried via {@link GL2ES2#glGetAttribLocation(int, String)} (GLSL).
         *
         * The location will be cached.
         *
         * @return -1 if there is no such attribute available,
         *         otherwise >= 0
         * @throws RenderException if no program is attached
         * @throws RenderException if the program is not linked and no location was cached.
         *
         * @see getAttributeLocation(String)
         * @see bindAttributeLocation
         * @see GL2ES2#glGetAttribLocation(int, String)
         */
        GLint resolveLocation(const GL& gl, stringview_t name) {
            return resolveLocation2(gl, name, false).location();
        }

      private:
        impl::DataLoc& resolveLocation2(const GL&, stringview_t name, bool forceMap) {
            impl::DataLoc *dl = &m_activeAttribMap.get(name);
            GLint location = dl->location();
            if( 0 > location ) {
                if (!m_shaderProgram) throw RenderException("No program is attached", E_FILE_LINE);
                if (!m_shaderProgram->linked()) throw RenderException("Program is not linked", E_FILE_LINE);
                location = ::glGetAttribLocation(m_shaderProgram->program(), string_t(name).c_str());
                impl::DataLoc dl_new(name, location);
                if( dl->valid() ) {
                    updateDataLoc(*dl, location);
                } else {
                    dl = &dl_new;
                }
                if( 0 <= location ) {
                    dl = &m_activeAttribMap.put2(name, *dl);
                    if (debug()) {
                        jau_INFO_PRINT("ShaderState: resolveLocation(1) failed, no location for: %s", string_t(name).c_str());
                    }
                } else {
                    if( forceMap ) {
                        dl = &m_activeAttribMap.put2(name, *dl);
                    } else {
                        dl = &m_activeAttribMap.novalue();
                    }
                    if(verbose()) {
                        jau_INFO_PRINT("ShaderState: resolveLocation(1) failed, no location for: %s", string_t(name).c_str());
                    }
                }
            }
            return *dl;
        }

      public:
        /**
         * Validates the location of a shader attribute.
         *
         * If GLArrayData::location() is invalid, it is queried via GLArrayDara::resolveLocation() (GLSL).
         *
         * @return true if successful, otherwise false
         *
         * @throws RenderException if no program is attached
         * @throws RenderException if the program is not linked and no location was cached.
         *
         * @see bindAttributeLocation
         * @see getAttributeLocation(int, String)
         * @see getAttribute(String)
         */
        bool resolveLocation(const GL& gl, const GLArrayDataSRef& data) {
            resolveLocation2(gl, data, false);
            return data->hasLocation();
        }

      private:
        impl::DataLoc& resolveLocation2(const GL& gl, const GLArrayDataSRef& data, bool forceMap) {
            if ( data->hasLocation() ) {
                return updateAttributeCache(data);
            }
            impl::DataLoc *dl = &m_activeAttribMap.get(data->name());
            GLint location = dl->location();
            if( 0 <= location ) {
                data->setLocation(location);
            } else {
                if (!m_shaderProgram) throw RenderException("No program is attached", E_FILE_LINE);
                if (!m_shaderProgram->linked()) throw RenderException("Program is not linked", E_FILE_LINE);
                if ( data->resolveLocation(gl, m_shaderProgram->program()) ) {
                    dl = &updateDataLoc(*dl, data, true);
                    if (debug()) {
                        jau_INFO_PRINT("ShaderState: resolveLocation(2): %s, loc: %d", string_t(data->name()).c_str(), data->location());
                    }
                } else {
                    dl = &updateDataLoc(*dl, data, forceMap);
                    if(verbose()) {
                        jau_INFO_PRINT("ShaderState: resolveLocation(2) failed, no location for: %s", string_t(data->name()).c_str());
                    }
                }
            }
            return *dl;
        }

      public:
        //
        // Enabled Vertex Arrays and its data
        //

        /**
         * @return true if the named attribute is enable
         */
        bool isAttributeEnabled(const stringview_t name) const {
            return m_activeAttribMap.get(name).enabled();
        }

        /**
         * @return true if the {@link GLArrayData} attribute is enable
         */
        bool isAttributeEnabled(const GLArrayDataSRef& data) const {
            return isAttributeEnabled(data->name());
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
         * @throws RenderException if the program is not linked and no location was cached.
         *
         * @see glEnableVertexAttribArray
         * @see glDisableVertexAttribArray
         * @see glVertexAttribPointer
         * @see getVertexAttribPointer
         */
        bool enableAttribute(const GL& gl, const stringview_t name) {
            impl::DataLoc &dl = resolveLocation2(gl, name, true);
            if( dl.valid() ) {
                dl.setEnabled(true);
            }
            const GLint location = dl.location();
            if( 0 > location ) {
                if(verbose()) {
                    jau_INFO_PRINT("ShaderState: enableAttribute(1) failed, no data for: %s", string_t(name).c_str());
                }
                return false;
            }
            if (debug()) {
                jau_INFO_PRINT("ShaderState: enableAttribute(1): %s, loc: %d", string_t(name).c_str(), location);
            }
            ::glEnableVertexAttribArray(location);
            return true;
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
         * @throws RenderException if the program is not linked and no location was cached.
         *
         * @see glEnableVertexAttribArray
         * @see glDisableVertexAttribArray
         * @see glVertexAttribPointer
         * @see getVertexAttribPointer
         * @see GLArrayDataEditable#enableBuffer(GL, bool)
         */
        bool enableAttribute(const GL& gl, const GLArrayDataSRef& data) {
            impl::DataLoc &dl = resolveLocation2(gl, data, true);
            if( dl.valid() ) {
                dl.setEnabled(true);
            }
            const GLint location = dl.location();
            if( 0 > location ) {
                if(verbose()) {
                    jau_INFO_PRINT("ShaderState: enableAttribute(2) failed, no data for: %s", string_t(data->name()).c_str());
                }
                return false;
            }
            if (debug()) {
                jau_INFO_PRINT("ShaderState: enableAttribute(2): %s, loc: %d", string_t(data->name()).c_str(), location);
            }
            ::glEnableVertexAttribArray(data->location());
            return true;
        }

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
         * @throws RenderException if no program is attached
         * @throws RenderException if the program is not linked and no location was cached.
         *
         * @see glEnableVertexAttribArray
         * @see glDisableVertexAttribArray
         * @see glVertexAttribPointer
         * @see getVertexAttribPointer
         */
        bool disableAttribute(const GL& gl, const string_t& name) {
            impl::DataLoc &dl = resolveLocation2(gl, name, false);
            if( dl.valid() ) {
                dl.setEnabled(false);
            }
            const GLint location = dl.location();
            if( 0 > location ) {
                if(verbose()) {
                    jau_INFO_PRINT("ShaderState: disableAttribute(1) failed, no data for: %s", string_t(name).c_str());
                }
                return false;
            }
            if (debug()) {
                jau_INFO_PRINT("ShaderState: disableAttribute(1): %s, loc: %d", string_t(name).c_str(), location);
            }
            ::glDisableVertexAttribArray(location);
            return true;
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
         * @throws RenderException if no program is attached
         * @throws RenderException if the program is not linked and no location was cached.
         *
         * @see glEnableVertexAttribArray
         * @see glDisableVertexAttribArray
         * @see glVertexAttribPointer
         * @see getVertexAttribPointer
         */
        bool disableAttribute(const GL& gl, const GLArrayDataSRef& data) {
            impl::DataLoc &dl = resolveLocation2(gl, data, false);
            if( dl.valid() ) {
                dl.setEnabled(false);
            }
            const GLint location = dl.location();
            if( 0 > location ) {
                if(verbose()) {
                    jau_INFO_PRINT("ShaderState: disableAttribute(2) failed, no data for: %s", string_t(data->name()).c_str());
                }
                return false;
            }
            if (debug()) {
                jau_INFO_PRINT("ShaderState: disableAttribute(2): %s, loc: %d", string_t(data->name()).c_str(), location);
            }
            ::glDisableVertexAttribArray(data->location());
            return true;
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
         * @throws RenderException if no program is attached
         * @throws RenderException if the program is not linked and no location was cached.
         *
         * @see glEnableVertexAttribArray
         * @see glDisableVertexAttribArray
         * @see glVertexAttribPointer
         * @see getVertexAttribPointer
         */
        bool vertexAttribPointer(const GL& gl, const GLArrayDataSRef& data) {
            resolveLocation2(gl, data, true); // always map
            if(!data->hasLocation()) {
                return false;
            }
            data->glVertexAttribPointer(gl);
            return true;
        }

        /**
         * Releases all mapped vertex attribute data,
         * disables all enabled attributes and loses all indices
         *
         * @see glEnableVertexAttribArray
         * @see glDisableVertexAttribArray
         * @see glVertexAttribPointer
         * @see getVertexAttribPointer
         * @see glReleaseAllVertexAttributes
         * @see glResetAllVertexAttributes
         * @see glResetAllVertexAttributes
         * @see ShaderProgram#glReplaceShader
         */
        void releaseAllAttributes(const GL& gl) {
            for (std::pair<const std::string_view, impl::DataLoc>& n : m_activeAttribMap.map()) {
                impl::DataLoc& dl = n.second;
                const GLArrayDataSRef &data = dl.data();
                if( data ) {
                    if ( resolveLocation(gl, data) ) {
                        ::glDisableVertexAttribArray(data->location());
                        data->setLocation(-1);
                    }
                } else if( dl.location() >= 0 ) {
                    ::glDisableVertexAttribArray(dl.location());
                }
            }
            m_activeAttribMap.clear();
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
         * @see glEnableVertexAttribArray
         * @see glDisableVertexAttribArray
         * @see glVertexAttribPointer
         * @see getVertexAttribPointer
         * @see glReleaseAllVertexAttributes
         * @see glResetAllVertexAttributes
         * @see glResetAllVertexAttributes
         * @see ShaderProgram#glReplaceShader
         */
        void disableAllAttributes(const GL& gl, bool removeFromState) {
            for (std::pair<const std::string_view, impl::DataLoc>& n : m_activeAttribMap.map()) {
                impl::DataLoc& dl = n.second;
                const GLArrayDataSRef &data = dl.data();
                if( data ) {
                    if ( resolveLocation(gl, data) ) {
                        ::glDisableVertexAttribArray(data->location());
                    }
                } else if( dl.location() >= 0 ) {
                    ::glDisableVertexAttribArray(dl.location());
                }
                if( removeFromState ) {
                    dl.setEnabled(false);
                }
            }
        }

      private:
        bool relocateAttribute(const GL &gl, impl::DataLoc &dl) {
            GLArrayDataSRef &data = dl.data();
            if( data->resolveLocation(gl, m_shaderProgram->program()) ) {
                if(debug()) {
                    jau_INFO_PRINT("ShaderState: relocateAttribute: %s, loc: %d", std::string(data->name()).c_str(), data->location());
                }
                if(dl.enabled()) {
                    // enable attrib, VBO and pass location/data
                    ::glEnableVertexAttribArray(data->location());
                }

                if( data->isVBO() ) {
                    ::glBindBuffer(GL_ARRAY_BUFFER, data->vboName());
                    data->glVertexAttribPointer(gl);
                    ::glBindBuffer(GL_ARRAY_BUFFER, 0);
                } else {
                    data->glVertexAttribPointer(gl);
                }
                return true;
            }
            return false;
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
         * @throws RenderException is the program is not linked
         *
         * @see attachShaderProgram(GL, ShaderProgram)
         */
        void resetAllAttributes(const GL& gl) {
            if(!m_shaderProgram->linked()) throw RenderException("Program is not linked", E_FILE_LINE);

            for(GLArrayDataSRef& ad : m_managedAttributes) {
                ad->setLocation(-1);
            }
            for (std::pair<const std::string_view, impl::DataLoc>& n : m_activeAttribMap.map()) {
                impl::DataLoc& dl = n.second;
                GLArrayDataSRef &data = dl.data();
                if( data ) {
                    if( relocateAttribute(gl, dl) ) {
                        dl.setLocation(data->location());
                    } else {
                        dl.setLocation(-1);
                    }
                } else {
                    dl.setLocation(-1);
                }
            }
        }

        void setAttribute(const GL& gl, impl::DataLoc &dl) {
            // get new location ..
            GLArrayDataSRef &data = dl.data();
            const stringview_t name = data->name();
            const GLint loc = data->location();

            if(0<=loc) {
                ::glBindAttribLocation(m_shaderProgram->program(), loc, string_t(name).c_str());

                if(dl.enabled()) {
                    // enable attrib, VBO and pass location/data
                    ::glEnableVertexAttribArray(loc);
                }

                if( data->isVBO() ) {
                    ::glBindBuffer(GL_ARRAY_BUFFER, data->vboName());
                    data->glVertexAttribPointer(gl);
                    ::glBindBuffer(GL_ARRAY_BUFFER, 0);
                } else {
                    data->glVertexAttribPointer(gl);
                }
            }
        }

        /**
         * preserves the attribute location .. (program not linked)
         */
        void setAllAttributes(const GL& gl) {
            for (std::pair<const std::string_view, impl::DataLoc>& n : m_activeAttribMap.map()) {
                setAttribute(gl, n.second);
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
         * The data will not be transfered to the GPU, use send() additionally.
         *
         * @param uniform the {@link GLUniformData} which lifecycle shall be managed
         */
        void manage(GLUniformData& data, bool enable=true) {
            if(enable) {
                m_managedUniforms.push_back(&data);
                m_activeUniformMap.put(data.name(), &data);
            } else {
                m_activeUniformMap.remove(data.name());
                std::erase(m_managedUniforms, &data);
            }
        }

        /// Returns true if given uniform is managed via manage()
        bool isManaged(const GLUniformData& uniform) const {
            return m_managedUniforms.end() != std::find(m_managedUniforms.begin(), m_managedUniforms.end(), &uniform); // NOLINT(modernize-use-ranges)
        }

        /**
         * Activate or de-activate a managed {@link GLUniform}.
         *
         * @param uniform the {@link GLUniformData} which lifecycle shall be managed
         * @param active true to activate uniform, otherwise de-activate it.
         * @return false if !isManaged() or de-activating a non-active uniform, otherwise true.
         *
         * @see manage(GLUniformData, boolean)
         * @see getActiveUniform(String)
         */
        bool setActive(GLUniformData &data, bool active) {
            if( !isManaged(data) ) {
                return false;
            }
            if(active) {
                m_activeUniformMap.put(data.name(), &data);
                return true;
            } else {
                return m_activeUniformMap.remove(data.name());
            }
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
         * @throws RenderException if no program is attached or in use
         * @see ShaderProgram#glReplaceShader
         */
        bool resolveLocation(const GL &gl, GLUniformData &data) {
            if (!data.hasLocation()) {
                if (!m_shaderProgram) throw RenderException("No program is attached", E_FILE_LINE);
                if (!m_shaderProgram->inUse()) throw RenderException("Program is not in use", E_FILE_LINE);
                if (!data.resolveLocation(gl, m_shaderProgram->program())) {
                    if (verbose()) {
                        jau_INFO_PRINT("ShaderState: resolving uniform failed, no location/index for: %s", string_t(data.name()).c_str());
                    }
                    return false;
                }
            }
            m_activeUniformMap.put(data.name(), &data);
            return true;
        }

        /**
         * Sends the uniform data to the GPU if it's location is valid, i.e. >= 0.
         *
         * This method uses the {@link GLUniformData}'s location if valid, i.e. >= 0.
         * If data's location is invalid, it will be retrieved via resolveLocation(GL, GLUniformData),
         * set and cached in this state.
         *
         * @param gl the current GL context
         * @param data the uniform to send
         * @param deactivateUnresolved pass true to de-activate if uniform is unresolved, defaults to false
         * @return false, if the location could not be determined, otherwise true
         * @throws RenderException if no program is attached or in use
         *
         * @see glGetUniformLocation
         * @see glGetUniformLocation
         * @see glUniform
         * @see getUniformLocation
         * @see ShaderProgram#glReplaceShader
         */
        bool send(const GL &gl, GLUniformData &data, bool deactivateUnresolved = false) {
            if (!data.hasLocation() && !resolveLocation(gl, data)) {
                if (deactivateUnresolved) {
                    m_activeUniformMap.remove(data.name());
                }
                return false;
            }
            return data.send(gl);
        }

        /**
         * Send all active uniforms
         * @param gl the current GL context
         * @param data the uniform to send
         * @param deactivateUnresolved pass true to de-activate if uniform is unresolved, defaults to false
         * @return number of successfully send resolved uniforms
         */
        size_t sendAllUniforms(const GL &gl, bool deactivateUnresolved=false) {
            size_t c = 0;
            for (const std::pair<const std::string_view, GLUniformData *> &n : m_activeUniformMap.map()) {
                if( send(gl, *n.second, deactivateUnresolved) ) {
                    ++c;
                }
            }
            return c;
        }

        /// Returns true if given uniform data is active, i.e. previously resolved/send and used in current program.
        bool isActive(const GLUniformData& uniform) const {
            return &uniform == m_activeUniformMap.get(uniform.name());
        }

        /**
         * Get the active uniform data, previously resolved/send and used in current program.
         *
         * @param name uniform name
         * @return the GLUniformData object, nullptr if not previously resolved.
         */
        GLUniformData* getActiveUniform(const stringview_t name) {
            return m_activeUniformMap.get(name);
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
         * @throws RenderException is the program is not in use
         *
         * @see attachShaderProgram(GL, ShaderProgram)
         */
        bool resetAllUniforms(const GL& gl) noexcept {
            if (!m_shaderProgram->inUse()) {
                if (verbose()) {
                    jau_INFO_PRINT("ShaderState: program not in use: %s", m_shaderProgram->toString().c_str());
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
                        jau_INFO_PRINT("ShaderState: resetAllUniforms: %s", string_t(data->name()).c_str());
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
            sb.append("\n ],").append(" activeAttributes [");
            for (const std::pair<const std::string_view, impl::DataLoc>& n : m_activeAttribMap.map()) {
                const impl::DataLoc& dl = n.second;
                sb.append("\n  ").append(n.first)
                  .append(": enabled ").append(jau::to_string(dl.enabled())).append(", ");
                if( dl.data() ) {
                    sb.append(dl.data()->toString());
                } else {
                    sb.append("location ").append(jau::to_string(dl.location()));
                }
            }
            sb.append("\n ],").append(" managedAttributes [");
            for(const GLArrayDataSRef& ad : m_managedAttributes) {
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
        bool              m_verbose            = false;
        ShaderProgramSRef m_shaderProgram      = nullptr;
        bool              m_resetAllShaderData = false;

        typedef std::reference_wrapper<GLUniformData> GLUniformDataRef;

        jau::StringViewHashMapWrap<impl::DataLoc, impl::DataLocNone, impl::DataLocNone{}> m_activeAttribMap;
        std::vector<GLArrayDataSRef>             m_managedAttributes;

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
