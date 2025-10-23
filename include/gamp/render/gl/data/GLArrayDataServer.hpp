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

#ifndef GAMP_GLARRAYDATASERVER_HPP_
#define GAMP_GLARRAYDATASERVER_HPP_

#include <cmath>

#include <jau/basic_types.hpp>
#include <jau/float_types.hpp>
#include <jau/string_util.hpp>

#include <gamp/render/gl/data/GLArrayData.hpp>
#include <gamp/render/gl/data/GLArrayDataClient.hpp>
#include <gamp/render/gl/data/GLBuffers.hpp>
#include <gamp/render/gl/data/GLMappedBuffer.hpp>
#include <gamp/render/gl/data/impl/GLDataArrayHandler.hpp>
#include <gamp/render/gl/data/impl/GLSLArrayHandler.hpp>
#include <gamp/render/gl/data/impl/GLSLArrayHandlerInterleaved.hpp>
#include <gamp/render/gl/data/impl/GLSLSubArrayHandler.hpp>
#include <gamp/render/gl/GLLiterals.hpp>
#include <gamp/render/gl/glsl/ShaderState.hpp>
#include <memory>

namespace gamp::render::gl::data {
    using namespace gamp::render::gl;

    /** \addtogroup Gamp_GLData
     *
     *  @{
     */
    template <typename Value_type>
    class GLArrayDataServer;

    template <typename Value_type>
    using GLArrayDataServerRef = std::shared_ptr<GLArrayDataServer<Value_type>>;

    typedef GLArrayDataServer<float>      GLFloatArrayDataServer;
    typedef GLArrayDataServerRef<float>   GLFloatArrayDataServerRef;
    typedef GLArrayDataServer<uint32_t>    GLUIntArrayDataServer;
    typedef GLArrayDataServerRef<uint32_t> GLUIntArrayDataServerRef;

    /**
     * Server data buffer for VBO GLArrayData usage of given template-type Value_type.
     *
     * The GL data type is determined via template type glType<Value_type>().
     */
    template <typename Value_type>
    class GLArrayDataServer : public GLArrayDataClient<Value_type> {
      public:
        typedef Value_type                     value_type;
        typedef GLArrayDataClient<value_type>  client_t;
        typedef GLArrayDataProxy<value_type>   proxy_t;
        typedef std::shared_ptr<proxy_t>       proxy_ref;
        using typename proxy_t::buffer_ref;
        using typename proxy_t::buffer_t;
        using client_t::m_buffer;

        typedef GLArrayDataServer<value_type> server_t;
        typedef std::shared_ptr<server_t>     server_ref;

        //
        // lifetime matters
        //

        /**
         * Create a VBO, using a custom GLSL array attribute name
         * and starting with a new created Buffer object with initialElementCount size
         *
         * The GL data type is determined via template type glType<Value_type>().
         * @param name  The custom name for the GL attribute
         * @param compsPerElement component count per element
         * @param normalized Whether the data shall be normalized
         * @param initialElementCount
         * @param vboUsage {@link GL2ES2#GL_STREAM_DRAW}, {@link GL#GL_STATIC_DRAW} or {@link GL#GL_DYNAMIC_DRAW}
         */
        static server_ref createGLSL(std::string_view name, GLsizei compsPerElement,
                                     bool normalized, GLsizei initialElementCount, GLenum vboUsage) {
            server_ref r = std::make_shared<GLArrayDataServer>(Private(),
               name, compsPerElement,
               normalized, /*stride=*/0, initialElementCount, client_t::DEFAULT_GROWTH_FACTOR,
               /*isVertexAttribute=*/true, std::move(std::make_unique<impl::GLSLArrayHandler<value_type>>()),
               /*vboName=*/0, /*vboOffset=*/0, vboUsage, GL_ARRAY_BUFFER);
            r->m_glArrayHandler->set(r);
            return r;
        }

