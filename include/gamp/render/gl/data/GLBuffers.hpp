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

#ifndef GAMP_GLBUFFERS_HPP_
#define GAMP_GLBUFFERS_HPP_

#include <GL/gl.h>
#include <gamp/render/gl/GLTypes.hpp>

namespace gamp::render::gl::data {
    using namespace gamp::render::gl;

    /** \addtogroup Gamp_GLData
     *
     *  @{
     */

    /** Compatible with ssize_t */
    typedef GLsizeiptr glmemsize_t;

    /**
     * OpenGL buffer related routines.
     */
    class GLBuffers {
      public:
        /**
         * @param glType GL primitive type
         * @return false if one of GL primitive unsigned types, otherwise true
         *              GL_UNSIGNED_BYTE, <br/>
         *              GL_UNSIGNED_SHORT, <br/>
         *              GL_UNSIGNED_INT, <br/>
         *              GL_HILO16_NV <br/>
         */
        constexpr static bool isSignedGLType(GLenum glType) {
            switch (glType) { // 29
                case GL_UNSIGNED_BYTE:
                case GL_UNSIGNED_SHORT:
                case GL_UNSIGNED_INT:
                case GL_HILO16_NV: // GL2
                    return false;
                default: return true;

            }
        }

        /**
         * @param glType GL primitive type
         * @return false if one of GL primitive floating point types, otherwise true
         *              GL_FLOAT, <br/>
         *              GL_HALF_FLOAT, <br/>
         *              GL_HALF_FLOAT_OES, <br/>
         *              GL_DOUBLE <br/>
         */
        constexpr static bool isGLTypeFixedPoint(GLenum glType) {
            switch(glType) {
                case GL_FLOAT:
                case GL_HALF_FLOAT:
                // case GL_HALF_FLOAT_OES: // GLES2
                case GL_DOUBLE:  // GL2GL3
                    return false;

                default:
                    return true;
            }
        }

