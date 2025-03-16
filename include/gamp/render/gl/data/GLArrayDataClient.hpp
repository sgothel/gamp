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

#ifndef GAMP_GLARRAYDATACLIENT_HPP_
#define GAMP_GLARRAYDATACLIENT_HPP_

#include <cmath>
#include <memory>

#include <jau/basic_types.hpp>
#include <jau/float_types.hpp>
#include <jau/int_types.hpp>
#include <jau/type_info.hpp>

#include <gamp/render/gl/data/GLArrayDataProxy.hpp>
#include <gamp/render/gl/data/GLBuffers.hpp>
#include <gamp/render/gl/data/impl/GLSLArrayHandler.hpp>
#include <gamp/render/gl/glsl/ShaderState.hpp>

namespace gamp::render::gl::data {
    using namespace gamp::render::gl;

    /** \addtogroup Gamp_GLData
     *
     *  @{
     */

    template<typename Value_type>
    class GLArrayDataClient;

    template<typename Value_type>
    using GLArrayDataClientRef = std::shared_ptr<GLArrayDataClient<Value_type>>;

    typedef GLArrayDataClient<float> GLFloatArrayDataClient;
    typedef GLArrayDataClientRef<float> GLFloatArrayDataClientRef;

    /**
     * Client data buffer for non VBO GLArrayData usage of given template-type Value_type.
     *
     * The GL data type is determined via template type glType<Value_type>().
     */
    template<typename Value_type>
    class GLArrayDataClient : public GLArrayDataProxy<Value_type> {
      public:
        typedef Value_type value_type;
        typedef GLArrayDataProxy<value_type> proxy_t;
        using typename proxy_t::buffer_t;
        using typename proxy_t::buffer_ref;

        typedef GLArrayDataClient<value_type> client_t;
        typedef std::shared_ptr<client_t> client_ref;

        /** Default growth factor using the golden ratio 1.618 */
        inline static constexpr float DEFAULT_GROWTH_FACTOR = std::numbers::phi_v<float>;

        /**
         * Create a client side buffer object, using a custom GLSL array attribute name
         * and starting with a new created Buffer object with initialElementCount size
         *
         * The GL data type is determined via template type glType<Value_type>().
         * @param name  The custom name for the GL attribute.
         * @param compsPerElement The array component number
         * @param normalized Whether the data shall be normalized
         * @param initialElementCount
         */
        static client_ref createGLSL(const std::string& name, GLsizei compsPerElement,
                                     bool normalized, size_t initialElementCount)
        {
            client_ref r = std::make_shared<GLArrayDataClient>(Private(),
                name, compsPerElement,
                normalized, /*stride=*/0, initialElementCount, DEFAULT_GROWTH_FACTOR,
                /*isVertexAttribute=*/true, std::move(std::make_unique<impl::GLSLArrayHandler<value_type>>()),
                /*vboName=*/0, /*vboOffset=*/0, /*vboUsage=*/0, /*vboTarget=*/0);
            r->m_glArrayHandler->set(r);
            return r;
        }

        /**
         * Create a client side buffer object, using a custom GLSL array attribute name
         * and starting with a given Buffer object incl it's stride.
         *
         * The GL data type is determined via template type glType<Value_type>().
         * @param name  The custom name for the GL attribute.
         * @param compsPerElement The array component number
         * @param normalized Whether the data shall be normalized
         * @param stride
         * @param buffer the user define data, taking ownership
         */
        static client_ref createGLSL(const std::string& name, GLsizei compsPerElement,
                                     bool normalized, GLsizei stride, buffer_t&& buffer)
        {
            client_ref r = std::make_shared<GLArrayDataClient>(Private(),
                name, compsPerElement,
                normalized, stride, std::move(buffer), DEFAULT_GROWTH_FACTOR, 0 /* mappedElementCount */,
                /*isVertexAttribute=*/true, std::move(std::make_unique<impl::GLSLArrayHandler<value_type>>()),
                /*vboName=*/0, /*vboOffset=*/0, /*vboUsage=*/0, /*vboTarget=*/0);
            r->m_glArrayHandler->set(r);
            return r;
        }

        std::string_view className() const noexcept override { return "GLArrayDataClient"; }

        const jau::type_info& classSignature() const noexcept override {
            return jau::static_ctti<client_t>();
        }

        const GLArrayDataClientRef<value_type> shared() { return GLArrayData::shared_from_base<GLArrayDataClient>(); }

        void associate(ShaderState& st, bool enable) override {
            if( enable ) {
                m_shaderState = &st;
            } else {
                m_shaderState = nullptr;
            }
        }

