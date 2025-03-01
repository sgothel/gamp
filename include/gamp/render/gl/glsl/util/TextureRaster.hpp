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

#ifndef GAMP_GLSLSHADERSTATE_HPP_
#define GAMP_GLSLSHADERSTATE_HPP_

#include <gamp/Gamp.hpp>
#include <gamp/render/gl/GLTypes.hpp>
#include <gamp/render/gl/data/GLArrayData.hpp>
#include <gamp/render/gl/data/GLUniformData.hpp>
#include <gamp/render/gl/glsl/ShaderProgram.hpp>

namespace gamp::render::gl::glsl {
    using namespace gamp::render::gl;
    using namespace gamp::render::gl::data;

    /** \addtogroup Gamp_GLSL
     *
     *  @{
     */

    class GLSLTextureRaster  {
      private:
        bool textureVertFlipped;
        int textureUnit;

        ShaderProgram sp;
        PMVMatrix4f m_pmvMatrix;
        GLUniformData m_pmvMatrixUniform;
        GLUniformData m_activeTexUniform;
        GLArrayDataServer m_interleavedVBO;

        GLSLTextureRaster(int textureUnit, bool textureVertFlipped) {
            this.textureVertFlipped = textureVertFlipped;
            this.textureUnit = textureUnit;
        }

        public int getTextureUnit() { return textureUnit; }

        static final String shaderBasename = "texture01_xxx";
        static final String shaderSrcPath = "../../shader";
        static final String shaderBinPath = "../../shader/bin";

        public void init(final GL2ES2 gl) {
            // Create & Compile the shader objects
            final ShaderCode rsVp = ShaderCode.create(gl, GL2ES2.GL_VERTEX_SHADER, this.getClass(),
                                                      shaderSrcPath, shaderBinPath, shaderBasename, true);
            final ShaderCode rsFp = ShaderCode.create(gl, GL2ES2.GL_FRAGMENT_SHADER, this.getClass(),
                                                      shaderSrcPath, shaderBinPath, shaderBasename, true);
            rsVp.defaultShaderCustomization(gl, true, true);
            rsFp.defaultShaderCustomization(gl, true, true);

            // Create & Link the shader program
            sp = new ShaderProgram();
            sp.add(rsVp);
            sp.add(rsFp);
            if(!sp.link(gl, System.err)) {
                throw new GLException("Couldn't link program: "+sp);
            }
            sp.useProgram(gl, true);

            // setup mgl_PMVMatrix
            pmvMatrix = new PMVMatrix4f();
            pmvMatrix.loadPIdentity();
            pmvMatrix.loadMvIdentity();
            pmvMatrixUniform = new GLUniformData("mgl_PMVMatrix", 4, 4, pmvMatrix.getSyncPMv()); // P, Mv
            if( pmvMatrixUniform.setLocation(gl, sp.program()) < 0 ) {
                throw new GLException("Couldn't locate "+pmvMatrixUniform+" in shader: "+sp);
            }
            gl.glUniform(pmvMatrixUniform);

            activeTexUniform = new GLUniformData("mgl_Texture0", textureUnit);
            if( activeTexUniform.setLocation(gl, sp.program()) < 0 ) {
                throw new GLException("Couldn't locate "+activeTexUniform+" in shader: "+sp);
            }
            gl.glUniform(activeTexUniform);

            final float[] s_quadTexCoords;
            if( textureVertFlipped ) {
                s_quadTexCoords = s_quadTexCoords01;
            } else {
                s_quadTexCoords = s_quadTexCoords00;
            }

            interleavedVBO = GLArrayDataServer.createGLSLInterleaved(3+2, GL.GL_FLOAT, false, 2*4, GL.GL_STATIC_DRAW);
            {
                final GLArrayData vArrayData = interleavedVBO.addGLSLSubArray("mgl_Vertex",        3, GL.GL_ARRAY_BUFFER);
                if( vArrayData.setLocation(gl, sp.program()) < 0 ) {
                    throw new GLException("Couldn't locate "+vArrayData+" in shader: "+sp);
                }
                final GLArrayData tArrayData = interleavedVBO.addGLSLSubArray("mgl_MultiTexCoord", 2, GL.GL_ARRAY_BUFFER);
                if( tArrayData.setLocation(gl, sp.program()) < 0 ) {
                    throw new GLException("Couldn't locate "+tArrayData+" in shader: "+sp);
                }
                final FloatBuffer ib = (FloatBuffer)interleavedVBO.getBuffer();
                for(int i=0; i<4; i++) {
                    ib.put(s_quadVertices,  i*3, 3);
                    ib.put(s_quadTexCoords, i*2, 2);
                }
            }
            interleavedVBO.seal(gl, true);
            interleavedVBO.enableBuffer(gl, false);

            sp.useProgram(gl, false);
        }

        public void reshape(final GL2ES2 gl, final int x, final int y, final int width, final int height) {
            if(null != sp) {
                pmvMatrix.loadPIdentity();
                pmvMatrix.orthoP(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 10.0f);

                pmvMatrix.loadMvIdentity();

                sp.useProgram(gl, true);
                gl.glUniform(pmvMatrixUniform);
                sp.useProgram(gl, false);
            }
        }

        public void dispose(final GL2ES2 gl) {
            if(null != pmvMatrixUniform) {
                pmvMatrixUniform = null;
            }
            pmvMatrix=null;
            if(null != interleavedVBO) {
                interleavedVBO.destroy(gl);
                interleavedVBO=null;
            }
            if(null != sp) {
                sp.destroy(gl);
                sp=null;
            }
        }

        public void display(final GL2ES2 gl) {
            if(null != sp) {
                sp.useProgram(gl, true);
                interleavedVBO.enableBuffer(gl, true);

                gl.glDrawArrays(GL.GL_TRIANGLE_STRIP, 0, 4);

                interleavedVBO.enableBuffer(gl, false);
                sp.useProgram(gl, false);
            }
        }

        private static final float[] s_quadVertices = {
          -1f, -1f, 0f, // LB
           1f, -1f, 0f, // RB
          -1f,  1f, 0f, // LT
           1f,  1f, 0f  // RT
        };
        private static final float[] s_quadTexCoords00 = {
            0f, 0f, // LB
            1f, 0f, // RB
            0f, 1f, // LT
            1f, 1f  // RT
        };
        private static final float[] s_quadTexCoords01 = {
            0f, 1f, // LB
            1f, 1f, // RB
            0f, 0f, // LT
            1f, 0f  // RT
        };
    };


    /**@}*/
}

#endif // GAMP_GLSLSHADERSTATE_HPP_