        /**
         * @param glType shall be one of (31) <br/>
         *              GL_BYTE, GL_UNSIGNED_BYTE, <br/>
         *              GL_UNSIGNED_BYTE_3_3_2, GL_UNSIGNED_BYTE_2_3_3_REV, <br/>
         *              <br/>
         *              GL_SHORT, GL_UNSIGNED_SHORT, <br/>
         *              GL_UNSIGNED_SHORT_5_6_5, GL_UNSIGNED_SHORT_5_6_5_REV, <br/>
         *              GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_SHORT_4_4_4_4_REV, <br/>
         *              GL_UNSIGNED_SHORT_5_5_5_1, GL_UNSIGNED_SHORT_1_5_5_5_REV, <br/>
         *              GL_UNSIGNED_SHORT_8_8_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, <br/>
         *              GL.GL_HALF_FLOAT, GLES2.GL_HALF_FLOAT_OES: <br/>
         *              <br/>
         *              GL_FIXED, GL_INT <br/>
         *              GL_UNSIGNED_INT, GL_UNSIGNED_INT_8_8_8_8, <br/>
         *              GL_UNSIGNED_INT_8_8_8_8_REV, GL_UNSIGNED_INT_10_10_10_2, <br/>
         *              GL_UNSIGNED_INT_2_10_10_10_REV, GL_UNSIGNED_INT_24_8, <br/>
         *              GL_UNSIGNED_INT_10F_11F_11F_REV, GL_UNSIGNED_INT_5_9_9_9_REV <br/>
         *              GL_HILO16_NV, GL_SIGNED_HILO16_NV <br/>
         *              <br/>
         *              GL2GL3.GL_FLOAT_32_UNSIGNED_INT_24_8_REV <br/>
         *              <br/>
         *              GL_FLOAT, GL_DOUBLE <br/>
         *
         * @return 0 if glType is unhandled, otherwise the actual value > 0
         */
        constexpr static GLsizei sizeOfGLType(GLenum glType) {
            switch (glType) { // 29
                // case GL2.GL_BITMAP:
                case GL_BYTE:
                case GL_UNSIGNED_BYTE:
                case GL_UNSIGNED_BYTE_3_3_2: // GL2GL3
                case GL_UNSIGNED_BYTE_2_3_3_REV: // GL2GL3
                    return sizeof(uint8_t);

                case GL_SHORT:
                case GL_UNSIGNED_SHORT:
                case GL_UNSIGNED_SHORT_5_6_5:
                case GL_UNSIGNED_SHORT_5_6_5_REV: // GL2GL3
                case GL_UNSIGNED_SHORT_4_4_4_4:
                case GL_UNSIGNED_SHORT_4_4_4_4_REV: // GL2GL3
                case GL_UNSIGNED_SHORT_5_5_5_1:
                case GL_UNSIGNED_SHORT_1_5_5_5_REV: // GL2GL3
                case GL_UNSIGNED_SHORT_8_8_APPLE: // GL2
                case GL_UNSIGNED_SHORT_8_8_REV_APPLE: // GL2
                case GL_HALF_FLOAT:
                // case GL_HALF_FLOAT_OES: // GLES2
                    return sizeof(uint16_t);

                case GL_FIXED:
                case GL_INT: // GL2ES2
                case GL_UNSIGNED_INT:
                case GL_UNSIGNED_INT_8_8_8_8: // GL2GL3
                case GL_UNSIGNED_INT_8_8_8_8_REV: // GL2GL3
                case GL_UNSIGNED_INT_10_10_10_2: // GL2ES2
                case GL_UNSIGNED_INT_2_10_10_10_REV: // GL2ES2
                case GL_UNSIGNED_INT_24_8:
                case GL_UNSIGNED_INT_10F_11F_11F_REV:
                case GL_UNSIGNED_INT_5_9_9_9_REV: // GL2ES3
                case GL_HILO16_NV: // GL2
                case GL_SIGNED_HILO16_NV: // GL2
                    return sizeof(uint32_t);

                case GL_FLOAT_32_UNSIGNED_INT_24_8_REV: // GL2ES3
                    return sizeof(uint64_t);

                case GL_FLOAT:
                    return sizeof(jau::float32_t);

                case GL_DOUBLE: // GL2GL3
                    return sizeof(jau::float64_t);

                default: return 0;
            }
        }

#if 0
        /**
         * @param glType shall be one of (31) <br/>
         *              GL_BYTE, GL_UNSIGNED_BYTE, <br/>
         *              GL_UNSIGNED_BYTE_3_3_2, GL_UNSIGNED_BYTE_2_3_3_REV, <br/>
         *              <br/>
         *              GL_SHORT, GL_UNSIGNED_SHORT, <br/>
         *              GL_UNSIGNED_SHORT_5_6_5, GL_UNSIGNED_SHORT_5_6_5_REV, <br/>
         *              GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_SHORT_4_4_4_4_REV, <br/>
         *              GL_UNSIGNED_SHORT_5_5_5_1, GL_UNSIGNED_SHORT_1_5_5_5_REV, <br/>
         *              GL_UNSIGNED_SHORT_8_8_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, <br/>
         *              GL_HALF_FLOAT, GL_HALF_FLOAT_OES <br/>
         *              <br/>
         *              GL_FIXED, GL_INT <br/>
         *              GL_UNSIGNED_INT, GL_UNSIGNED_INT_8_8_8_8, <br/>
         *              GL_UNSIGNED_INT_8_8_8_8_REV, GL_UNSIGNED_INT_10_10_10_2, <br/>
         *              GL_UNSIGNED_INT_2_10_10_10_REV, GL_UNSIGNED_INT_24_8, <br/>
         *              GL_UNSIGNED_INT_10F_11F_11F_REV, GL_UNSIGNED_INT_5_9_9_9_REV <br/>
         *              GL_HILO16_NV, GL_SIGNED_HILO16_NV <br/>
         *              <br/>
         *              GL_FLOAT_32_UNSIGNED_INT_24_8_REV <br/>
         *              <br/>
         *              GL_FLOAT, GL_DOUBLE <br/>
         *
         * @return null if glType is unhandled, otherwise the new Buffer object
         */
        static glbuffer_ref newGLBuffer(GLenum glType, glmemsize_t numElements) {
            switch (glType) { // 29
                case GL_BYTE:
                    return jau::DataBuffer<int8_t, glmemsize_t>::create(numElements);

                case GL_UNSIGNED_BYTE:
                case GL_UNSIGNED_BYTE_3_3_2: // GL2GL3
                case GL_UNSIGNED_BYTE_2_3_3_REV: // GL2GL3
                    return jau::DataBuffer<uint8_t, glmemsize_t>::create(numElements);

                case GL_SHORT:
                    return jau::DataBuffer<int16_t, glmemsize_t>::create(numElements);

                case GL_UNSIGNED_SHORT:
                case GL_UNSIGNED_SHORT_5_6_5:
                case GL_UNSIGNED_SHORT_5_6_5_REV: // GL2GL3
                case GL_UNSIGNED_SHORT_4_4_4_4:
                case GL_UNSIGNED_SHORT_4_4_4_4_REV: // GL2GL3
                case GL_UNSIGNED_SHORT_5_5_5_1:
                case GL_UNSIGNED_SHORT_1_5_5_5_REV: // GL2GL3
                case GL_UNSIGNED_SHORT_8_8_APPLE: // GL2
                case GL_UNSIGNED_SHORT_8_8_REV_APPLE: // GL2
                case GL_HALF_FLOAT:
                // case GL_HALF_FLOAT_OES: // GLES2
                    return jau::DataBuffer<uint16_t, glmemsize_t>::create(numElements);

                case GL_INT: // GL2ES2
                    return jau::DataBuffer<int32_t, glmemsize_t>::create(numElements);

                case GL_FIXED:
                case GL_SIGNED_HILO16_NV: // GL2
                case GL_UNSIGNED_INT:
                case GL_UNSIGNED_INT_8_8_8_8: // GL2GL3
                case GL_UNSIGNED_INT_8_8_8_8_REV: // GL2GL3
                case GL_UNSIGNED_INT_10_10_10_2: // GL2ES2
                case GL_UNSIGNED_INT_2_10_10_10_REV: // GL2ES2
                case GL_UNSIGNED_INT_24_8:
                case GL_UNSIGNED_INT_10F_11F_11F_REV:
                case GL_UNSIGNED_INT_5_9_9_9_REV: // GL2GL3
                case GL_HILO16_NV: // GL2
                    return jau::DataBuffer<uint32_t, glmemsize_t>::create(numElements);

                case GL_FLOAT_32_UNSIGNED_INT_24_8_REV: // GL2ES3
                    return jau::DataBuffer<uint64_t, glmemsize_t>::create(numElements);

                case GL_FLOAT:
                    return jau::DataBuffer<jau::float32_t, glmemsize_t>::create(numElements);

                case GL_DOUBLE: // GL2GL3
                    return jau::DataBuffer<jau::float64_t, glmemsize_t>::create(numElements);

                default: return nullptr;
            }
        }

