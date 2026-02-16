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

#ifndef GAMP_GLSLSHADERCODE_HPP_
#define GAMP_GLSLSHADERCODE_HPP_

#include <fstream>

#include <jau/basic_types.hpp>
#include <jau/debug.hpp>
#include <jau/io/file_util.hpp>
#include <jau/io/io_util.hpp>
#include <jau/string_util.hpp>

#include <gamp/Gamp.hpp>
#include <gamp/render/gl/glsl/ShaderUtil.hpp>
#include "jau/cpp_lang_util.hpp"

namespace gamp::render::gl::glsl {
    using namespace gamp::render::gl;

    /** @defgroup Gamp_GLSL Gamp GL Shader Support
     *  OpenGL Shading Language types and functionality.
     *
     *  @{
     */

    class ShaderCode;
    typedef std::shared_ptr<ShaderCode> ShaderCodeSRef;

    /**
     * Convenient shader code class to use and instantiate vertex or fragment programs.
     * <p>
     * A documented example of how to use this code is available
     * {@link #create(GL2ES2, int, Class, String, String, String, boolean) here} and
     * {@link #create(GL2ES2, int, int, Class, String, String[], String, String) here}.
     * </p>
     * <p>
     * Support for {@link GL4#GL_TESS_CONTROL_SHADER} and {@link GL4#GL_TESS_EVALUATION_SHADER}
     * was added since 2.2.1.
     * </p>
     */
    class ShaderCode {
      public:
        static bool DEBUG_CODE;

        /** Unique resource suffix for {@link GL2ES2#GL_VERTEX_SHADER} in source code: <code>{@value}</code> */
        static constexpr std::string_view SUFFIX_VERTEX_SOURCE   =  "vp" ;

        /** Unique resource suffix for {@link GL2ES2#GL_VERTEX_SHADER} in binary: <code>{@value}</code> */
        static constexpr std::string_view SUFFIX_VERTEX_BINARY   = "bvp" ;

        /** Unique resource suffix for {@link GL3#GL_GEOMETRY_SHADER} in source code: <code>{@value}</code> */
        static constexpr std::string_view SUFFIX_GEOMETRY_SOURCE =  "gp" ;

        /** Unique resource suffix for {@link GL3#GL_GEOMETRY_SHADER} in binary: <code>{@value}</code> */
        static constexpr std::string_view SUFFIX_GEOMETRY_BINARY = "bgp" ;

        /**
         * Unique resource suffix for {@link GL3ES3#GL_COMPUTE_SHADER} in source code: <code>{@value}</code>
         * @since 2.3.2
         */
        static constexpr std::string_view SUFFIX_COMPUTE_SOURCE =  "cp" ;

        /**
         * Unique resource suffix for {@link GL3ES3#GL_COMPUTE_SHADER} in binary: <code>{@value}</code>
         * @since 2.3.2
         */
        static constexpr std::string_view SUFFIX_COMPUTE_BINARY = "bcp" ;

        /**
         * Unique resource suffix for {@link GL4#GL_TESS_CONTROL_SHADER} in source code: <code>{@value}</code>
         * @since 2.2.1
         */
        static constexpr std::string_view SUFFIX_TESS_CONTROL_SOURCE =  "tcp" ;

        /**
         * Unique resource suffix for {@link GL4#GL_TESS_CONTROL_SHADER} in binary: <code>{@value}</code>
         * @since 2.2.1
         */
        static constexpr std::string_view SUFFIX_TESS_CONTROL_BINARY = "btcp" ;

        /**
         * Unique resource suffix for {@link GL4#GL_TESS_EVALUATION_SHADER} in source code: <code>{@value}</code>
         * @since 2.2.1
         */
        static constexpr std::string_view SUFFIX_TESS_EVALUATION_SOURCE =  "tep" ;

        /**
         * Unique resource suffix for {@link GL4#GL_TESS_EVALUATION_SHADER} in binary: <code>{@value}</code>
         * @since 2.2.1
         */
        static constexpr std::string_view SUFFIX_TESS_EVALUATION_BINARY = "btep" ;

        /** Unique resource suffix for {@link GL2ES2#GL_FRAGMENT_SHADER} in source code: <code>{@value}</code> */
        static constexpr std::string_view SUFFIX_FRAGMENT_SOURCE =  "fp" ;

        /** Unique resource suffix for {@link GL2ES2#GL_FRAGMENT_SHADER} in binary: <code>{@value}</code> */
        static constexpr std::string_view SUFFIX_FRAGMENT_BINARY = "bfp" ;

        /** Unique relative path for binary shader resources for {@link GLES2#GL_NVIDIA_PLATFORM_BINARY_NV NVIDIA}: <code>{@value}</code> */
        static constexpr std::string_view SUB_PATH_NVIDIA = "nvidia" ;

      private:
        struct Private{ explicit Private() = default; };

      public:

        /** Private ctor for `ShaderCodeRef create(...). */
        ShaderCode(Private, GLenum type, size_t count, const source_list_t& sources) noexcept
        : m_shaderBinaryFormat(0), m_shaderType(0), m_id(0), m_compiled(false)
        {
            if(sources.size() != count) {
                jau_ERR_PRINT("shader number (%zu) and sourceFiles array (%zu) of different length.", count, sources.size());
                return;
            }
            if( !isValidShaderType(type) ) {
                jau_ERR_PRINT("Invalid shader type: %u", type);
                return;
            }
            if ( !jau::do_noexcept([&]() {
                m_shader.reserve(count);
                m_shader.resize(count, 0); } ) ) {
                return;
            }
            m_shaderSource = sources;
            // m_shaderBinary = null;
            m_shaderType   = type;
            m_id = nextID();
        }
        /**
         * @param type either {@link GL2ES2#GL_VERTEX_SHADER}, {@link GL2ES2#GL_FRAGMENT_SHADER}, {@link GL3#GL_GEOMETRY_SHADER},
         *                    {@link GL4#GL_TESS_CONTROL_SHADER}, {@link GL4#GL_TESS_EVALUATION_SHADER} or {@link GL3ES3#GL_COMPUTE_SHADER}.
         * @param count number of shaders
         * @param sources shader sources, organized as <code>source[count][strings-per-shader]</code>.
         * @return successfully created valid ShaderCodeRef or nullptr on failure
         */
        static ShaderCodeSRef create(GLenum type, size_t count, const source_list_t& sources) {
            ShaderCodeSRef res = std::make_shared<ShaderCode>(Private(), type, count, sources);
            if( res->isValid() ) {
                return res;
            }
            return nullptr;
        }

        /** Private ctor for `ShaderCodeRef create(...). */
        ShaderCode(Private, GLenum type, size_t count, GLenum binFormat, const bytes_t& binary) noexcept
        : m_shaderBinaryFormat(0), m_shaderType(0), m_id(0), m_compiled(false)
        {
            if( !isValidShaderType(type) ) {
                jau_ERR_PRINT("Invalid shader type: %u", type);
                return;
            }
            if ( !jau::do_noexcept([&]() {
                m_shader.reserve(count);
                m_shader.resize(count, 0); } ) ) {
                return;
            }
            // shaderSource = null;
            m_shaderBinaryFormat = binFormat;
            m_shaderBinary = binary;
            m_shaderType   = type;
            m_id = nextID();
        }
        /**
         * @param type either {@link GL2ES2#GL_VERTEX_SHADER}, {@link GL2ES2#GL_FRAGMENT_SHADER}, {@link GL3#GL_GEOMETRY_SHADER},
         *                    {@link GL4#GL_TESS_CONTROL_SHADER}, {@link GL4#GL_TESS_EVALUATION_SHADER} or {@link GL3ES3#GL_COMPUTE_SHADER}.
         * @param count number of shaders
         * @param binary binary buffer containing the shader binaries,
         * @return successfully created valid ShaderCodeRef or nullptr on failure
         */
        static ShaderCodeSRef create(GLenum type, size_t count, GLenum binFormat, const bytes_t& binary) {
            ShaderCodeSRef res = std::make_shared<ShaderCode>(Private(), type, count, binFormat, binary);
            if( res->isValid() ) {
                return res;
            }
            return nullptr;
        }