        //
        // Data read access
        //

        constexpr bool enabled() const noexcept { return m_bufferEnabled; }

        /** Is the buffer written to the VBO ? */
        constexpr bool isVBOWritten() const noexcept { return m_bufferWritten; }

        //
        // Data and GL state modification ..
        //

        /** Marks the buffer written to the VBO */
        void setVBOWritten(bool written) noexcept {
            m_bufferWritten = (0 == GLArrayData::m_mappedElemCount) ? written : true;
        }

        void destroy(GL& gl) override {
            clear(gl);
            proxy_t::destroy(gl);
        }

        /**
         * Clears this buffer and resets states accordingly.
         * <p>
         * Implementation calls {@link #seal(GL&, bool) seal(gl, false)} and {@link #clear()},
         * i.e. turns-off the GL& buffer and then clearing it.
         * </p>
         * <p>
         * The position is set to zero, the limit is set to the capacity, and the mark is discarded.
         * </p>
         * <p>
         * Invoke this method before using a sequence of get or put operations to fill this buffer.
         * </p>
         * <p>
         * This method does not actually erase the data in the buffer and will most often be used when erasing the underlying memory is suitable.
         * </p>
         * @see #seal(GL&, bool)
         * @see #clear()
         */
        void clear(GL& gl) {
            seal(gl, false);
            clear();
        }

        /**
         * Convenience method calling {@link #seal(bool)} and {@link #enableBuffer(GL&, bool)}.
         *
         * @see #seal(bool)
         * @see #enableBuffer(GL&, bool)
         */
        void seal(GL& gl, bool seal_) {
            seal(seal_);
            enableBuffer(gl, seal_);
        }

        /**
         * Enables the buffer if <code>enable</code> is <code>true</code>,
         * and transfers the data if required.
         *
         * In case {@link #isVBO() VBO is used}, it is bound accordingly for the data transfer and association,
         * i.e. it issued {@link #bindBuffer(GL&, bool)}.
         * The VBO buffer is unbound when the method returns.
         *
         * Disables the buffer if <code>enable</code> is <code>false</code>.
         *
         * The action will only be executed,
         * if the internal enable state differs,
         * or 'setEnableAlways' was called with 'true'.
         *
         * It is up to the user to enable/disable the array properly,
         * ie in case of multiple data sets for the same vertex attribute (VA).
         * Meaning in such case usage of one set while expecting another one
         * to be used for the same VA implies decorating each usage with enable/disable.
         *
         * @see #setEnableAlways(bool)
         */
        void enableBuffer(GL& gl, bool enable) {
            if( m_enableBufferAlways || m_bufferEnabled != enable ) {
                if( enable ) {
                    checkSeal(true);
                    // init/generate VBO name if not done yet
                    init_vbo(gl);
                }
                m_glArrayHandler->enableState(gl, enable, m_shaderState);
                m_bufferEnabled = enable;
            }
        }

        /**
         * if <code>bind</code> is true and the data uses {@link #isVBO() VBO},
         * the latter will be bound and data written to the GPU if required.
         * <p>
         * If  <code>bind</code> is false and the data uses {@link #isVBO() VBO},
         * the latter will be unbound.
         * </p>
         * <p>
         * This method is exposed to allow data VBO arrays, i.e. {@link GL&#GL&_ELEMENT_ARRAY_BUFFER},
         * to be bounded and written while keeping the VBO bound. The latter is in contrast to {@link #enableBuffer(GL&, bool)},
         * which leaves the VBO unbound, since it's not required for vertex attributes or pointers.
         * </p>
         *
         * @param gl current GL& object
         * @param bind true if VBO shall be bound and data written,
         *        otherwise clear VBO binding.
         * @return true if data uses VBO and action was performed, otherwise false
         */
        bool bindBuffer(GL& gl, bool bind) {
            if( bind ) {
                checkSeal(true);
                // init/generate VBO name if not done yet
                init_vbo(gl);
            }
            return m_glArrayHandler->bindBuffer(gl, bind);
        }

        /**
         * Affects the behavior of 'enableBuffer'.
         *
         * The default is 'false'
         *
         * This is useful when you mix up
         * GLArrayData usage with conventional GL array calls
         * or in case of a buggy GL& VBO implementation.
         *
         * @see #enableBuffer(GL&, bool)
         */
        void setEnableAlways(bool always) { m_enableBufferAlways = always; }

        //
        // Data modification ..
        //

