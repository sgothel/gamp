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

#ifndef GAMP_GLARRAYHANDLERFLAT_HPP_
#define GAMP_GLARRAYHANDLERFLAT_HPP_

#include <gamp/render/gl/data/GLArrayData.hpp>

namespace gamp::render::gl::data::impl {
    using namespace gamp::render::gl;
    using namespace gamp::render::gl::data;

    /** \addtogroup Gamp_GLImpl
     *
     *  @{
     */

    /**
     * Handles consistency of interleaved array state.
     */
    class GLSubArrayHandler {
      public:
        virtual ~GLSubArrayHandler() noexcept  = default;
        /**
         * Implementation shall associate the data with the array
         *
         * @param gl current GL object
         * @param ext extension object allowing passing of an implementation detail
         */
        virtual void syncData(const GL &gl, ShaderState* st) = 0;

        /**
         * Implementation shall enable or disable the array state.
         *
         * @param gl current GL object
         * @param enable true if array shall be enabled, otherwise false.
         * @param ext extension object allowing passing of an implementation detail
         */
        virtual void enableState(const GL &gl, bool enable, ShaderState* st) = 0;

        virtual const GLArrayDataRef& data() = 0;
    };

    /**@}*/
}  // namespace gamp::render::gl::data::impl

#endif /* GAMP_GLARRAYHANDLERFLAT_HPP_ */
