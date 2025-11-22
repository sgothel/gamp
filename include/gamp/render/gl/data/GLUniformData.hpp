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

#ifndef GAMP_GLSLUNIFORMDATA_HPP_
#define GAMP_GLSLUNIFORMDATA_HPP_

#include <jau/basic_types.hpp>
#include <jau/cpp_lang_util.hpp>
#include <jau/debug.hpp>
#include <jau/float_types.hpp>
#include <jau/io/file_util.hpp>
#include <jau/io/io_util.hpp>
#include <jau/math/vec4f.hpp>
#include <jau/string_util.hpp>
#include <jau/math/util/syncbuffer.hpp>

#include <gamp/Gamp.hpp>
#include <gamp/render/gl/data/GLArrayData.hpp>
#include <gamp/render/gl/glsl/ShaderUtil.hpp>
#include <gamp/render/gl/GLTypes.hpp>
#include <gamp/render/gl/glsl/ShaderProgram.hpp>

namespace gamp::render::gl::data {
    using namespace gamp::render::gl;
    using namespace jau::math::util;

    /** \addtogroup Gamp_GLData
     *
     *  @{
     */

    /**
     * GLSL uniform data wrapper encapsulating data to be uploaded to the GPU as a uniform.
     */
    class GLUniformData {
      public:
        /** Signature, denoting a uniform buffer, i.e. GLUniformBuffer. */
        static const jau::type_info& bufferSignature() { return jau::static_ctti<void*>(); }

        virtual ~GLUniformData() noexcept = default;

        /** Return the underlying data buffer pointer. */
        virtual const void* data() const noexcept = 0;

        /** Returns type signature of implementing class's stored component value type. */
        const jau::type_info& compSignature() const noexcept { return m_signature; }

        /** Return the uniform name as used in the shader */
        const stringview_t name() const noexcept { return m_name; }

        /** Returns true if instance refers to a uniform buffer object */
        constexpr bool isBuffer() const noexcept { return 0!=m_bufferSize; }
        constexpr GLuint bufferName() const noexcept { return m_bufferName; }
        constexpr GLsizeiptr bufferSize() const noexcept { return m_bufferSize; }

        /// Returns the uniform's location, -1 if no location has been retrieved or set or if isBuffer()
        constexpr GLint location() const noexcept { return isBuffer() ? -1 : m_location; }
        /// Returns the uniform buffer's index, GL_INVALID_INDEX if no index has been retrieved or set or is if !isBuffer()
        constexpr GLuint bufferIndex() const noexcept { return isBuffer() ? m_bufferIndex : GL_INVALID_INDEX; }
        /// Returns the uniform buffer's global binding point, GL_INVALID_INDEX if !isBuffer()
        constexpr GLuint bufferBinding() const noexcept { return isBuffer() ? m_bufferGlobalBinding : GL_INVALID_INDEX; }

        /// Returns true if buffer index or location is valid, otherwise false
        bool hasLocation() const noexcept { return isBuffer() ? GL_INVALID_INDEX != bufferIndex() : 0 <= location(); }

        // Clears the location or buffer-index
        void clearLocation() noexcept {
            if( isBuffer() ) {
                m_bufferIndex = GL_INVALID_INDEX;
            } else {
                m_location = -1;
            }
        }

        /**
         * Retrieves the location or buffer-index of the shader uniform (buffer) with {@link #getName()} from the linked shader program,
         * depending on isBuffer().
         *
         * @param gl
         * @param program
         * @return true if successful, i.e. retrieved buffer-index != GL_INVALID_INDEX or location >= 0. False otherwise
         */
        bool resolveLocation(const GL&, GLuint program) noexcept;

