/*
 * Author: Sven Gothel <sgothel@jausoft.com> and Svenson Han Gothel
 * Copyright (c) 2022-2024 Gothel Software e.K.
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

#include <jau/float_types.hpp>
#include <gamp/renderer/gl/gltypes.hpp>
#include <gamp/renderer/gl/data/glarraydata.hpp>
#include <gamp/renderer/gl/data/gluniformdata.hpp>
#include <gamp/renderer/gl/glsl/shadercode.hpp>
#include <gamp/renderer/gl/glsl/shaderprogram.hpp>

using namespace gamp::render::gl;
using namespace gamp::render::gl::glsl;

std::atomic<size_t> ShaderCode::m_nextID = 1;
std::atomic<size_t> ShaderProgram::m_nextID = 1;

bool GLProfile::isValidArrayDataType(GLenum index, GLsizei comps, GLenum type,
                                     bool isVertexAttribPointer, bool throwException) const 
{
    if( isGLES1() ) {
        if(isVertexAttribPointer) {
            if(throwException) {
                throw GLException("Illegal array type for "+std::string(getGLArrayName(index))+" on profile GLES1: VertexAttribPointer", E_FILE_LINE);
            }
            return false;
        }
        switch(index) {
            case GL_VERTEX_ARRAY:
            case GL_TEXTURE_COORD_ARRAY:
                switch(type) {
                    case GL_BYTE:
                    case GL_SHORT:
                    case GL_FIXED:
                    case GL_FLOAT:
                        break;
                    default:
                        if(throwException) {
                            throw GLException("Illegal data type for "+std::string(getGLArrayName(index))+" on profile GLES1: "+jau::to_hexstring(type), E_FILE_LINE);
                        }
                        return false;
                }
                switch(comps) {
                    case 0:
                    case 2:
                    case 3:
                    case 4:
                        break;
                    default:
                        if(throwException) {
                            throw GLException("Illegal component number for "+std::string(getGLArrayName(index))+" on profile GLES1: "+std::to_string(comps), E_FILE_LINE);
                        }
                        return false;
                }
                break;
            case GL_NORMAL_ARRAY:
                switch(type) {
                    case GL_BYTE:
                    case GL_SHORT:
                    case GL_FIXED:
                    case GL_FLOAT:
                        break;
                    default:
                        if(throwException) {
                            throw GLException("Illegal data type for "+std::string(getGLArrayName(index))+" on profile GLES1: "+jau::to_hexstring(type), E_FILE_LINE);
                        }
                        return false;
                }
                switch(comps) {
                    case 0:
                    case 3:
                        break;
                    default:
                        if(throwException) {
                            throw GLException("Illegal component number for "+std::string(getGLArrayName(index))+" on profile GLES1: "+std::to_string(comps), E_FILE_LINE);
                        }
                        return false;
                }
                break;
            case GL_COLOR_ARRAY:
                switch(type) {
                    case GL_UNSIGNED_BYTE:
                    case GL_FIXED:
                    case GL_FLOAT:
                        break;
                    default:
                        if(throwException) {
                            throw new GLException("Illegal data type for "+std::string(getGLArrayName(index))+" on profile GLES1: "+jau::to_hexstring(type), E_FILE_LINE);
                        }
                        return false;
                }
                switch(comps) {
                    case 0:
                    case 4:
                        break;
                    default:
                        if(throwException) {
                            throw GLException("Illegal component number for "+std::string(getGLArrayName(index))+" on profile GLES1: "+std::to_string(comps), E_FILE_LINE);
                        }
                        return false;
                }
                break;
            default: break;
        }
    } else if( isGLES3() ) {
        // simply ignore !isVertexAttribPointer case, since it is simulated anyway ..
        switch(type) {
            case GL_UNSIGNED_BYTE:
            case GL_BYTE:
            case GL_UNSIGNED_SHORT:
            case GL_SHORT:
            case GL_INT: // GL2ES2
            case GL_UNSIGNED_INT:
            case GL_HALF_FLOAT:
            case GL_FLOAT:
            case GL_FIXED:
            case GL_INT_2_10_10_10_REV: // GL3ES3
            case GL_UNSIGNED_INT_2_10_10_10_REV: // GL3ES2
            case GL_UNSIGNED_INT_10F_11F_11F_REV:
                break;
            default:
                if(throwException) {
                    throw GLException("Illegal data type on profile GLES3: "+jau::to_hexstring(type), E_FILE_LINE);
                }
                return false;
        }
        /** unable to validate .. could be any valid type/component combination
        switch(comps) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
                break;
            default:
                if(throwException) {
                    throw new GLException("Illegal component number on profile GLES3: "+std::to_string(comps));
                }
                return false;
        } */
    } else if( isGLES2() ) {
        // simply ignore !isVertexAttribPointer case, since it is simulated anyway ..
        switch(type) {
            case GL_UNSIGNED_BYTE:
            case GL_BYTE:
            case GL_UNSIGNED_SHORT:
            case GL_SHORT:
            case GL_FLOAT:
            case GL_FIXED:
                break;
            default:
                if(throwException) {
                    throw GLException("Illegal data type on profile GLES2: "+jau::to_hexstring(type), E_FILE_LINE);
                }
                return false;
        }
        /** unable to validate .. could be any valid type/component combination
        switch(comps) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
                break;
            default:
                if(throwException) {
                    throw GLException("Illegal component number on profile GLES2: "+std::to_string(comps), E_FILE_LINE);
                }
                return false;
        } */
    } else if( isGL2ES2() ) {
        if(isVertexAttribPointer) {
            switch(type) {
                case GL_UNSIGNED_BYTE:
                case GL_BYTE:
                case GL_UNSIGNED_SHORT:
                case GL_SHORT:
                case GL_INT: // GL3ES2
                case GL_UNSIGNED_INT:
                case GL_HALF_FLOAT:
                case GL_FLOAT:
                case GL_FIXED:
                case GL_INT_2_10_10_10_REV: // GL3ES3
                case GL_UNSIGNED_INT_2_10_10_10_REV: // GL3ES2
                case GL_DOUBLE: // GL2GL3
                case GL_UNSIGNED_INT_10F_11F_11F_REV:
                    break;
                default:
                    if(throwException) {
                        throw GLException("Illegal data type on profile GL2: "+jau::to_hexstring(type), E_FILE_LINE);
                    }
                    return false;
            }
            switch(comps) {
                case 0:
                case 1:
                case 2:
                case 3:
                case 4:
                    break;
                default:
                    if(throwException) {
                        throw GLException("Illegal component number on profile GL2: "+std::to_string(comps), E_FILE_LINE);
                    }
                    return false;
            }
        } else {
            switch(index) {
                case GL_VERTEX_ARRAY:
                    switch(type) {
                        case GL_SHORT:
                        case GL_FLOAT:
                        case GL_INT: // GL2ES2
                        case GL_DOUBLE: // GL2GL3
                            break;
                        default:
                            if(throwException) {
                                throw GLException("Illegal data type for "+std::string(getGLArrayName(index))+" on profile GL2: "+jau::to_hexstring(type), E_FILE_LINE);
                            }
                            return false;
                    }
                    switch(comps) {
                        case 0:
                        case 2:
                        case 3:
                        case 4:
                            break;
                        default:
                            if(throwException) {
                                throw GLException("Illegal component number for "+std::string(getGLArrayName(index))+" on profile GL2: "+std::to_string(comps), E_FILE_LINE);
                            }
                            return false;
                    }
                    break;
                case GL_NORMAL_ARRAY:
                    switch(type) {
                        case GL_BYTE:
                        case GL_SHORT:
                        case GL_FLOAT:
                        case GL_INT: // GL2ES2
                        case GL_DOUBLE: // GL2GL3
                            break;
                        default:
                            if(throwException) {
                                throw GLException("Illegal data type for "+std::string(getGLArrayName(index))+" on profile GL2: "+jau::to_hexstring(type), E_FILE_LINE);
                            }
                            return false;
                    }
                    switch(comps) {
                        case 0:
                        case 3:
                            break;
                        default:
                            if(throwException) {
                                throw GLException("Illegal component number for "+std::string(getGLArrayName(index))+" on profile GLES1: "+std::to_string(comps), E_FILE_LINE);
                            }
                            return false;
                    }
                    break;
                case GL_COLOR_ARRAY:
                    switch(type) {
                        case GL_UNSIGNED_BYTE:
                        case GL_BYTE:
                        case GL_UNSIGNED_SHORT:
                        case GL_SHORT:
                        case GL_FLOAT:
                        case GL_INT: // GL2ES2
                        case GL_UNSIGNED_INT:
                        case GL_DOUBLE: // GL2GL3
                            break;
                        default:
                            if(throwException) {
                                throw GLException("Illegal data type for "+std::string(getGLArrayName(index))+" on profile GL2: "+jau::to_hexstring(type), E_FILE_LINE);
                            }
                            return false;
                    }
                    switch(comps) {
                        case 0:
                        case 3:
                        case 4:
                            break;
                        default:
                            if(throwException) {
                                throw GLException("Illegal component number for "+std::string(getGLArrayName(index))+" on profile GL2: "+std::to_string(comps), E_FILE_LINE);
                            }
                            return false;
                    }
                    break;
                case GL_TEXTURE_COORD_ARRAY:
                    switch(type) {
                        case GL_SHORT:
                        case GL_FLOAT:
                        case GL_INT: // GL2ES2
                        case GL_DOUBLE: // GL2GL3
                            break;
                        default:
                            if(throwException) {
                                throw GLException("Illegal data type for "+std::string(getGLArrayName(index))+" on profile GL2: "+jau::to_hexstring(type), E_FILE_LINE);
                            }
                            return false;
                    }
                    switch(comps) {
                        case 0:
                        case 1:
                        case 2:
                        case 3:
                        case 4:
                            break;
                        default:
                            if(throwException) {
                                throw GLException("Illegal component number for "+std::string(getGLArrayName(index))+" on profile GL2: "+std::to_string(comps), E_FILE_LINE);
                            }
                            return false;
                    }
                    break;
                default: break;
            }
        }
    }
    return true;
}

