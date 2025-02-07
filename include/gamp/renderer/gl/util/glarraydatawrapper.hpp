/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2010-2025 Gothel Software e.K.
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

#ifndef GAMP_GLARRAYDATAWRAPPER_HPP_
#define GAMP_GLARRAYDATAWRAPPER_HPP_

#include <jau/basic_types.hpp>
#include <jau/darray.hpp>
#include <jau/int_types.hpp>
#include <jau/string_cfmt.hpp>
#include <jau/string_util.hpp>

#include <cstdint>
#include <gamp/renderer/gl/glliterals.hpp>
#include <gamp/renderer/gl/glarraydata.hpp>
#include <gamp/renderer/gl/util/glbuffers.hpp>

namespace gamp::render::gl::util {
    using namespace gamp::render::gl;

    /**
     *
     * The total number of bytes hold by the referenced buffer is:
     * getComponentSize()* getComponentNumber() * getElementNumber()
     *
     */
    class GLArrayDataWrapper : public GLArrayData {
      public:
        ~GLArrayDataWrapper() noexcept override = default;

        static constexpr bool DEBUG_MODE = true;

        /**
         * Create a VBO, using a predefined fixed function array index, wrapping the given data.
         * <p>
         * This buffer is always {@link #sealed()}.
         * </p>
         * @param index The GL array index
         * @param comps The array component number
         * @param dataType The array index GL data type
         * @param normalized Whether the data shall be normalized
         * @param stride
         * @param buffer the user define data
         * @param vboName
         * @param vboOffset
         * @param vboUsage {@link GL2ES2#GL_STREAM_DRAW}, {@link GL#GL_STATIC_DRAW} or {@link GL#GL_DYNAMIC_DRAW}
         * @param vboTarget {@link GL#GL_ARRAY_BUFFER} or {@link GL#GL_ELEMENT_ARRAY_BUFFER}
         * @return the new create instance
         *
         * @throws GLException
         */
        static GLArrayDataWrapper createFixed(GLsizei index, GLsizei comps, GLenum dataType, bool normalized, GLsizei stride,
                                              const buffer_ref& buffer, GLuint vboName, uintptr_t vboOffset, GLenum vboUsage, GLenum vboTarget) {
            return GLArrayDataWrapper("", index, comps, dataType, normalized, stride, buffer, 0 /* mappedElementCount */,
                                      false, vboName, vboOffset, vboUsage, vboTarget);
        }

        /**
         * Create a VBO, using a predefined fixed function array index, wrapping the mapped data characteristics.
         * <p>
         * This buffer is always {@link #sealed()}.
         * </p>
         * @param index The GL array index
         * @param comps The array component number
         * @param dataType The array index GL data type
         * @param normalized Whether the data shall be normalized
         * @param stride
         * @param mappedElementCount
         * @param vboName
         * @param vboOffset
         * @param vboUsage {@link GL2ES2#GL_STREAM_DRAW}, {@link GL#GL_STATIC_DRAW} or {@link GL#GL_DYNAMIC_DRAW}
         * @param vboTarget {@link GL#GL_ARRAY_BUFFER} or {@link GL#GL_ELEMENT_ARRAY_BUFFER}
         * @return the new create instance
         *
         * @throws GLException
         */
        static GLArrayDataWrapper createFixed(GLsizei index, GLsizei comps, GLenum dataType, bool normalized, GLsizei stride,
                                              GLsizei mappedElementCount, GLuint vboName, uintptr_t vboOffset, GLenum vboUsage, GLenum vboTarget) {
            return GLArrayDataWrapper("", index, comps, dataType, normalized, stride, nullptr, mappedElementCount,
                                      false, vboName, vboOffset, vboUsage, vboTarget);
        }