        /** Returns element count. One element consist compsPerElem() components. */
        constexpr GLsizei count() const noexcept { return m_count; }
        /** Returns component count per element, i.e. rows() * columns(). */
        constexpr GLsizei components() const noexcept { return m_rows*m_columns; }
        /** Returns row count, i.e. matrices usually have a row count > 1. */
        constexpr GLsizei rows() const noexcept { return m_rows; }
        /** Returns column count, i.e. matrices and vectors usually have a column count > 1. */
        constexpr GLsizei columns() const noexcept { return m_columns; }
        /** Return true if rows > 1 && columns > 1 */
        constexpr bool isMatrix() const noexcept { return m_rows > 1 && m_columns > 1; }
        /** Return true if rows == 1 && columns > 1 */
        constexpr bool isVector() const noexcept { return m_rows == 1 && m_columns > 1; }
        /** Return true if rows == 1 && columns == 1 */
        constexpr bool isScalar() const noexcept { return m_rows == 1 && m_columns == 1; }

        string_t toString() const {
            string_t sb("GLUniformData[");
            sb.append(compSignature().name())
              .append(", name ").append(m_name);
            if( isBuffer() ) {
                sb.append(", buffer[index ").append(jau::toHexString(m_bufferIndex))
                  .append(", binding ").append(jau::toHexString(m_bufferGlobalBinding))
                  .append(", name ").append(std::to_string(m_bufferName)).append(", ")
                  .append(std::to_string(m_bufferSize)).append(" bytes]");
            } else {
                sb.append(", location ").append(std::to_string(m_location));
            }
            sb.append(", size ").append(std::to_string(m_rows)).append("x").append(std::to_string(m_columns))
              .append(", count ").append(std::to_string(m_count));
            sb.append(", data ");
            if( isMatrix() ) {
                if( compSignature() == jau::float_ctti::f32() ) {
                    sb.append("\n");
                    const GLfloat* d = reinterpret_cast<const GLfloat*>(data());
                    for( GLsizei i = 0; i < m_count; ++i ) {
                        if( i > 0 ) { sb.append("\n"); }
                        jau::mat_to_string<GLfloat>(sb, std::to_string(i)+": ", "%10.5f",
                                                    d+static_cast<ptrdiff_t>(i*m_rows*m_columns), m_rows, m_columns, false);
                    }
                }
            } else if( isVector() ) {
                if( compSignature() == jau::float_ctti::f32() ) {
                    const GLfloat* d = reinterpret_cast<const GLfloat*>(data());
                    jau::row_to_string<GLfloat>(sb, "%10.5f", d, 1, m_columns, false, 0);
                }
            } else {
                if( compSignature() == jau::float_ctti::f32() ) {
                    const GLfloat d = *reinterpret_cast<const GLfloat*>(data());
                    sb.append(std::to_string(d));
                } else if( compSignature() == jau::int_ctti::i32() ) {
                    const GLint d = *reinterpret_cast<const GLint*>(data());
                    sb.append(std::to_string(d));
                }
            }
            sb.append(" ]");
            return sb;
        }

        GLUniformData(const GLUniformData&) = delete;
        GLUniformData(GLUniformData&&) = delete;
        GLUniformData& operator=(const GLUniformData&) = delete;

        /** Releases OpenGL resources, i.e. bufferName() if used. */
        void destroy(GL&) {
            if( m_bufferName != 0 ) {
                ::glDeleteBuffers(1, &m_bufferName);
                m_bufferName = 0;
            }
        }

        /** Sends the uniform data to the GPU, i.e. issues [glUniform](https://docs.gl/es3/glUniform) or the aequivalent uniform buffer transfer. */
        bool send(const GL& gl) noexcept;
        /** Sends a subset of the uniform buffer to the GPU, specialization must have passed bufferSignature() for compSignature(). */
        bool sendSub(const GL& gl, GLintptr offset, GLsizeiptr size) noexcept;

      protected:
        /**
         * For plain uniforms
         * @param name persistent std::string_view name of uniform, must be valid through the lifecycle of this instance
         */
        GLUniformData(const jau::type_info &sig, stringview_t name,
                      GLsizei rows, GLsizei columns, size_t count)
        : m_signature(sig), m_name(name), m_location(-1),
          m_rows(rows), m_columns(columns), m_count(castOrThrow<size_t, GLsizei>(count)),
          m_bufferSize(0), m_bufferName(0), m_bufferGlobalBinding(GL_INVALID_INDEX) {}

