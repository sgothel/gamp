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

#ifndef GAMP_GLARRAYHANDLERFLAT_HPP_
#define GAMP_GLARRAYHANDLERFLAT_HPP_

#include <gamp/renderer/gl/data/glarraydata.hpp>

namespace gamp::render::gl::data::impl {
    using namespace gamp::render::gl;
    using namespace gamp::render::gl::data;

    /** \addtogroup Gamp_GLImpl
     *
     *  @{
     */

    /**
     * Handles consistency of interleaved array state.
     */
    class GLSubArrayHandler {
      public:
        virtual ~GLSubArrayHandler() noexcept  = default;
        /**
         * Implementation shall associate the data with the array
         *
         * @param gl current GL object
         * @param ext extension object allowing passing of an implementation detail
         */
        virtual void syncData(const GL &gl, ShaderState* st) = 0;

        /**
         * Implementation shall enable or disable the array state.
         *
         * @param gl current GL object
         * @param enable true if array shall be enabled, otherwise false.
         * @param ext extension object allowing passing of an implementation detail
         */
        virtual void enableState(const GL &gl, bool enable, ShaderState* st) = 0;

        virtual const GLArrayDataRef& data() = 0;
    };

    /**@}*/
}  // namespace gamp::render::gl::data::impl

#endif /* GAMP_GLARRAYHANDLERFLAT_HPP_ */