        /**
         * Creates a complete {@link ShaderCode} object while reading all shader source of <code>sourceFiles</code>,
         * which location is resolved using the <code>context</code> class, see readShaderSource().
         *
         * @param gl current GL object to determine whether a shader compiler is available. If null, no validation is performed.
         * @param type either {@link GL2ES2#GL_VERTEX_SHADER}, {@link GL2ES2#GL_FRAGMENT_SHADER}, {@link GL3#GL_GEOMETRY_SHADER},
         *                    {@link GL4#GL_TESS_CONTROL_SHADER}, {@link GL4#GL_TESS_EVALUATION_SHADER} or {@link GL3ES3#GL_COMPUTE_SHADER}.
         * @param count number of shaders
         * @param context class used to help resolving the source location
         * @param sourceFiles array of source locations, organized as <code>sourceFiles[count]</code> -> <code>shaderSources[count][1]</code>
         * @return successfully created valid ShaderCodeRef or nullptr on failure
         *
         * @see #readShaderSource()
         */
        static ShaderCodeSRef create(GL& gl, GLenum type, size_t count, const string_list_t& sourceFiles) {
            if(!ShaderUtil::isShaderCompilerAvailable(gl)) {
                jau_ERR_PRINT("No shader compiler available for %s", gl.toString());
                return nullptr;
            }
            if( !isValidShaderType(type) ) {
                jau_ERR_PRINT("Invalid shader type: %u", type);
                return nullptr;
            }
            string_list_t one_string;
            one_string.reserve(1);
            one_string.resize(1, "");
            const size_t sourceFileCount = sourceFiles.size();
            source_list_t shaderSources;
            bool ok = true;
            if(sourceFileCount > 0) {
                // sourceFiles.length and count is validated in ctor
                shaderSources.reserve(sourceFileCount);
                shaderSources.resize(sourceFileCount, one_string);
                for(size_t i=0; i<sourceFileCount && ok; ++i) {
                    ok = readShaderSource(sourceFiles[i], shaderSources[i][0]);
                }
            }
            return ok ? create(type, count, shaderSources) : nullptr;
        }

#if 0
// FIXME JAU COMPLETE

        /**
         * Creates a complete {@link ShaderCode} object while reading all shader sources from {@link Uri} <code>sourceLocations</code>
         * via {@link #readShaderSource(Uri, boolean)}.
         *
         * @param gl current GL object to determine whether a shader compiler is available. If null, no validation is performed.
         * @param type either {@link GL2ES2#GL_VERTEX_SHADER}, {@link GL2ES2#GL_FRAGMENT_SHADER}, {@link GL3#GL_GEOMETRY_SHADER},
         *                    {@link GL4#GL_TESS_CONTROL_SHADER}, {@link GL4#GL_TESS_EVALUATION_SHADER} or {@link GL3ES3#GL_COMPUTE_SHADER}.
         * @param count number of shaders
         * @param sourceLocations array of {@link Uri} source locations, organized as <code>sourceFiles[count]</code> -> <code>shaderSources[count][1]</code>
         * @return successfully created valid ShaderCodeRef or nullptr on failure
         *
         * @see #readShaderSource(Uri, boolean)
         * @since 2.3.2
         */
        static ShaderCodeRef create(GL& gl, GLenum type, size_t count,
                                    const std::vector<Uri>& sourceLocations, bool mutableStringBuilder) noexcept {
            if(null != gl && !ShaderUtil.isShaderCompilerAvailable(gl)) {
                return null;
            }

            CharSequence[][] shaderSources = null;
            if(null!=sourceLocations) {
                // sourceFiles.length and count is validated in ctor
                shaderSources = new CharSequence[sourceLocations.length][1];
                for(int i=0; i<sourceLocations.length; i++) {
                    try {
                        shaderSources[i][0] = readShaderSource(sourceLocations[i], mutableStringBuilder);
                    } catch (final IOException ioe) {
                        throw RuntimeException("readShaderSource("+sourceLocations[i]+") error: ", ioe);
                    }
                    if(null == shaderSources[i][0]) {
                        shaderSources = null;
                    }
                }
            }
            if(null==shaderSources) {
                return null;
            }
            return new ShaderCode(type, count, shaderSources);
        }

        /**
         * Creates a complete {@link ShaderCode} object while reading the shader binary of <code>binaryFile</code>,
         * which location is resolved using the <code>context</code> class, see {@link #readShaderBinary(Class, String)}.
         *
         * @param type either {@link GL2ES2#GL_VERTEX_SHADER}, {@link GL2ES2#GL_FRAGMENT_SHADER}, {@link GL3#GL_GEOMETRY_SHADER},
         *                    {@link GL4#GL_TESS_CONTROL_SHADER}, {@link GL4#GL_TESS_EVALUATION_SHADER} or {@link GL3ES3#GL_COMPUTE_SHADER}.
         * @param count number of shaders
         * @param context class used to help resolving the source location
         * @param binFormat a valid native binary format as they can be queried by {@link ShaderUtil#getShaderBinaryFormats(GL)}.
         * @param sourceFiles array of source locations, organized as <code>sourceFiles[count]</code>
         * @return successfully created valid ShaderCodeRef or nullptr on failure
         *
         * @see #readShaderBinary(Class, String)
         * @see ShaderUtil#getShaderBinaryFormats(GL)
         */
        public static ShaderCode create(final int type, final int count, final Class<?> context, int binFormat, final String binaryFile) noexcept {
            ByteBuffer shaderBinary = null;
            if(null!=binaryFile && 0<=binFormat) {
                try {
                    shaderBinary = readShaderBinary(context, binaryFile);
                } catch (final IOException ioe) {
                    throw RuntimeException("readShaderBinary("+binaryFile+") error: ", ioe);
                }
                if(null == shaderBinary) {
                    binFormat = -1;
                }
            }
            if(null==shaderBinary) {
                return null;
            }
            return new ShaderCode(type, count, binFormat, shaderBinary);
        }

#endif

        /**
         * Returns a unique suffix for shader resources as follows:
         * <ul>
         *   <li>Source<ul>
         *     <li>{@link GL2ES2#GL_VERTEX_SHADER vertex}: {@link #SUFFIX_VERTEX_SOURCE}</li>
         *     <li>{@link GL2ES2#GL_FRAGMENT_SHADER fragment}: {@link #SUFFIX_FRAGMENT_SOURCE}</li>
         *     <li>{@link GL3#GL_GEOMETRY_SHADER geometry}: {@link #SUFFIX_GEOMETRY_SOURCE}</li>
         *     <li>{@link GL4#GL_TESS_CONTROL_SHADER tess-ctrl}: {@link #SUFFIX_TESS_CONTROL_SOURCE}</li>
         *     <li>{@link GL4#GL_TESS_EVALUATION_SHADER tess-eval}: {@link #SUFFIX_TESS_EVALUATION_SOURCE}</li>
         *     <li>{@link GL3ES3#GL_COMPUTE_SHADER}: {@link #SUFFIX_COMPUTE_SOURCE}</li>
         *     </ul></li>
         *   <li>Binary<ul>
         *     <li>{@link GL2ES2#GL_VERTEX_SHADER vertex}: {@link #SUFFIX_VERTEX_BINARY}</li>
         *     <li>{@link GL2ES2#GL_FRAGMENT_SHADER fragment}: {@link #SUFFIX_FRAGMENT_BINARY}</li>
         *     <li>{@link GL3#GL_GEOMETRY_SHADER geometry}: {@link #SUFFIX_GEOMETRY_BINARY}</li>
         *     <li>{@link GL4#GL_TESS_CONTROL_SHADER tess-ctrl}: {@link #SUFFIX_TESS_CONTROL_BINARY}</li>
         *     <li>{@link GL4#GL_TESS_EVALUATION_SHADER tess-eval}: {@link #SUFFIX_TESS_EVALUATION_BINARY}</li>
         *     <li>{@link GL3ES3#GL_COMPUTE_SHADER}: {@link #SUFFIX_COMPUTE_BINARY}</li>
         *     </ul></li>
         * </ul>
         * @param binary true for a binary resource, false for a source resource
         * @param type either {@link GL2ES2#GL_VERTEX_SHADER}, {@link GL2ES2#GL_FRAGMENT_SHADER}, {@link GL3#GL_GEOMETRY_SHADER},
         *                    {@link GL4#GL_TESS_CONTROL_SHADER}, {@link GL4#GL_TESS_EVALUATION_SHADER} or {@link GL3ES3#GL_COMPUTE_SHADER}.
         * @return suffix for valid shader type or zero sized view if invalid
         *
         * @see #create(GL2ES2, int, Class, String, String, String, boolean)
         */
        static std::string_view getFileSuffix(bool binary, GLenum type) noexcept {
            switch (type) {
                case GL_VERTEX_SHADER: // GL2ES2
                    return binary?SUFFIX_VERTEX_BINARY:SUFFIX_VERTEX_SOURCE;
                case GL_FRAGMENT_SHADER: // GL2ES2
                    return binary?SUFFIX_FRAGMENT_BINARY:SUFFIX_FRAGMENT_SOURCE;
                case GL_GEOMETRY_SHADER: // GL3ES3
                    return binary?SUFFIX_GEOMETRY_BINARY:SUFFIX_GEOMETRY_SOURCE;
                case GL_TESS_CONTROL_SHADER: // GL3ES3
                    return binary?SUFFIX_TESS_CONTROL_BINARY:SUFFIX_TESS_CONTROL_SOURCE;
                case GL_TESS_EVALUATION_SHADER: // GL3ES3
                    return binary?SUFFIX_TESS_EVALUATION_BINARY:SUFFIX_TESS_EVALUATION_SOURCE;
                case GL_COMPUTE_SHADER: // GL3ES3
                    return binary?SUFFIX_COMPUTE_BINARY:SUFFIX_COMPUTE_SOURCE;
                default:
                    return "";
            }
        }

