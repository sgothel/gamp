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

#ifndef GAMP_RENDER_RENDERTYPES_HPP_
#define GAMP_RENDER_RENDERTYPES_HPP_

#include <jau/basic_types.hpp>
#include <jau/int_math_ct.hpp>
#include <jau/int_types.hpp>
#include <jau/float_types.hpp>
#include <jau/string_util.hpp>
#include <jau/enum_util.hpp>
#include <jau/util/VersionNumber.hpp>

#include <gamp/GampTypes.hpp>
#include <gamp/render/RenderContext.hpp>

namespace gamp::wt {
    class Surface;
    typedef std::shared_ptr<Surface> SurfaceSRef;
}

namespace gamp::render {

    /** @addtogroup Gamp_Render
     *
     *  @{
     */
    using namespace jau::enums;

    class RenderException : public GampException {
      public:
        RenderException(std::string const& m, const char* file, int line) noexcept
        : GampException("RenderException", m, file, line) {}
    };

    /**@}*/

} // namespace gamp::render


#endif /* GAMP_RENDER_RENDERTYPES_HPP_ */
