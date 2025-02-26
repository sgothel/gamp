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

#ifndef GAMP_GLSLSHADERUTIL_HPP_
#define GAMP_GLSLSHADERUTIL_HPP_

#include <GL/gl.h>
#include <gamp/renderer/gl/glliterals.hpp>
#include <gamp/renderer/gl/gltypes.hpp>

namespace gamp::render::gl::glsl {
    using namespace gamp::render::gl;

    /** \addtogroup Gamp_GLSL
     *
     *  @{
     */

    typedef std::vector<GLuint> shader_list_t;
    typedef std::vector<GLenum> name_list_t;
    typedef std::vector<GLint> int_list_t;
    typedef std::vector<GLfloat> float_list_t;
    typedef std::string string_t;
    typedef std::string_view stringview_t;
    typedef std::vector<string_t> string_list_t;
    typedef std::vector<string_list_t> source_list_t;
    typedef std::vector<uint8_t> bytes_t;

    /** Returns type signature of std::vector<T>. */
    template<typename T>
    const jau::type_info& vectorSignature() noexcept {
        return jau::static_ctti<std::vector<T>>();
    }

    class ShaderUtil {
      public:
        static std::string getShaderInfoLog(GL&, GLuint shaderObj) noexcept {
            GLint infoLogLength = 0;
            glGetShaderiv(shaderObj, GL_INFO_LOG_LENGTH, &infoLogLength);

            if(infoLogLength==0) {
                return "(no info log)";
            }
            GLsizei charsWritten = 0;
            std::string infoLog;
            infoLog.reserve(infoLogLength + 1);  // incl. EOS
            infoLog.resize(infoLogLength);       // excl. EOS
            glGetShaderInfoLog(shaderObj, infoLogLength, &charsWritten, &infoLog[0]);
            return infoLog;
        }

        static std::string getProgramInfoLog(GL&, GLuint programObj) noexcept {
            GLint infoLogLength = 0;
            glGetShaderiv(programObj, GL_INFO_LOG_LENGTH, &infoLogLength);

            if(infoLogLength==0) {
                return "(no info log)";
            }
            GLsizei charsWritten = 0;
            std::string infoLog;
            infoLog.reserve(infoLogLength + 1);  // incl. EOS
            infoLog.resize(infoLogLength);       // excl. EOS
            glGetProgramInfoLog(programObj, infoLogLength, &charsWritten, &infoLog[0]);
            return infoLog;
        }

        static bool isShaderStatusValid(GL& gl, GLuint shaderObj, GLenum name, bool verbose=false) noexcept {
            GLint ires = 0;
            glGetShaderiv(shaderObj, name, &ires);

            const bool res = ires==1;
            if(!res && verbose) {
                jau::PLAIN_PRINT(true, "Shader status invalid: %s", getShaderInfoLog(gl, shaderObj).c_str());
            }
            return res;
        }

        static bool isShaderStatusValid(GL& gl, const shader_list_t& shaders, GLenum name, bool verbose=false) noexcept {
            bool res = true;
            for (GLuint s : shaders) {
                res = isShaderStatusValid(gl, s, name, verbose) && res;
            }
            return res;
        }

        static bool isProgramStatusValid(GL& gl, GLuint programObj, GLenum name, bool verbose=false) noexcept {
            GLint ires = 0;
            glGetProgramiv(programObj, name, &ires);
            const bool res = ires==1;
            if(!res && verbose) {
                jau::PLAIN_PRINT(true, "Program status invalid: %s", getProgramInfoLog(gl, programObj).c_str());
            }
            return ires==1;
        }

        static bool isProgramLinkStatusValid(GL& gl, GLuint programObj, bool verbose=false) noexcept {
            if(!glIsProgram(programObj)) {
                if(verbose) {
                    jau::PLAIN_PRINT(true, "Program name invalid: %u", programObj);
                }
                return false;
            }
            if(!isProgramStatusValid(gl, programObj, GL_LINK_STATUS)) {
                if(verbose) {
                    jau::PLAIN_PRINT(true, "Program link failed: %u\n\t%s", programObj, getProgramInfoLog(gl, programObj).c_str());
                }
                return false;
            }
            return true;
        }

        /**
         * Performs {@link GL2ES2#glValidateProgram(int)}
         * <p>
         * One shall only call this method while debugging and only if all required
         * resources by the shader are set.
         * </p>
         * <p>
         * Note: It is possible that a working shader program will fail validation.
         * This has been experienced on NVidia APX2500 and Tegra2.
         * </p>
         * @see GL2ES2#glValidateProgram(int)
         **/
        static bool isProgramExecStatusValid(GL& gl, GLuint programObj, bool verbose=false) noexcept {
            glValidateProgram(programObj);
            if(!isProgramStatusValid(gl, programObj, GL_VALIDATE_STATUS)) {
                if(verbose) {
                    jau::PLAIN_PRINT(true, "Program validation failed: %u\n\t%s", programObj, getProgramInfoLog(gl, programObj).c_str());
                }
                return false;
            }
            return true;
        }