        /**
         * Create a VBO, using a custom GLSL array attribute name
         * and starting with a given Buffer object incl it's stride
         *
         * The GL data type is determined via template type glType<Value_type>().
         * @param name  The custom name for the GL attribute
         * @param compsPerElement component count per element
         * @param normalized Whether the data shall be normalized
         * @param stride in bytes from one element to the other. If zero, compsPerElement * compSizeInBytes
         * @param buffer the user define data, taking ownership
         * @param vboUsage {@link GL2ES2#GL_STREAM_DRAW}, {@link GL#GL_STATIC_DRAW} or {@link GL#GL_DYNAMIC_DRAW}
         */
        static server_ref createGLSL(std::string_view name, GLsizei compsPerElement,
                                     bool normalized, GLsizei stride, buffer_t&& buffer, GLenum vboUsage) {
            server_ref r = std::make_shared<GLArrayDataServer>(Private(),
               name, compsPerElement,
               normalized, stride, std::move(buffer), client_t::DEFAULT_GROWTH_FACTOR,
               /*isVertexAttribute=*/true, std::move(std::make_unique<impl::GLSLArrayHandler<value_type>>()),
               /*vboName=*/0, /*vboOffset=*/0, vboUsage, GL_ARRAY_BUFFER);
            r->m_glArrayHandler->set(r);
            return r;
        }

        /**
         * Create a VBO, using a custom GLSL array attribute name
         * intended for GPU buffer storage mapping, see {@link GLMappedBuffer}, via {@link #mapStorage(GL, int)} and {@link #mapStorage(GL, long, long, int)}.
         *
         * The GL data type is determined via template type glType<Value_type>().
         * @param name  The custom name for the GL attribute
         * @param compsPerElement component count per element
         * @param normalized Whether the data shall be normalized
         * @param mappedElementCount
         * @param vboUsage {@link GL2ES2#GL_STREAM_DRAW}, {@link GL#GL_STATIC_DRAW} or {@link GL#GL_DYNAMIC_DRAW}
         */
        static server_ref createGLSLMapped(std::string_view name, GLsizei compsPerElement,
                                           bool normalized, GLsizei mappedElementCount, GLenum vboUsage) {
            server_ref r = std::make_shared<GLArrayDataServer>(Private(),
               name, compsPerElement,
               normalized, /*stride=*/0, mappedElementCount,
               /*isVertexAttribute=*/true, std::move(std::make_unique<impl::GLSLArrayHandler<value_type>>()),
               /*vboName=*/0, /*vboOffset=*/0, vboUsage, GL_ARRAY_BUFFER);
            r->m_glArrayHandler->set(r);
            return r;
        }

        /**
         * Create a VBO data object for any target w/o render pipeline association, ie {@link GL#GL_ELEMENT_ARRAY_BUFFER}.
         *
         * Hence no index, name for a fixed function pipeline nor vertex attribute is given.
         *
         * The GL data type is determined via template type glType<Value_type>().
         * @param compsPerElement component count per element
         * @param initialElementCount
         * @param vboUsage {@link GL2ES2#GL_STREAM_DRAW}, {@link GL#GL_STATIC_DRAW} or {@link GL#GL_DYNAMIC_DRAW}
         * @param vboTarget {@link GL#GL_ELEMENT_ARRAY_BUFFER}, ..
         */
        static server_ref createData(GLsizei compsPerElement,
                                     GLsizei initialElementCount, GLenum vboUsage, GLenum vboTarget) {
            server_ref r = std::make_shared<GLArrayDataServer>(Private(),
               "data", compsPerElement,
               /*normalized=*/false, /*stride=*/0, initialElementCount, client_t::DEFAULT_GROWTH_FACTOR,
               /*isVertexAttribute=*/false, std::move(std::make_unique<impl::GLDataArrayHandler<value_type>>()),
               /*vboName=*/0, /*vboOffset=*/0, vboUsage, vboTarget);
            r->m_glArrayHandler->set(r);
            return r;
        }

        /**
         * Create a VBO data object for any target w/o render pipeline association, ie {@link GL#GL_ELEMENT_ARRAY_BUFFER}.
         *
         * Hence no index, name for a fixed function pipeline nor vertex attribute is given.
         *
         * The GL data type is determined via template type glType<Value_type>().
         * @param compsPerElement component count per element
         * @param stride in bytes from one element to the other. If zero, compsPerElement * compSizeInBytes
         * @param buffer the user define data, taking ownership
         * @param vboUsage {@link GL2ES2#GL_STREAM_DRAW}, {@link GL#GL_STATIC_DRAW} or {@link GL#GL_DYNAMIC_DRAW}
         * @param vboTarget {@link GL#GL_ELEMENT_ARRAY_BUFFER}, ..
         * {@link GL#glGenBuffers(int, int[], int)
         */
        static server_ref createData(GLsizei compsPerElement,
                                     GLsizei stride, buffer_t&& buffer, GLenum vboUsage, GLenum vboTarget) {
            server_ref r = std::make_shared<GLArrayDataServer>(Private(),
               "data", compsPerElement,
               /*normalized=*/false, stride, std::move(buffer), client_t::DEFAULT_GROWTH_FACTOR,
               /*isVertexAttribute=*/false, std::move(std::make_unique<impl::GLDataArrayHandler<value_type>>()),
               /*vboName=*/0, /*vboOffset=*/0, vboUsage, vboTarget);
            r->m_glArrayHandler->set(r);
            return r;
        }