        /**
         * Create a VBO, using a custom GLSL array attribute name, wrapping the given data.
         * <p>
         * This buffer is always {@link #sealed()}.
         * </p>
         * @param name  The custom name for the GL attribute, maybe null if gpuBufferTarget is {@link GL#GL_ELEMENT_ARRAY_BUFFER}
         * @param comps The array component number
         * @param dataType The array index GL data type
         * @param normalized Whether the data shall be normalized
         * @param stride
         * @param buffer the user define data
         * @param vboName
         * @param vboOffset
         * @param vboUsage {@link GL2ES2#GL_STREAM_DRAW}, {@link GL#GL_STATIC_DRAW} or {@link GL#GL_DYNAMIC_DRAW}
         * @param vboTarget {@link GL#GL_ARRAY_BUFFER} or {@link GL#GL_ELEMENT_ARRAY_BUFFER}
         * @return the new create instance
         * @throws GLException
         */
        static GLArrayDataWrapper createGLSL(const std::string& name, GLsizei comps, GLenum dataType, bool normalized, GLsizei stride,
                                             const buffer_ref& buffer, GLuint vboName, uintptr_t vboOffset, GLenum vboUsage, GLenum vboTarget) {
            return GLArrayDataWrapper(name, 0, comps, dataType, normalized, stride, buffer, 0 /* mappedElementCount */,
                                      true, vboName, vboOffset, vboUsage, vboTarget);
        }

        /**
         * Create a VBO, using a custom GLSL array attribute name, wrapping the mapped data characteristics.
         * <p>
         * This buffer is always {@link #sealed()}.
         * </p>
         * @param name  The custom name for the GL attribute, maybe null if gpuBufferTarget is {@link GL#GL_ELEMENT_ARRAY_BUFFER}
         * @param comps The array component number
         * @param dataType The array index GL data type
         * @param normalized Whether the data shall be normalized
         * @param stride
         * @param mappedElementCount
         * @param vboName
         * @param vboOffset
         * @param vboUsage {@link GL2ES2#GL_STREAM_DRAW}, {@link GL#GL_STATIC_DRAW} or {@link GL#GL_DYNAMIC_DRAW}
         * @param vboTarget {@link GL#GL_ARRAY_BUFFER} or {@link GL#GL_ELEMENT_ARRAY_BUFFER}
         * @return the new create instance
         * @throws GLException
         */
        static GLArrayDataWrapper createGLSL(const std::string& name, GLsizei comps, GLenum dataType, bool normalized, GLsizei stride,
                                             GLsizei mappedElementCount, GLuint vboName, uintptr_t vboOffset, GLenum vboUsage, GLenum vboTarget) {
            return GLArrayDataWrapper(name, 0, comps, dataType, normalized, stride, nullptr, mappedElementCount,
                                      true, vboName, vboOffset, vboUsage, vboTarget);
        }

        bool validate() const noexcept {
            if( !m_alive ) {
                return false;
            }
            // Skip GLProfile based index, comps, type validation, might not be future proof.
            // glp.isValidArrayDataType(getIndex(), getCompsPerElem(), getCompType(), isVertexAttribute(), throwException);
            return true;
        }

        // virtual void associate(Object obj, boolean enable) = 0;

        bool isVertexAttribute() const noexcept override { return m_isVertexAttr; }

        GLenum index() const noexcept override { return m_index; }

        const std::string& name() const noexcept override { return m_name; }

        void setName(const std::string& newName) noexcept override { m_name = newName; }

        GLint location() const noexcept override { return m_location; }

        void setLocation(GLint v) noexcept override { m_location = v; }

        GLint retrieveLocation(GLuint program) noexcept override {
            m_location = glGetAttribLocation(program, m_name.c_str());
            return m_location;
        }

        void bindLocation(GLuint program, GLint location) noexcept override {
            if( 0 <= location ) {
                m_location = location;
                glBindAttribLocation(program, location, m_name.c_str());
            }
        }

        bool isVBO() const noexcept override { return m_vboEnabled; }

        uintptr_t vboOffset() const noexcept override { return m_vboEnabled?m_vboOffset:0; }

        GLuint vboName() const noexcept override { return m_vboEnabled?m_vboName:0; }

        GLenum vboUsage() const noexcept override { return m_vboEnabled?m_vboUsage:0; }

        GLenum vboTarget() const noexcept override { return m_vboEnabled?m_vboTarget:0; }

