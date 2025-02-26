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

#ifndef GAMP_GLSLARRAYHANDLER_HPP_
#define GAMP_GLSLARRAYHANDLER_HPP_

#include <jau/basic_types.hpp>

#include <gamp/renderer/gl/data/impl/glarrayhandler.hpp>
#include <gamp/renderer/gl/data/impl/glsubarrayhandler.hpp>
#include <gamp/renderer/gl/glmisc.hpp>
#include <gamp/renderer/gl/glsl/shaderstate.hpp>

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
                    } else if( st.getAttribLocation(gl, m_ad) >= 0 ) {
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
                st.enableVertexAttribArray(gl, m_ad);
            } else {
                st.disableVertexAttribArray(gl, m_ad);
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