        /** Creates shaders.size() new shaders and stores them in the shaders list. */
        static void createShader(GL&, GLenum type, shader_list_t& shaders) noexcept {
            for (GLuint & shader : shaders) {
                shader = glCreateShader(type);
            }
        }

      private:
        static constexpr const char* compilerInfoKey = "gamp.renderer.gl.glsl.CompilerInfo";
        class CompilerInfo : public Attachable {
          public:
            /** flag whether shader compiler is supported */
            bool shaderCompilerAvail;
            /** List of binary shader formats */
            name_list_t binFormats;
        };
        typedef std::shared_ptr<CompilerInfo> CompilerInfoRef;

        static CompilerInfoRef getOrCreateCompilerInfo(GL& gl) noexcept {
            AttachableRef o = gl.getAttachedObject(compilerInfoKey);
            if( o ) {
                CompilerInfoRef ci = std::static_pointer_cast<CompilerInfo>(o);
                return ci;
            }
            CompilerInfoRef ci = std::make_shared<CompilerInfo>();
            if (gl.hasGLSL()) {
                GLint param = 0;
                glGetIntegerv(GL_NUM_SHADER_BINARY_FORMATS, &param);
                const GLenum err = glGetError();
                const size_t numFormats = GL_NO_ERROR == err ? param : 0;
                if(numFormats>0) {
                    ci->binFormats.reserve(numFormats);
                    ci->binFormats.resize(numFormats);
                    int_list_t il(numFormats, 0);
                    glGetIntegerv(GL_SHADER_BINARY_FORMATS, il.data());
                    for(size_t i=0; i<numFormats; ++i) {
                        ci->binFormats[i] = (GLenum)il[i];
                    }
                }
            }
            if(gl.isGLES2()) {
                GLboolean param = 0;
                glGetBooleanv(GL_SHADER_COMPILER, &param); // GL2ES2
                const GLenum err = glGetError();
                const bool v = GL_NO_ERROR == err && param!=0;
                ci->shaderCompilerAvail = v || ci->binFormats.empty(); // alt assume compiler w/o binary fmts
            } else if( gl.isGL2ES2() ) {
                ci->shaderCompilerAvail = true;
            } else {
                ci->shaderCompilerAvail = false;
            }
            gl.attachObject(compilerInfoKey, ci);
            return ci;
        }

      public:
        /**
         * If supported, queries the natively supported shader binary formats using
         * {@link GL2ES2#GL_NUM_SHADER_BINARY_FORMATS} and {@link GL2ES2#GL_SHADER_BINARY_FORMATS}
         * via {@link GL2ES2#glGetIntegerv(int, int[], int)}.
         */
        static name_list_t getShaderBinaryFormats(GL& gl) noexcept {
            const CompilerInfoRef& ci = getOrCreateCompilerInfo(gl);
            return ci->binFormats;
        }

        /** Returns true if a hader compiler is available, otherwise false. */
        static bool isShaderCompilerAvailable(GL& gl) noexcept {
            const CompilerInfoRef& ci = getOrCreateCompilerInfo(gl);
            return ci->shaderCompilerAvail;
        }

        /** Returns true if GeometryShader is supported, i.e. whether GLContext is &ge; 3.2 or ARB_geometry_shader4 extension is available. */
        static bool isGeometryShaderSupported(GL& gl) noexcept {
          return gl.version() >= Version3_2 ||
                 gl.isExtensionAvailable("ARB_geometry_shader4");
        }

        static void shaderSource(GL& gl, GLuint shader, const string_list_t& source) {
            if(!isShaderCompilerAvailable(gl)) {
                throw GLException("No compiler is available", E_FILE_LINE);
            }
            const size_t count = source.size();
            throwOnOverflow<size_t, GLsizei>(count);
            if(count==0) {
                throw GLException("No sources specified", E_FILE_LINE);
            }
            std::vector<GLint> lengths(count, 0);
            for(size_t i=0; i<count; ++i) {
                size_t l = source[i].length();
                throwOnOverflow<size_t, GLint>(l);
                lengths[i] = (GLint)l;
            }
            std::vector<GLchar*> sourcePtr(count, nullptr);
            for(size_t i = 0; i<count; ++i) {
                sourcePtr[i] = (GLchar*)source[i].data();
            }
            glShaderSource(shader, (GLsizei)count, sourcePtr.data(), lengths.data());
        }