        /**
         * @param glType shall be one of (31) <br/>
         *              GL_BYTE, GL_UNSIGNED_BYTE, <br/>
         *              GL_UNSIGNED_BYTE_3_3_2, GL_UNSIGNED_BYTE_2_3_3_REV, <br/>
         *              <br/>
         *              GL_SHORT, GL_UNSIGNED_SHORT, <br/>
         *              GL_UNSIGNED_SHORT_5_6_5, GL_UNSIGNED_SHORT_5_6_5_REV, <br/>
         *              GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_SHORT_4_4_4_4_REV, <br/>
         *              GL_UNSIGNED_SHORT_5_5_5_1, GL_UNSIGNED_SHORT_1_5_5_5_REV, <br/>
         *              GL_UNSIGNED_SHORT_8_8_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, <br/>
         *              GL_HALF_FLOAT, GL_HALF_FLOAT_OES <br/>
         *              <br/>
         *              GL_FIXED, GL_INT <br/>
         *              GL_UNSIGNED_INT, GL_UNSIGNED_INT_8_8_8_8, <br/>
         *              GL_UNSIGNED_INT_8_8_8_8_REV, GL_UNSIGNED_INT_10_10_10_2, <br/>
         *              GL_UNSIGNED_INT_2_10_10_10_REV, GL_UNSIGNED_INT_24_8, <br/>
         *              GL_UNSIGNED_INT_10F_11F_11F_REV, GL_UNSIGNED_INT_5_9_9_9_REV <br/>
         *              GL_HILO16_NV, GL_SIGNED_HILO16_NV <br/>
         *              <br/>
         *              GL_FLOAT_32_UNSIGNED_INT_24_8_REV <br/>
         *              <br/>
         *              GL_FLOAT, GL_DOUBLE <br/>
         * @return null if glType is unhandled or parent is null or bufLen is 0, otherwise the new Buffer object
         */
        static glbuffer_ref sliceGLBuffer(const glbuffer_ref& parent, glmemsize_t pos, glmemsize_t length, GLenum glType) {
            if (!parent || length == 0) {
                return nullptr;
            }
            switch (glType) { // 29
                case GL_BYTE:
                case GL_UNSIGNED_BYTE:
                case GL_UNSIGNED_BYTE_3_3_2: // GL2GL3
                case GL_UNSIGNED_BYTE_2_3_3_REV: // GL2GL3
                    return parent->slice(pos, length); // slice and duplicate

                case GL_SHORT:
                case GL_UNSIGNED_SHORT:
                case GL_UNSIGNED_SHORT_5_6_5:
                case GL_UNSIGNED_SHORT_5_6_5_REV: // GL2GL3
                case GL_UNSIGNED_SHORT_4_4_4_4:
                case GL_UNSIGNED_SHORT_4_4_4_4_REV: // GL2GL3
                case GL_UNSIGNED_SHORT_5_5_5_1:
                case GL_UNSIGNED_SHORT_1_5_5_5_REV: // GL2GL3
                case GL_UNSIGNED_SHORT_8_8_APPLE: // GL2
                case GL_UNSIGNED_SHORT_8_8_REV_APPLE: // GL2
                case GL_HALF_FLOAT:
                // case GL_HALF_FLOAT_OES: // GLES2
                    return parent->slice(pos, length); // .asShortBuffer(); // slice and duplicate

                case GL_FIXED:
                case GL_INT: // GL2ES2
                case GL_UNSIGNED_INT:
                case GL_UNSIGNED_INT_8_8_8_8: // GL2GL3
                case GL_UNSIGNED_INT_8_8_8_8_REV: // GL2GL3
                case GL_UNSIGNED_INT_10_10_10_2: // GL2ES2
                case GL_UNSIGNED_INT_2_10_10_10_REV: // GL2ES2
                case GL_UNSIGNED_INT_24_8:
                case GL_UNSIGNED_INT_10F_11F_11F_REV:
                case GL_HILO16_NV: // GL2
                case GL_UNSIGNED_INT_5_9_9_9_REV: // GL2ES3
                case GL_SIGNED_HILO16_NV: // GL2
                    return parent->slice(pos, length); // .asIntBuffer(); // slice and duplicate may change byte order

                case GL_FLOAT_32_UNSIGNED_INT_24_8_REV: // GL2ES3
                    return parent->slice(pos, length); // .asLongBuffer(); // slice and duplicate may change byte order
                    break;

                case GL_FLOAT:
                    return parent->slice(pos, length); // .asFloatBuffer(); // slice and duplicate may change byte order

                case GL_DOUBLE: // GL2GL3
                    return parent->slice(pos, length); // .asDoubleBuffer(); // slice and duplicate may change byte order

                default: return nullptr;
            }

        }

#endif