        /**
         * Returns a unique relative path for binary shader resources as follows:
         * <ul>
         *   <li>{@link GLES2#GL_NVIDIA_PLATFORM_BINARY_NV NVIDIA}: {@link #SUB_PATH_NVIDIA}</li>
         * </ul>
         * @return path for valid binary shader types or zero sized view if invalid
         *
         * @see #create(GL2ES2, int, Class, String, String, String, boolean)
         */
        static std::string_view getBinarySubPath(GLenum binFormat) noexcept {
            switch (binFormat) {
                case GL_NVIDIA_PLATFORM_BINARY_NV: // GLES2
                    return SUB_PATH_NVIDIA;
                default:
                    return "";
            }
        }

        /**
         * Convenient creation method for instantiating a complete {@link ShaderCode} object
         * either from source code using {@link #create(GL2ES2, int, int, Class, String[])},
         * or from a binary code using {@link #create(int, int, Class, int, String)},
         * whatever is available first.
         * <p>
         * The source and binary location names are expected w/o suffixes which are
         * resolved and appended using the given {@code srcSuffixOpt} and {@code binSuffixOpt}
         * if not {@code null}, otherwise {@link #getFileSuffix(boolean, int)} determines the suffixes.
         * </p>
         * <p>
         * Additionally, the binary resource is expected within a subfolder of <code>binRoot</code>
         * which reflects the vendor specific binary format, see {@link #getBinarySubPath(int)}.
         * All {@link ShaderUtil#getShaderBinaryFormats(GL)} are being iterated
         * using the binary subfolder, the first existing resource is being used.
         * </p>
         *
         * Example:
         * <pre>
         *   Your std JVM layout (plain or within a JAR):
         *
         *      org/test/glsl/MyShaderTest.class
         *      org/test/glsl/shader/vertex.vp
         *      org/test/glsl/shader/fragment.fp
         *      org/test/glsl/shader/bin/nvidia/vertex.bvp
         *      org/test/glsl/shader/bin/nvidia/fragment.bfp
         *
         *   Your Android APK layout:
         *
         *      classes.dex
         *      assets/org/test/glsl/shader/vertex.vp
         *      assets/org/test/glsl/shader/fragment.fp
         *      assets/org/test/glsl/shader/bin/nvidia/vertex.bvp
         *      assets/org/test/glsl/shader/bin/nvidia/fragment.bfp
         *      ...
         *
         *   Your invocation in org/test/glsl/MyShaderTest.java:
         *
         *      ShaderCode vp0 = ShaderCode.create(gl, GL2ES2.GL_VERTEX_SHADER, 1, this.getClass(),
         *                                         "shader", new String[] { "vertex" }, null,
         *                                         "shader/bin", "vertex", null, true);
         *      ShaderCode fp0 = ShaderCode.create(gl, GL2ES2.GL_FRAGMENT_SHADER, 1, this.getClass(),
         *                                         "shader", new String[] { "vertex" }, null,
         *                                         "shader/bin", "fragment", null, true);
         *      ShaderProgram sp0 = new ShaderProgram();
         *      sp0.add(gl, vp0, System.err);
         *      sp0.add(gl, fp0, System.err);
         *      st.attachShaderProgram(gl, sp0, true);
         * </pre>
         * A simplified entry point is {@link #create(GL, int, String, String, String, boolean)}.
         *
         * <p>
         * The location is finally being resolved using the <code>context</code> class, see {@link #readShaderBinary(Class, String)}.
         * </p>
         *
         * @param gl current GL object to determine whether a shader compiler is available (if <code>source</code> is used),
         *           or to determine the shader binary format (if <code>binary</code> is used).
         * @param type either {@link GL2ES2#GL_VERTEX_SHADER}, {@link GL2ES2#GL_FRAGMENT_SHADER}, {@link GL3#GL_GEOMETRY_SHADER},
         *                    {@link GL4#GL_TESS_CONTROL_SHADER}, {@link GL4#GL_TESS_EVALUATION_SHADER} or {@link GL3ES3#GL_COMPUTE_SHADER}.
         * @param count number of shaders
         * @param context class used to help resolving the source and binary location
         * @param srcRoot relative <i>root</i> path for <code>srcBasenames</code> optional
         * @param srcBasenames basenames w/o path or suffix relative to <code>srcRoot</code> for the shader's source code
         * @param srcSuffixOpt optional custom suffix for shader's source file,
         *                     if {@code null} {@link #getFileSuffix(boolean, int)} is being used.
         * @param binRoot relative <i>root</i> path for <code>binBasenames</code>
         * @param binBasename basename w/o path or suffix relative to <code>binRoot</code> for the shader's binary code
         * @param binSuffixOpt optional custom suffix for shader's binary file,
         *                     if {@code null} {@link #getFileSuffix(boolean, int)} is being used.
         * @return successfully created valid ShaderCodeRef or nullptr on failure
         *
         * @see #create(GL2ES2, int, int, Class, String[])
         * @see #create(int, int, Class, int, String)
         * @see #readShaderSource(Class, String)
         * @see #getFileSuffix(boolean, int)
         * @see ShaderUtil#getShaderBinaryFormats(GL)
         * @see #getBinarySubPath(int)
         *
         * @since 2.3.2
         */
        static ShaderCodeSRef create(GL& gl, GLenum type, size_t count,
                                    stringview_t srcRoot, const string_list_t& srcBasenames, stringview_t srcSuffixOpt,
                                    stringview_t binRoot, stringview_t binBasename, stringview_t binSuffixOpt) noexcept {
            ShaderCodeSRef res;
            string_t srcPathsString;
            string_t binFileName;

            if( !srcBasenames.empty() && ShaderUtil::isShaderCompilerAvailable(gl) ) {
                string_list_t srcPaths;
                srcPaths.resize(srcBasenames.size());
                stringview_t srcSuffix = !srcSuffixOpt.empty() ? srcSuffixOpt : getFileSuffix(false, type);
                if( !srcRoot.empty() ) {
                    for(size_t i=0; i<srcPaths.size(); ++i) {
                        srcPaths[i] = string_t(srcRoot).append("/").append(srcBasenames[i]).append(".").append(srcSuffix);
                    }
                } else {
                    for(size_t i=0; i<srcPaths.size(); ++i) {
                        srcPaths[i] = string_t(srcBasenames[i]).append(".").append(srcSuffix);
                    }
                }
                res = create(gl, type, count, srcPaths);
                if(res) {
                    return res;
                }
                for(const string_t& s : srcPaths) {
                    if( !srcPathsString.empty() ) {
                        srcPathsString.append(";");
                    }
                    srcPathsString.append(s);
                }
            }
            if( !binBasename.empty() ) {
                name_list_t binFmts = ShaderUtil::getShaderBinaryFormats(gl);
                stringview_t binSuffix = !binSuffixOpt.empty() ? binSuffixOpt : getFileSuffix(true, type);
                for(GLenum bFmt : binFmts) {
                    string_t bFmtPath = string_t(getBinarySubPath(bFmt));
                    if(bFmtPath.empty()) continue;
                    binFileName = string_t(binRoot).append("/").append(bFmtPath).append("/").append(binBasename).append(".").append(binSuffix);
                    // res = create(type, count, bFmt, binFileName);
                }
            }
            return res;
        }

