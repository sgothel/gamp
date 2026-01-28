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

#ifndef GAMP_GLSLARRAYHANDLER_HPP_
#define GAMP_GLSLARRAYHANDLER_HPP_

#include <jau/basic_types.hpp>

#include <gamp/render/gl/data/impl/GLArrayHandler.hpp>
#include <gamp/render/gl/data/impl/GLSubArrayHandler.hpp>
#include <gamp/render/gl/glsl/ShaderState.hpp>

namespace gamp::render::gl::data::impl {
    using namespace gamp::render::gl;
    using namespace gamp::render::gl::data;

    /** \addtogroup Gamp_GLImpl
     *
     *  @{
     */

    /**
     * Used for 1:1 GLSL arrays, i.e. where the buffer data
     * represents this array only.
     */
    template <typename Value_type>
    class GLSLArrayHandler : public GLArrayHandler<Value_type> {
      private:
        using GLArrayHandler<Value_type>::m_ad;

      public:
        GLSLArrayHandler() noexcept = default;

        void setSubArrayVBOName(GLuint) override {
            throw jau::UnsupportedOperationException("@GLSLArrayHandler", E_FILE_LINE);
        }

        void addSubHandler(GLSLSubArrayHandler&&) override {
            throw jau::UnsupportedOperationException("@GLSLArrayHandler", E_FILE_LINE);
        }

        void enableState(GL& gl, bool enable, ShaderState* st) override {
            if( st ) {
                enableShaderState(gl, enable, *st);
            } else {
                enableSimple(gl, enable);
            }
        }

        void enableShaderState(GL& gl, bool enable, ShaderState& st) {
            if( !m_ad ) {
                throw jau::IllegalStateError("GLArrayDataClient not set", E_FILE_LINE);
            }
            if( enable ) {
                /*
                 * This would be the non optimized code path:
                 *
                if(ad.isVBO()) {
                    m_ad->glBindBuffer(gl, true);
                    if(!ad.isVBOWritten()) {
                        if(null!=buffer) {
                            m_ad->glBufferData(gl);
                        }
                        ad.setVBOWritten(true);
                    }
                }
                st.vertexAttribPointer(glsl, ad);
                */
                if( m_ad->isVBO() ) {
                    // bind and refresh the VBO / vertex-attr only if necessary
                    if( !m_ad->isVBOWritten() ) {
                        m_ad->glBindBuffer(gl, true);
                        if( m_ad->usesClientMem() ) {
                            m_ad->glBufferData(gl);
                        }
                        m_ad->setVBOWritten(true);
                        st.vertexAttribPointer(gl, m_ad);
                        m_ad->glBindBuffer(gl, false);
                    } else if( st.resolveLocation(gl, m_ad) ) {
                        // didn't experience a performance hit on this query ..
                        // (using ShaderState's location query above to validate the location)
                        GLuint tempI;
                        ::glGetVertexAttribiv(m_ad->location(), GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, reinterpret_cast<GLint*>(&tempI));
                        if( m_ad->vboName() != tempI ) {
                            m_ad->glBindBuffer(gl, true);
                            st.vertexAttribPointer(gl, m_ad);
                            m_ad->glBindBuffer(gl, false);
                        }
                    }
                } else if( m_ad->usesClientMem() ) {
                    st.vertexAttribPointer(gl, m_ad);
                }
                st.enableAttribute(gl, m_ad);
            } else {
                st.disableAttribute(gl, m_ad);
            }
        }

      private:
        void enableSimple(const GL& gl, bool enable) {
            GLint location = m_ad->location();
            if( 0 > location ) {
                return;
            }
            if( enable ) {
                /*
                 * This would be the non optimized code path:
                 *
                if(m_ad->isVBO()) {
                    m_ad->glBindBuffer(gl, true);
                    if(!m_ad->isVBOWritten()) {
                        if(buffer) {
                             ::glBufferData(m_ad->vboTarget(), m_ad->byteCount(), buffer->memAtPosition(), m_ad->vboUsage());
                        }
                        m_ad->setVBOWritten(true);
                    }
                }
                st.vertexAttribPointer(glsl, ad);
                */
                if( m_ad->isVBO() ) {
                    // bind and refresh the VBO / vertex-attr only if necessary
                    if( !m_ad->isVBOWritten() ) {
                        m_ad->glBindBuffer(gl, true);
                        if( m_ad->usesClientMem() ) {
                            ::glBufferData(m_ad->vboTarget(), m_ad->byteCount(), m_ad->data(), m_ad->vboUsage());
                        }
                        m_ad->setVBOWritten(true);
                        m_ad->glVertexAttribPointer(gl);
                        m_ad->glBindBuffer(gl, false);
                    } else {
                        // didn't experience a performance hit on this query ..
                        // (using ShaderState's location query above to validate the location)
                        GLuint tempI;
                        glGetVertexAttribiv(location, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, reinterpret_cast<GLint*>(&tempI));
                        if( m_ad->vboName() != tempI ) {
                            m_ad->glBindBuffer(gl, true);
                            m_ad->glVertexAttribPointer(gl);
                            m_ad->glBindBuffer(gl, false);
                        }
                    }
                } else if( m_ad->usesClientMem() ) {
                    m_ad->glVertexAttribPointer(gl);
                }
                ::glEnableVertexAttribArray(location);
            } else {
                ::glDisableVertexAttribArray(location);
            }
        }
    };

    /**@}*/
}  // namespace gamp::render::gl::data::impl

#endif  // GAMP_GLSLARRAYHANDLER_HPP_