        static GLint glGetInteger(GLenum pname) {
            int tmp[] = { 0 };
            glGetIntegerv(pname, tmp);
            return tmp[0];
        }

        /**
         * Returns the number of bytes required to read/write a memory buffer via OpenGL
         * using the current GL pixel storage state and the given parameters.
         *
         * <p>This method is security critical, hence it throws an exception (fail-fast)
         * in case of an invalid alignment. In case we forgot to handle
         * proper values, please contact the maintainer.</p>
         *
         * @param glp the current GLProfile
         * @param bytesPerPixel bytes per pixel, i.e. via {@link #bytesPerPixel(int, int)}.
         * @param width in pixels
         * @param height in pixels
         * @param depth in pixels
         * @param pack true for read mode GPU -> CPU (pack), otherwise false for write mode CPU -> GPU (unpack)
         * @return required minimum size of the buffer in bytes
         * @throws GLException if alignment is invalid. Please contact the maintainer if this is our bug.
         */
        static glmemsize_t byteSize(const GL& gl,
                                    GLsizei bytesPerPixel, GLsizei width, GLsizei height, GLsizei depth,
                                    bool pack) {
            glmemsize_t width2 = 0;
            glmemsize_t rowLength = 0;
            glmemsize_t skipRows = 0;
            glmemsize_t skipPixels = 0;
            GLsizei     alignment = 1;
            glmemsize_t imageHeight = 0;
            GLsizei     skipImages = 0;

            if (pack) {
              alignment = glGetInteger(GL_PACK_ALIGNMENT);                   // es2, es3, gl3
              if( gl.isGL2ES3() ) {
                  rowLength = glGetInteger(GL_PACK_ROW_LENGTH);          // es3, gl3
                  skipRows = glGetInteger(GL_PACK_SKIP_ROWS);            // es3, gl3
                  skipPixels = glGetInteger(GL_PACK_SKIP_PIXELS);        // es3, gl3
                  if (depth > 1 && gl.isGL2GL3() && gl.version() >= Version1_2) {
                      imageHeight = glGetInteger(GL_PACK_IMAGE_HEIGHT);  // gl3, GL_VERSION_1_2
                      skipImages = glGetInteger(GL_PACK_SKIP_IMAGES);    // gl3, GL_VERSION_1_2
                  }
              }
            } else {
              alignment = glGetInteger(GL_UNPACK_ALIGNMENT);                 // es2, es3, gl3
              if( gl.isGL2ES3() ) {
                  rowLength = glGetInteger(GL_UNPACK_ROW_LENGTH);        // es3, gl3
                  skipRows = glGetInteger(GL_UNPACK_SKIP_ROWS);          // es3, gl3
                  skipPixels = glGetInteger(GL_UNPACK_SKIP_PIXELS);      // es3, gl3
                  if( depth > 1 &&
                      ( gl.isGL3ES3() ||
                        ( gl.isGL2GL3() && gl.version() >= Version1_2 )
                      )
                    ) {
                      imageHeight = glGetInteger(GL_UNPACK_IMAGE_HEIGHT);// es3, gl3, GL_VERSION_1_2
                      skipImages = glGetInteger(GL_UNPACK_SKIP_IMAGES);  // es3, gl3, GL_VERSION_1_2
                  }
              }
            }

            // Try to deal somewhat correctly with potentially invalid values
            width2      = std::max<glmemsize_t>(0, width);
            height      = std::max<GLsizei>(1, height); // min 1D
            depth       = std::max<GLsizei>(1, depth ); // min 1 * imageSize
            skipRows    = std::max<glmemsize_t>(0, skipRows);
            skipPixels  = std::max<glmemsize_t>(0, skipPixels);
            alignment   = std::max<GLsizei>(1, alignment);
            skipImages  = std::max<GLsizei>(0, skipImages);

            imageHeight = ( imageHeight > 0 ) ? imageHeight : height;
            rowLength   = ( rowLength   > 0 ) ? rowLength   : width2;

            glmemsize_t rowLengthInBytes = rowLength  * bytesPerPixel;
            glmemsize_t skipBytes        = skipPixels * bytesPerPixel;

            switch(alignment) {
                case 1:
                    break;
                case 2:
                case 4:
                case 8: {
                        // x % 2n == x & (2n - 1)
                        glmemsize_t remainder = rowLengthInBytes & ( alignment - 1L );
                        if (remainder > 0) {
                            rowLengthInBytes += alignment - remainder;
                        }
                        remainder = skipBytes & ( alignment - 1L );
                        if (remainder > 0) {
                            skipBytes += alignment - remainder;
                        }
                    }
                    break;
                default:
                    throw GLException("Invalid alignment "+std::to_string(alignment)+", must be 2**n (1,2,4,8). Pls notify the maintainer in case this is our bug.", E_FILE_LINE);
            }

            /**
             * skipImages, depth, skipPixels and skipRows are static offsets.
             *
             * skipImages and depth are in multiples of image size.
             *
             * skipBytes and rowLengthInBytes are aligned
             *
             * rowLengthInBytes is the aligned byte offset
             * from line n to line n+1 at the same x-axis position.
             */
            return
                skipBytes +                                                  // aligned skipPixels * bpp
              ( skipImages + depth  - 1 ) * imageHeight * rowLengthInBytes + // aligned whole images
              ( skipRows   + height - 1 ) * rowLengthInBytes +               // aligned lines
                width2                    * bytesPerPixel;                   // last line
        }

