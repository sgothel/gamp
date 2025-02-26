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

#ifndef GAMP_GLSLARRAYHANDLERINTERLEAVED_HPP_
#define GAMP_GLSLARRAYHANDLERINTERLEAVED_HPP_

#include <jau/basic_types.hpp>
#include <gamp/renderer/gl/data/impl/glarrayhandler.hpp>
#include <gamp/renderer/gl/data/impl/glslsubarrayhandler.hpp>

namespace gamp::render::gl::data::impl {
    using namespace gamp::render::gl;
    using namespace gamp::render::gl::data;

    /** \addtogroup Gamp_GLImpl
     *
     *  @{
     */

    /**
     * Interleaved fixed function arrays, i.e. where this buffer data
     * represents many arrays.
     */
    template<typename Value_type>
    class GLSLArrayHandlerInterleaved : public GLArrayHandler<Value_type> {
      private:
        std::vector<GLSLSubArrayHandler> subArrays;
        using GLArrayHandler<Value_type>::m_ad;

        void syncSubData(const GL& gl, ShaderState* st) {
          for(auto & subArray : subArrays) {
              subArray.syncData(gl, st);
          }
        }

      public:
        typedef GLArrayHandler<Value_type> glarray_t;

        GLSLArrayHandlerInterleaved() noexcept = default;

        void setSubArrayVBOName(GLuint vboName) override {
          for(auto & subArray : subArrays) {
              subArray.data()->setVBOName(vboName);
          }
        }

        void addSubHandler(GLSLSubArrayHandler&& handler) override {
          subArrays.push_back(std::move(handler));
        }

        void enableState(GL& gl, bool enable, ShaderState* st) override {
            if(enable) {
                if(!m_ad->isVBO()) {
                    throw jau::InternalError("Interleaved handle is not VBO: "+m_ad->toString(), E_FILE_LINE);
                }
                glarray_t::bindBuffer(gl, true);
                // sub data will decide whether to update the vertex attrib pointer
                syncSubData(gl, st);
                glarray_t::bindBuffer(gl, false);
            }
            for(auto & subArray : subArrays) {
                subArray.enableState(gl, enable, st);
            }
        }
    };

    /**@}*/
}

#endif /* GAMP_GLSLARRAYHANDLERINTERLEAVED_HPP_ */
