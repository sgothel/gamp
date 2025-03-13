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

#ifndef GAMP_GLARRAYDATAPROXY_HPP_
#define GAMP_GLARRAYDATAPROXY_HPP_

#include <memory>

#include <jau/darray.hpp>

#include <gamp/render/gl/data/GLArrayData.hpp>
#include <gamp/render/gl/data/GLBuffers.hpp>

namespace gamp::render::gl::data {
    using namespace gamp::render::gl;

    /** \addtogroup Gamp_GLData
     *
     *  @{
     */

    template<typename Value_type>
    class GLArrayDataProxy;

    template<typename Value_type>
    using GLArrayDataProxyRef = std::shared_ptr<GLArrayDataProxy<Value_type>>;

    /**
     * Proxying a data buffer for GLArrayData usage of given template-type Value_type.
     *
     * The GL data type is determined via template type glType<Value_type>().
     */
    template<typename Value_type>
    class GLArrayDataProxy : public GLArrayData {
      public:
        typedef Value_type value_type;
        typedef GLArrayDataProxy<value_type> proxy_t;
        typedef std::shared_ptr<proxy_t> proxy_ref;

        typedef jau::darray<value_type, glmemsize_t> buffer_t;
        typedef std::unique_ptr<buffer_t> buffer_ref;

        ~GLArrayDataProxy() noexcept override = default;

        /**
         * Create a VBO, using a custom GLSL array attribute name, proxying the given data.
         *
         * The GL data type is determined via template type glType<Value_type>().
         *
         * This buffer is always {@link #sealed()}.
         *
         * @param name  The custom name for the GL attribute, maybe null if gpuBufferTarget is {@link GL#GL_ELEMENT_ARRAY_BUFFER}
         * @param componentsPerElement The array component number
         * @param normalized Whether the data shall be normalized
         * @param stride
         * @param buffer the user define data, passed through by reference
         * @param vboName
         * @param vboOffset
         * @param vboUsage GL_STREAM_DRAW, GL_STATIC_DRAW or GL_DYNAMIC_DRAW
         * @param vboTarget GL_ARRAY_BUFFER or GL_ELEMENT_ARRAY_BUFFER
         * @return the new create instance
         * @throws GLException
         */
        static proxy_ref createGLSL(const std::string& name, GLsizei componentsPerElement, bool normalized, GLsizei stride,
                                    buffer_t& buffer, GLuint vboName, uintptr_t vboOffset, GLenum vboUsage, GLenum vboTarget) {
            return std::make_shared<GLArrayDataProxy>(Private(),
                name, componentsPerElement, normalized, stride, buffer,
                /*isVertexAttribute=*/true, vboName, vboOffset, vboUsage, vboTarget);
        }

        /**
         * Create a VBO, using a custom GLSL array attribute name, proxying the mapped data characteristics.
         *
         * The GL data type is determined via template type glType<Value_type>().
         *
         * This buffer is always {@link #sealed()}.
         *
         * @param name  The custom name for the GL attribute, maybe null if gpuBufferTarget is {@link GL#GL_ELEMENT_ARRAY_BUFFER}
         * @param componentsPerElement The array component number
         * @param normalized Whether the data shall be normalized
         * @param stride
         * @param mappedElementCount
         * @param vboName
         * @param vboOffset
         * @param vboUsage GL_STREAM_DRAW, GL_STATIC_DRAW or GL_DYNAMIC_DRAW
         * @param vboTarget GL_ARRAY_BUFFER or GL_ELEMENT_ARRAY_BUFFER
         * @return the new create instance
         * @throws GLException
         */
        static proxy_ref createGLSL(const std::string& name, GLsizei componentsPerElement, bool normalized, GLsizei stride,
                                    GLsizei mappedElementCount, GLuint vboName, uintptr_t vboOffset, GLenum vboUsage, GLenum vboTarget) {
            return std::make_shared<GLArrayDataProxy>(Private(),
                name, componentsPerElement, normalized, stride, mappedElementCount,
                /*isVertexAttribute=*/true, vboName, vboOffset, vboUsage, vboTarget);
        }

        /** Returns true if using mapped memory, otherwise false for client-data */
        constexpr bool usesMappedMem() const noexcept { return !m_bufferptr; }
        /** Returns true if using client-data, otherwise false for mapped memory */
        constexpr bool usesClientMem() const noexcept { return m_bufferptr; }

        std::string_view className() const noexcept override { return "GLArrayDataProxy"; }

        const jau::type_info& classSignature() const noexcept override {
            return jau::static_ctti<proxy_t>();
        }