        /**
         * Returns the number of bytes required to read/write a memory buffer via OpenGL
         * using the current GL pixel storage state and the given parameters.
         *
         * <p>This method is security critical, hence it throws an exception (fail-fast)
         * in case either the format, type or alignment is unhandled. In case we forgot to handle
         * proper values, please contact the maintainer.</p>
         *
         * <p> See {@link #bytesPerPixel(int, int)}. </p>
         *
         * @param gl the current GL object
         *
         * @param tmp a pass through integer array of size >= 1 used to store temp data (performance)
         *
         * @param format must be one of (27) <br/>
         *              GL_COLOR_INDEX GL_STENCIL_INDEX <br/>
         *              GL_DEPTH_COMPONENT GL_DEPTH_STENCIL <br/>
         *              GL_RED GL_RED_INTEGER <br/>
         *              GL_GREEN GL_GREEN_INTEGER <br/>
         *              GL_BLUE GL_BLUE_INTEGER <br/>
         *              GL_ALPHA GL_LUMINANCE (12) <br/>
         *              <br/>
         *              GL_LUMINANCE_ALPHA GL_RG <br/>
         *              GL_RG_INTEGER GL_HILO_NV <br/>
         *              GL_SIGNED_HILO_NV (5) <br/>
         *              <br/>
         *              GL_YCBCR_422_APPLE <br/>
         *              <br/>
         *              GL_RGB GL_RGB_INTEGER <br/>
         *              GL_BGR GL_BGR_INTEGER (4)<br/>
         *              <br/>
         *              GL_RGBA GL_RGBA_INTEGER <br/>
         *              GL_BGRA GL_BGRA_INTEGER <br/>
         *              GL_ABGR_EXT (5)<br/>
         *
         * @param type must be one of (32) <br/>
         *              GL_BITMAP, <br/>
         *              GL_BYTE, GL_UNSIGNED_BYTE, <br/>
         *              GL_UNSIGNED_BYTE_3_3_2, GL_UNSIGNED_BYTE_2_3_3_REV, <br/>
         *              <br/>
         *              GL_SHORT, GL_UNSIGNED_SHORT, <br/>
         *              GL_UNSIGNED_SHORT_5_6_5, GL_UNSIGNED_SHORT_5_6_5_REV, <br/>
         *              GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_SHORT_4_4_4_4_REV, <br/>
         *              GL_UNSIGNED_SHORT_5_5_5_1, GL_UNSIGNED_SHORT_1_5_5_5_REV, <br/>
         *              GL_UNSIGNED_SHORT_8_8_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, <br/>
         *              GL_HALF_FLOAT, GL_HALF_FLOAT_OES <br/>
         *              <br/>
         *              GL_FIXED, GL_INT <br/>
         *              GL_UNSIGNED_INT, GL_UNSIGNED_INT_8_8_8_8, <br/>
         *              GL_UNSIGNED_INT_8_8_8_8_REV, GL_UNSIGNED_INT_10_10_10_2, <br/>
         *              GL_UNSIGNED_INT_2_10_10_10_REV, GL_UNSIGNED_INT_24_8, <br/>
         *              GL_UNSIGNED_INT_10F_11F_11F_REV, GL_UNSIGNED_INT_5_9_9_9_REV <br/>
         *              GL_HILO16_NV, GL_SIGNED_HILO16_NV <br/>
         *              <br/>
         *              GL_FLOAT_32_UNSIGNED_INT_24_8_REV <br/>
         *              <br/>
         *              GL_FLOAT, GL_DOUBLE <br/>
         *
         * @param width in pixels
         * @param height in pixels
         * @param depth in pixels
         * @param pack true for read mode GPU -> CPU, otherwise false for write mode CPU -> GPU
         * @return required minimum size of the buffer in bytes
         * @throws GLException if format, type or alignment is not handled. Please contact the maintainer if this is our bug.
         */
        static size_t byteSize(const GL& gl,
                               GLenum format, GLenum type, GLsizei width, GLsizei height, GLsizei depth,
                               bool pack)
        {
            if (width < 0) return 0;
            if (height < 0) return 0;
            if (depth < 0) return 0;

            GLsizei bytesPerPixel_ = bytesPerPixel(format, type);
            return byteSize(gl, bytesPerPixel_, width, height, depth, pack);
        }

