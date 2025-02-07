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

#ifndef GAMP_GLARRAYDATA_HPP_
#define GAMP_GLARRAYDATA_HPP_

#include <memory>

#include <jau/darray.hpp>
#include <jau/int_types.hpp>
#include <jau/mem_buffers.hpp>

#include <gamp/renderer/gl/gltypes.hpp>

namespace gamp::render::gl {

    /**
     *
     * The total number of bytes hold by the referenced buffer is:
     * getComponentSize()* getComponentNumber() * getElementNumber()
     *
     */
    class GLArrayData {
      public:
        typedef jau::MemBuffer buffer_t;
        typedef std::shared_ptr<buffer_t> buffer_ref;

        virtual ~GLArrayData() noexcept = default;

        /**
         * Implementation and type dependent object association.
         * <p>
         * One currently known use case is to associate a {@link com.jogamp.opengl.util.glsl.ShaderState ShaderState}
         * to an GLSL aware vertex attribute object, allowing to use the ShaderState to handle it's
         * data persistence, location and state change.<br/>
         * This is implicitly done via {@link com.jogamp.opengl.util.glsl.ShaderState#ownAttribute(GLArrayData, boolean) shaderState.ownAttribute(GLArrayData, boolean)}.
         * </p>
         * @param obj implementation and type dependent association
         * @param enable pass true to enable the association and false to disable it.
         */
        // virtual void associate(Object obj, boolean enable) = 0;

        /**
         * Returns true if this data set is intended for a GLSL vertex shader attribute,
         * otherwise false, ie intended for fixed function vertex pointer
         */
        virtual bool isVertexAttribute() const noexcept = 0;

        /**
         * The index of the predefined array index, see list below,
         * or -1 in case of a shader attribute array.
         *
         * @see GLPointerFunc#GL_VERTEX_ARRAY
         * @see GLPointerFunc#GL_NORMAL_ARRAY
         * @see GLPointerFunc#GL_COLOR_ARRAY
         * @see GLPointerFunc#GL_TEXTURE_COORD_ARRAY
         */
        virtual GLenum index() const noexcept = 0;

        /**
         * The name of the reflecting shader array attribute.
         */
        virtual const std::string& name() const noexcept = 0;

        /**
         * Set a new name for this array.
         * <p>
         * This clears the location, i.e. sets it to -1.
         * </p>
         * @see setLocation(int)
         * @see setLocation(GL2ES2, int)
         */
        virtual void setName(const std::string& newName) noexcept = 0;

        /**
         * Returns the shader attribute location for this name,
         * -1 if not yet determined
         */
        virtual GLint location() const noexcept = 0;

        /**
         * Sets the given location of the shader attribute
         *
         * @see com.jogamp.opengl.util.glsl.ShaderState#vertexAttribPointer(GL2ES2, GLArrayData)
         */
        virtual void setLocation(GLint v) noexcept = 0;

        /**
         * Retrieves the location of the shader attribute from the linked shader program.
         * <p>
         * No validation is performed within the implementation.
         * </p>
         * @param gl
         * @param program
         * @return &ge;0 denotes a valid attribute location as found and used in the given shader program.
         *         &lt;0 denotes an invalid location, i.e. not found or used in the given shader program.
         */
        virtual GLint retrieveLocation(GLuint program) noexcept = 0;

        /**
         * Binds the location of the shader attribute to the given location for the unlinked shader program.
         * <p>
         * No validation is performed within the implementation.
         * </p>
         * @param gl
         * @param program
         */
        virtual void bindLocation(GLuint program, GLint location) noexcept = 0;

        /**
         * Determines whether the data is server side (VBO) and enabled,
         * or a client side array (false).
         */
        virtual bool isVBO() const noexcept = 0;

        /**
         * The VBO buffer offset if isVBO()
         */
        virtual uintptr_t vboOffset() const noexcept = 0;

        /**
         * The VBO name or 0 if not a VBO
         */
        virtual GLuint vboName() const noexcept = 0;