        /**
         * Create a VBO data object for any target w/o render pipeline association, i.e. {@link GL#GL_ELEMENT_ARRAY_BUFFER},
         * intended for GPU buffer storage mapping, see {@link GLMappedBuffer}, via {@link #mapStorage(GL, int)} and {@link #mapStorage(GL, long, long, int)}.
         *
         * No index, name for a fixed function pipeline nor vertex attribute is given.
         *
         * The GL data type is determined via template type glType<Value_type>().
         * @param compsPerElement component count per element
         * @param mappedElementCount
         * @param vboUsage {@link GL2ES2#GL_STREAM_DRAW}, {@link GL#GL_STATIC_DRAW} or {@link GL#GL_DYNAMIC_DRAW}
         * @param vboTarget {@link GL#GL_ELEMENT_ARRAY_BUFFER}, ..
         */
        static server_ref createDataMapped(GLsizei compsPerElement,
                                           GLsizei mappedElementCount, GLenum vboUsage, GLenum vboTarget) {
            server_ref r = std::make_shared<GLArrayDataServer>(Private(),
               "mdata", compsPerElement,
               /*normalized=*/false, /*stride=*/0, mappedElementCount,
               /*isVertexAttribute=*/false, std::move(std::make_unique<impl::GLDataArrayHandler<value_type>>()),
               /*vboName=*/0, /*vboOffset=*/0, vboUsage, vboTarget);
            r->m_glArrayHandler->set(r);
            return r;
        }

        /**
         * Create a VBO for GLSL interleaved array data
         * starting with a new created Buffer object with initialElementCount size.
         *
         * User needs to <i>configure</i> the interleaved segments via {@link #addGLSLSubArray(int, int, int)}.
         *
         * The GL data type is determined via template type glType<Value_type>().
         * @param compsPerElement The total number of all interleaved components per element.
         * @param normalized Whether the data shall be normalized
         * @param initialElementCount The initial number of all interleaved elements
         * @param vboUsage {@link GL2ES2#GL_STREAM_DRAW}, {@link GL#GL_STATIC_DRAW} or {@link GL#GL_DYNAMIC_DRAW}
         */
        static server_ref createGLSLInterleaved(GLsizei compsPerElement,
                                                bool normalized, GLsizei initialElementCount, GLenum vboUsage) {
            server_ref r = std::make_shared<GLArrayDataServer>(Private(),
               mgl_InterleaveArray, compsPerElement,
               normalized, /*stride=*/0, initialElementCount, client_t::DEFAULT_GROWTH_FACTOR,
               /*isVertexAttribute=*/false, std::move(std::make_unique<impl::GLSLArrayHandlerInterleaved<value_type>>()),
               /*vboName=*/0, /*vboOffset=*/0, vboUsage, GL_ARRAY_BUFFER);
            r->m_glArrayHandler->set(r);
            return r;
        }

        /**
         * Create a VBO for GLSL interleaved array data
         * starting with a given Buffer object incl it's stride
         * <p>User needs to <i>configure</i> the interleaved segments via {@link #addGLSLSubArray(int, int, int)}.</p>
         *
         * The GL data type is determined via template type glType<Value_type>().
         * @param compsPerElement The total number of all interleaved components per element.
         * @param normalized Whether the data shall be normalized
         * @param stride in bytes from one element of a sub-array to the other. If zero, compsPerElement * compSizeInBytes
         * @param buffer The user define data of all interleaved elements, taking ownership
         * @param vboUsage {@link GL2ES2#GL_STREAM_DRAW}, {@link GL#GL_STATIC_DRAW} or {@link GL#GL_DYNAMIC_DRAW}
         */
        static server_ref createGLSLInterleaved(GLsizei compsPerElement,
                                                bool normalized, GLsizei stride, buffer_t&& buffer, GLenum vboUsage) {
            server_ref r = std::make_shared<GLArrayDataServer>(Private(),
               mgl_InterleaveArray, compsPerElement,
               normalized, stride, std::move(buffer), client_t::DEFAULT_GROWTH_FACTOR,
               /*isVertexAttribute=*/true, std::move(std::make_unique<impl::GLSLArrayHandlerInterleaved<value_type>>()),
               /*vboName=*/0, /*vboOffset=*/0, vboUsage, GL_ARRAY_BUFFER);
            r->m_glArrayHandler->set(r);
            return r;
        }

