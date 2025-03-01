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

#ifndef GAMP_GLSLARRAYHANDLERINTERLEAVED_HPP_
#define GAMP_GLSLARRAYHANDLERINTERLEAVED_HPP_

#include <jau/basic_types.hpp>
#include <gamp/render/gl/data/impl/GLArrayHandler.hpp>
#include <gamp/render/gl/data/impl/GLSLSubArrayHandler.hpp>

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