        /**
         * The VBO usage or 0 if not a VBO
         * @return 0 if not a GPU buffer, otherwise {@link GL2ES2#GL_STREAM_DRAW}, {@link GL#GL_STATIC_DRAW} or {@link GL#GL_DYNAMIC_DRAW}
         */
        virtual GLenum vboUsage() const noexcept = 0;

        /**
         * The VBO target or 0 if not a VBO
         * @return 0 if not a GPU buffer, otherwise {@link GL#GL_ARRAY_BUFFER} or {@link GL#GL_ELEMENT_ARRAY_BUFFER}
         */
        virtual GLenum vboTarget() const noexcept = 0;

        /** The Buffer holding the data, may be null if a GPU buffer without client bound data */
        virtual const buffer_ref& buffer() const noexcept = 0;

        /**
         * The number of components per element
         */
        virtual GLsizei compsPerElem() const noexcept = 0;

        /**
         * The component's GL data type, ie. GL_FLOAT
         */
        virtual GLenum compType() const noexcept = 0;

        /**
         * The component's size in bytes
         */
        virtual GLsizei bytesPerComp() const noexcept = 0;

        /**
         * Returns true if data has been {@link com.jogamp.opengl.util.GLArrayDataEditable#seal(boolean) sealed} (flipped to read), otherwise false (writing mode).

         * @see com.jogamp.opengl.util.GLArrayDataEditable#seal(boolean)
         * @see com.jogamp.opengl.util.GLArrayDataEditable#seal(GL, boolean)
         */
        virtual bool sealed() const noexcept = 0;

        /**
         * Returns the element position (written elements) if not {@link #sealed()} or
         * the element limit (available to read) after {@link #sealed()} (flip).
         * <p>
         * On element consist out of {@link #getCompsPerElem()} components.
         * </p>
         * @see sealed()
         * @see getByteCount()
         * @see elemPosition()
         * @see remainingElems()
         * @see getElemCapacity()
         */
        virtual size_t elemCount() const noexcept = 0;

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
        virtual size_t elemPosition() const noexcept = 0;

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
        virtual size_t remainingElems() const noexcept = 0;

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
        virtual size_t elemCapacity() const noexcept = 0;

        /**
         * Returns the byte position (written elements) if not {@link #sealed()} or
         * the byte limit (available to read) after {@link #sealed()} (flip).
         * @see sealed()
         * @see getElemCount()
         * @see bytePosition()
         * @see remainingBytes()
         * @see getByteCapacity()
         */
        virtual size_t byteCount() const noexcept = 0;

        /**
         * Returns the bytes position.
         * @see elemPosition
         * @see getByteCount
         * @see remainingElems
         * @see getElemCapacity
         */
        virtual size_t bytePosition() const noexcept = 0;

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
        virtual size_t remainingBytes() const noexcept = 0;

        /**
         * Return the capacity in bytes.
         * @see getElemCapacity
         * @see getByteCount
         * @see bytePosition
         * @see remainingBytes
         */
        virtual size_t byteCapacity() const noexcept = 0;

        /**
         * Returns a string with detailed buffer fill stats.
         */
        virtual std::string fillStatsToString() const noexcept = 0;

        /**
         * Returns a string with detailed buffer element stats, i.e. sealed, count, position, remaining, limit and capacity.
         */
        virtual std::string elemStatsToString() const noexcept = 0;

        /**
         * True, if GL shall normalize fixed point data while converting
         * them into float.
         * <p>
         * Default behavior (of the fixed function pipeline) is <code>true</code>
         * for fixed point data type and <code>false</code> for floating point data types.
         * </p>
         */
        virtual bool normalized() const noexcept = 0;

        /**
         * @return the byte offset between consecutive components
         */
        virtual GLsizei stride() const noexcept = 0;

        virtual void destroy(const GL& gl) noexcept = 0;

        virtual std::string toString() const noexcept = 0;
    };


}  // namespace gamp::render::gl

#endif /* GAMP_GLARRAYDATA_HPP_ */