        /**
         * Returns the number of bytes required for one pixel with the the given OpenGL format and type.
         *
         * <p>This method is security critical, hence it throws an exception (fail-fast)
         * in case either the format, type or alignment is unhandled. In case we forgot to handle
         * proper values, please contact the maintainer.</p>
         *
         * <p> See {@link #componentCount(int)}. </p>
         *
         * @param format must be one of (27) <br/>
         *              GL_COLOR_INDEX GL_STENCIL_INDEX <br/>
         *              GL_DEPTH_COMPONENT GL_DEPTH_STENCIL <br/>
         *              GL_RED GL_RED_INTEGER <br/>
         *              GL_GREEN GL_GREEN_INTEGER <br/>
         *              GL_BLUE GL_BLUE_INTEGER <br/>
         *              GL_ALPHA GL_LUMINANCE (12) <br/>
         *              <br/>
         *              GL_LUMINANCE_ALPHA GL_RG <br/>
         *              GL_RG_INTEGER GL_HILO_NV <br/>
         *              GL_SIGNED_HILO_NV (5) <br/>
         *              <br/>
         *              GL_YCBCR_422_APPLE <br/>
         *              <br/>
         *              GL_RGB GL_RGB_INTEGER <br/>
         *              GL_BGR GL_BGR_INTEGER (4)<br/>
         *              <br/>
         *              GL_RGBA GL_RGBA_INTEGER <br/>
         *              GL_BGRA GL_BGRA_INTEGER <br/>
         *              GL_ABGR_EXT (5)<br/>
         *
         * @param type must be one of (32) <br/>
         *              GL_BITMAP, <br/>
         *              GL_BYTE, GL_UNSIGNED_BYTE, <br/>
         *              GL_UNSIGNED_BYTE_3_3_2, GL_UNSIGNED_BYTE_2_3_3_REV, <br/>
         *              <br/>
         *              GL_SHORT, GL_UNSIGNED_SHORT, <br/>
         *              GL_UNSIGNED_SHORT_5_6_5, GL_UNSIGNED_SHORT_5_6_5_REV, <br/>
         *              GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_SHORT_4_4_4_4_REV, <br/>
         *              GL_UNSIGNED_SHORT_5_5_5_1, GL_UNSIGNED_SHORT_1_5_5_5_REV, <br/>
         *              GL_UNSIGNED_SHORT_8_8_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, <br/>
         *              GL_HALF_FLOAT, GL_HALF_FLOAT_OES <br/>
         *              <br/>
         *              GL_FIXED, GL_INT <br/>
         *              GL_UNSIGNED_INT, GL_UNSIGNED_INT_8_8_8_8, <br/>
         *              GL_UNSIGNED_INT_8_8_8_8_REV, GL_UNSIGNED_INT_10_10_10_2, <br/>
         *              GL_UNSIGNED_INT_2_10_10_10_REV, GL_UNSIGNED_INT_24_8, <br/>
         *              GL_UNSIGNED_INT_10F_11F_11F_REV, GL_UNSIGNED_INT_5_9_9_9_REV <br/>
         *              GL_HILO16_NV, GL_SIGNED_HILO16_NV <br/>
         *              <br/>
         *              GL_FLOAT_32_UNSIGNED_INT_24_8_REV <br/>
         *              <br/>
         *              GL_FLOAT, GL_DOUBLE <br/>
         *
         * @return required size of one pixel in bytes
         * @throws GLException if format or type alignment is not handled. Please contact the maintainer if this is our bug.
         */
        static GLsizei bytesPerPixel(GLenum format, GLenum type) {
            GLsizei compSize = 0;

            GLsizei compCount = componentCount(format);

            switch (type) /* 30 */ {
                case GL_BITMAP: // GL2
                  if (GL_COLOR_INDEX == format || GL_STENCIL_INDEX == format) {
                      compSize = 1;
                  } else {
                      throw GLException("BITMAP type only supported for format COLOR_INDEX and STENCIL_INDEX, not "+jau::to_hexstring(format), E_FILE_LINE);
                  }
                  break;
                case GL_BYTE:
                case GL_UNSIGNED_BYTE:
                  compSize = 1;
                  break;
                case GL_SHORT:
                case GL_UNSIGNED_SHORT:
                case GL_HALF_FLOAT:
                // case GL_HALF_FLOAT_OES: // GLES2
                  compSize = 2;
                  break;
                case GL_FIXED:
                case GL_INT: // GL2ES2
                case GL_UNSIGNED_INT:
                case GL_FLOAT:
                  compSize = 4;
                  break;
                case GL_DOUBLE: // GL2GL3
                  compSize = 8;
                  break;

                case GL_UNSIGNED_BYTE_3_3_2: // GL2GL3
                case GL_UNSIGNED_BYTE_2_3_3_REV: // GL2GL3
                  compSize = 1;
                  compCount = 1;
                  break;
                case GL_UNSIGNED_SHORT_5_6_5:
                case GL_UNSIGNED_SHORT_5_6_5_REV: // GL2GL3
                case GL_UNSIGNED_SHORT_4_4_4_4:
                case GL_UNSIGNED_SHORT_4_4_4_4_REV: // GL2GL3
                case GL_UNSIGNED_SHORT_5_5_5_1:
                case GL_UNSIGNED_SHORT_1_5_5_5_REV: // GL2GL3
                case GL_UNSIGNED_SHORT_8_8_APPLE: // GL2
                case GL_UNSIGNED_SHORT_8_8_REV_APPLE: // GL2
                  compSize = 2;
                  compCount = 1;
                  break;
                case GL_HILO16_NV:  // GL2
                case GL_SIGNED_HILO16_NV: // GL2
                  compSize = 2;
                  compCount = 2;
                  break;
                case GL_UNSIGNED_INT_8_8_8_8: // GL2GL3
                case GL_UNSIGNED_INT_8_8_8_8_REV: // GL2GL3
                case GL_UNSIGNED_INT_10_10_10_2: // GL2ES2
                case GL_UNSIGNED_INT_2_10_10_10_REV: // GL2ES2
                case GL_UNSIGNED_INT_24_8:
                case GL_UNSIGNED_INT_10F_11F_11F_REV:
                case GL_UNSIGNED_INT_5_9_9_9_REV: // GL2ES3
                  compSize = 4;
                  compCount = 1;
                  break;
                case GL_FLOAT_32_UNSIGNED_INT_24_8_REV: // GL2ES3
                  compSize = 8;
                  compCount = 1;
                  break;

                default:
                  throw GLException("type "+jau::to_hexstring(type)+"/"+"format "+jau::to_hexstring(format)+" not supported [yet], pls notify the maintainer in case this is our bug.", E_FILE_LINE);
            }
            return compCount * compSize;
        }

