/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2014-2025 Gothel Software e.K.
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

#ifndef GAMP_GLMAPPEDBUFFER_HPP_
#define GAMP_GLMAPPEDBUFFER_HPP_

#include <cmath>
#include <gamp/renderer/gl/data/glbuffers.hpp>
#include <memory>

#include <jau/basic_types.hpp>
#include <jau/float_types.hpp>
#include <jau/string_util.hpp>

namespace gamp::render::gl::data {
    using namespace gamp::render::gl;

    /** \addtogroup Gamp_GLData
     *
     *  @{
     */

    /**
     * OpenGL mapped buffer storage object reflecting it's
     * <ul>
     *   <li>storage size</li>
     *   <li>storage memory if mapped</li>
     *   <li>mutable usage or immutable flags</li>
     * </ul>
     * <p>
     * Buffer storage is created via:
     * <ul>
     *   <li>{@link GL#glBufferData(int, long, java.nio.Buffer, int)} - storage creation via target</li>
     *   <li>{@link GL2#glNamedBufferDataEXT(int, long, java.nio.Buffer, int)} - storage creation, direct</li>
     *   <li>{@link GL4#glNamedBufferData(int, long, java.nio.Buffer, int)} - storage creation, direct</li>
     *   <li>{@link GL4#glBufferStorage(int, long, Buffer, int)} - storage creation via target</li>
     *   <li>{@link GL4#glNamedBufferStorage(int, long, Buffer, int)} - storage creation, direct</li>
     * </ul>
     * Note that storage <i>recreation</i> as mentioned above also invalidate a previous storage instance,
     * i.e. disposed the buffer's current storage if exist and attaches a new storage instance.
     * </p>
     * <p>
     * Buffer storage is disposed via:
     * <ul>
     *   <li>{@link GL#glDeleteBuffers(int, IntBuffer)} - explicit, direct, via {@link #notifyBuffersDeleted(int, IntBuffer)} or {@link #notifyBuffersDeleted(int, int[], int)}</li>
     *   <li>{@link GL#glBufferData(int, long, java.nio.Buffer, int)} - storage recreation via target</li>
     *   <li>{@link GL2#glNamedBufferDataEXT(int, long, java.nio.Buffer, int)} - storage recreation, direct</li>
     *   <li>{@link GL4#glNamedBufferData(int, long, java.nio.Buffer, int)} - storage recreation, direct</li>
     *   <li>{@link GL4#glBufferStorage(int, long, Buffer, int)} - storage recreation via target</li>
     *   <li>{@link GL4#glNamedBufferStorage(int, long, Buffer, int)} - storage recreation, direct</li>
     * </ul>
     * </p>
     * <p>
     * GL buffer storage is mapped via
     * <ul>
     *   <li>{@link GL#mapBuffer(int, int)}</li>
     *   <li>{@link GL#mapBufferRange(int, long, long, int)}</li>
     *   <li>{@link GL2#mapNamedBufferEXT(int, int)}</li>
     *   <li>{@link GL2#mapNamedBufferRangeEXT(int, long, long, int)}</li>
     *   <li>{@link GL4#mapNamedBuffer(int, int)}</li>
     *   <li>{@link GL4#mapNamedBufferRange(int, long, long, int)}</li>
     * </ul>
     * </p>
     * <p>
     * GL buffer storage is unmapped via
     * <ul>
     *   <li>{@link GL#glDeleteBuffers(int, IntBuffer)} - buffer deletion</li>
     *   <li>{@link GL#glUnmapBuffer(int)} - explicit via target</li>
     *   <li>{@link GL2#glUnmapNamedBufferEXT(int)} - explicit direct</li>
     *   <li>{@link GL4#glUnmapNamedBuffer(int)} - explicit direct</li>
     *   <li>{@link GL#glBufferData(int, long, java.nio.Buffer, int)} - storage recreation via target</li>
     *   <li>{@link GL2#glNamedBufferDataEXT(int, long, java.nio.Buffer, int)} - storage recreation, direct</li>
     *   <li>{@link GL4#glNamedBufferData(int, long, java.nio.Buffer, int)} - storage recreation, direct</li>
     *   <li>{@link GL4#glBufferStorage(int, long, Buffer, int)} - storage creation via target</li>
     *   <li>{@link GL4#glNamedBufferStorage(int, long, Buffer, int)} - storage creation, direct</li>
     * </ul>
     * </p>
     */
    class GLMappedBuffer {
      private:
        GLuint m_name;
        glmemsize_t m_size;
        GLbitfield m_mutableUsage;
        GLbitfield m_immutableFlags;
        void* m_mappedBuffer;
        
