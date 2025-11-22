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

#include <jau/float_types.hpp>
#include <gamp/render/gl/GLTypes.hpp>
#include <gamp/render/gl/data/GLArrayData.hpp>
#include <gamp/render/gl/data/GLUniformData.hpp>
#include <gamp/render/gl/glsl/ShaderCode.hpp>
#include <gamp/render/gl/glsl/ShaderProgram.hpp>

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
                throw RenderException("Illegal array type for "+std::string(getGLArrayName(index))+" on profile GLES1: VertexAttribPointer", E_FILE_LINE);
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
                            throw RenderException("Illegal data type for "+std::string(getGLArrayName(index))+" on profile GLES1: "+jau::toHexString(type), E_FILE_LINE);
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
                            throw RenderException("Illegal component number for "+std::string(getGLArrayName(index))+" on profile GLES1: "+std::to_string(comps), E_FILE_LINE);
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
                            throw RenderException("Illegal data type for "+std::string(getGLArrayName(index))+" on profile GLES1: "+jau::toHexString(type), E_FILE_LINE);
                        }
                        return false;
                }
                switch(comps) {
                    case 0:
                    case 3:
                        break;
                    default:
                        if(throwException) {
                            throw RenderException("Illegal component number for "+std::string(getGLArrayName(index))+" on profile GLES1: "+std::to_string(comps), E_FILE_LINE);
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
                            throw RenderException("Illegal data type for "+std::string(getGLArrayName(index))+" on profile GLES1: "+jau::toHexString(type), E_FILE_LINE);
                        }
                        return false;
                }
                switch(comps) {
                    case 0:
                    case 4:
                        break;
                    default:
                        if(throwException) {
                            throw RenderException("Illegal component number for "+std::string(getGLArrayName(index))+" on profile GLES1: "+std::to_string(comps), E_FILE_LINE);
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
                    throw RenderException("Illegal data type on profile GLES3: "+jau::toHexString(type), E_FILE_LINE);
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
                    throw GLException("Illegal component number on profile GLES3: "+std::to_string(comps));
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
                    throw RenderException("Illegal data type on profile GLES2: "+jau::toHexString(type), E_FILE_LINE);
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
                        throw RenderException("Illegal data type on profile GL2: "+jau::toHexString(type), E_FILE_LINE);
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
                        throw RenderException("Illegal component number on profile GL2: "+std::to_string(comps), E_FILE_LINE);
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
                                throw RenderException("Illegal data type for "+std::string(getGLArrayName(index))+" on profile GL2: "+jau::toHexString(type), E_FILE_LINE);
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
                                throw RenderException("Illegal component number for "+std::string(getGLArrayName(index))+" on profile GL2: "+std::to_string(comps), E_FILE_LINE);
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
                                throw RenderException("Illegal data type for "+std::string(getGLArrayName(index))+" on profile GL2: "+jau::toHexString(type), E_FILE_LINE);
                            }
                            return false;
                    }
                    switch(comps) {
                        case 0:
                        case 3:
                            break;
                        default:
                            if(throwException) {
                                throw RenderException("Illegal component number for "+std::string(getGLArrayName(index))+" on profile GLES1: "+std::to_string(comps), E_FILE_LINE);
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
                                throw RenderException("Illegal data type for "+std::string(getGLArrayName(index))+" on profile GL2: "+jau::toHexString(type), E_FILE_LINE);
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
                                throw RenderException("Illegal component number for "+std::string(getGLArrayName(index))+" on profile GL2: "+std::to_string(comps), E_FILE_LINE);
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
                                throw RenderException("Illegal data type for "+std::string(getGLArrayName(index))+" on profile GL2: "+jau::toHexString(type), E_FILE_LINE);
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
                                throw RenderException("Illegal component number for "+std::string(getGLArrayName(index))+" on profile GL2: "+std::to_string(comps), E_FILE_LINE);
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

bool gamp::render::gl::data::GLUniformData::resolveLocation(const GL&, GLuint program) noexcept {
    if( isBuffer() ) {
        m_bufferIndex = ::glGetUniformBlockIndex(program, string_t(m_name).c_str());
        if (m_bufferIndex == GL_INVALID_INDEX) {
            return false;
        }
        if( 0 == m_bufferName ) {
            // buffer init once
            ::glGenBuffers(1, &m_bufferName);
            if( 0 == bufferName() ) {
                return false;
            }
            ::glBindBuffer(GL_UNIFORM_BUFFER, bufferName());
            ::glBufferData(GL_UNIFORM_BUFFER, bufferSize(), nullptr, GL_STREAM_DRAW);
            ::glBindBuffer(GL_UNIFORM_BUFFER, 0);

            ::glUniformBlockBinding(program, m_bufferIndex, m_bufferGlobalBinding); // per program/index

            // only once
            ::glBindBufferBase(GL_UNIFORM_BLOCK_BINDING, m_bufferGlobalBinding, bufferName());
        } else {
            ::glUniformBlockBinding(program, m_bufferIndex, m_bufferGlobalBinding); // per program/index
        }
        return true;
    } else {
        m_location = ::glGetUniformLocation(program, string_t(m_name).c_str());
        return m_location >= 0;
    }
}
bool gamp::render::gl::data::GLUniformData::sendSub(const GL&, GLintptr offset, GLsizeiptr size) noexcept {
    if( !hasLocation() || !isBuffer() || 0 == bufferName() || offset+size > bufferSize()) {
        return false;
    }
    ::glBindBuffer(GL_UNIFORM_BUFFER, bufferName());
    ::glBufferSubData(GL_UNIFORM_BUFFER, offset, size, reinterpret_cast<const uint8_t*>(data())+offset);
    ::glBindBuffer(GL_UNIFORM_BUFFER, 0);
    return true;
}

bool gamp::render::gl::data::GLUniformData::send(const GL&) noexcept {
    if( !hasLocation() ) { return false; }
    if( isBuffer() ) {
        if( 0 == bufferName() || 0 == bufferSize() ) {
            return false;
        }
        ::glBindBuffer(GL_UNIFORM_BUFFER, bufferName());
        ::glBufferSubData(GL_UNIFORM_BUFFER, 0, bufferSize(), data());
        ::glBindBuffer(GL_UNIFORM_BUFFER, 0);
    } else if( columns() > 1 ) {
        if( rows() > 1 ) {
            if( compSignature() != jau::float_ctti::f32() ) {
                return false;
            }
            switch( columns() ) {
                case 2:  ::glUniformMatrix2fv(location(), count(), false, reinterpret_cast<const GLfloat*>(data())); break;
                case 3:  ::glUniformMatrix3fv(location(), count(), false, reinterpret_cast<const GLfloat*>(data())); break;
                case 4:  ::glUniformMatrix4fv(location(), count(), false, reinterpret_cast<const GLfloat*>(data())); break;
                default: return false;
            }
        } else {
            if( compSignature() == jau::float_ctti::f32() ) {
                switch( components() ) {
                    case 1:  ::glUniform1fv(location(), count(), reinterpret_cast<const GLfloat*>(data())); break;
                    case 2:  ::glUniform2fv(location(), count(), reinterpret_cast<const GLfloat*>(data())); break;
                    case 3:  ::glUniform3fv(location(), count(), reinterpret_cast<const GLfloat*>(data())); break;
                    case 4:  ::glUniform4fv(location(), count(), reinterpret_cast<const GLfloat*>(data())); break;
                    default: return false;
                }
            } else if( compSignature() == jau::int_ctti::i32() ) {
                switch( components() ) {
                    case 1:  ::glUniform1iv(location(), count(), reinterpret_cast<const GLint*>(data())); break;
                    case 2:  ::glUniform2iv(location(), count(), reinterpret_cast<const GLint*>(data())); break;
                    case 3:  ::glUniform3iv(location(), count(), reinterpret_cast<const GLint*>(data())); break;
                    case 4:  ::glUniform4iv(location(), count(), reinterpret_cast<const GLint*>(data())); break;
                    default: return false;
                }
            } else {
                return false;
            }
        }
    } else {
        if( compSignature() == jau::int_ctti::i32() ) {
            ::glUniform1i(location(), *reinterpret_cast<const GLint*>(data()));
        } else if( compSignature() == jau::float_ctti::f32() ) {
            ::glUniform1f(location(), *reinterpret_cast<const GLfloat*>(data()));
        } else {
            return false;
        }
    }
    return true;
}
