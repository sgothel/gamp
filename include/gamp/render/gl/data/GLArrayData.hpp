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

#ifndef GAMP_GLARRAYDATA_HPP_
#define GAMP_GLARRAYDATA_HPP_

#include <memory>

#include <gamp/render/gl/GLTypes.hpp>
#include <gamp/render/gl/data/GLBuffers.hpp>
#include <string_view>
#include <jau/backtrace.hpp>
#include <jau/type_info.hpp>

namespace gamp::render::gl::glsl {
    class ShaderState; // fwd
}

namespace gamp::render::gl::data {

    using namespace gamp::render::gl;
    using namespace gamp::render::gl::glsl;

    /** @defgroup Gamp_GLData Gamp GL Data Managment
     *  OpenGL rendering data management.
     *
     *  @{
     */

    class GLArrayData;
    typedef std::shared_ptr<GLArrayData> GLArrayDataRef;

    /**
     * Interface for a generic data buffer to be used for OpenGL arrays.
     */
    class GLArrayData : public std::enable_shared_from_this<GLArrayData> {
      public:
        virtual ~GLArrayData() noexcept = default;

        const GLArrayDataRef shared() { return shared_from_this(); }

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
        virtual void associate(ShaderState& /*st*/, bool /*enable*/) {
            // nop
        }

        /**
         * Validates this instance's parameter. Called automatically by {@link GLArrayDataClient} and {@link GLArrayDataServer}.
         * {@link GLArrayDataWrapper} does not validate it's instance by itself.
         *
         * @param glp the GLProfile to use
         * @throws GLException if this instance has invalid parameter
         */
        virtual void validate(const GL& gl) const {
            if( !m_alive ) {
                throw RenderException("Instance !alive " + toString(), E_FILE_LINE);
            }
            if( isVertexAttribute() && !gl.glProfile().hasGLSL() ) {
                throw RenderException("GLSL not supported on " + gl.toString() + ", " + toString(), E_FILE_LINE);
            }
            // Skip GLProfile based index, comps, type validation, might not be future proof.
            // glp.isValidArrayDataType(getIndex(), getCompsPerElem(), getCompType(), isVertexAttribute(), throwException);
        }

        /**
         * Returns true if this data set is intended for a GLSL vertex shader attribute,
         * otherwise false, ie intended for fixed function vertex pointer
         */
        constexpr bool isVertexAttribute() const noexcept { return m_isVertexAttr; }

        /**
         * The name of the reflecting shader array attribute.
         */
        constexpr const std::string& name() const noexcept { return m_name; }

        /**
         * Set a new name for this array.
         * <p>
         * This clears the location, i.e. sets it to -1.
         * </p>
         * @see setLocation(int)
         * @see setLocation(GL2ES2, int)
         */
        void setName(const std::string& newName) noexcept {
            m_location = -1;
            m_name = newName;
        }

        /**
         * Returns the shader attribute location for this name,
         * -1 if not yet determined
         */
        constexpr GLint location() const noexcept { return m_location; }

        /**
         * Sets the given location of the shader attribute
         *
         * @see com.jogamp.opengl.util.glsl.ShaderState#vertexAttribPointer(GL2ES2, GLArrayData)
         */
        void setLocation(GLint loc) noexcept { m_location = loc; }

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
        GLint setLocation(const GL&, GLuint program) noexcept {
            m_location = glGetAttribLocation(program, m_name.c_str());
            return m_location;
        }

        /**
         * Binds the location of the shader attribute to the given location for the unlinked shader program.
         * <p>
         * No validation is performed within the implementation.
         * </p>
         * @param gl
         * @param program
         * @return the given location
         */
        GLint setLocation(const GL&, GLuint program, GLint loc) noexcept {
            m_location = loc;
            glBindAttribLocation(program, loc, m_name.c_str());
            return loc;
        }


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
        GLint retrieveLocation(const GL&, GLuint program) noexcept {
            m_location = glGetAttribLocation(program, m_name.c_str());
            return m_location;
        }

        /**
         * Binds the location of the shader attribute to the given location for the unlinked shader program.
         * <p>
         * No validation is performed within the implementation.
         * </p>
         * @param gl
         * @param program
         */
        void bindLocation(const GL&, GLuint program, GLint location) noexcept {
            if( 0 <= location ) {
                m_location = location;
                glBindAttribLocation(program, location, m_name.c_str());
            }
        }