        /**
         * Simplified variation of {@link #create(GL2ES2, int, int, Class, String, String[], String, String, String, String, boolean)}.
         * <p>
         * Convenient creation method for instantiating a complete {@link ShaderCode} object
         * either from source code using {@link #create(GL2ES2, int, int, Class, String[])},
         * or from a binary code using {@link #create(int, int, Class, int, String)},
         * whatever is available first.
         * </p>
         * <p>
         * The source and binary location names are expected w/o suffixes which are
         * resolved and appended using {@link #getFileSuffix(boolean, int)}.
         * </p>
         * <p>
         * Additionally, the binary resource is expected within a subfolder of <code>binRoot</code>
         * which reflects the vendor specific binary format, see {@link #getBinarySubPath(int)}.
         * All {@link ShaderUtil#getShaderBinaryFormats(GL)} are being iterated
         * using the binary subfolder, the first existing resource is being used.
         * </p>
         *
         * Example:
         * <pre>
         *   Your std JVM layout (plain or within a JAR):
         *
         *      org/test/glsl/MyShaderTest.class
         *      org/test/glsl/shader/vertex.vp
         *      org/test/glsl/shader/fragment.fp
         *      org/test/glsl/shader/bin/nvidia/vertex.bvp
         *      org/test/glsl/shader/bin/nvidia/fragment.bfp
         *
         *   Your Android APK layout:
         *
         *      classes.dex
         *      assets/org/test/glsl/shader/vertex.vp
         *      assets/org/test/glsl/shader/fragment.fp
         *      assets/org/test/glsl/shader/bin/nvidia/vertex.bvp
         *      assets/org/test/glsl/shader/bin/nvidia/fragment.bfp
         *      ...
         *
         *   Your invocation in org/test/glsl/MyShaderTest.java:
         *
         *      ShaderCode vp0 = ShaderCode.create(gl, GL2ES2.GL_VERTEX_SHADER, 1, this.getClass(),
         *                                         "shader", new String[] { "vertex" }, "shader/bin", "vertex", true);
         *      ShaderCode fp0 = ShaderCode.create(gl, GL2ES2.GL_FRAGMENT_SHADER, 1, this.getClass(),
         *                                         "shader", new String[] { "vertex" }, "shader/bin", "fragment", true);
         *      ShaderProgram sp0 = new ShaderProgram();
         *      sp0.add(gl, vp0, System.err);
         *      sp0.add(gl, fp0, System.err);
         *      st.attachShaderProgram(gl, sp0, true);
         * </pre>
         * A simplified entry point is {@link #create(GL2ES2, int, Class, String, String, String, boolean)}.
         *
         * <p>
         * The location is finally being resolved using the <code>context</code> class, see {@link #readShaderBinary(Class, String)}.
         * </p>
         *
         * @param gl current GL object to determine whether a shader compiler is available (if <code>source</code> is used),
         *           or to determine the shader binary format (if <code>binary</code> is used).
         * @param type either {@link GL2ES2#GL_VERTEX_SHADER}, {@link GL2ES2#GL_FRAGMENT_SHADER}, {@link GL3#GL_GEOMETRY_SHADER},
         *                    {@link GL4#GL_TESS_CONTROL_SHADER}, {@link GL4#GL_TESS_EVALUATION_SHADER} or {@link GL3ES3#GL_COMPUTE_SHADER}.
         * @param count number of shaders
         * @param context class used to help resolving the source and binary location
         * @param srcRoot relative <i>root</i> path for <code>srcBasenames</code> optional
         * @param srcBasenames basenames w/o path or suffix relative to <code>srcRoot</code> for the shader's source code
         * @param binRoot relative <i>root</i> path for <code>binBasenames</code>
         * @param binBasename basename w/o path or suffix relative to <code>binRoot</code> for the shader's binary code
         * @return successfully created valid ShaderCodeRef or nullptr on failure
         *
         * @see #create(GL2ES2, int, int, Class, String, String[], String, String, String, String, boolean)
         * @see #readShaderSource(Class, String)
         * @see #getFileSuffix(boolean, int)
         * @see ShaderUtil#getShaderBinaryFormats(GL)
         * @see #getBinarySubPath(int)
         */
        static ShaderCodeSRef create(GL& gl, GLenum type, size_t count,
                                    stringview_t srcRoot, const string_list_t& srcBasenames,
                                    stringview_t binRoot, stringview_t binBasename) noexcept {
                return create(gl, type, count, srcRoot, srcBasenames, "", binRoot, binBasename, "");
        }

        /**
         * Simplified variation of {@link #create(GL2ES2, int, int, Class, String, String[], String, String, String, String, boolean)}.
         * <p>
         * Example:
         * <pre>
         *   Your std JVM layout (plain or within a JAR):
         *
         *      org/test/glsl/MyShaderTest.class
         *      org/test/glsl/shader/vertex.vp
         *      org/test/glsl/shader/fragment.fp
         *      org/test/glsl/shader/bin/nvidia/vertex.bvp
         *      org/test/glsl/shader/bin/nvidia/fragment.bfp
         *
         *   Your Android APK layout:
         *
         *      classes.dex
         *      assets/org/test/glsl/shader/vertex.vp
         *      assets/org/test/glsl/shader/fragment.fp
         *      assets/org/test/glsl/shader/bin/nvidia/vertex.bvp
         *      assets/org/test/glsl/shader/bin/nvidia/fragment.bfp
         *      ...
         *
         *   Your invocation in org/test/glsl/MyShaderTest.java:
         *
         *      ShaderCode vp0 = ShaderCode.create(gl, GL2ES2.GL_VERTEX_SHADER, this.getClass(),
         *                                         "shader", "shader/bin", "vertex", null, null, true);
         *      ShaderCode fp0 = ShaderCode.create(gl, GL2ES2.GL_FRAGMENT_SHADER, this.getClass(),
         *                                         "shader", "shader/bin", "fragment", null, null, true);
         *      ShaderProgram sp0 = new ShaderProgram();
         *      sp0.add(gl, vp0, System.err);
         *      sp0.add(gl, fp0, System.err);
         *      st.attachShaderProgram(gl, sp0, true);
         * </pre>
         * </p>
         *
         * @param gl current GL object to determine whether a shader compiler is available (if <code>source</code> is used),
         *           or to determine the shader binary format (if <code>binary</code> is used).
         * @param type either {@link GL2ES2#GL_VERTEX_SHADER}, {@link GL2ES2#GL_FRAGMENT_SHADER}, {@link GL3#GL_GEOMETRY_SHADER},
         *                    {@link GL4#GL_TESS_CONTROL_SHADER}, {@link GL4#GL_TESS_EVALUATION_SHADER} or {@link GL3ES3#GL_COMPUTE_SHADER}.
         * @param context class used to help resolving the source and binary location
         * @param srcRoot relative <i>root</i> path for <code>basename</code> optional
         * @param binRoot relative <i>root</i> path for <code>basename</code>
         * @param basename basename w/o path or suffix relative to <code>srcRoot</code> and <code>binRoot</code>
         *                 for the shader's source and binary code.
         * @param srcSuffixOpt optional custom suffix for shader's source file,
         *                     if {@code null} {@link #getFileSuffix(boolean, int)} is being used.
         * @param binSuffixOpt optional custom suffix for shader's binary file,
         *                     if {@code null} {@link #getFileSuffix(boolean, int)} is being used.
         * @return successfully created valid ShaderCodeRef or nullptr on failure
         *
         * @see #create(GL2ES2, int, int, Class, String, String[], String, String, String, String, boolean)
         * @since 2.3.2
         */
        static ShaderCodeSRef create(GL& gl, GLenum type, stringview_t srcRoot, stringview_t binRoot,
                                    stringview_t basename, stringview_t srcSuffixOpt, stringview_t binSuffixOpt) noexcept {
            string_list_t srcBasenames = { string_t(basename) };
            return create(gl, type, 1, srcRoot, srcBasenames, srcSuffixOpt, binRoot, basename, binSuffixOpt);
        }