        /**
         * Clears this buffer and resets states accordingly.
         * <p>
         * The position is set to zero, the limit is set to the capacity, and the mark is discarded.
         * </p>
         * <p>
         * Invoke this method before using a sequence of get or put operations to fill this buffer.
         * </p>
         * <p>
         * This method does not actually erase the data in the buffer and will most often be used when erasing the underlying memory is suitable.
         * </p>
         * @see #clear(GL&)
         */
        void clear() {
            m_buffer.clear();
            proxy_t::m_sealed      = false;
            m_bufferEnabled = false;
            m_bufferWritten = proxy_t::usesClientMem() ? false : true;
        }

        /**
         * <p>If <i>seal</i> is true, it
         * disables write operations to the buffer.
         * Calls flip, ie limit:=position and position:=0.</p>
         *
         * <p>If <i>seal</i> is false, it
         * enable write operations continuing
         * at the buffer position, where you left off at seal(true),
         * ie position:=limit and limit:=capacity.</p>
         *
         * @see #seal(bool)
         * @see #sealed()
         */
        void seal(bool seal) {
            if( proxy_t::sealed() == seal ) return;
            proxy_t::m_sealed = seal;
            m_bufferWritten = proxy_t::usesClientMem() ? false : true;
            if( seal ) {
                m_buffer.flip();
            } else {
                m_buffer.setPosition(m_buffer.limit());
                m_buffer.setLimit(m_buffer.size());
            }
        }

        /**
         * Rewinds this buffer. The position is set to zero and the mark is discarded.
         * <p>
         * Invoke this method before a sequence of put or get operations.
         * </p>
         */
        void rewind() {
            m_buffer.rewind();
        }

      public:
        template<typename T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, bool> = true>
        constexpr_cxx20 T get() {
            if( sizeof(Value_type) > sizeof(T) ) {
                throw jau::IllegalArgumentError("Buffer `"+toString()+"` incompatible with type `"+jau::static_ctti<T>().name()+"`", E_FILE_LINE);
            }
            return static_cast<T>( m_buffer.get() );
        }

        template<typename... Targs,
            std::enable_if_t< jau::is_all_same_v<Targs...> &&
                              sizeof(value_type) >= sizeof(jau::first_type<Targs...>) &&
                              ( std::is_integral_v<jau::first_type<Targs...>> ||
                                std::is_floating_point_v<jau::first_type<Targs...>> ), bool> = true>
        constexpr_cxx20 client_t& putN(const Targs&...args) {
            if( !proxy_t::sealed() ) {
                growIfNeeded(sizeof...(args));
                m_buffer.putN(jau::Bool::False, args...);
            }
            return *this;
        }

        constexpr_cxx20 client_t& put(const value_type v) {
            if( !proxy_t::sealed() ) {
                growIfNeeded(1);
                m_buffer.put(v);
            }
            return *this;
        }
        constexpr_cxx20 client_t& puti(const int32_t v) {
            return put(static_cast<value_type>(v));
        }
        constexpr_cxx20 client_t& putf(const float v) {
            return put(static_cast<value_type>(v));
        }
        constexpr_cxx20 client_t& put2f(const float x, const float y) {
            return putN(x, y);
        }
        constexpr_cxx20 client_t& put3f(const float x, const float y, const float z) {
            return putN(x, y, z);
        }
        constexpr_cxx20 client_t& put4f(const float x, const float y, const float z, const float w) {
            return putN(x, y, z, w);
        }
        constexpr_cxx20 client_t& put2f(const jau::math::Vec2f& v) {
            return putN(v.x, v.y);
        }
        constexpr_cxx20 client_t& put3f(const jau::math::Vec3f& v) {
            return putN(v.x, v.y, v.z);
        }
        constexpr_cxx20 client_t& put4f(const jau::math::Vec4f& v) {
            return putN(v.x, v.y, v.z, v.w);
        }

        std::string toString() const noexcept override { return toString(false); }
        std::string toString(bool withData) const noexcept override {
            std::string r("GLArrayDataWrapper[");
            r.append(proxy_t::m_name)
             .append(", location ").append(std::to_string(proxy_t::m_location))
             .append(", isVertexAttribute ").append(std::to_string(proxy_t::m_isVertexAttr))
             .append(", usesShaderState ").append(std::to_string(nullptr!=m_shaderState))
             .append(", dataType ").append(jau::to_hexstring(proxy_t::m_compType))
             .append(", compsPerElem ").append(std::to_string(proxy_t::compsPerElem()))
             .append(", stride ").append(std::to_string(proxy_t::m_strideB)).append("b ").append(std::to_string(proxy_t::m_strideL)).append("c")
             .append(", mappedElements ").append(std::to_string(proxy_t::m_mappedElemCount))
             .append(", ").append(proxy_t::elemStatsToString())
             .append(", enabled ").append(std::to_string(m_bufferEnabled))
             .append(", written ").append(std::to_string(m_bufferWritten));
            if( withData ) {
                r.append(", buffer ").append(proxy_t::usesClientMem()?m_buffer.toString():"nil");
            }
            r.append(", alive ").append(std::to_string(proxy_t::m_alive)).append("]");
            return r;
        }