        /**
         * For Buffer Objects
         * @param name persistent std::string_view name of uniform, must be valid through the lifecycle of this instance
         */
        GLUniformData(stringview_t name,
                      GLsizei rows, GLsizei columns, size_t count, GLsizeiptr bufferSize, GLuint globalBufferBinding)
        : m_signature(bufferSignature()), m_name(name), m_bufferIndex(GL_INVALID_INDEX),
          m_rows(rows), m_columns(columns), m_count(castOrThrow<size_t, GLsizei>(count)),
          m_bufferSize(bufferSize), m_bufferName(0), m_bufferGlobalBinding(globalBufferBinding) {}

      private:
        const jau::type_info& m_signature;
        stringview_t m_name;
        union {
            GLint   m_location;
            GLuint  m_bufferIndex;
        };
        GLsizei     m_rows, m_columns, m_count;
        GLsizeiptr  m_bufferSize;
        GLuint      m_bufferName;
        /// global UBO binding point for glUniformBlockBinding and glBindBufferRange/glBindBufferBase
        GLuint      m_bufferGlobalBinding;
    };
    typedef std::shared_ptr<GLUniformData> GLUniformDataRef;

    inline std::ostream& operator<<(std::ostream& out, const GLUniformData& v) {
        return out << v.toString();
    }

    class GLUniformSyncPMVMat4fExt : public GLUniformData {
      public:
        typedef SyncMatrices4<GLfloat> SyncMats4f;

      private:
        PMVMat4f& m_mat;
        mutable SyncMats4f m_data;

      public:
        /**
         * GLUniformSyncPMVMat4fExt ctor
         * @param name persistent std::string_view name of uniform, must be valid through the lifecycle of this instance
         */
        GLUniformSyncPMVMat4fExt(stringview_t name, PMVMat4f &mat, SyncMats4f &&data)
        : GLUniformData(data.compSignature(), name,
                        /*rows=*/4, /*columns=*/4, /*count=*/data.matrixCount()),
          m_mat(mat), m_data(data) {}

        /**
         * Shared GLUniformSyncPMVMat4fExt ctor
         * @param name persistent std::string_view name of uniform, must be valid through the lifecycle of this instance
         */
        static std::shared_ptr<GLUniformSyncPMVMat4fExt> createShared(stringview_t name, PMVMat4f &mat, SyncMats4f &&data) {
            return std::make_shared<GLUniformSyncPMVMat4fExt>(name, mat, std::move(data));
        }
        const void* data() const noexcept override { return m_data.syncedData(); }

        constexpr const GLfloat* floats() const noexcept { return m_data.floats(); }

        constexpr const PMVMat4f& pmv() const noexcept { return m_mat; }
        constexpr PMVMat4f& pmv() noexcept { return m_mat; }
    };
    typedef std::shared_ptr<GLUniformSyncPMVMat4fExt> GLUniformSyncPMVMat4fExtRef;

    class GLUniformSyncPMVMat4f : public GLUniformData {
      public:
        typedef SyncMatrices4<GLfloat> SyncMats4f;
      private:
        PMVMat4f m_mat;
        mutable SyncMats4f m_data;

      public:
        /**
         * GLUniformSyncPMVMat4f ctor
         * @param name persistent std::string_view name of uniform, must be valid through the lifecycle of this instance
         */
        GLUniformSyncPMVMat4f(stringview_t name, PMVMat4f &&mat)
        : GLUniformData(mat.compSignature(), name,
                        /*rows=*/4, /*columns=*/4, /*count=*/mat.matrixCount()),
          m_mat(mat), m_data(m_mat.makeSyncPMvReq()) {}