        /**
         * Determines whether the data is server side (VBO) and enabled,
         * or a client side array (false).
         */
        constexpr bool isVBO() const noexcept { return m_vboEnabled; }

        /**
         * The VBO buffer offset if isVBO()
         */
        constexpr uintptr_t vboOffset() const noexcept { return m_vboEnabled?m_vboOffset:0; }

        /**
         * The VBO name or 0 if not a VBO
         */
        constexpr GLuint vboName() const noexcept { return m_vboEnabled?m_vboName:0; }

        /**
         * The VBO usage or 0 if not a VBO
         * @return 0 if not a GPU buffer, otherwise {@link GL2ES2#GL_STREAM_DRAW}, {@link GL#GL_STATIC_DRAW} or {@link GL#GL_DYNAMIC_DRAW}
         */
        constexpr GLenum vboUsage() const noexcept { return m_vboEnabled?m_vboUsage:0; }

        /**
         * The VBO target or 0 if not a VBO
         * @return 0 if not a GPU buffer, otherwise {@link GL#GL_ARRAY_BUFFER} or {@link GL#GL_ELEMENT_ARRAY_BUFFER}
         */
        constexpr GLenum vboTarget() const noexcept { return m_vboEnabled?m_vboTarget:0; }

        /**
         * Enable or disable use of VBO.
         * Only possible if a VBO buffer name is defined.
         * @see #setVBOName(int)
         */
        virtual void setVBOEnabled(bool vboEnabled) {
            m_vboEnabled = vboEnabled;
        }

        /**
         * Set the VBO buffer name, if valid (!= 0) enable use of VBO,
         * otherwise (==0) disable VBO usage.
         *
         * @see #setVBOEnabled(boolean)
         */
        void setVBOName(GLuint vboName) noexcept {
            m_vboName = vboName;
            setVBOEnabled(0 != vboName);
        }

        /**
         * @param vboUsage {@link GL2ES2#GL_STREAM_DRAW}, {@link GL#GL_STATIC_DRAW} or {@link GL#GL_DYNAMIC_DRAW}
         */
        void setVBOUsage(GLenum vboUsage) noexcept {
            m_vboUsage = vboUsage;
        }

        /**
         * @param vboTarget either {@link GL#GL_ARRAY_BUFFER} or {@link GL#GL_ELEMENT_ARRAY_BUFFER}
         */
        void setVBOTarget(GLenum vboTarget) noexcept {
            m_vboTarget = vboTarget;
        }

        /** The number of components per element */
        constexpr GLsizei compsPerElem() const noexcept { return m_compsPerElement; }

        /** The component's GL data type, e.g. GL_FLOAT */
        constexpr GLenum compType() const noexcept { return m_compType; }

        /** The component's static compile-time-type-info (CTTI) jau::type_info, e.g. jau::float_ctti::f32(). */
        constexpr const jau::type_info& compTypeSignature() const noexcept { return m_compTypeSignature; }

        /** The component's size in bytes */
        constexpr GLsizei bytesPerComp() const noexcept { return m_bytesPerComp; }

        /**
         * Returns true if data has been {@link com.jogamp.opengl.util.GLArrayDataEditable#seal(boolean) sealed} (flipped to read), otherwise false (writing mode).

         * @see com.jogamp.opengl.util.GLArrayDataEditable#seal(boolean)
         * @see com.jogamp.opengl.util.GLArrayDataEditable#seal(GL, boolean)
         */
        constexpr bool sealed() const noexcept { return m_sealed; }

        /**
         * True, if GL shall normalize fixed point data while converting
         * them into float.
         * <p>
         * Default behavior (of the fixed function pipeline) is <code>true</code>
         * for fixed point data type and <code>false</code> for floating point data types.
         * </p>
         */
        constexpr bool normalized() const noexcept { return m_normalized; }

        /**
         * @return the byte offset between consecutive components
         */
        GLsizei stride() const noexcept {  return m_strideB; }

        /** Returns class name of implementing class. */
        virtual std::string_view className() const noexcept { return "GLArrayData"; }