        /**
         * Simplified variation of {@link #create(GL2ES2, int, Class, String, String, String, String, String, boolean)}.
         * <p>
         * Example:
         * <pre>
         *   Your std JVM layout (plain or within a JAR):
         *
         *      org/test/glsl/MyShaderTest.class
         *      org/test/glsl/shader/vertex.vp
         *      org/test/glsl/shader/fragment.fp
         *      org/test/glsl/shader/bin/nvidia/vertex.bvp
         *      org/test/glsl/shader/bin/nvidia/fragment.bfp
         *
         *   Your Android APK layout:
         *
         *      classes.dex
         *      assets/org/test/glsl/shader/vertex.vp
         *      assets/org/test/glsl/shader/fragment.fp
         *      assets/org/test/glsl/shader/bin/nvidia/vertex.bvp
         *      assets/org/test/glsl/shader/bin/nvidia/fragment.bfp
         *      ...
         *
         *   Your invocation in org/test/glsl/MyShaderTest.java:
         *
         *      ShaderCode vp0 = ShaderCode.create(gl, GL2ES2.GL_VERTEX_SHADER, this.getClass(),
         *                                         "shader", "shader/bin", "vertex", true);
         *      ShaderCode fp0 = ShaderCode.create(gl, GL2ES2.GL_FRAGMENT_SHADER, this.getClass(),
         *                                         "shader", "shader/bin", "fragment", true);
         *      ShaderProgram sp0 = new ShaderProgram();
         *      sp0.add(gl, vp0, System.err);
         *      sp0.add(gl, fp0, System.err);
         *      st.attachShaderProgram(gl, sp0, true);
         * </pre>
         * </p>
         *
         * @param gl current GL object to determine whether a shader compiler is available (if <code>source</code> is used),
         *           or to determine the shader binary format (if <code>binary</code> is used).
         * @param type either {@link GL2ES2#GL_VERTEX_SHADER}, {@link GL2ES2#GL_FRAGMENT_SHADER}, {@link GL3#GL_GEOMETRY_SHADER},
         *                    {@link GL4#GL_TESS_CONTROL_SHADER}, {@link GL4#GL_TESS_EVALUATION_SHADER} or {@link GL3ES3#GL_COMPUTE_SHADER}.
         * @param context class used to help resolving the source and binary location
         * @param srcRoot relative <i>root</i> path for <code>basename</code> optional
         * @param binRoot relative <i>root</i> path for <code>basename</code>
         * @param basenames basename w/o path or suffix relative to <code>srcRoot</code> and <code>binRoot</code>
         *                  for the shader's source and binary code.
         * @return successfully created valid ShaderCodeRef or nullptr on failure
         */
        static ShaderCodeSRef create(GL& gl, GLenum type,
                                    stringview_t srcRoot, stringview_t binRoot, stringview_t basename) noexcept {
            return create(gl, type, srcRoot, binRoot, basename, "", "");
        }

        /** Returns the unique shader id for successfully created instances, zero if instance creation failed. */
        constexpr size_t id() const noexcept { return m_id; }

        /** Returns true if this instance is valid, i.e. 0 != id() */
        constexpr bool isValid() const noexcept { return 0 != m_id; }

        /** Returns true if this instance is valid and compiled. */
        constexpr bool isCompiled() const noexcept { return m_compiled; }

        GLenum shaderType() const noexcept { return m_shaderType; }
        string_t shaderTypeStr() const noexcept { return string_t(shaderTypeString(m_shaderType)); }

        GLenum shaderBinaryFormat() const noexcept { return m_shaderBinaryFormat; }
        const bytes_t& shaderBinary() const noexcept { return m_shaderBinary; }
        const source_list_t& shaderSource() const noexcept { return m_shaderSource; }

        const shader_list_t& shader() const noexcept { return m_shader; }

        bool compile(GL& gl, bool verbose=false) {
            if(!isValid()) return false;
            if(isCompiled()) return true;

            // Create & Compile the vertex/fragment shader objects
            if(!m_shaderSource.empty()) {
                if(DEBUG_CODE) {
                    jau_PLAIN_PRINT(true, "ShaderCode.compile");
                    dumpSource(); // NOLINT(bugprone-exception-escape)
                }
                m_compiled=ShaderUtil::createAndCompileShader(gl, m_shader, m_shaderType,
                                                              m_shaderSource, verbose);
            } else if(!m_shaderBinary.empty()) {
                m_compiled=ShaderUtil::createAndLoadShader(gl, m_shader, m_shaderType,
                                                           m_shaderBinaryFormat, m_shaderBinary, verbose);
            } else if(DEBUG_CODE) {
                jau_PLAIN_PRINT(true, "ShaderCode.compile: No code");
                dumpSource(); // NOLINT(bugprone-exception-escape)
            }
            return m_compiled;
        }

        void destroy(GL& gl) noexcept {
            if(isCompiled()) {
                ShaderUtil::deleteShader(gl, shader());
                m_compiled=false;
            }
            if(!m_shaderBinary.empty()) {
                m_shaderBinary.clear();
            }
            m_shaderSource.clear();
            m_shaderBinaryFormat=0;
            m_shaderType=0;
        }

        constexpr bool operator==(const ShaderCode& rhs) const noexcept {
            if(this==&rhs) { return true; }
            return m_id == rhs.m_id;
        }
        constexpr std::size_t hash_code() const noexcept { return m_id; }

        string_t toString() const {
            string_t r("ShaderCode[id=");
            r.append(std::to_string(m_id)).append(", type=").append(shaderTypeStr())
             .append(", valid=").append(std::to_string(isValid())).append(", compiled=").append(std::to_string(m_compiled))
             .append(", ").append(std::to_string(m_shader.size())).append(" shader: ");
            for(GLuint s : m_shader) {
                r.append(" ").append(std::to_string(s));
            }
            if(!m_shaderSource.empty()) {
                r.append(", source]");
            } else if(!m_shaderBinary.empty()) {
                r.append(", binary]");
            }
            return r;
        }


        void dumpSource() {
            if(m_shaderSource.empty()) {
                jau_PLAIN_PRINT(true, "<no shader source>");
                return;
            }
            const size_t sourceCount = m_shaderSource.size();
            const size_t shaderCount = m_shader.size();
            jau_PLAIN_PRINT(true, "");
            jau_PLAIN_PRINT(true, "ShaderCode[id=%zu, type=%s, valid=%d, compiled=%d, %zu/%zu shader:",
                m_id, shaderTypeStr(), isValid(), m_compiled, m_shader.size(), shaderCount);
            if( 0 == shaderCount ) {
                jau_PLAIN_PRINT(true, "none]");
            }
            for(size_t i=0; i<shaderCount; ++i) {
                jau_PLAIN_PRINT(true, "");
                jau_PLAIN_PRINT(true, "Shader #%zu/%zu name %u", i, shaderCount, m_shader[i]);
                jau_PLAIN_PRINT(true, "--------------------------------------------------------------");
                if(i>=sourceCount) {
                    jau_PLAIN_PRINT(true, "<no shader source>");
                } else {
                    const string_list_t& src = m_shaderSource[i];
                    int lineno=0;

                    for(size_t j=0; j<src.size(); j++) {
                        jau_PLAIN_PRINT(false, "%4d: // Segment %zu/%zu:", lineno, j, src.size());
                        std::istringstream reader(src[j]);
                        string_t line;
                        while (std::getline(reader, line)) {
                            ++lineno;
                            jau_PLAIN_PRINT(false, "%4d: %s", lineno, line);
                        }
                    }
                }
                jau_PLAIN_PRINT(true, "--------------------------------------------------------------");
            }
            jau_PLAIN_PRINT(true, "]");
        }