        /**
         * Returns the number of components required for the given OpenGL format.
         *
         * <p>This method is security critical, hence it throws an exception (fail-fast)
         * in case either the format, type or alignment is unhandled. In case we forgot to handle
         * proper values, please contact the maintainer.</p>
         *
         * @param format must be one of (27) <br/>
         *              GL_COLOR_INDEX GL_STENCIL_INDEX <br/>
         *              GL_DEPTH_COMPONENT GL_DEPTH_STENCIL <br/>
         *              GL_RED GL_RED_INTEGER <br/>
         *              GL_GREEN GL_GREEN_INTEGER <br/>
         *              GL_BLUE GL_BLUE_INTEGER <br/>
         *              GL_ALPHA GL_LUMINANCE (12) <br/>
         *              <br/>
         *              GL_LUMINANCE_ALPHA GL_RG <br/>
         *              GL_RG_INTEGER GL_HILO_NV <br/>
         *              GL_SIGNED_HILO_NV (5) <br/>
         *              <br/>
         *              GL_YCBCR_422_APPLE <br/>
         *              <br/>
         *              GL_RGB GL_RGB_INTEGER <br/>
         *              GL_BGR GL_BGR_INTEGER (4)<br/>
         *              <br/>
         *              GL_RGBA GL_RGBA_INTEGER <br/>
         *              GL_BGRA GL_BGRA_INTEGER <br/>
         *              GL_ABGR_EXT (5)<br/>
         *
         * @return number of components required for the given OpenGL format
         * @throws GLException if format is not handled. Please contact the maintainer if this is our bug.
         */
        static GLsizei componentCount(GLenum format) {
            GLint compCount;

            switch (format) /* 26 */ {
                case GL_COLOR_INDEX: // GL2
                case GL_STENCIL_INDEX: // GL2ES2
                case GL_DEPTH_COMPONENT: // GL2ES2
                case GL_DEPTH_STENCIL:
                case GL_RED: // GL2ES2
                case GL_RED_INTEGER: // GL2ES3
                case GL_GREEN: // GL2ES3
                case GL_GREEN_INTEGER: // GL2GL3
                case GL_BLUE: // GL2ES3
                case GL_BLUE_INTEGER: // GL2GL3
                case GL_ALPHA:
                case GL_LUMINANCE:
                  compCount = 1;
                  break;
                case GL_LUMINANCE_ALPHA:
                case GL_RG: // GL2ES2
                case GL_RG_INTEGER: // GL2ES3
                case GL_HILO_NV: // GL2
                case GL_SIGNED_HILO_NV: // GL2
                  compCount = 2;
                  break;
                case GL_RGB:
                case GL_RGB_INTEGER: // GL2ES3
                case GL_BGR:
                case GL_BGR_INTEGER: // GL2GL3
                  compCount = 3;
                  break;
                case GL_YCBCR_422_APPLE: // GL2
                  compCount = 3;
                  break;
                case GL_RGBA:
                case GL_RGBA_INTEGER: // GL2ES3
                case GL_BGRA:
                case GL_BGRA_INTEGER: // GL2GL3
                case GL_ABGR_EXT: // GL2
                  compCount = 4;
                  break;
                /* FIXME ??
                 case GL_HILO_NV:
                  elements = 2;
                  break; */
                default:
                  throw GLException("format "+jau::to_hexstring(format)+" not supported [yet], pls notify the maintainer in case this is our bug.", E_FILE_LINE);
            }
            return compCount;
        }

    /**@}*/

    };
}

#endif //  GAMP_GLBUFFERS_HPP_
