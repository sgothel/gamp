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

#ifndef GAMP_GLDATAARRAYHANDLER_HPP_
#define GAMP_GLDATAARRAYHANDLER_HPP_

#include <jau/basic_types.hpp>
#include <gamp/render/gl/data/impl/GLArrayHandler.hpp>
#include <gamp/render/gl/data/impl/GLSubArrayHandler.hpp>

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