        /**
         * Shared GLUniformSyncPMVMat4f ctor
         * @param name persistent std::string_view name of uniform, must be valid through the lifecycle of this instance
         */
        GLUniformSyncPMVMat4f(stringview_t name, PMVData derivedMatrices = PMVData::none)
        : GLUniformSyncPMVMat4f(name, PMVMat4f(derivedMatrices)) {}

        static std::shared_ptr<GLUniformSyncPMVMat4f> createShared(stringview_t name, PMVData derivedMatrices=PMVData::none) {
            return std::make_shared<GLUniformSyncPMVMat4f>(name, derivedMatrices);
        }
        const void* data() const noexcept override { return m_data.syncedData(); }

        constexpr const GLfloat* floats() const noexcept { return m_data.floats(); }

        constexpr const PMVMat4f& pmv() const noexcept { return m_mat; }
        constexpr PMVMat4f& pmv() noexcept { return m_mat; }
    };
    typedef std::shared_ptr<GLUniformSyncPMVMat4f> GLUniformSyncPMVMat4fRef;

    class GLUniformSyncMatrices4f : public GLUniformData {
      public:
        typedef SyncMatrices4<GLfloat> SyncMats4f;
      private:
        mutable SyncMats4f m_data;
      public:
        /**
         * GLUniformSyncMatrices4f ctor
         * @param name persistent std::string_view name of uniform, must be valid through the lifecycle of this instance
         */
        GLUniformSyncMatrices4f(stringview_t name, SyncMats4f &&data)
        : GLUniformData(data.compSignature(), name,
                        /*rows=*/4, /*columns=*/4, /*count=*/data.matrixCount()),
          m_data(data) {}

        /**
         * Shared GLUniformSyncMatrices4f ctor
         * @param name persistent std::string_view name of uniform, must be valid through the lifecycle of this instance
         */
        static std::shared_ptr<GLUniformSyncMatrices4f> createShared(stringview_t name, SyncMats4f &&data) {
            return std::make_shared<GLUniformSyncMatrices4f>(name, std::move(data));
        }
        const void* data() const noexcept override { return m_data.syncedData(); }

        constexpr const GLfloat* floats() const noexcept { return m_data.floats(); }
    };
    typedef std::shared_ptr<GLUniformSyncMatrices4f> GLUniformSyncMatrices4fRef;

    class GLUniformVec4f : public GLUniformData {
      private:
        jau::math::Vec4f m_data;

      public:
        /**
         * GLUniformVec4f ctor
         * @param name persistent std::string_view name of uniform, must be valid through the lifecycle of this instance
         */
        GLUniformVec4f(stringview_t name, const jau::math::Vec4f& v)
        : GLUniformData(jau::float_ctti::f32(), name,
                        /*rows=*/1, /*columns=*/4, /*count=*/1),
          m_data(v) {}

        /**
         * Shared GLUniformVec4f ctor
         * @param name persistent std::string_view name of uniform, must be valid through the lifecycle of this instance
         */
        static std::shared_ptr<GLUniformVec4f> createShared(stringview_t name, const jau::math::Vec4f& v) {
            return std::make_shared<GLUniformVec4f>(name, v);
        }
        const void* data() const noexcept override { return m_data.cbegin(); }

        constexpr const jau::math::Vec4f& vec4f() const noexcept { return m_data; }
        constexpr jau::math::Vec4f& vec4f() noexcept { return m_data; }
    };
    typedef std::shared_ptr<GLUniformVec4f> GLUniformVec4fRef;

    class GLUniformVec3f : public GLUniformData {
      private:
        jau::math::Vec3f m_data;

      public:
        /**
         * GLUniformVec3f ctor
         * @param name persistent std::string_view name of uniform, must be valid through the lifecycle of this instance
         */
        GLUniformVec3f(stringview_t name, const jau::math::Vec3f& v)
        : GLUniformData(jau::float_ctti::f32(), name,
                        /*rows=*/1, /*columns=*/3, /*count=*/1),
          m_data(v) {}