        /**
         * Adds <code>data</code> after the line containing <code>tag</code>.
         *
         * @param shaderIdx the shader index to be used.
         * @param tag search string
         * @param fromIndex start search <code>tag</code> begininig with this index
         * @param data the text to be inserted. Shall end with an EOL '\n' character.
         * @return index after the inserted <code>data</code> or std::string::npos if tag wasn't found or on error
         */
        size_t insertShaderSource(size_t shaderIdx, stringview_t tag, size_t fromIndex, stringview_t data) noexcept {
            if(m_shaderSource.empty()) {
                jau_ERR_PRINT("no shader source");
                return string_t::npos;
            }
            const size_t shaderCount = m_shader.size();
            if(shaderIdx>=shaderCount) {
                jau_ERR_PRINT("shaderIdx %zu not within shader bounds %zu", shaderIdx, shaderCount);
                return string_t::npos;
            }
            const size_t sourceCount = m_shaderSource.size();
            if(shaderIdx>=sourceCount) {
                jau_ERR_PRINT("shaderIdx %zu not within source bounds %zu", shaderIdx, shaderCount);
                return string_t::npos;
            }
            string_list_t& src = m_shaderSource[shaderIdx];
            if(src.empty()) {
                jau_ERR_PRINT("no shader source at for shader index %zu", shaderIdx);
                return string_t::npos;
            }
            size_t curEndIndex = 0;
            for(auto & sb : src) {
                curEndIndex += sb.length();
                if(fromIndex < curEndIndex) {
                    size_t insertIdx = sb.find(tag, fromIndex);
                    if(string_t::npos != insertIdx) {
                        insertIdx += tag.length();
                        size_t eol = sb.find('\n', insertIdx); // eol: covers \n and \r\n
                        if(string_t::npos == eol) {
                            eol = sb.find('\r', insertIdx); // eol: covers \r 'only'
                        }
                        if(string_t::npos != eol) {
                            insertIdx = eol+1;  // eol found
                        } else {
                            sb.insert(insertIdx, "\n"); // add eol
                            ++insertIdx;
                        }
                        sb.insert(insertIdx, data);
                        return insertIdx+data.length();
                    }
                }
            }
            return string_t::npos;
        }

        /**
         * Adds <code>data</code> at <code>position</code> in shader source for shader <code>shaderIdx</code>.
         *
         * @param shaderIdx the shader index to be used.
         * @param position in shader source segments of shader <code>shaderIdx</code>, if exceeding source length (like std::string::npos or) data will be appended
         * @param data the text to be inserted. Shall end with an EOL '\n' character
         * @return index after the inserted `data` or std::string::npos on error
         */
        size_t insertShaderSource(size_t shaderIdx, size_t position, stringview_t data) noexcept {
            if(m_shaderSource.empty()) {
                jau_ERR_PRINT("no shader source");
                return string_t::npos;
            }
            const size_t shaderCount = m_shader.size();
            if(shaderIdx>=shaderCount) {
                jau_ERR_PRINT("shaderIdx %zu not within shader bounds %zu", shaderIdx, shaderCount);
                return string_t::npos;
            }
            const size_t sourceCount = m_shaderSource.size();
            if(shaderIdx>=sourceCount) {
                jau_ERR_PRINT("shaderIdx %zu not within source bounds %zu", shaderIdx, shaderCount);
                return string_t::npos;
            }
            string_list_t& src = m_shaderSource[shaderIdx];
            if(src.empty()) {
                jau_ERR_PRINT("no shader source at for shader index %zu", shaderIdx);
                return string_t::npos;
            }
            size_t curEndIndex = 0;
            size_t j=0;
            for(; j<src.size()-1 && position > curEndIndex; ++j) {
                curEndIndex += src[j].length();
            }
            string_t& sb = src[j];
            const size_t lastEndIndex = curEndIndex;
            curEndIndex += src[j].length();
            if( position > curEndIndex ) {
                position = curEndIndex;
            }
            sb.insert(position - lastEndIndex, data);
            return position+data.length();
        }

        /**
         * Adds shader source located in <code>path</code>,
         * either relative to the <code>location</code> or absolute <i>as-is</i>
         * at <code>position</code> in shader source for shader <code>shaderIdx</code>.
         *
         * @param shaderIdx the shader index to be used.
         * @param position in shader source segments of shader <code>shaderIdx</code>, -1 will append data
         * @param path location of shader source
         * @return index after the inserted code or std::string::npos on error
         */
        size_t insertShaderSourceFile(size_t shaderIdx, size_t position, const string_t& path) noexcept {
            string_t data;
            if( !readShaderSource(path, data) ) {
                return string_t::npos;
            }
            return insertShaderSource(shaderIdx, position, data);
        }

        /**
         * Replaces <code>oldName</code> with <code>newName</code> in all shader sources.
         * <p>
         * In case <code>oldName</code> and <code>newName</code> are equal, no action is performed.
         * </p>
         *
         * @param oldName the to be replace string
         * @param newName the replacement string
         * @return the number of replacements, zero on error or no replacement
         */
        size_t replaceInShaderSource(const string_t& oldName, const string_t& newName) noexcept {
            if(m_shaderSource.empty() || oldName == newName) {
                return 0;
            }
            const size_t oldNameLen = oldName.length();
            const size_t newNameLen = newName.length();
            size_t num = 0;
            const size_t sourceCount = m_shaderSource.size();
            for(size_t shaderIdx = 0; shaderIdx<sourceCount; ++shaderIdx) {
                string_list_t& src = m_shaderSource[shaderIdx];
                for(auto & sb : src) {
                    size_t curPos = 0;
                    while(curPos<sb.length()-oldNameLen+1) {
                        const size_t startIdx = sb.find(oldName, curPos);
                        if(string_t::npos != startIdx) {
                            sb.replace(startIdx, oldNameLen, newName);
                            curPos = startIdx + newNameLen;
                            ++num;
                        } else {
                            curPos = sb.length();
                        }
                    }
                }
            }
            return num;
        }

      private:
        /** Returns true if successful, otherwise false. */
        static bool readShaderSource(const string_t& conn, string_t& result, int& lineno) noexcept {
            if(DEBUG_CODE) {
                if(0 == lineno) {
                    result.append("// '"+conn+"'\n"); // conn.getURL().toExternalForm()
                } else {
                    result.append("// included @ line "+std::to_string(lineno)+": '"+conn+"'\n"); // conn.getURL().toExternalForm()
                }
            }
            std::ifstream reader(conn);
            string_t line;
            while ( std::getline(reader, line) ) {
                ++lineno;
                if (line.starts_with("#include ")) {
                    string_t includeFile;
                    {
                        string_t s = line.substr(9);
                        jau::trimInPlace( s );
                        // Bug 1283: Remove shader include filename quotes if exists at start and end only
                        if( s.starts_with("\"") && s.ends_with("\"")) {
                            s = s.substr(1, s.length()-2);
                        }
                        includeFile = s;
                    }
                    string_t nextConn = gamp::resolve_asset(includeFile);

                    if( nextConn.empty() && !jau::io::fs::isAbsolute(includeFile) ) {
                        // Try relative of current shader location
                        includeFile = jau::io::fs::dirname(conn).append("/").append(includeFile);
                        if( jau::io::fs::exists(includeFile) ) {
                            nextConn = includeFile;
                        }
                    }
                    if( nextConn.empty() ) {
                        return false;
                    }
                    if( DEBUG_CODE ) {
                        jau_PLAIN_PRINT(true, "readShaderSource: including '%s' -> '%s'", includeFile, nextConn);
                    }
                    lineno = readShaderSource(nextConn, result, lineno);
                } else {
                    result.append(line + "\n");
                }
            }
            return true;
        }

