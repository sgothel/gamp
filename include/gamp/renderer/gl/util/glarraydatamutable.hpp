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

#ifndef GAMP_GLARRAYDATAMUTABLE_HPP_
#define GAMP_GLARRAYDATAMUTABLE_HPP_

#include <gamp/renderer/gl/glarraydata.hpp>

namespace gamp::render::gl::util {
    using namespace gamp::render::gl;

    /**
     *
     * The total number of bytes hold by the referenced buffer is:
     * getComponentSize()* getComponentNumber() * getElementNumber()
     *
     */
    class GLArrayDataMutable : public GLArrayData {
      public:
        virtual bool enabled() = 0;

        /**
         * Is the buffer written to the VBO ?
         */
        virtual bool isVBOWritten() = 0;

        /**
         * Marks the buffer written to the VBO
         */
        virtual void setVBOWritten(bool written) = 0;

        //
        // Data and GL state modification ..
        //

        void destroy(const GL& gl) noexcept override = 0;

        /**
         * Clears this buffer and resets states accordingly.
         * <p>
         * Implementation calls {@link #seal(const GL&, bool) seal(gl, false)} and {@link #clear()},
         * i.e. turns-off the const GL& buffer and then clearing it.
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
         * @see #seal(const GL&, bool)
         * @see #clear()
         */
        virtual void clear(const GL& gl) = 0;

        /**
         * Convenience method calling {@link #seal(bool)} and {@link #enableBuffer(const GL&, bool)}.
         *
         * @see #seal(bool)
         * @see #enableBuffer(const GL&, bool)
         *
         */
        virtual void seal(const GL& gl, bool seal) = 0;

        /**
         * Enables the buffer if <code>enable</code> is <code>true</code>,
         * and transfers the data if required.
         * In case {@link #isVBO() VBO is used}, it is bound accordingly for the data transfer and association,
         * i.e. it issued {@link #bindBuffer(const GL&, bool)}.
         * The VBO buffer is unbound when the method returns.
         * <p>
         * Disables the buffer if <code>enable</code> is <code>false</code>.
         * </p>
         *
         * <p>The action will only be executed,
         * if the internal enable state differs,
         * or 'setEnableAlways' was called with 'true'.</b>
         *
         * <p>It is up to the user to enable/disable the array properly,
         * ie in case of multiple data sets for the same vertex attribute (VA).
         * Meaning in such case usage of one set while expecting another one
         * to be used for the same VA implies decorating each usage with enable/disable.</p>
         *
         * @see #setEnableAlways(bool)
         */
        virtual void enableBuffer(const GL& gl, bool enable) = 0;

        /**
         * if <code>bind</code> is true and the data uses {@link #isVBO() VBO},
         * the latter will be bound and data written to the GPU if required.
         * <p>
         * If  <code>bind</code> is false and the data uses {@link #isVBO() VBO},
         * the latter will be unbound.
         * </p>
         * <p>
         * This method is exposed to allow data VBO arrays, i.e. {@link const GL&#const GL&_ELEMENT_ARRAY_BUFFER},
         * to be bounded and written while keeping the VBO bound. The latter is in contrast to {@link #enableBuffer(const GL&, bool)},
         * which leaves the VBO unbound, since it's not required for vertex attributes or pointers.
         * </p>
         *
         * @param gl current const GL& object
         * @param bind true if VBO shall be bound and data written,
         *        otherwise clear VBO binding.
         * @return true if data uses VBO and action was performed, otherwise false
         */
        virtual bool bindBuffer(const GL& gl, bool bind) = 0;

        /**
         * Affects the behavior of 'enableBuffer'.
         *
         * The default is 'false'
         *
         * This is useful when you mix up
         * const GL&ArrayData usage with conventional const GL& array calls
         * or in case of a buggy const GL& VBO implementation.
         *
         * @see #enableBuffer(const GL&, bool)
         */
        virtual void setEnableAlways(bool always) = 0;

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
         * @see #clear(const GL&)
         */
        virtual void clear() = 0;

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
        virtual void seal(bool seal) = 0;

        /**
         * Rewinds this buffer. The position is set to zero and the mark is discarded.
         * <p>
         * Invoke this method before a sequence of put or get operations.
         * </p>
         */
        virtual void rewind() = 0;

        virtual void padding(int doneInByteSize) = 0;
        virtual void put(const buffer_ref& v) = 0;

        virtual void putb(uint8_t v) = 0;
        virtual void put3b(uint8_t v1, uint8_t v2, uint8_t v3) = 0;
        virtual void put4b(uint8_t v1, uint8_t v2, uint8_t v3, uint8_t v4) = 0;
        virtual void putb(uint8_t src[], int offset, int length) = 0;

        virtual void puts(uint16_t v) = 0;
        virtual void put3s(uint16_t v1, uint16_t v2, uint16_t v3) = 0;
        virtual void put4s(uint16_t v1, uint16_t v2, uint16_t v3, uint16_t v4) = 0;
        virtual void puts(uint16_t src[], int offset, int length) = 0;

        virtual void puti(int v) = 0;
        virtual void put3i(int v1, int v2, int v3) = 0;
        virtual void put4i(int v1, int v2, int v3, int v4) = 0;
        virtual void puti(int src[], int offset, int length) = 0;

        virtual void putx(int v) = 0;

        virtual void putf(float v) = 0;
        virtual void put3f(float v1, float v2, float v3) = 0;
        virtual void put4f(float v1, float v2, float v3, float v4) = 0;
        virtual void putf(float src[], int offset, int length) = 0;
    };


}  // namespace gamp::render::gl::util

#endif /* GAMP_GLARRAYDATAMUTABLE_HPP_ */