        /** Returns type signature of implementing class. See compTypeSignature() as well. */
        virtual const jau::type_info& classSignature() const noexcept = 0;

        /** The data pointer, may be null if using a memory-mapped GPU buffer */
        virtual const void* data() const noexcept = 0;

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
        virtual GLsizei elemCount() const noexcept = 0;

        /**
         * Returns the byte position (written elements) if not {@link #sealed()} or
         * the byte limit (available to read) after {@link #sealed()} (flip).
         * @see sealed()
         * @see getElemCount()
         * @see bytePosition()
         * @see remainingBytes()
         * @see getByteCapacity()
         */
        virtual glmemsize_t byteCount() const noexcept = 0;

        /**
         * Return the capacity in bytes.
         * @see getElemCapacity
         * @see getByteCount
         * @see bytePosition
         * @see remainingBytes
         */
        virtual glmemsize_t byteCapacity() const noexcept = 0;

        /**
         * Returns a string with detailed buffer element stats, i.e. sealed, count, position, remaining, limit and capacity.
         */
        virtual std::string elemStatsToString() const noexcept { return ""; }

        virtual void destroy(GL&) {
            // m_buffer = nullptr;
            m_vboName=0;
            m_vboEnabled=false;
            m_vboOffset=0;
            m_alive = false;
        }

      protected:
        std::string toStringImpl() const noexcept {
            std::string r("GLArrayData");
            r.append("[").append(m_name)
             .append(", location ").append(std::to_string(m_location))
             .append(", isVertexAttribute ").append(std::to_string(m_isVertexAttr))
             .append(", dataType ").append(jau::to_hexstring(m_compType))
             .append(", compsPerElem ").append(std::to_string(m_compsPerElement))
             .append(", stride ").append(std::to_string(m_strideB)).append("b ").append(std::to_string(m_strideL)).append("c")
             .append(", vboEnabled ").append(std::to_string(m_vboEnabled))
             .append(", vboName ").append(std::to_string(m_vboName))
             .append(", vboUsage ").append(jau::to_hexstring(m_vboUsage))
             .append(", vboTarget ").append(jau::to_hexstring(m_vboTarget))
             .append(", vboOffset ").append(std::to_string(m_vboOffset))
             .append(", alive ").append(std::to_string(m_alive)).append("]");
            return r;
        }

      public:
        virtual std::string toString() const noexcept { return toStringImpl(); }

        //
        // OpenGL pass through funcs
        //

        /** Sends (creates, updates) the data to the bound vboName buffer, see glBindBuffer(). Issues [glBufferData](https://docs.gl/es3/glBufferData) */
        void glBufferData(const GL&, glmemsize_t size) const noexcept {
            ::glBufferData(vboTarget(), size, data(), vboUsage());
        }
        /** Binds the vboName() buffer to its vboTarget() on the GPU, i.e. issues [glBindBuffer](https://docs.gl/es3/glBindBuffer) */
        void glBindBuffer(const GL&, bool on) const noexcept {
            ::glBindBuffer(vboTarget(), on ? vboName() : 0);
        }

        /**
         * Associates the vboName() buffer as an vertex attribute on the GPU, or sends the data if !isVBO().
         *
         * Does nothing, if location() is undefined or no data set.
         *
         * Issues [glVertexAttribPointer](https://docs.gl/es3/glVertexAttribPointer)
         */
        void glVertexAttribPointer(const GL&) const noexcept {
          if(compsPerElem()==0 || location()<0) return;
          if(isVBO()) {
              ::glVertexAttribPointer(location(), compsPerElem(), compType(),
                                      normalized(), stride(), reinterpret_cast<void*>(vboOffset())); // NOLINT(performance-no-int-to-ptr)
          } else {
              ::glVertexAttribPointer(location(), compsPerElem(), compType(),
                                      normalized(), stride(), data());
          }
        }

      protected:
        struct Private{ explicit Private() = default; };

        template <typename ChildT>
        std::shared_ptr<ChildT> shared_from_base() {
            return std::static_pointer_cast<ChildT>(shared_from_this());
        }