      public:
        /**
         * Reads shader source from file located in `path` and appends it to result.
         *
         * @param path location of shader source
         * @param result storage to appends the source code
         * @return true if successful, otherwise false
         */
        static bool readShaderSource(stringview_t path0, string_t& result) noexcept {
            const string_t path(path0);
            const string_t conn = gamp::resolve_asset(path);
            if( DEBUG_CODE ) {
                jau_PLAIN_PRINT(true, "readShaderSource: %s -> %s", path, conn);
            }
            if (!conn.empty()) {
                int lineno=0;
                return readShaderSource(conn, result, lineno);
            }
            return false;
        }

        /**
         * Reads shader source from file located in `path`, returning the string.
         *
         * Final location lookup is performed via `gamp::resolve_asset(path)`.
         *
         * @param path location of shader source
         * @return a non empty string containing the source code if successful, otherwise an empty string
         * @see gamp::resolve_asset
         */
        static string_t readShaderSource(stringview_t path) noexcept {
            string_t result;
            if( !readShaderSource(path, result) ) {
                result.clear();
            }
            return result;
        }

#if 0
// FIXME JAU COMPLETE

        /**
         * Reads shader binary located in <code>path</code>,
         * either relative to the <code>context</code> class or absolute <i>as-is</i>.
         * <p>
         * Final location lookup is perfomed via {@link ClassLoader#getResource(String)} and {@link ClassLoader#getSystemResource(String)},
         * see {@link IOUtil#getResource(Class, String)}.
         * </p>
         *
         * @param context class used to help resolve the source location
         * @param path location of shader binary
         *
         * @see IOUtil#getResource(Class, String)
         */
        public static ByteBuffer readShaderBinary(final Class<?> context, final String path) noexcept {
            final URLConnection conn = IOUtil.getResource(path, context.getClassLoader(), context);
            if (conn == null) {
                return null;
            }
            final BufferedInputStream bis = new BufferedInputStream( conn.getInputStream() );
            try {
                return IOUtil.copyStream2ByteBuffer( bis );
            } finally {
                IOUtil.close(bis, false);
            }
        }

        /**
         * Reads shader binary located from {@link Uri#absolute} {@link Uri} <code>binLocation</code>.
         * @param binLocation {@link Uri} location of shader binary
         * @since 2.3.2
         */
        public static ByteBuffer readShaderBinary(final Uri binLocation) noexcept {
            final URLConnection conn = IOUtil.openURL(binLocation.toURL(), "ShaderCode ");
            if (conn == null) {
                return null;
            }
            final BufferedInputStream bis = new BufferedInputStream( conn.getInputStream() );
            try {
                return IOUtil.copyStream2ByteBuffer( bis );
            } finally {
                IOUtil.close(bis, false);
            }
        }
#endif

        // Shall we use: #ifdef GL_FRAGMENT_PRECISION_HIGH .. #endif for using highp in fragment shader if avail ?
        /** Default precision of {@link GL#isGLES2() ES2} for {@link GL2ES2#GL_VERTEX_SHADER vertex-shader}: {@value #es2_default_precision_vp} */
        constexpr static std::string_view es2_default_precision_vp = "\nprecision highp float;\nprecision highp int;\n/*precision lowp sampler2D;*/\n/*precision lowp samplerCube;*/\n";
        /** Default precision of {@link GL#isGLES2() ES2} for {@link GL2ES2#GL_FRAGMENT_SHADER fragment-shader}: {@value #es2_default_precision_fp} */
        constexpr static std::string_view es2_default_precision_fp = "\nprecision mediump float;\nprecision mediump int;\n/*precision lowp sampler2D;*/\n/*precision lowp samplerCube;*/\n";

        /** Default precision of {@link GL#isGLES3() ES3} for {@link GL2ES2#GL_VERTEX_SHADER vertex-shader}: {@value #es3_default_precision_vp} */
        constexpr static std::string_view es3_default_precision_vp = es2_default_precision_vp;
        /**
         * Default precision of {@link GL#isGLES3() ES3} for {@link GL2ES2#GL_FRAGMENT_SHADER fragment-shader}: {@value #es3_default_precision_fp},
         * same as for {@link GL2ES2#GL_VERTEX_SHADER vertex-shader}, i.e {@link #es3_default_precision_vp},
         * due to ES 3.x requirements of using same precision for uniforms!
         */
        constexpr static std::string_view es3_default_precision_fp = es3_default_precision_vp;

        /** Default precision of GLSL &ge; 1.30 as required until &lt; 1.50 for {@link GL2ES2#GL_VERTEX_SHADER vertex-shader} or {@link GL3#GL_GEOMETRY_SHADER geometry-shader}: {@value #gl3_default_precision_vp_gp}. See GLSL Spec 1.30-1.50 Section 4.5.3. */
        constexpr static std::string_view gl3_default_precision_vp_gp = "\nprecision highp float;\nprecision highp int;\n";
        /** Default precision of GLSL &ge; 1.30 as required until &lt; 1.50 for {@link GL2ES2#GL_FRAGMENT_SHADER fragment-shader}: {@value #gl3_default_precision_fp}. See GLSL Spec 1.30-1.50 Section 4.5.3. */
        constexpr static std::string_view gl3_default_precision_fp = "\nprecision highp float;\nprecision mediump int;\n/*precision mediump sampler2D;*/\n";

        /** <i>Behavior</i> for GLSL extension directive, see {@link #createExtensionDirective(String, String)}, value {@value}. */
        constexpr static std::string_view REQUIRE = "require";
        /** <i>Behavior</i> for GLSL extension directive, see {@link #createExtensionDirective(String, String)}, value {@value}. */
        constexpr static std::string_view ENABLE = "enable";
        /** <i>Behavior</i> for GLSL extension directive, see {@link #createExtensionDirective(String, String)}, value {@value}. */
        constexpr static std::string_view DISABLE = "disable";
        /** <i>Behavior</i> for GLSL extension directive, see {@link #createExtensionDirective(String, String)}, value {@value}. */
        constexpr static std::string_view WARN = "warn";

      private:
        constexpr static std::string_view vp_130_defines =
                                                       "#define attribute in\n"
                                                       "#define varying out\n"
                                                       "#define IN in\n";
        constexpr static std::string_view vp_100_defines =
                                                        "#define IN\n";
        constexpr static std::string_view fp_130_defines =
                                               "#define varying in\n"
                                               "#define IN in\n"
                                               "out vec4 mgl_FragColor;\n"
                                               "#define texture2D texture\n";

        constexpr static std::string_view fp_100_defines =
                                               "#define IN\n"
                                               "#define mgl_FragColor gl_FragColor\n";
      public:
        /**
         * Creates a GLSL extension directive.
         * <p>
         * Prefer {@link #ENABLE} over {@link #REQUIRE}, since the latter will force a failure if not supported.
         * </p>
         *
         * @param extensionName
         * @param behavior shall be either {@link #REQUIRE}, {@link #ENABLE}, {@link #DISABLE} or {@link #WARN}
         * @return the complete extension directive
         */
        static string_t createExtensionDirective(const string_t& extensionName, const string_t& behavior) {
            return "#extension " + extensionName + " : " + behavior + "\n";
        }

        /**
         * Add GLSL version at the head of this shader source code.
         * @param gl a GL context, which must have been made current once
         * @return the index after the inserted data, maybe 0 if nothing has be inserted.
         */
        size_t addGLSLVersion(const GL& gl) noexcept {
            return insertShaderSource(0, 0, gl.glProfile().getGLSLVersionString());
        }