        /**
         * Create a VBO for GLSL interleaved array data
         * intended for GPU buffer storage mapping, see {@link GLMappedBuffer}, via {@link #mapStorage(GL, int)} and {@link #mapStorage(GL, long, long, int)}.
         *
         * User needs to <i>configure</i> the interleaved segments via {@link #addGLSLSubArray(int, int, int)}.
         *
         * The GL data type is determined via template type glType<Value_type>().
         * @param compsPerElement The total number of all interleaved components per element.
         * @param normalized Whether the data shall be normalized
         * @param mappedElementCount The total number of all interleaved elements
         * @param vboUsage {@link GL2ES2#GL_STREAM_DRAW}, {@link GL#GL_STATIC_DRAW} or {@link GL#GL_DYNAMIC_DRAW}
         */
        static server_ref createGLSLInterleavedMapped(GLsizei compsPerElement,
                                                      bool normalized, GLsizei mappedElementCount, GLenum vboUsage) {
            server_ref r = std::make_shared<GLArrayDataServer>(Private(),
               mgl_InterleaveArray, compsPerElement,
               normalized, /*stride=*/0, mappedElementCount,
               /*isVertexAttribute=*/false, std::move(std::make_unique<impl::GLSLArrayHandlerInterleaved<value_type>>()),
               /*vboName=*/0, /*vboOffset=*/0, vboUsage, GL_ARRAY_BUFFER);
            r->m_glArrayHandler->set(r);
            return r;
        }

        std::string_view className() const noexcept override { return "GLArrayDataServer"; }

        const jau::type_info& classSignature() const noexcept override {
            return jau::static_ctti<proxy_t>();
        }

        const GLArrayDataServerRef<value_type> shared() { return GLArrayData::shared_from_base<GLArrayDataServer>(); }

        /**
         * Configure a segment of this GLSL interleaved array (see {@link #createGLSLInterleaved(int, int, boolean, int, int)}).
         *
         * This method may be called several times as long the sum of interleaved components does not
         * exceed the total component count of the created interleaved array.
         *
         * The memory of this interleaved array is being used (shared)
         *
         * Must be called before using the array, eg: {@link #seal(boolean)}, {@link #putf(float)}, ..
         * @param name  The custom name for the GL attribute, maybe null if vboTarget is {@link GL#GL_ELEMENT_ARRAY_BUFFER}
         * @param compsPerElement This interleaved array segment's component count per element
         * @param vboTarget {@link GL#GL_ARRAY_BUFFER} or {@link GL#GL_ELEMENT_ARRAY_BUFFER}
         */
        proxy_ref addGLSLSubArray(std::string_view name, GLsizei compsPerElement, GLenum vboTarget) {
            if( m_interleavedOffset >= uintptr_t(client_t::compsPerElem()) * uintptr_t(client_t::bytesPerComp()) ) {
                const GLsizei iOffC = m_interleavedOffset / client_t::bytesPerComp();
                throw RenderException("Interleaved offset > total components (" + std::to_string(iOffC) + " > " + std::to_string(client_t::compsPerElem()) + ")", E_FILE_LINE);
            }
            const long  subStrideB = (0 == client_t::stride()) ? client_t::compsPerElem() * client_t::bytesPerComp() : client_t::stride();
            proxy_ref ad;
            if( 0 < client_t::m_mappedElemCount ) {
                ad = proxy_t::createGLSL(name, compsPerElement,
                                         client_t::normalized(), subStrideB, client_t::m_mappedElemCount,
                                         client_t::vboName(), m_interleavedOffset, client_t::vboUsage(), vboTarget);
            } else {
                ad = proxy_t::createGLSL(name, compsPerElement,
                                         client_t::normalized(), subStrideB, client_t::m_buffer,
                                         client_t::vboName(), m_interleavedOffset, client_t::vboUsage(), vboTarget);
            }
            ad->setVBOEnabled(client_t::isVBO());
            m_interleavedOffset += compsPerElement * client_t::bytesPerComp();
            if( GL_ARRAY_BUFFER == vboTarget ) {
                GLArrayDataRef ad0 = std::static_pointer_cast<GLArrayData>(ad);
                client_t::m_glArrayHandler->addSubHandler( std::move( impl::GLSLSubArrayHandler( ad0 ) ) );
            }
            return ad;
        }