void gamp::render::gl::data::GLUniformData::send(const GL&) const {
    if( location() < 0 ) { return; }
    if( columns() > 1 ) {
        if( rows() > 1 ) {
            if( compSignature() != jau::float_ctti::f32() ) {
                throw GLException("glUniformMatrix matrix only supported for float: " + toString(), E_FILE_LINE);
            }
            switch( columns() ) {
                case 2:  ::glUniformMatrix2fv(location(), count(), false, reinterpret_cast<const GLfloat*>(data())); break;
                case 3:  ::glUniformMatrix3fv(location(), count(), false, reinterpret_cast<const GLfloat*>(data())); break;
                case 4:  ::glUniformMatrix4fv(location(), count(), false, reinterpret_cast<const GLfloat*>(data())); break;
                default: throw GLException("glUniformMatrix only available for [2..4] columns: " + toString(), E_FILE_LINE);
            }
        } else {
            if( compSignature() == jau::float_ctti::f32() ) {
                switch( components() ) {
                    case 1:  ::glUniform1fv(location(), count(), reinterpret_cast<const GLfloat*>(data())); break;
                    case 2:  ::glUniform2fv(location(), count(), reinterpret_cast<const GLfloat*>(data())); break;
                    case 3:  ::glUniform3fv(location(), count(), reinterpret_cast<const GLfloat*>(data())); break;
                    case 4:  ::glUniform4fv(location(), count(), reinterpret_cast<const GLfloat*>(data())); break;
                    default: throw GLException("glUniform float32 vector only available for [1..4] columns: " + toString(), E_FILE_LINE);
                }
            } else if( compSignature() == jau::int_ctti::i32() ) {
                switch( components() ) {
                    case 1:  ::glUniform1iv(location(), count(), reinterpret_cast<const GLint*>(data())); break;
                    case 2:  ::glUniform2iv(location(), count(), reinterpret_cast<const GLint*>(data())); break;
                    case 3:  ::glUniform3iv(location(), count(), reinterpret_cast<const GLint*>(data())); break;
                    case 4:  ::glUniform4iv(location(), count(), reinterpret_cast<const GLint*>(data())); break;
                    default: throw GLException("glUniform int32 vector only available for [1..4] columns: " + toString(), E_FILE_LINE);
                }
            } else {
                throw GLException("glUniformMatrix vector only supports integer and float: " + toString(), E_FILE_LINE);
            }
        }
    } else {
        if( compSignature() == jau::int_ctti::i32() ) {
            ::glUniform1i(location(), *reinterpret_cast<const GLint*>(data()));
        } else if( compSignature() == jau::float_ctti::f32() ) {
            ::glUniform1f(location(), *reinterpret_cast<const GLfloat*>(data()));
        } else {
            throw GLException("glUniform scalar only available for int32 and float32", E_FILE_LINE);
        }
    }
}