        /**
         * Returning element-count from given componentCount, rounding up to componentsPerElement.
         */
        constexpr GLsizei compsToElemCount(GLsizei componentCount) const noexcept {
            return (componentCount + proxy_t::compsPerElem() - 1) / proxy_t::compsPerElem();
        }

        /**
         * Increase the capacity of the buffer if necessary to add given spareComponents components.
         * <p>
         * Buffer will not change if remaining free slots, capacity less position, satisfy spareComponents components.
         * </p>
         * @param spareComponents number of components to add if necessary.
         * @return true if buffer size has changed, i.e. grown. Otherwise false.
         */
        bool growIfNeeded(glmemsize_t spareComponents) {
            if( m_buffer.size() == 0 || m_buffer.remaining() < spareComponents ) {
                const glmemsize_t has_comps      = m_buffer.size();
                const GLsizei required_elems = compsToElemCount(has_comps + spareComponents);
                const GLsizei grown_elems    = compsToElemCount(glmemsize_t(std::round(has_comps * m_growthFactor)));
                return reserve( std::max(grown_elems, required_elems) );
            }
            return false;
        }

        /**
         * Increase the capacity of the buffer to given elementCount element size,
         * i.e. elementCount * componentsPerElement components.
         *
         * Buffer will not change if given elementCount is lower or equal current capacity.
         *
         * @param elementCount number of elements to hold.
         * @return true if buffer size has changed, i.e. grown. Otherwise false.
         */
        bool reserve(GLsizei elementCount) {
            if( !proxy_t::usesClientMem() || !proxy_t::m_alive || proxy_t::sealed() ) {
                throw RenderException("Invalid state: " + toString(), E_FILE_LINE);
            }
            // add the stride delta
            elementCount += (elementCount / proxy_t::compsPerElem()) * (proxy_t::m_strideL - proxy_t::compsPerElem());

            const glmemsize_t osize = m_buffer.size();
            const glmemsize_t nsize = elementCount * proxy_t::compsPerElem();
            if( nsize <= osize ) {
                return false;
            }
            m_buffer.resize(nsize);
            if( DEBUG_MODE ) {
                jau::PLAIN_PRINT(true, "*** Size: Reserve: comps: %zd, %zd / %zd -> %zd / %zd; %s",
                    proxy_t::compsPerElem(), (osize / proxy_t::compsPerElem()), osize,
                    (nsize / proxy_t::compsPerElem()), nsize, m_buffer.toString().c_str());
            }
            return true;
        }

        /**
         * Returns this buffer's growth factor.
         * <p>
         * Default is {@link #DEFAULT_GROWTH_FACTOR}, i.e. the golden ratio 1.618.
         * </p>
         * @see #setGrowthFactor(float)
         * @see #DEFAULT_GROWTH_FACTOR
         */
        constexpr float growthFactor() const noexcept { return m_growthFactor; }

        /**
         * Sets a new growth factor for this buffer.
         * <p>
         * Default is {@link #DEFAULT_GROWTH_FACTOR}, i.e. the golden ratio 1.618.
         * </p>
         * @param v new growth factor, which will be clipped to a minimum of 1, i.e. 0% minimum growth.
         * @see #getGrowthFactor()
         * @see #DEFAULT_GROWTH_FACTOR
         */
        void setGrowthFactor(float v) noexcept {
            m_growthFactor = std::max(1.0f, v);
        }

        // non public matters

      protected:
        void checkSeal(bool test) {
            if( !proxy_t::m_alive ) {
                throw RenderException("Invalid state: " + toString(), E_FILE_LINE);
            }
            if( proxy_t::sealed() != test ) {
                if( test ) {
                    throw RenderException("Not Sealed yet, seal first:\n\t" + toString(), E_FILE_LINE);
                } else {
                    throw RenderException("Already Sealed, can't modify VBO:\n\t" + toString(), E_FILE_LINE);
                }
            }
        }

