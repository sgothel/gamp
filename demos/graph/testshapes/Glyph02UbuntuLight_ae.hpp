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

#ifndef GAMP_DEMOS_GRAPH_TESTSHAPES_GLYPH02UBUNTULIGHT_AE_HPP_
#define GAMP_DEMOS_GRAPH_TESTSHAPES_GLYPH02UBUNTULIGHT_AE_HPP_

#include <gamp/graph/OutlineShape.hpp>
#include <jau/debug.hpp>

/**
 * GPU based resolution independent test object
 * - Ubuntu-Light, lower case 'æ'
 * - TTF Shape for Glyph 193
 */
class Glyph02UbuntuLight_ae  {

    public:
      static void addShapeToRegion(gamp::graph::OutlineShape& shape) {
        using namespace gamp::graph;

        // Ubuntu-Light, lower case 'æ'

        constexpr OutlineShape::size_type pos0 = 0;

        // Start TTF Shape for Glyph 193
        if( true ) {
            // Original Inner e-shape: Winding.CCW
            // Moved into OutlineShape reverse -> Winding.CW -> OK
            //
            // Shape.MoveTo:
            shape.closeLastOutline(false);
            shape.addEmptyOutline();
            shape.addVertex(pos0, 0.728000f, 0.300000f, 0, true);
            // 000: B4: quad-to p0-p1-p2h **** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.726000f, 0.381000f, 0, false);
            shape.addVertex(pos0, 0.690000f, 0.426000f, 0, true);
            // 002: B6: quad-to pMh-p0-p1
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.654000f, 0.471000f, 0, false);
            shape.addVertex(pos0, 0.588000f, 0.471000f, 0, true);
            // 003: B4: quad-to p0-p1-p2h **** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.553000f, 0.471000f, 0, false);
            shape.addVertex(pos0, 0.526000f, 0.457000f, 0, true);
            // 005: B5: quad-to pMh-p0-p1h ***** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.498000f, 0.443000f, 0, false);
            shape.addVertex(pos0, 0.478000f, 0.420000f, 0, true);
            // 006: B5: quad-to pMh-p0-p1h ***** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.457000f, 0.396000f, 0, false);
            shape.addVertex(pos0, 0.446000f, 0.365000f, 0, true);
            // 007: B6: quad-to pMh-p0-p1
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.434000f, 0.334000f, 0, false); // NOLINT
            shape.addVertex(pos0, 0.432000f, 0.300000f, 0, true);
            // 008: B1: line-to p0-p1
            // Shape.LineTo:
            shape.addVertex(pos0, 0.728000f, 0.300000f, 0, true);
            jau::PLAIN_PRINT(true, "Glyph02UbuntuLight_ae.shape01a.winding_area: %s", jau::math::geom::to_string(shape.windingOfLastOutline()).c_str());
            shape.closeLastOutline(false);
        }

        if( true ) {
            // Original Outer shape: Winding.CW
            // Moved into OutlineShape reverse -> Winding.CCW -> OK
            //
            // Shape.MoveTo:
            shape.closeLastOutline(false);
            shape.addEmptyOutline();
            shape.addVertex(pos0, 0.252000f, -0.011000f, 0, true);
            // 009: B4: quad-to p0-p1-p2h **** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.208000f, -0.011000f, 0, false);
            shape.addVertex(pos0, 0.171000f, -0.002000f, 0, true);
            // 011: B5: quad-to pMh-p0-p1h ***** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.133000f, 0.007000f, 0, false);
            shape.addVertex(pos0, 0.106000f, 0.026000f, 0, true);
            // 012: B5: quad-to pMh-p0-p1h ***** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.079000f, 0.046000f, 0, false);
            shape.addVertex(pos0, 0.064000f, 0.076000f, 0, true);
            // 013: B6: quad-to pMh-p0-p1
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.048000f, 0.107000f, 0, false);
            shape.addVertex(pos0, 0.048000f, 0.151000f, 0, true);
            // 014: B4: quad-to p0-p1-p2h **** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.048000f, 0.193000f, 0, false);
            shape.addVertex(pos0, 0.064000f, 0.223000f, 0, true);
            // 016: B5: quad-to pMh-p0-p1h ***** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.080000f, 0.253000f, 0, false);
            shape.addVertex(pos0, 0.109000f, 0.272000f, 0, true);
            // 017: B5: quad-to pMh-p0-p1h ***** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.138000f, 0.292000f, 0, false);
            shape.addVertex(pos0, 0.178000f, 0.301000f, 0, true);
            // 018: B6: quad-to pMh-p0-p1
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.218000f, 0.310000f, 0, false);
            shape.addVertex(pos0, 0.265000f, 0.310000f, 0, true);
            // 019: B4: quad-to p0-p1-p2h **** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.279000f, 0.310000f, 0, false);
            shape.addVertex(pos0, 0.294000f, 0.309000f, 0, true);
            // 021: B5: quad-to pMh-p0-p1h ***** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.310000f, 0.307000f, 0, false);
            shape.addVertex(pos0, 0.324000f, 0.305000f, 0, true);
            // 022: B5: quad-to pMh-p0-p1h ***** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.339000f, 0.302000f, 0, false);
            shape.addVertex(pos0, 0.349000f, 0.300000f, 0, true);
            // 023: B6: quad-to pMh-p0-p1
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.360000f, 0.297000f, 0, false);
            shape.addVertex(pos0, 0.364000f, 0.295000f, 0, true);
            // 024: B1: line-to p0-p1
            // Shape.LineTo:
            shape.addVertex(pos0, 0.364000f, 0.327000f, 0, true);
            // 025: B4: quad-to p0-p1-p2h **** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.364000f, 0.354000f, 0, false);
            shape.addVertex(pos0, 0.360000f, 0.379000f, 0, true);
            // 027: B5: quad-to pMh-p0-p1h ***** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.356000f, 0.405000f, 0, false);
            shape.addVertex(pos0, 0.343000f, 0.425000f, 0, true);
            // 028: B5: quad-to pMh-p0-p1h ***** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.329000f, 0.446000f, 0, false);
            shape.addVertex(pos0, 0.305000f, 0.458000f, 0, true);
            // 029: B6: quad-to pMh-p0-p1
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.280000f, 0.471000f, 0, false);
            shape.addVertex(pos0, 0.240000f, 0.471000f, 0, true);
            // 030: B4: quad-to p0-p1-p2h **** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.186000f, 0.471000f, 0, false);
            shape.addVertex(pos0, 0.156000f, 0.464000f, 0, true);
            // 032: B6: quad-to pMh-p0-p1
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.126000f, 0.456000f, 0, false);
            shape.addVertex(pos0, 0.113000f, 0.451000f, 0, true);
            // 033: B1: line-to p0-p1
            // Shape.LineTo:
            shape.addVertex(pos0, 0.105000f, 0.507000f, 0, true);
            // 034: B4: quad-to p0-p1-p2h **** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.122000f, 0.515000f, 0, false);
            shape.addVertex(pos0, 0.158000f, 0.522000f, 0, true);
            // 036: B6: quad-to pMh-p0-p1
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.194000f, 0.529000f, 0, false);
            shape.addVertex(pos0, 0.243000f, 0.529000f, 0, true);
            // 037: B4: quad-to p0-p1-p2h **** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.314000f, 0.529000f, 0, false);
            shape.addVertex(pos0, 0.354000f, 0.503000f, 0, true);
            // 039: B6: quad-to pMh-p0-p1
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.395000f, 0.476000f, 0, false);
            shape.addVertex(pos0, 0.412000f, 0.431000f, 0, true);
            // 040: B4: quad-to p0-p1-p2h **** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.445000f, 0.480000f, 0, false);
            shape.addVertex(pos0, 0.491000f, 0.504000f, 0, true);
            // 042: B6: quad-to pMh-p0-p1
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.537000f, 0.529000f, 0, false);
            shape.addVertex(pos0, 0.587000f, 0.529000f, 0, true);
            // 043: B4: quad-to p0-p1-p2h **** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.682000f, 0.529000f, 0, false);
            shape.addVertex(pos0, 0.738000f, 0.467000f, 0, true);
            // 045: B6: quad-to pMh-p0-p1
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.795000f, 0.405000f, 0, false);
            shape.addVertex(pos0, 0.795000f, 0.276000f, 0, true);
            // 046: B4: quad-to p0-p1-p2h **** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.795000f, 0.268000f, 0, false);
            shape.addVertex(pos0, 0.795000f, 0.260000f, 0, true);
            // 048: B6: quad-to pMh-p0-p1
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.794000f, 0.252000f, 0, false);
            shape.addVertex(pos0, 0.793000f, 0.245000f, 0, true);
            // 049: B1: line-to p0-p1
            // Shape.LineTo:
            shape.addVertex(pos0, 0.430000f, 0.245000f, 0, true);
            // 050: B4: quad-to p0-p1-p2h **** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.433000f, 0.150000f, 0, false);
            shape.addVertex(pos0, 0.477000f, 0.099000f, 0, true);
            // 052: B6: quad-to pMh-p0-p1
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.521000f, 0.048000f, 0, false);
            shape.addVertex(pos0, 0.617000f, 0.048000f, 0, true);
            // 053: B4: quad-to p0-p1-p2h **** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.670000f, 0.048000f, 0, false);
            shape.addVertex(pos0, 0.701000f, 0.058000f, 0, true);
            // 055: B6: quad-to pMh-p0-p1
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.732000f, 0.068000f, 0, false);
            shape.addVertex(pos0, 0.746000f, 0.075000f, 0, true);
            // 056: B1: line-to p0-p1
            // Shape.LineTo:
            shape.addVertex(pos0, 0.758000f, 0.019000f, true);
            // 057: B4: quad-to p0-p1-p2h **** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.744000f, 0.011000f, 0, false);
            shape.addVertex(pos0, 0.706000f, 0.000000f, 0, true);
            // 059: B6: quad-to pMh-p0-p1
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.667000f, -0.011000f, 0, false);
            shape.addVertex(pos0, 0.615000f, -0.011000f, 0, true);
            // 060: B4: quad-to p0-p1-p2h **** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.558000f, -0.011000f, 0, false);
            shape.addVertex(pos0, 0.514000f, 0.003000f, 0, true);
            // 062: B6: quad-to pMh-p0-p1
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.470000f, 0.017000f, 0, false);
            shape.addVertex(pos0, 0.437000f, 0.049000f, 0, true);
            // 063: B4: quad-to p0-p1-p2h **** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.426000f, 0.040000f, 0, false);
            shape.addVertex(pos0, 0.410000f, 0.030000f, 0, true);
            // 065: B5: quad-to pMh-p0-p1h ***** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.393000f, 0.019000f, 0, false);
            shape.addVertex(pos0, 0.370000f, 0.010000f, 0, true);
            // 066: B5: quad-to pMh-p0-p1h ***** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.347000f, 0.001000f, 0, false);
            shape.addVertex(pos0, 0.318000f, -0.005000f, 0, true); // NOLINT
            // 067: B6: quad-to pMh-p0-p1
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.289000f, -0.011000f, 0, false);
            shape.addVertex(pos0, 0.252000f, -0.011000f, 0, true);
            jau::PLAIN_PRINT(true, "Glyph02UbuntuLight_ae.shape02a.winding_area: %s", jau::math::geom::to_string(shape.windingOfLastOutline()).c_str());
            shape.closeLastOutline(false);
        }

        if( true ) {
            // Original Inner a-shape: Winding.CCW
            // Moved into OutlineShape reverse -> Winding.CW -> OK now
            //
            // Shape.MoveTo:
            shape.closeLastOutline(false);
            shape.addEmptyOutline();
            shape.addVertex(pos0, 0.365000f, 0.238000f, 0, true);
            // 068: B4: quad-to p0-p1-p2h **** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.354000f, 0.243000f, 0, false);
            shape.addVertex(pos0, 0.330000f, 0.248000f, 0, true);
            // 070: B6: quad-to pMh-p0-p1
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.305000f, 0.254000f, 0, false);
            shape.addVertex(pos0, 0.263000f, 0.254000f, 0, true);
            // 071: B4: quad-to p0-p1-p2h **** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.239000f, 0.254000f, 0, false);
            shape.addVertex(pos0, 0.213000f, 0.251000f, 0, true);
            // 073: B5: quad-to pMh-p0-p1h ***** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.187000f, 0.247000f, 0, false);
            shape.addVertex(pos0, 0.165000f, 0.236000f, 0, true);
            // 074: B5: quad-to pMh-p0-p1h ***** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.143000f, 0.224000f, 0, false);
            shape.addVertex(pos0, 0.129000f, 0.204000f, 0, true);
            // 075: B6: quad-to pMh-p0-p1
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.115000f, 0.184000f, 0, false);
            shape.addVertex(pos0, 0.115000f, 0.151000f, 0, true);
            // 076: B4: quad-to p0-p1-p2h **** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.115000f, 0.122000f, 0, false);
            shape.addVertex(pos0, 0.125000f, 0.102000f, 0, true);
            // 078: B5: quad-to pMh-p0-p1h ***** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.135000f, 0.082000f, 0, false);
            shape.addVertex(pos0, 0.153000f, 0.070000f, 0, true);
            // 079: B5: quad-to pMh-p0-p1h ***** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.172000f, 0.058000f, 0, false);
            shape.addVertex(pos0, 0.197000f, 0.053000f, 0, true);
            // 080: B6: quad-to pMh-p0-p1
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.222000f, 0.047000f, 0, false);
            shape.addVertex(pos0, 0.252000f, 0.047000f, 0, true);
            // 081: B4: quad-to p0-p1-p2h **** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.314000f, 0.047000f, 0, false);
            shape.addVertex(pos0, 0.350000f, 0.063000f, 0, true);
            // 083: B6: quad-to pMh-p0-p1
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.386000f, 0.080000f, 0, false);
            shape.addVertex(pos0, 0.400000f, 0.093000f, 0, true);
            // 084: B4: quad-to p0-p1-p2h **** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.384000f, 0.119000f, 0, false);
            shape.addVertex(pos0, 0.375000f, 0.154000f, 0, true);
            // 086: B6: quad-to pMh-p0-p1
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.366000f, 0.190000f, 0, false);
            shape.addVertex(pos0, 0.365000f, 0.238000f, 0, true);
            jau::PLAIN_PRINT(true, "Glyph02UbuntuLight_ae.shape03a.winding_area: %s", jau::math::geom::to_string(shape.windingOfLastOutline()).c_str());
            shape.closeLastOutline(false);
        }
        // End Shape for Glyph 193

        shape.setIsQuadraticNurbs();
        shape.setSharpness(OutlineShape::DEFAULT_SHARPNESS);
    }

};

#endif // GLYPH02UBUNTULIGHT