        const GLArrayDataProxyRef<value_type> shared() { return GLArrayData::shared_from_base<GLArrayDataProxy>(); }

        /** Returns client-data pointer at current position if usesClientMem(), otherwise nullptr */
        const void* data() const noexcept override { return usesClientMem() ? reinterpret_cast<const void*>(m_bufferptr->position_ptr()) : nullptr; }

        GLsizei elemCount() const noexcept override {
            if( usesClientMem() ) {
                if( m_sealed ) {
                    return static_cast<GLsizei>( (m_bufferptr->limit() * m_bytesPerComp) / m_strideB );
                } else {
                    return static_cast<GLsizei>( (m_bufferptr->position() * m_bytesPerComp) / m_strideB );
                }
            } else {
                return m_mappedElemCount;
            }
        }

        glmemsize_t byteCount() const noexcept override {
            if( usesClientMem() ) {
                if( m_sealed ) {
                    return m_bufferptr->limit() * m_bytesPerComp ;
                } else {
                    return m_bufferptr->position() * m_bytesPerComp ;
                }
            } else {
                return m_mappedElemCount * m_compsPerElement * m_bytesPerComp ;
            }
        }

        glmemsize_t byteCapacity() const noexcept override {
            if( usesClientMem() ) {
                return m_bufferptr->capacity() * m_bytesPerComp;
            } else {
                return 0;
            }
        }

        std::string elemStatsToString() const noexcept override {
            const size_t elem_limit = ( (usesClientMem() ? m_bufferptr->limit() : 0) * m_bytesPerComp ) / m_strideB;
            return jau::format_string("sealed %d, elements %s cnt, [%s pos .. %s rem .. %s lim .. %s cap]",
                                 sealed(),
                                 jau::to_decstring(elemCount()).c_str(),
                                 jau::to_decstring(elemPosition()).c_str(),
                                 jau::to_decstring(remainingElems()).c_str(),
                                 jau::to_decstring(elem_limit).c_str(),
                                 jau::to_decstring(elemCapacity()).c_str());
        }

        void destroy(GL& gl) override {
            GLArrayData::destroy(gl);
            if( usesClientMem() ) {
                m_bufferptr->clear();
            }
        }

        std::string toString() const noexcept override { return toString(false); }
        virtual std::string toString(bool withData) const noexcept {
            std::string r(className());
            r.append("[").append(m_name)
             .append(", location ").append(std::to_string(m_location))
             .append(", isVertexAttribute ").append(std::to_string(m_isVertexAttr))
             .append(", dataType ").append(jau::to_hexstring(m_compType))
             .append(", compsPerElem ").append(std::to_string(m_compsPerElement))
             .append(", stride ").append(std::to_string(m_strideB)).append("b ").append(std::to_string(m_strideL)).append("c")
             .append(", mappedElemCount ").append(std::to_string(m_mappedElemCount))
             .append(", ").append(elemStatsToString());
            if( withData ) {
                r.append(", buffer ").append(usesClientMem()?m_bufferptr->toString():"nil");
            }
            r.append(", vboEnabled ").append(std::to_string(m_vboEnabled))
             .append(", vboName ").append(std::to_string(m_vboName))
             .append(", vboUsage ").append(jau::to_hexstring(m_vboUsage))
             .append(", vboTarget ").append(jau::to_hexstring(m_vboTarget))
             .append(", vboOffset ").append(std::to_string(m_vboOffset))
             .append(", alive ").append(std::to_string(m_alive)).append("]");
            return r;
        }

        /** The Buffer holding the data, nullptr if !usesClientMem() */
        const buffer_t* buffer_ptr() const noexcept { return m_bufferptr; }
        /** The Buffer holding the data, nullptr if !usesClientMem() */
        buffer_t* buffer_ptr() noexcept { return m_bufferptr; }

        /**
         * Returns the element position.
         * <p>
         * On element consist out of {@link #getCompsPerElem()} components.
         * </p>
         * @see bytePosition()
         * @see getElemCount()
         * @see remainingElems()
         * @see getElemCapacity()
         */
        glmemsize_t elemPosition() const noexcept {
            if( usesClientMem() ) {
                return ( m_bufferptr->position() * m_bytesPerComp ) / m_strideB ;
            } else {
                return m_mappedElemCount;
            }
        }