        struct Private{ explicit Private() = default; };

        /** Private client-mem ctor w/ passing custom buffer */
        GLArrayDataClient(Private, const std::string& name, GLsizei componentsPerElement,
                          bool normalized, GLsizei stride, buffer_t&& data, float growthFactor,
                          bool isVertexAttribute, impl::GLArrayHandlerPtr<value_type>&& glArrayHandler,
                          GLuint vboName, uintptr_t vboOffset, GLenum vboUsage, GLenum vboTarget)
        : proxy_t(typename proxy_t::Private(),
                  name, componentsPerElement,
                  normalized, stride, m_buffer, glType<value_type>, jau::make_ctti<value_type>(),
                  isVertexAttribute, vboName, vboOffset, vboUsage, vboTarget),
          m_buffer( std::move(data) ),
          m_glArrayHandler(std::move(glArrayHandler)),
          m_growthFactor(growthFactor),
          m_bufferEnabled(false), m_bufferWritten(false), m_enableBufferAlways(false),
          m_shaderState(nullptr), m_isValidated(false)
        {
            proxy_t::m_sealed = false;
        }

        /** Private client-mem ctor w/o passing custom buffer */
        GLArrayDataClient(Private, const std::string& name, GLsizei componentsPerElement,
                          bool normalized, GLsizei stride, GLsizei initialElementCount, float growthFactor,
                          bool isVertexAttribute, impl::GLArrayHandlerPtr<value_type>&& glArrayHandler,
                          GLuint vboName, uintptr_t vboOffset, GLenum vboUsage, GLenum vboTarget)
        : proxy_t(typename proxy_t::Private(),
                  name, componentsPerElement,
                  normalized, stride, m_buffer,
                  isVertexAttribute, vboName, vboOffset, vboUsage, vboTarget),
          m_buffer(nullptr, initialElementCount),
          m_glArrayHandler(std::move(glArrayHandler)),
          m_growthFactor(growthFactor),
          m_bufferEnabled(false), m_bufferWritten(false), m_enableBufferAlways(false),
          m_shaderState(nullptr), m_isValidated(false)
        {
            proxy_t::m_sealed = false;
        }

        /// using memory mapped elements
        GLArrayDataClient(Private,
                          const std::string& name, GLsizei componentsPerElement,
                          bool normalized, GLsizei stride, GLsizei mappedElementCount,
                          bool isVertexAttribute, impl::GLArrayHandlerPtr<value_type>&& glArrayHandler,
                          GLuint vboName, uintptr_t vboOffset, GLenum vboUsage, GLenum vboTarget)
        : proxy_t(typename proxy_t::Private(),
                  name, componentsPerElement,
                  normalized, stride, mappedElementCount,
                  isVertexAttribute, vboName, vboOffset, vboUsage, vboTarget),
          m_buffer(),
          m_glArrayHandler(std::move(glArrayHandler)),
          m_growthFactor(0),
          m_bufferEnabled(false), m_bufferWritten(true), m_enableBufferAlways(false),
          m_shaderState(nullptr), m_isValidated(false)
        {
            proxy_t::m_sealed = false;
        }

        virtual void init_vbo(const GL& gl) {
            if( !m_isValidated ) {
                m_isValidated = true;
                proxy_t::validate(gl);
            }
        }

        buffer_t m_buffer;
        impl::GLArrayHandlerPtr<value_type> m_glArrayHandler;

        float m_growthFactor;
        bool m_bufferEnabled;
        bool m_bufferWritten;
        bool m_enableBufferAlways;
        ShaderState* m_shaderState;

      private:
        bool m_isValidated = false;
        static constexpr bool DEBUG_MODE = false;
    };

    /**@}*/

}  // namespace gamp::render::gl::data

namespace gamp::render::gl::data::impl {
    template<typename Value_type>
    bool GLArrayHandler<Value_type>::bindBuffer(GL& gl, bool bind) {
        if( !m_ad || !m_ad->isVBO() ) {
            return false;
        }
        if( bind ) {
            // always bind and refresh the VBO mgr,
            // in case more than one gl*Pointer objects are in use
            m_ad->glBindBuffer(gl, true);
            if( !m_ad->isVBOWritten() ) {
                if( m_ad->usesClientMem() ) {
                    m_ad->glBufferData(gl);
                }
                m_ad->setVBOWritten(true);
            }
        } else {
            m_ad->glBindBuffer(gl, false);
        }
        return true;
    }
}

#endif /* GAMP_GLARRAYDATACLIENT_HPP_ */