      public:
        GLMappedBuffer(GLuint name, glmemsize_t size, GLbitfield mutableUsage, GLbitfield immutableFlags) noexcept {
            m_name = name;
            m_size = size;
            m_mutableUsage = mutableUsage;
            m_immutableFlags = immutableFlags;
            m_mappedBuffer = nullptr;
        }

        void reset(glmemsize_t size, GLbitfield mutableUsage, GLbitfield immutableFlags) noexcept {
            m_name = 0;
            m_size = size;
            m_mutableUsage = mutableUsage;
            m_immutableFlags = immutableFlags;
            m_mappedBuffer = nullptr;
        }
        void setMappedBuffer(void* buffer) noexcept {
            m_mappedBuffer = buffer;
        }

        /** Return the buffer name */
        GLuint name() const noexcept { return m_name; }

        /** Return the buffer's storage size. */
        glmemsize_t size() const noexcept { return m_size; }

        /**
         * Returns <code>true</code> if buffer's storage is mutable, i.e.
         * created via {@link GL#glBufferData(int, long, java.nio.Buffer, int)}.
         * <p>
         * Returns <code>false</code> if buffer's storage is immutable, i.e.
         * created via {@link GL4#glBufferStorage(int, long, Buffer, int)}.
         * </p>
         * @return
         */
        bool isMutableStorage() const noexcept { return 0 != m_mutableUsage; }

        /**
         * Returns the mutable storage usage or 0 if storage is not {@link #isMutableStorage() mutable}.
         */
        GLbitfield mutableUsage() const noexcept { return m_mutableUsage; }

        /**
         * Returns the immutable storage flags, invalid if storage is {@link #isMutableStorage() mutable}.
         */
        GLbitfield immutableFlags() const noexcept { return m_immutableFlags; }

        /**
         * Returns the mapped ByteBuffer, or null if not mapped.
         * Mapping may occur via:
         * <ul>
         *   <li>{@link GL#glMapBuffer(int, int)}</li>
         *   <li>{@link GL#glMapBufferRange(int, long, long, int)}</li>
         *   <li>{@link GL2#glMapNamedBufferEXT(int, int)}</li>
         *   <li>{@link GL2#glMapNamedBufferRangeEXT(int, long, long, int)}
         * </ul>
         */
        const void* data() const noexcept { return m_mappedBuffer; }
        
        void* data() noexcept { return m_mappedBuffer; }

        std::string toString(bool skipMappedBuffer=false) const {
            std::string s0;
            if( isMutableStorage() ) {
                s0 = jau::format_string("GLMappedBuffer[name %u, size %zu, mutable usage 0x%X", m_name, m_size, m_mutableUsage);
            } else {
                s0 = jau::format_string("GLMappedBuffer[name %u, size %zu, immutable flags 0x%X", m_name, m_size, m_immutableFlags);
            }
            if(skipMappedBuffer) {
                return s0.append("]");
            } else {
                return s0.append(", mapped ").append(jau::to_hexstring(m_mappedBuffer)).append("]");
            }
        }
    };
    typedef std::unique_ptr<GLMappedBuffer> GLMappedBufferPtr;
    
    /**@}*/
}

#endif // GAMP_GLMAPPEDBUFFER_HPP_