      public:
        /** Private ctor for shared_ptr. */
        GLArrayData(Private,
                    const std::string& name, GLsizei componentsPerElement, GLenum componentType, jau::type_info compTypeSignature,
                    bool normalized, GLsizei stride, GLsizei mappedElementCount,
                    bool isVertexAttribute, GLuint vboName, uintptr_t vboOffset, GLenum vboUsage, GLenum vboTarget)
        {
            if( GL_ELEMENT_ARRAY_BUFFER == vboTarget ) {
                // OK ..
            } else if( (0 == vboUsage && 0 == vboTarget) || GL_ARRAY_BUFFER == vboTarget ) {
                // Set/Check name .. - Required for GLSL case. Validation and debug-name for FFP.
                m_name = name;
                if( m_name.empty() ) {
                    throw RenderException("Missing attribute name:\n\t" + toStringImpl(), E_FILE_LINE);
                }
            } else if( 0 < vboTarget ) {
                throw RenderException("Invalid GPUBuffer target: " + jau::to_hexstring(vboTarget)
                    + ":\n\t" + toStringImpl(), E_FILE_LINE);
            }

            // immutable types
            m_compType = componentType;
            m_compTypeSignature = compTypeSignature;
            m_bytesPerComp  = GLBuffers::sizeOfGLType(componentType);
            if( 0 == m_bytesPerComp ) {
                throw RenderException("Given componentType not supported: " + jau::to_hexstring(componentType) + ":\n\t" + toStringImpl(), E_FILE_LINE);
            }
            if( 0 >= componentsPerElement ) {
                throw RenderException("Invalid number of components: " + std::to_string(componentsPerElement) + ":\n\t" + toStringImpl(), E_FILE_LINE);
            }
            m_compsPerElement = componentsPerElement;

            if( 0 < stride && stride < componentsPerElement * m_bytesPerComp ) {
                throw RenderException("stride (" + std::to_string(stride) + ") lower than component bytes, " + std::to_string(componentsPerElement)
                    + " * " + std::to_string(m_bytesPerComp) + ":\n\t" + toStringImpl(), E_FILE_LINE);
            }
            if( 0 < stride && stride % m_bytesPerComp != 0 ) {
                throw RenderException("stride (" + std::to_string(stride) + ") not a multiple of bpc "
                    + std::to_string(m_bytesPerComp) + ":\n\t" + toStringImpl(), E_FILE_LINE);
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
            m_location   = -1;
            // m_buffer     = data;
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
                    throw RenderException("invalid gpuBufferUsage: " + jau::to_hexstring(vboUsage) + ":\n\t" + toStringImpl(), E_FILE_LINE);
            }
            switch( vboTarget ) {
                case 0:  // nop
                case GL_ARRAY_BUFFER: // GL
                case GL_ELEMENT_ARRAY_BUFFER: // GL
                    break;
                default:
                    throw RenderException("invalid gpuBufferTarget: " + jau::to_hexstring(vboTarget) + ":\n\t" + toStringImpl(), E_FILE_LINE);
            }
            m_vboUsage  = vboUsage;
            m_vboTarget = vboTarget;
            m_alive     = true;
            m_sealed    = true;
        }

      protected:
        // immutable
        GLenum m_compType;
        jau::type_info m_compTypeSignature;
        // Class<?> compClazz;
        GLsizei     m_bytesPerComp;
        GLsizei     m_compsPerElement;
        /** stride in bytes; strideB >= compsPerElement * bytesPerComp */
        GLsizei     m_strideB;
        /** stride in logical components */
        GLsizei     m_strideL;
        bool        m_normalized;
        GLsizei     m_mappedElemCount;
        bool        m_isVertexAttr;

        // mutable
        bool        m_alive;
        GLint       m_location;
        // buffer_ref  m_buffer;
        std::string m_name;
        GLuint      m_vboName;
        uintptr_t   m_vboOffset;
        bool        m_vboEnabled;
        GLenum      m_vboUsage;
        GLenum      m_vboTarget;
        bool        m_sealed;

    };

    inline std::ostream& operator<<(std::ostream& out, const GLArrayData& v) {
        return out << v.toString();
    }

    /**@}*/

}  // namespace gamp::render::gl

#endif /* GAMP_GLARRAYDATA_HPP_ */
