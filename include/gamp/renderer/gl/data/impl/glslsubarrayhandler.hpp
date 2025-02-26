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

#ifndef GAMP_GLSLARRAYHANDLERFLAT_HPP_
#define GAMP_GLSLARRAYHANDLERFLAT_HPP_

#include <gamp/renderer/gl/glsl/shaderstate.hpp>
#include <gamp/renderer/gl/data/glarraydata.hpp>
#include <gamp/renderer/gl/data/impl/glsubarrayhandler.hpp>

namespace gamp::render::gl::data::impl {
    using namespace gamp::render::gl;
    using namespace gamp::render::gl::data;

    /** \addtogroup Gamp_GLImpl
     *
     *  @{
     */
     
    /**
     * Used for interleaved GLSL arrays, i.e. where the buffer data itself is handled
     * separately and interleaves many arrays.
     */
    class GLSLSubArrayHandler : public GLSubArrayHandler {
      private:
        GLArrayDataRef m_ad;

      public:
        GLSLSubArrayHandler(GLArrayDataRef & ad)
        : m_ad(ad) { }

        void syncData(const GL& gl, ShaderState* st) override {
            if( st ) {
                st->vertexAttribPointer(gl, m_ad);
            } else {
                if( 0 <= m_ad->location() ) {
                    m_ad->glVertexAttribPointer(gl);
                }
            }
            /**
             * Due to probable application VBO switching, this might not make any sense ..
             *
            if(!written) {
                st.vertexAttribPointer(glsl, ad);
            } else if(st.getAttribLocation(glsl, ad) >= 0) {
                final int[] qi = new int[1];
                glsl.glGetVertexAttribiv(ad.getLocation(), GL2ES2.GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, qi, 0);
                if(ad.getVBOName() != qi[0]) {
                    System.err.println("XXX1: "+ad.getName()+", vbo ad "+ad.getVBOName()+", gl "+qi[0]+", "+ad);
                    st.vertexAttribPointer(glsl, ad);
                } else {
                    System.err.println("XXX0: "+ad.getName()+", vbo ad "+ad.getVBOName()+", gl "+qi[0]+", "+ad);
                }
            }*/
        }

        void enableState(const GL& gl, bool enable, ShaderState* st) override {
            if( st ) {
                if( enable ) {
                    st->enableVertexAttribArray(gl, m_ad);
                } else {
                    st->disableVertexAttribArray(gl, m_ad);
                }
            } else {
                GLint location = m_ad->location();
                if( 0 <= location ) {
                    if( enable ) {
                        ::glEnableVertexAttribArray(location);
                    } else {
                        ::glDisableVertexAttribArray(location);
                    }
                }
            }
        }
        
        const GLArrayDataRef& data() override { return m_ad; }
    };
    /**@}*/
}  // namespace gamp::render::gl::data::impl

#endif /* GAMP_GLSLARRAYHANDLERFLAT_HPP_ */

