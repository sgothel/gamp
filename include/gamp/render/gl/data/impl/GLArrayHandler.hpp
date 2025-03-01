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

#ifndef GAMP_GLARRAYHANDLER_HPP_
#define GAMP_GLARRAYHANDLER_HPP_

#include <memory>

#include <gamp/render/gl/GLTypes.hpp>
#include <gamp/render/gl/data/impl/GLSLSubArrayHandler.hpp>

namespace gamp::render::gl::data {
    template<typename Value_type>
    class GLArrayDataClient;

    template<typename Value_type>
    using GLArrayDataClientRef = std::shared_ptr<GLArrayDataClient<Value_type>>;
}

namespace gamp::render::gl::data::impl {
    using namespace gamp::render::gl;
    using namespace gamp::render::gl::data;

    /** @defgroup Gamp_GLImpl Gamp GL Implementation Details
     *  Non public OpenGL implementation specifics, do not use directly.
     *
     *  @{
     */

    /**
     * Handles consistency of buffer data and array state.<br/>
     * Implementations shall consider buffer types (VBO, ..), interleaved, etc.<br/>
     * They also need to consider array state types, i.e. fixed function or GLSL.<br/>
     */
    template <typename Value_type>
    class GLArrayHandler {
      protected:
        GLArrayDataClientRef<Value_type> m_ad = nullptr;

      public:
        GLArrayHandler() noexcept = default;
        virtual ~GLArrayHandler() noexcept = default;

        void set(const GLArrayDataClientRef<Value_type>& ad) noexcept { m_ad = ad; }

        /**
         * if <code>bind</code> is true and the data uses VBO,
         * the latter will be bound and data written to the GPU if required.
         * <p>
         * If  <code>bind</code> is false and the data uses VBO,
         * the latter will be unbound.
         * </p>
         *
         * @param gl current GL object
         * @param bind true if VBO shall be bound and data written,
         *        otherwise clear VBO binding.
         * @return true if data uses VBO and action was performed, otherwise false
         */
        bool bindBuffer(GL& gl, bool bind);

        /**
         * Implementation shall enable or disable the array state.
         * <p>
         * Before enabling the array state,
         * implementation shall synchronize the data with the GPU
         * and associate the data with the array.
         * </p>
         *
         * @param gl current GL object
         * @param enable true if array shall be enabled, otherwise false.
         * @param ext extension object allowing passing of an implementation detail
         */
        virtual void enableState(GL& gl, bool enable, ShaderState* st) = 0;

        /**
         * Supporting interleaved arrays, where sub handlers may handle
         * the array state and the <i>master</i> handler the buffer consistency.
         *
         * @param handler the sub handler
         * @throws UnsupportedOperationException if this array handler does not support interleaved arrays
         */
        virtual void addSubHandler(GLSLSubArrayHandler&& handler) = 0;

        virtual void setSubArrayVBOName(GLuint vboName) = 0;
    };
    template<typename Value_type>
    using GLArrayHandlerPtr = std::unique_ptr<GLArrayHandler<Value_type>>;

    /**@}*/

}

#endif /* GAMP_GLARRAYHANDLER_HPP_ */