        /**
         * The current number of remaining elements.
         * <p>
         * On element consist out of {@link #getCompsPerElem()} components.
         * </p>
         * Returns the number of elements between the current position and the limit, i.e. remaining elements to write in this buffer.
         * @see remainingBytes()
         * @see getElemCount()
         * @see elemPosition()
         * @see getElemCapacity()
         */
        glmemsize_t remainingElems() const noexcept {
            if( usesClientMem() ) {
                return (m_bufferptr->remaining() * m_bytesPerComp) / m_strideB;
            } else {
                return 0;
            }
        }

        /**
         * Return the element capacity.
         * <p>
         * On element consist out of {@link #getCompsPerElem()} components.
         * </p>
         * @see getByteCapacity()
         * @see getElemCount()
         * @see elemPosition()
         * @see remainingElems()
         */
        glmemsize_t elemCapacity() const noexcept {
            if( usesClientMem() ) {
                return ( m_bufferptr->capacity() * m_bytesPerComp ) / m_strideB ;
            } else {
                return 0;
            }
        }

        /**
         * Returns the bytes position.
         * @see elemPosition
         * @see getByteCount
         * @see remainingElems
         * @see getElemCapacity
         */
        glmemsize_t bytePosition() const noexcept {
            if( usesClientMem() ) {
                return m_bufferptr->position() * m_bytesPerComp;
            } else {
                return m_mappedElemCount * m_compsPerElement * m_bytesPerComp ;
            }
        }

        /**
         * The current number of remaining bytes.
         * <p>
         * Returns the number of bytes between the current position and the limit, i.e. remaining bytes to write in this buffer.
         * </p>
         * @see remainingElems
         * @see getByteCount
         * @see bytePosition
         * @see getByteCapacity
         */
        glmemsize_t remainingBytes() const noexcept {
            if( usesClientMem() ) {
                return m_bufferptr->remaining() * m_bytesPerComp;
            } else {
                return 0;
            }
        }

        /**
         * Returns a string with detailed buffer fill stats.
         */
        std::string fillStatsToString() const noexcept {
            const size_t cnt_bytes = byteCount();
            const size_t cap_bytes = byteCapacity();
            const float  filled    = (float)cnt_bytes / (float)cap_bytes;
            return jau::format_string("elements %s cnt / %s cap, bytes %s cnt / %s cap, filled %.1f%%, left %.1f%%",
                                 jau::to_decstring(elemCount()).c_str(),
                                 jau::to_decstring(elemCapacity()).c_str(),
                                 jau::to_decstring(cnt_bytes).c_str(),
                                 jau::to_decstring(cap_bytes).c_str(),
                                 filled * 100.0f, (1.0f - filled) * 100.0f);
        }

        //
        // OpenGL pass through funcs
        //

        void glBufferData(const GL& gl) const noexcept {
            GLArrayData::glBufferData(gl, remainingBytes());
        }

      protected:
        struct Private{ explicit Private() = default; };

      public:
        // using passing through client buffer_t
        GLArrayDataProxy(Private,
                          const std::string& name, GLsizei componentsPerElement,
                          bool normalized, GLsizei stride, buffer_t& buffer,
                          bool isVertexAttribute, GLuint vboName, uintptr_t vboOffset, GLenum vboUsage, GLenum vboTarget)
        : GLArrayData(GLArrayData::Private(), name, componentsPerElement, glType<value_type>(), jau::make_ctti<value_type>(),
                      normalized, stride, /*mappedElementCount=*/0, isVertexAttribute, vboName, vboOffset, vboUsage, vboTarget),
          m_bufferptr( &buffer )
        { }

        // using memory mapped elements
        GLArrayDataProxy(Private,
                         const std::string& name, GLsizei componentsPerElement,
                         bool normalized, GLsizei stride, GLsizei mappedElementCount,
                         bool isVertexAttribute, GLuint vboName, uintptr_t vboOffset, GLenum vboUsage, GLenum vboTarget)
        : GLArrayData(GLArrayData::Private(), name, componentsPerElement, glType<value_type>(), jau::make_ctti<value_type>(),
                      normalized, stride, mappedElementCount, isVertexAttribute, vboName, vboOffset, vboUsage, vboTarget),
          m_bufferptr(nullptr)
        {
            if( 0 == mappedElementCount ) {
                throw jau::IllegalArgumentError("mappedElementCount:=" + std::to_string(mappedElementCount)
                    + " specified for memory map:\n\t" + toStringImpl(), E_FILE_LINE);
            }
        }

      protected:
        // mutable
        buffer_t* m_bufferptr;
    };

    /**@}*/

}  // namespace gamp::render::gl

#endif /* GAMP_GLARRAYDATAPROXY_HPP_ */
