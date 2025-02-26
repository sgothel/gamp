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

#ifndef GAMP_GLDATAARRAYHANDLER_HPP_
#define GAMP_GLDATAARRAYHANDLER_HPP_

#include <jau/basic_types.hpp>
#include <gamp/renderer/gl/data/impl/glarrayhandler.hpp>
#include <gamp/renderer/gl/data/impl/glsubarrayhandler.hpp>

namespace gamp::render::gl::data::impl {
    using namespace gamp::render::gl;
    using namespace gamp::render::gl::data;

    /** \addtogroup Gamp_GLImpl
     *
     *  @{
     */
     
    /**
     * Used for pure VBO data arrays, i.e. where the buffer data
     * does not represents a specific array name.
     */
    template <typename Value_type>
    class GLDataArrayHandler : public GLArrayHandler<Value_type> {
      private:
        typedef GLArrayHandler<Value_type> glarray_t;
        using GLArrayHandler<Value_type>::m_ad;
        
      public:    
        GLDataArrayHandler() noexcept = default;
    
        void setSubArrayVBOName(GLuint) override {
            throw jau::UnsupportedOperationException("@GLDataArrayHandler", E_FILE_LINE);
        }

        void addSubHandler(GLSLSubArrayHandler&&) override {
            throw jau::UnsupportedOperationException("@GLDataArrayHandler", E_FILE_LINE);
        }
    
        void enableState(GL& gl, bool enable, ShaderState*) override {
        if(enable) {
            if(!m_ad->isVBO()) {
                // makes no sense otherwise
                throw GLException("GLDataArrayHandler can only handle VBOs.", E_FILE_LINE);
            }
            glarray_t::bindBuffer(gl, true);
            glarray_t::bindBuffer(gl, false);
        }
        // no array association
      }
    };

    /**@}*/
}

#endif /* GAMP_GLDATAARRAYHANDLER_HPP_ */

