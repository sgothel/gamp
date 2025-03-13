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

#include <cstddef>
#include <memory>
#include <utility>

#include <jau/basic_types.hpp>
#include <jau/cpp_lang_util.hpp>
#include <jau/debug.hpp>
#include <jau/file_util.hpp>
#include <jau/float_types.hpp>
#include <jau/io_util.hpp>
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
        virtual ~GLUniformData() noexcept = default;

        /** Return the underlying data buffer pointer. */
        virtual const void* data() const noexcept = 0;

        /** Returns type signature of implementing class's stored component value type. */
        const jau::type_info& compSignature() const noexcept { return m_signature; }

        /** Return the uniform name as used in the shader */
        const string_t& name() const noexcept { return m_name; }

        constexpr GLint location() const noexcept { return m_location; }

        /**
         * Sets the given location of the shader uniform.
         * @return the given location
         */
        GLint setLocation(GLint location) noexcept { m_location=location; return location; }

        /**
         * Retrieves the location of the shader uniform with {@link #getName()} from the linked shader program.
         * <p>
         * No validation is performed within the implementation.
         * </p>
         * @param gl
         * @param program
         * @return &ge;0 denotes a valid uniform location as found and used in the given shader program.
         *         &lt;0 denotes an invalid location, i.e. not found or used in the given shader program.
         */
        int setLocation(const GL&, GLuint program) noexcept {
            m_location = ::glGetUniformLocation(program, m_name.c_str());
            return m_location;
        }

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
              .append(", name ").append(m_name)
              .append(", location ").append(std::to_string(m_location))
              .append(", size ").append(std::to_string(m_rows)).append("x").append(std::to_string(m_columns))
              .append(", count ").append(std::to_string(m_count))
              .append(", data ");
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

        /** Sends the uniform data to the GPU, i.e. issues, [glUniform](https://docs.gl/es3/glUniform) */
        void send(const GL& gl) const;

      protected:
        GLUniformData(const jau::type_info& sig, std::string  name, GLint location,
                      GLsizei rows, GLsizei columns, size_t count)
        : m_signature(sig), m_name(std::move(name)), m_location(location),
          m_rows(rows), m_columns(columns), m_count(castOrThrow<size_t, GLsizei>(count)) {}

      private:
        const jau::type_info& m_signature;
        std::string m_name;
        GLint       m_location;
        GLsizei     m_rows, m_columns, m_count;
    };
    typedef std::shared_ptr<GLUniformData> GLUniformDataRef;

    class GLUniformSyncMatrices4f : public GLUniformData {
      public:
        typedef SyncMatrices4<GLfloat> SyncMats4f;
      private:
        SyncMats4f& m_data;
      public:
        GLUniformSyncMatrices4f(const string_t& name, SyncMats4f& data)
        : GLUniformData(data.compSignature(), name,
                        /*location=*/-1, /*rows=*/4, /*columns=*/4, /*count=*/data.matrixCount()),
          m_data(data) {}

        static std::shared_ptr<GLUniformSyncMatrices4f> create(const string_t& name, SyncMats4f& data) {
            return std::make_shared<GLUniformSyncMatrices4f>(name, data);
        }
        const void* data() const noexcept override { return m_data.syncedData(); }

        constexpr const GLfloat* floats() const { return m_data.floats(); }
    };
    typedef std::shared_ptr<GLUniformSyncMatrices4f> GLUniformSyncMatrices4fRef;

    class GLUniformVec4f : public GLUniformData {
      private:
        jau::math::Vec4f m_data;

      public:
        GLUniformVec4f(const string_t& name, const jau::math::Vec4f& v)
        : GLUniformData(jau::float_ctti::f32(), name,
                        /*location=*/-1, /*rows=*/1, /*columns=*/4, /*count=*/1),
          m_data(v) {}

        static std::shared_ptr<GLUniformVec4f> create(const string_t& name, const jau::math::Vec4f& v) {
            return std::make_shared<GLUniformVec4f>(name, v);
        }
        const void* data() const noexcept override { return m_data.cbegin(); }

        constexpr const jau::math::Vec4f& vec4f() const { return m_data; }
        constexpr jau::math::Vec4f& vec4f() { return m_data; }
    };
    typedef std::shared_ptr<GLUniformVec4f> GLUniformVec4fRef;

    class GLUniformVec3f : public GLUniformData {
      private:
        jau::math::Vec3f m_data;

      public:
        GLUniformVec3f(const string_t& name, const jau::math::Vec3f& v)
        : GLUniformData(jau::float_ctti::f32(), name,
                        /*location=*/-1, /*rows=*/1, /*columns=*/3, /*count=*/1),
          m_data(v) {}

        static std::shared_ptr<GLUniformVec3f> create(const string_t& name, const jau::math::Vec3f& v) {
            return std::make_shared<GLUniformVec3f>(name, v);
        }
        const void* data() const noexcept override { return m_data.cbegin(); }

        constexpr const jau::math::Vec3f& vec3f() const { return m_data; }
        constexpr jau::math::Vec3f& vec3f() { return m_data; }
    };
    typedef std::shared_ptr<GLUniformVec3f> GLUniformVec3fRef;

    /**@}*/
}

#endif // GAMP_GLSLUNIFORMDATA_HPP_