        /**
         * Shared GLUniformVec3f ctor
         * @param name persistent std::string_view name of uniform, must be valid through the lifecycle of this instance
         */
        static std::shared_ptr<GLUniformVec3f> createShared(stringview_t name, const jau::math::Vec3f& v) {
            return std::make_shared<GLUniformVec3f>(name, v);
        }
        const void* data() const noexcept override { return m_data.cbegin(); }

        constexpr const jau::math::Vec3f& vec3f() const noexcept { return m_data; }
        constexpr jau::math::Vec3f& vec3f() noexcept { return m_data; }
    };
    typedef std::shared_ptr<GLUniformVec3f> GLUniformVec3fRef;

    class GLUniformVec2f : public GLUniformData {
      private:
        jau::math::Vec2f m_data;

      public:
        /**
         * GLUniformVec2f ctor
         * @param name persistent std::string_view name of uniform, must be valid through the lifecycle of this instance
         */
        GLUniformVec2f(stringview_t name, const jau::math::Vec2f& v)
        : GLUniformData(jau::float_ctti::f32(), name,
                        /*rows=*/1, /*columns=*/2, /*count=*/1),
          m_data(v) {}

        /**
         * Shared GLUniformVec2f ctor
         * @param name persistent std::string_view name of uniform, must be valid through the lifecycle of this instance
         */
        static std::shared_ptr<GLUniformVec2f> createShared(stringview_t name, const jau::math::Vec2f& v) {
            return std::make_shared<GLUniformVec2f>(name, v);
        }
        const void* data() const noexcept override { return m_data.cbegin(); }

        constexpr const jau::math::Vec2f& vec2f() const noexcept { return m_data; }
        constexpr jau::math::Vec2f& vec2f() noexcept { return m_data; }
    };
    typedef std::shared_ptr<GLUniformVec2f> GLUniformVec2fRef;

    class GLUniformScalarF32 : public GLUniformData {
      private:
        float m_data;

      public:
        /**
         * GLUniformF32 ctor
         * @param name persistent std::string_view name of uniform, must be valid through the lifecycle of this instance
         */
        GLUniformScalarF32(stringview_t name, float v)
        : GLUniformData(jau::float_ctti::f32(), name,
                        /*rows=*/1, /*columns=*/1, /*count=*/1),
          m_data(v) {}

        /**
         * Shared GLUniformF32 ctor
         * @param name persistent std::string_view name of uniform, must be valid through the lifecycle of this instance
         */
        static std::shared_ptr<GLUniformScalarF32> createShared(stringview_t name, float v) {
            return std::make_shared<GLUniformScalarF32>(name, v);
        }
        const void* data() const noexcept override { return &m_data; }

        constexpr float scalar() const noexcept { return m_data; }
        constexpr float& scalar() noexcept { return m_data; }
    };
    typedef std::shared_ptr<GLUniformScalarF32> GLUniformScalarF32Ref;

    template<typename T>
    class GLUniformBuffer : public GLUniformData {
      private:
        T m_data;

      public:
        /**
         * GLUniformBuffer ctor
         * @param name persistent std::string_view name of uniform, must be valid through the lifecycle of this instance
         */
        GLUniformBuffer(stringview_t name, T &&instance, GLuint globalBufferBinding)
        : GLUniformData(name,
                        /*rows=*/1, /*columns=*/1, /*count=*/1,
                        sizeof(T), globalBufferBinding),
          m_data(instance) {}

        /**
         * GLUniformBuffer ctor
         * @param name persistent std::string_view name of uniform, must be valid through the lifecycle of this instance
         */
        static std::shared_ptr<GLUniformBuffer> createShared(stringview_t name, T &&instance) {
            return std::make_shared<GLUniformBuffer>(name, instance);
        }
        const void* data() const noexcept override { return &m_data; }

        constexpr const T& blob() const noexcept { return m_data; }
        constexpr T& blob() noexcept { return m_data; }
    };
    template<typename T>
    using GLUniformBufferRef = std::shared_ptr<GLUniformBuffer<T>>;

    /**@}*/
}

#endif // GAMP_GLSLUNIFORMDATA_HPP_