        const buffer_ref& buffer() const noexcept override { return m_buffer; }

        GLsizei compsPerElem() const noexcept override { return m_compsPerElement; }

        GLenum compType() const noexcept override { return m_compType; }

        GLsizei bytesPerComp() const noexcept override { return m_bytesPerComp; }

        bool sealed() const noexcept override { return m_sealed; }

        size_t elemCount() const noexcept override {
            if( 0 != m_mappedElemCount ) {
                return m_mappedElemCount;
            } else if( m_buffer ) {
                if( m_sealed ) {
                    return (m_buffer->limit() * m_bytesPerComp) / m_strideB;
                } else {
                    return (m_buffer->position() * m_bytesPerComp) / m_strideB;
                }
            } else {
                return 0;
            }
        }

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
        size_t elemPosition() const noexcept override {
            if( 0 != m_mappedElemCount ) {
                return m_mappedElemCount;
            } else if( m_buffer ) {
                return ( m_buffer->position() * m_bytesPerComp ) / m_strideB ;
            } else {
                return 0;
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
        size_t remainingElems() const noexcept override {
            if( m_buffer ) {
                return (m_buffer->remaining() * m_bytesPerComp) / m_strideB;
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
        size_t elemCapacity() const noexcept override {
            if( m_buffer ) {
                return ( m_buffer->capacity() * m_bytesPerComp ) / m_strideB ;
            } else {
                return 0;
            }
        }

        /**
         * Returns the byte position (written elements) if not {@link #sealed()} or
         * the byte limit (available to read) after {@link #sealed()} (flip).
         * @see sealed()
         * @see getElemCount()
         * @see bytePosition()
         * @see remainingBytes()
         * @see getByteCapacity()
         */
        size_t byteCount() const noexcept override {
            if( 0 != m_mappedElemCount ) {
                return m_mappedElemCount * m_compsPerElement * m_bytesPerComp ;
            } else if( m_buffer ) {
                if( m_sealed ) {
                    return m_buffer->limit() * m_bytesPerComp ;
                } else {
                    return m_buffer->position() * m_bytesPerComp ;
                }
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
        size_t bytePosition() const noexcept override {
            if( 0 != m_mappedElemCount ) {
                return m_mappedElemCount * m_compsPerElement * m_bytesPerComp ;
            } else if( m_buffer ) {
                return m_buffer->position() * m_bytesPerComp;
            } else {
                return 0;
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
        size_t remainingBytes() const noexcept override {
            if( m_buffer ) {
                return m_buffer->remaining() * m_bytesPerComp;
            } else {
                return 0;
            }
        }

        /**
         * Return the capacity in bytes.
         * @see getElemCapacity
         * @see getByteCount
         * @see bytePosition
         * @see remainingBytes
         */
        size_t byteCapacity() const noexcept override {
            if( m_buffer ) {
                return m_buffer->capacity() * m_bytesPerComp;
            } else {
                return 0;
            }
        }

        /**
         * Returns a string with detailed buffer fill stats.
         */
        std::string fillStatsToString() const noexcept override {
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

        /**
         * Returns a string with detailed buffer element stats, i.e. sealed, count, position, remaining, limit and capacity.
         */
        std::string elemStatsToString() const noexcept override {
            const size_t elem_limit = m_buffer ? (m_buffer->limit() * m_bytesPerComp) / m_strideB : 0;
            return jau::format_string("sealed %d, elements %s cnt, [%s pos .. %s rem .. %s lim .. %s cap]",
                                 sealed(),
                                 jau::to_decstring(elemCount()).c_str(),
                                 jau::to_decstring(elemPosition()).c_str(),
                                 jau::to_decstring(remainingElems()).c_str(),
                                 jau::to_decstring(elem_limit).c_str(),
                                 jau::to_decstring(elemCapacity()).c_str());
        }

        /**
         * True, if GL shall normalize fixed point data while converting
         * them into float.
         * <p>
         * Default behavior (of the fixed function pipeline) is <code>true</code>
         * for fixed point data type and <code>false</code> for floating point data types.
         * </p>
         */
        bool normalized() const noexcept override { return m_normalized; }

        /**
         * @return the byte offset between consecutive components
         */
        GLsizei stride() const noexcept override {  return m_strideB; }

        void destroy(const GL&) noexcept override {
            m_buffer = nullptr;
            m_vboName=0;
            m_vboEnabled=false;
            m_vboOffset=0;
            m_alive = false;
        }

        std::string toString() const noexcept override {
            std::string r("GLArrayDataWrapper[");
            r.append(m_name).append(", index ").append(std::to_string(m_index))
             .append(", location ").append(std::to_string(m_location))
             .append(", isVertexAttribute ").append(std::to_string(m_isVertexAttr))
             .append(", dataType ").append(jau::to_hexstring(m_compType))
             .append(", compsPerElem ").append(std::to_string(m_compsPerElement))
             .append(", stride ").append(std::to_string(m_strideB)).append("b ").append(std::to_string(m_strideL)).append("c")
             .append(", mappedElemCount ").append(std::to_string(m_mappedElemCount))
             .append(", ").append(elemStatsToString())
             .append(", buffer ").append(m_buffer?m_buffer->toString():"nil")
             .append(", vboEnabled ").append(std::to_string(m_vboEnabled))
             .append(", vboName ").append(std::to_string(m_vboName))
             .append(", vboUsage ").append(jau::to_hexstring(m_vboUsage))
             .append(", vboTarget ").append(jau::to_hexstring(m_vboTarget))
             .append(", vboOffset ").append(std::to_string(m_vboOffset))
             .append(", alive ").append(std::to_string(m_alive)).append("]");
            return r;
        }

        /**
         * Copy Constructor
         * <p>
         * Buffer is sliced, i.e. sharing content but using own state.
         * </p>
         * <p>
         * All other values are simply copied.
         * </p>
         */
      public:
        GLArrayDataWrapper(const GLArrayDataWrapper& src) {
            // immutable types
            m_compType        = src.m_compType;
            m_bytesPerComp    = src.m_bytesPerComp;
            m_compsPerElement = src.m_compsPerElement;
            m_strideB         = src.m_strideB;
            m_strideL         = src.m_strideL;
            m_normalized      = src.m_normalized;
            m_mappedElemCount = src.m_mappedElemCount;
            m_isVertexAttr    = src.m_isVertexAttr;

            // mutable types
            m_alive    = src.m_alive;
            m_index    = src.m_index;
            m_location = src.m_location;
            m_name     = src.m_name;
            if( src.m_buffer ) {
                if( src.m_buffer->position() == 0 ) {
                    m_buffer = src.m_buffer->slice();
                } else {
                    m_buffer = src.m_buffer->slice(0, src.m_buffer->limit());
                }
            } else {
                m_buffer = nullptr;
            }
            m_vboName    = src.m_vboName;
            m_vboOffset  = src.m_vboOffset;
            m_vboEnabled = src.m_vboEnabled;
            m_vboUsage   = src.m_vboUsage;
            m_vboTarget  = src.m_vboTarget;
            m_sealed     = src.m_sealed;
        }

      protected:
        GLArrayDataWrapper(const std::string& name, GLenum index, GLsizei componentsPerElement, GLenum componentType,
                           bool normalized, GLsizei stride, const buffer_ref& data, GLsizei mappedElementCount,
                           bool isVertexAttribute, GLuint vboName, uintptr_t vboOffset, GLenum vboUsage, GLenum vboTarget)
        {
            if( 0 == mappedElementCount && nullptr != data ) {
                throw jau::IllegalArgumentError("mappedElementCount:=" + std::to_string(mappedElementCount) + " specified, but passing non null buffer", E_FILE_LINE);
            }
            // We can't have any dependence on the FixedFuncUtil class here for build bootstrapping reasons

            if( GL_ELEMENT_ARRAY_BUFFER == vboTarget ) {
                // OK ..
            } else if( (0 == vboUsage && 0 == vboTarget) || GL_ARRAY_BUFFER == vboTarget ) {
                // Set/Check name .. - Required for GLSL case. Validation and debug-name for FFP.
                m_name = name.empty() ? GLLiterals::getPredefinedArrayIndexName(index) : name;
                if( m_name.empty() ) {
                    throw GLException("Not a valid array buffer index: " + std::to_string(index), E_FILE_LINE);
                }
            } else if( 0 < vboTarget ) {
                throw GLException("Invalid GPUBuffer target: " + jau::to_hexstring(vboTarget), E_FILE_LINE);
            }

            // immutable types
            m_compType = componentType;
            m_bytesPerComp  = GLBuffers::sizeOfGLType(componentType);
            if( 0 == m_bytesPerComp ) {
                throw GLException("Given componentType not supported: " + jau::to_hexstring(componentType) + ":\n\t" + toString(), E_FILE_LINE);
            }
            if( 0 >= componentsPerElement ) {
                throw GLException("Invalid number of components: " + std::to_string(componentsPerElement), E_FILE_LINE);
            }
            m_compsPerElement = componentsPerElement;

            if( 0 < stride && stride < componentsPerElement * m_bytesPerComp ) {
                throw GLException("stride (" + std::to_string(stride) + ") lower than component bytes, " + std::to_string(componentsPerElement) + " * " + std::to_string(m_bytesPerComp), E_FILE_LINE);
            }
            if( 0 < stride && stride % m_bytesPerComp != 0 ) {
                throw GLException("stride (" + std::to_string(stride) + ") not a multiple of bpc " + std::to_string(m_bytesPerComp), E_FILE_LINE);
            }
            m_strideB = (0 == stride) ? componentsPerElement * m_bytesPerComp : stride;
            m_strideL = m_strideB / m_bytesPerComp;

            if( GLBuffers::isGLTypeFixedPoint(componentType) ) {
                m_normalized = normalized;
            } else {
                m_normalized = false;
            }
            m_mappedElemCount = mappedElementCount;
            m_isVertexAttr    = isVertexAttribute;

            // mutable types
            m_index      = index;
            m_location   = -1;
            m_buffer     = data;
            m_vboName    = vboName;
            m_vboOffset  = vboOffset;
            m_vboEnabled = 0 != vboName;

            switch( vboUsage ) {
                case 0:  // nop
                case GL_STATIC_DRAW: // GL
                case GL_DYNAMIC_DRAW: // GL
                case GL_STREAM_DRAW: // GL2ES2
                    break;
                default:
                    throw GLException("invalid gpuBufferUsage: " + jau::to_hexstring(vboUsage) + ":\n\t" + toString(), E_FILE_LINE);
            }
            switch( vboTarget ) {
                case 0:  // nop
                case GL_ARRAY_BUFFER: // GL
                case GL_ELEMENT_ARRAY_BUFFER: // GL
                    break;
                default:
                    throw GLException("invalid gpuBufferTarget: " + jau::to_hexstring(vboTarget) + ":\n\t" + toString(), E_FILE_LINE);
            }
            m_vboUsage  = vboUsage;
            m_vboTarget = vboTarget;
            m_alive     = true;
            m_sealed    = true;
        }

        // immutable types
        GLenum m_compType;
        // Class<?> compClazz;
        GLsizei m_bytesPerComp;
        GLsizei m_compsPerElement;
        /** stride in bytes; strideB >= compsPerElement * bytesPerComp */
        GLsizei m_strideB;
        /** stride in logical components */
        GLsizei m_strideL;
        bool    m_normalized;
        size_t  m_mappedElemCount;
        bool    m_isVertexAttr;

        // mutable types
        bool        m_alive;
        GLenum      m_index;
        GLint       m_location;
        std::string m_name;
        buffer_ref  m_buffer;
        GLuint      m_vboName;
        uintptr_t   m_vboOffset;
        bool        m_vboEnabled;
        GLenum      m_vboUsage;
        GLenum      m_vboTarget;
        bool        m_sealed;
    };

}  // namespace gamp::render::gl

#endif /* GAMP_GLARRAYDATAWRAPPER_HPP_ */