        /**
         * Adds default precision to source code at given position if required, i.e.
         * {@link #es2_default_precision_vp}, {@link #es2_default_precision_fp},
         * {@link #gl3_default_precision_vp_gp}, {@link #gl3_default_precision_fp} or none,
         * depending on the {@link GLContext#getGLSLVersionNumber() GLSL version} being used.
         * @param gl a GL context, which must have been made current once
         * @param pos position within this mutable shader source.
         * @return the index after the inserted data, maybe unchanged pos if nothing has be inserted.
         */
        size_t addDefaultShaderPrecision(const GL& gl, size_t pos) {
            std::string_view defaultPrecision;
            if( gl.glProfile().nativeGLES3() ) {
                switch ( m_shaderType ) {
                    case GL_VERTEX_SHADER:
                        defaultPrecision = es3_default_precision_vp; break;
                    case GL_FRAGMENT_SHADER:
                        defaultPrecision = es3_default_precision_fp; break;
                    case GL_COMPUTE_SHADER:
                        defaultPrecision = es3_default_precision_fp; break;
                    default:
                        defaultPrecision = "";
                        break;
                }
            } else if( gl.glProfile().nativeGLES2() ) {
                switch ( m_shaderType ) {
                    case GL_VERTEX_SHADER:
                        defaultPrecision = es2_default_precision_vp; break;
                    case GL_FRAGMENT_SHADER:
                        defaultPrecision = es2_default_precision_fp; break;
                    default:
                        defaultPrecision = "";
                        break;
                }
            } else if( requiresGL3DefaultPrecision(gl) ) {
                // GLSL [ 1.30 .. 1.50 [ needs at least fragement float default precision!
                switch ( m_shaderType ) {
                    case GL_VERTEX_SHADER:
                    case GL_GEOMETRY_SHADER:
                    case GL_TESS_CONTROL_SHADER:
                    case GL_TESS_EVALUATION_SHADER:
                        defaultPrecision = gl3_default_precision_vp_gp; break;
                    case GL_FRAGMENT_SHADER:
                        defaultPrecision = gl3_default_precision_fp; break;
                    case GL_COMPUTE_SHADER:
                        defaultPrecision = gl3_default_precision_fp; break;
                    default:
                        defaultPrecision = "";
                        break;
                }
            } else {
                defaultPrecision = "";
            }
            if( defaultPrecision.length() > 0 ) {
                pos = insertShaderSource(0, pos, string_t(defaultPrecision));
            }
            return pos;
        }

        /** Returns true, if GLSL version requires default precision, i.e. ES2 or GLSL [1.30 .. 1.50[. */
        constexpr static bool requiresDefaultPrecision(const GL& gl) noexcept {
            if( gl.glProfile().nativeGLES() ) {
                return true;
            }
            return requiresGL3DefaultPrecision(gl);
        }

        /** Returns true, if GL3 GLSL version requires default precision, i.e. GLSL [1.30 .. 1.50[. */
        constexpr static bool requiresGL3DefaultPrecision(const GL& gl) noexcept {
            if( !gl.glProfile().nativeGLES() ) {
                const jau::util::VersionNumber& glslVersion = gl.glProfile().glslVersion();
                return glslVersion >= Version1_30 && glslVersion < Version1_50;
            } else {
                return false;
            }
        }

        /**
         * Add GLSL version and shader-type dependent defines of this shader source code.
         * @param gl a GL context, which must have been made current once
         * @param pos position within this mutable shader source.
         * @return the index after the inserted data, maybe unchanged pos if nothing has be inserted.
         */
        size_t addGLSLDefines(const GL& gl, size_t pos) noexcept {
            const jau::util::VersionNumber& glslVersion = gl.glProfile().glslVersion();
            if ( GL_VERTEX_SHADER == m_shaderType ) {
                pos = insertShaderSource(0, pos, "\n");
                if( glslVersion >= Version1_30 ) {
                    pos = insertShaderSource(0, pos, vp_130_defines);
                } else {
                    pos = insertShaderSource(0, pos, vp_100_defines);
                }
                pos = insertShaderSource(0, pos, "\n");
            } else if ( GL_FRAGMENT_SHADER == m_shaderType ) {
                pos = insertShaderSource(0, pos, "\n");
                if( glslVersion >= Version1_30 ) {
                    pos = insertShaderSource(0, pos, fp_130_defines);
                } else {
                    pos = insertShaderSource(0, pos, fp_100_defines);
                }
                pos = insertShaderSource(0, pos, "\n");
            }
            return pos;
        }

        /**
         * Default customization of this shader source code.
         * @param gl a GL context, which must have been made current once
         * @param preludeVersion if true {@link GLContext#getGLSLVersionString()} is preluded, otherwise not. Defaults to true.
         * @param addDefaultPrecision if <code>true</code> default precision source code line(s) are added, i.e.
         *                            {@link #es2_default_precision_vp}, {@link #es2_default_precision_fp},
         *                            {@link #gl3_default_precision_vp_gp}, {@link #gl3_default_precision_fp} or none,
         *                            depending on the {@link GLContext#getGLSLVersionNumber() GLSL version} being used.
         *                            Defaults to true.
         * @param addDefaultDefines if true addGLSLDefines() is used to prelude common defines, defaults to true
         * @return the index after the inserted data, maybe 0 if nothing has be inserted.
         * @see #addGLSLVersion(GL2ES2)
         * @see #addDefaultShaderPrecision(GL2ES2, i, size_tnt)
         * @see #addGLSLDefines(GL2ES2, size_t)
         */
        size_t defaultShaderCustomization(const GL& gl, bool preludeVersion=true, bool addDefaultPrecision=true, bool addDefaultDefines=true) {
            size_t pos;
            if( preludeVersion ) {
                pos = addGLSLVersion(gl);
            } else {
                pos = 0;
            }
            if( addDefaultPrecision ) {
                pos = addDefaultShaderPrecision(gl, pos);
            }
            if( addDefaultDefines ) {
                pos = addGLSLDefines(gl, pos);
            }
            return pos;
        }

        /**
         * Default customization of this shader source code.
         * @param gl a GL context, which must have been made current once
         * @param preludeVersion if true {@link GLContext#getGLSLVersionString()} is preluded, otherwise not.
         * @param esDefaultPrecision optional default precision source code line(s) preluded if not null and if {@link GL#isGLES()}.
         *        You may use {@link #es2_default_precision_fp} for fragment shader and {@link #es2_default_precision_vp} for vertex shader.
         * @param addDefaultDefines if true addGLSLDefines() is used to prelude common defines, defaults to true
         * @return the index after the inserted data, maybe 0 if nothing has be inserted.
         * @see #addGLSLVersion(GL2ES2)
         * @see #addDefaultShaderPrecision(GL2ES2, size_t)
         * @see #addGLSLDefines(GL2ES2, size_t)
         */
        size_t defaultShaderCustomization(const GL& gl, bool preludeVersion, const string_t& esDefaultPrecision, bool addDefaultDefines=true) {
            size_t pos;
            if( preludeVersion ) {
                pos = addGLSLVersion(gl);
            } else {
                pos = 0;
            }
            if( gl.glProfile().nativeGLES() && esDefaultPrecision.length()>0 ) {
                pos = insertShaderSource(0, pos, esDefaultPrecision);
            } else {
                pos = addDefaultShaderPrecision(gl, pos);
            }
            if( addDefaultDefines ) {
                pos = addGLSLDefines(gl, pos);
            }
            return pos;
        }

        //----------------------------------------------------------------------
        // Internals only below this point
        //

      private:
        source_list_t m_shaderSource;
        bytes_t       m_shaderBinary;
        GLenum        m_shaderBinaryFormat;
        shader_list_t m_shader;
        GLenum        m_shaderType;
        size_t        m_id;
        bool          m_compiled;

        static size_t nextID() { return m_nextID++; }
        static std::atomic<size_t> m_nextID;
    };

    inline std::ostream& operator<<(std::ostream& out, const ShaderCode& v) {
        return out << v.toString();
    }

    /**@}*/
}

// injecting specialization of std::hash to namespace std of our types above
namespace std
{
    /** \addtogroup Gamp_GLSL
     *
     */

    template<> struct hash<gamp::render::gl::glsl::ShaderCode> {
        std::size_t operator()(gamp::render::gl::glsl::ShaderCode const& a) const noexcept {
            return a.hash_code();
        }
    };
    template<> struct hash<gamp::render::gl::glsl::ShaderCodeSRef> {
        std::size_t operator()(gamp::render::gl::glsl::ShaderCodeSRef const& a) const noexcept {
            return a->hash_code();
        }
    };

    /**@}*/
}

#endif // GAMP_GLSLSHADERCODE_HPP_