        void      setInterleavedOffset(uintptr_t interleavedOffset) noexcept { m_interleavedOffset = interleavedOffset; }
        uintptr_t interleavedOffset() const noexcept { return interleavedOffset; }

        //
        // Data matters GLArrayData
        //

        //
        // Data and GL state modification ..
        //

        void destroy(GL& gl) override {
            // super.destroy(gl):
            // - GLArrayDataClient.destroy(gl): disables & clears client-side buffer
            //   - GLArrayDataWrapper.destroy(gl) (clears all values 'vboName' ..)
            GLuint vboName = client_t::vboName();
            client_t::destroy(gl);
            if( vboName != 0 ) {
                ::glDeleteBuffers(1, &vboName);
                client_t::m_vboName = 0;
            }
        }

        //
        // data matters
        //

        /**
         * Convenient way do disable the VBO behavior and
         * switch to client side data one
         * Only possible if buffer is defined.
         */
        void setVBOEnabled(bool vboEnabled) override {
            client_t::checkSeal(false);
            client_t::setVBOEnabled(vboEnabled);
        }

#if 0
        GLMappedBuffer mapStorage(const GL& gl, final int access) {
            if( proxy_t::usesClientMem() ) {
                throw jau::IllegalStateError("user buffer not null", E_FILE_LINE);
            }
            if( m_mappedStorage ) {
                throw IllegalStateException("already mapped: " + m_mappedStorage);
            }
            proxy_t::checkSeal(true);
            proxy_t::bindBuffer(gl, true);
            ::glBufferData(getVBOTarget(), getByteCount(), null, getVBOUsage());
            GLMappedBuffer storage = gl.mapBuffer(getVBOTarget(), access);
            setMappedBuffer(storage);
            proxy_t::bindBuffer(gl, false);
            proxy_t::seal(false);
            proxy_t::rewind();
            return storage;
        }
        GLMappedBuffer mapStorage(const GL& gl, final long offset, final long length, final int access) {
            if( proxyproxy_t::usesClientBuffer() ) {
                throw IllegalStateException("user buffer not null");
            }
            if( null != m_mappedStorage ) {
                throw IllegalStateException("already mapped: " + m_mappedStorage);
            }
            checkSeal(true);
            bindBuffer(gl, true);
            ::glBufferData(getVBOTarget(), getByteCount(), null, getVBOUsage());
            GLMappedBuffer storage = gl.mapBufferRange(getVBOTarget(), offset, length, access);
            setMappedBuffer(storage);
            bindBuffer(gl, false);
            seal(false);
            rewind();
            return storage;
        }

      private:
        void setMappedBuffer(const GLMappedBuffer& storage) {
            m_mappedStorage = storage;
        TODO:
            m_buffer = storage.getMappedBuffer();
        }

        void unmapStorage(const GL& gl) {
            if( null == m_mappedStorage ) {
                throw IllegalStateException("not mapped");
            }
            m_mappedStorage = null;
            buffer          = null;
            seal(true);
            bindBuffer(gl, true);
            gl.glUnmapBuffer(getVBOTarget());
            bindBuffer(gl, false);
        }

      public:
#endif