        static void shaderSource(GL& gl, const shader_list_t& shaders, const source_list_t& sources) {
            const size_t sourceNum = sources.size();
            const size_t shaderNum = shaders.size();
            if(shaderNum==0 || sourceNum==0 || shaderNum!=sourceNum) {
                throw GLException("Invalid number of shaders and/or sources: shaders="+
                                      std::to_string(shaderNum)+", sources="+std::to_string(sourceNum), E_FILE_LINE);
            }
            for(size_t i=0; i<sourceNum; ++i) {
                shaderSource(gl, shaders[i], sources[i]);
            }
        }

        static void shaderBinary(GL& gl, const shader_list_t& shaders, GLenum binFormat, const bytes_t& bin) {
            if(getShaderBinaryFormats(gl).size()==0) {
                throw GLException("No binary formats supported", E_FILE_LINE);
            }

            const size_t shaderNum = shaders.size();
            throwOnOverflow<size_t, GLsizei>(shaderNum);
            if(shaderNum==0) {
                throw GLException("No shaders specified", E_FILE_LINE);
            }
            if(bin.empty()) {
                throw GLException("Null shader binary", E_FILE_LINE);
            }
            const size_t binLength = bin.size();
            throwOnOverflow<size_t, GLsizei>(binLength);
            if(0==binLength) {
                throw GLException("Empty shader binary (remaining == 0)", E_FILE_LINE);
            }
            glShaderBinary((GLsizei)shaderNum, shaders.data(), binFormat, bin.data(), (GLsizei)binLength);
        }

        static void compileShader(GL&, const shader_list_t& shaders) noexcept {
            for (GLuint shader : shaders) {
                glCompileShader(shader);
            }
        }

        static void attachShader(GL&, GLuint program, const shader_list_t& shaders) noexcept {
            for (GLuint shader : shaders) {
                glAttachShader(program, shader);
            }
        }

        static void detachShader(GL&, GLuint program, const shader_list_t& shaders) noexcept {
            for (GLuint shader : shaders) {
                glDetachShader(program, shader);
            }
        }

        static void deleteShader(GL&, const shader_list_t& shaders) noexcept {
            for (GLuint shader : shaders) {
                glDeleteShader(shader);
            }
        }

        /** Creates shader.size() new shaders, stored in the shader list. Then the bin shader are loaded into them. */
        static bool createAndLoadShader(GL& gl, shader_list_t& shader, GLenum shaderType,
                                        GLenum binFormat, const bytes_t& bin, bool verbose=false)
        {
            GLenum err = glGetError(); // flush previous errors ..
            if(err!=GL_NO_ERROR && verbose) {
                jau::PLAIN_PRINT(true, "createAndLoadShader: Pre GL Error: 0x%x", err);
            }

            createShader(gl, shaderType, shader);
            err = glGetError();
            if(err!=GL_NO_ERROR) {
                throw GLException("createAndLoadShader: CreateShader failed, GL Error: "+jau::to_hexstring(err), E_FILE_LINE);
            }

            shaderBinary(gl, shader, binFormat, bin);

            err = glGetError();
            if(err!=GL_NO_ERROR && verbose) {
                jau::PLAIN_PRINT(true, "createAndLoadShader: ShaderBinary failed, GL Error: 0x%x", err);
            }
            return err == GL_NO_ERROR;
        }

        /** Creates shader.size() new shaders, stored in the shader list. Then the source shader are loaded into them and compiled. */
        static bool createAndCompileShader(GL& gl, shader_list_t& shader, GLenum shaderType,
                                           const source_list_t& sources, bool verbose=false)
        {
            GLenum err = glGetError(); // flush previous errors ..
            if(err!=GL_NO_ERROR && verbose) {
                jau::PLAIN_PRINT(true, "createAndCompileShader: Pre GL Error: 0x%x", err);
            }

            createShader(gl, shaderType, shader);
            err = glGetError();
            if(err!=GL_NO_ERROR) {
                throw GLException("createAndCompileShader: CreateShader failed, GL Error: "+jau::to_hexstring(err), E_FILE_LINE);
            }
            shaderSource(gl, shader, sources);
            err = glGetError();
            if(err!=GL_NO_ERROR) {
                throw GLException("createAndCompileShader: ShaderSource failed, GL Error: "+jau::to_hexstring(err), E_FILE_LINE);
            }

            compileShader(gl, shader);
            err = glGetError();
            if(err!=GL_NO_ERROR && verbose) {
                jau::PLAIN_PRINT(true, "createAndCompileShader: CompileShader failed, GL Error: 0x%x", err);
            }

            return isShaderStatusValid(gl, shader, GL_COMPILE_STATUS, verbose) && err == GL_NO_ERROR;
        }
    };

    /**@}*/
}

#endif // GAMP_GLSLSHADERUTIL_HPP_