        std::string toString() const noexcept override { return toString(false); }
        std::string toString(bool withData) const noexcept override {
            std::string r(className());
            r.append("[").append(proxy_t::m_name)
            .append(", location ")
            .append(std::to_string(proxy_t::m_location))
            .append(", isVertexAttribute ")
            .append(std::to_string(proxy_t::m_isVertexAttr))
            .append(", usesShaderState ")
            .append(std::to_string(nullptr != client_t::m_shaderState))
            .append(", dataType ")
            .append(jau::toHexString(proxy_t::m_compType))
            .append(", compsPerElem ")
            .append(std::to_string(proxy_t::compsPerElem()))
            .append(", stride ")
            .append(std::to_string(proxy_t::m_strideB))
            .append("b ")
            .append(std::to_string(proxy_t::m_strideL))
            .append("c")
            .append(", mappedElements ")
            .append(std::to_string(proxy_t::m_mappedElemCount))
            .append(", ")
            .append(proxy_t::elemStatsToString())
            .append(", mappedStorage ")
            .append(jau::toHexString(m_mappedStorage ? m_mappedStorage.get() : nullptr))
            .append(", vboEnabled ")
            .append(std::to_string(client_t::m_vboEnabled))
            .append(", vboName ")
            .append(std::to_string(client_t::m_vboName))
            .append(", vboUsage ")
            .append(jau::toHexString(client_t::m_vboUsage))
            .append(", vboTarget ")
            .append(jau::toHexString(client_t::m_vboTarget))
            .append(", vboOffset ")
            .append(std::to_string(client_t::m_vboOffset))
            .append(", enabled ")
            .append(std::to_string(client_t::m_bufferEnabled))
            .append(", written ")
            .append(std::to_string(client_t::m_bufferWritten));
            if( withData ) {
                r.append(", buffer ").append(proxy_t::usesClientMem() ? m_buffer.toString() : "nil");
            }
            r.append(", alive ")
            .append(std::to_string(proxy_t::m_alive))
            .append("]");
            return r;
        }

        //
        // non public matters ..
        //

      protected:
        struct Private {
            explicit Private() = default;
        };

      public:
        /** Private client-mem ctor w/ passing custom buffer */
        GLArrayDataServer(Private, std::string_view name, GLsizei componentsPerElement,
                          bool normalized, GLsizei stride, buffer_t&& data, float growthFactor,
                          bool isVertexAttribute, impl::GLArrayHandlerPtr<value_type>&& glArrayHandler,
                          GLuint vboName, uintptr_t vboOffset, GLenum vboUsage, GLenum vboTarget)
        : client_t(typename client_t::Private(),
                   name, componentsPerElement,
                   normalized, stride, std::move(data), growthFactor,
                   isVertexAttribute, std::move(glArrayHandler),
                   vboName, vboOffset, vboUsage, vboTarget) {
            proxy_t::m_vboEnabled = true;
        }

        /** Private client-mem ctor w/o passing custom buffer */
        GLArrayDataServer(Private, std::string_view name, GLsizei componentsPerElement,
                          bool normalized, GLsizei stride, GLsizei initialElementCount, float growthFactor,
                          bool isVertexAttribute, impl::GLArrayHandlerPtr<value_type>&& glArrayHandler,
                          GLuint vboName, uintptr_t vboOffset, GLenum vboUsage, GLenum vboTarget)
        : client_t(typename client_t::Private(),
                   name, componentsPerElement,
                   normalized, stride, initialElementCount, growthFactor,
                   isVertexAttribute, std::move(glArrayHandler),
                   vboName, vboOffset, vboUsage, vboTarget) {
            proxy_t::m_vboEnabled = true;
        }

        /// using memory mapped elements
        GLArrayDataServer(Private, std::string_view name, GLsizei componentsPerElement,
                          bool normalized, GLsizei stride, GLsizei mappedElementCount,
                          bool isVertexAttribute, impl::GLArrayHandlerPtr<value_type>&& glArrayHandler,
                          GLuint vboName, uintptr_t vboOffset, GLenum vboUsage, GLenum vboTarget)
        : client_t(typename client_t::Private(),
                   name, componentsPerElement,
                   normalized, stride, mappedElementCount,
                   isVertexAttribute, std::move(glArrayHandler),
                   vboName, vboOffset, vboUsage, vboTarget) {
            proxy_t::m_vboEnabled = true;
        }

      protected:
        void init_vbo(const GL& gl) override {
            client_t::init_vbo(gl);
            if( proxy_t::isVBO() && proxy_t::m_vboName == 0 ) {
                GLuint tmp = 0;
                ::glGenBuffers(1, &tmp);
                proxy_t::m_vboName = tmp;
                if( 0 < m_interleavedOffset ) {
                    client_t::m_glArrayHandler->setSubArrayVBOName(proxy_t::m_vboName);
                }
            }
        }

        uintptr_t         m_interleavedOffset = 0;
        GLMappedBufferPtr m_mappedStorage     = nullptr;
    };

    /**@}*/
}  // namespace gamp::render::gl::data

#endif /* GAMP_GLARRAYDATACLIENT_HPP_ */
