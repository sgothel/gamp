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

#ifndef GAMP_DEMOS_GRAPH_TESTSHAPES_GLYPH05FSBI_AE_HPP_
#define GAMP_DEMOS_GRAPH_TESTSHAPES_GLYPH05FSBI_AE_HPP_

#include <gamp/graph/OutlineShape.hpp>
#include <jau/debug.hpp>

/**
 * GPU based resolution independent test object
 * - FreeSans, '0'
 * - TTF Shape for Glyph 19
 */
class Glyph05FreeSerifBoldItalic_ae {
    public:
      static void addShapeToRegion(gamp::graph::OutlineShape& shape) {
        using namespace gamp::graph;

        constexpr OutlineShape::size_type pos0 = 0;

        // Start TTF Shape for Glyph 168
        // 000: B0a: move-to p0
        // Shape.MoveTo:
        shape.closeLastOutline(false);
        shape.addEmptyOutline();
        shape.addVertex(pos0, 0.450000f, -0.013000f, 0, true);
        // 000: B4: quad-to p0-p1-p2h **** MID
        // Shape.QuadTo:
        shape.addVertex(pos0, 0.386000f, -0.013000f, 0, false);
        shape.addVertex(pos0, 0.353000f, 0.018000f, 0, true);
        // 002: B6: quad-to pMh-p0-p1
        // Shape.QuadTo:
        shape.addVertex(pos0, 0.319000f, 0.049000f, 0, false); // NOLINT
        shape.addVertex(pos0, 0.307000f, 0.118000f, 0, true);
        // 003: B4: quad-to p0-p1-p2h **** MID
        // Shape.QuadTo:
        shape.addVertex(pos0, 0.265000f, 0.049000f, 0, false);
        shape.addVertex(pos0, 0.225000f, 0.019000f, 0, true);
        // 005: B6: quad-to pMh-p0-p1
        // Shape.QuadTo:
        shape.addVertex(pos0, 0.184000f, -0.012000f, 0, false);
        shape.addVertex(pos0, 0.134000f, -0.012000f, 0, true);
        // 006: B4: quad-to p0-p1-p2h **** MID
        // Shape.QuadTo:
        shape.addVertex(pos0, 0.085000f, -0.012000f, 0, false);
        shape.addVertex(pos0, 0.053000f, 0.021000f, 0, true);
        // 008: B6: quad-to pMh-p0-p1
        // Shape.QuadTo:
        shape.addVertex(pos0, 0.020000f, 0.055000f, 0, false);
        shape.addVertex(pos0, 0.020000f, 0.106000f, 0, true);
        // 009: B4: quad-to p0-p1-p2h **** MID
        // Shape.QuadTo:
        shape.addVertex(pos0, 0.020000f, 0.185000f, 0, false);
        shape.addVertex(pos0, 0.062000f, 0.269000f, 0, true);
        // 011: B5: quad-to pMh-p0-p1h ***** MID
        // Shape.QuadTo:
        shape.addVertex(pos0, 0.105000f, 0.353000f, 0, false);
        shape.addVertex(pos0, 0.170000f, 0.407000f, 0, true);
        // 012: B6: quad-to pMh-p0-p1
        // Shape.QuadTo:
        shape.addVertex(pos0, 0.235000f, 0.462000f, 0, false);
        shape.addVertex(pos0, 0.296000f, 0.462000f, 0, true);
        // 013: B4: quad-to p0-p1-p2h **** MID
        // Shape.QuadTo:
        shape.addVertex(pos0, 0.328000f, 0.462000f, 0, false);
        shape.addVertex(pos0, 0.346000f, 0.448000f, 0, true);
        // 015: B6: quad-to pMh-p0-p1
        // Shape.QuadTo:
        shape.addVertex(pos0, 0.364000f, 0.433000f, 0, false);
        shape.addVertex(pos0, 0.377000f, 0.396000f, 0, true);
        // 016: B1: line-to p0-p1
        // Shape.LineTo:
        shape.addVertex(pos0, 0.395000f, 0.454000f, 0, true);
        // 017: B1: line-to p0-p1
        // Shape.LineTo:
        shape.addVertex(pos0, 0.498000f, 0.459000f, 0, true);
        // 018: B1: line-to p0-p1
        // Shape.LineTo:
        shape.addVertex(pos0, 0.478000f, 0.394000f, 0, true);
        // 019: B4: quad-to p0-p1-p2h **** MID
        // Shape.QuadTo:
        shape.addVertex(pos0, 0.510000f, 0.431000f, 0, false);
        shape.addVertex(pos0, 0.535000f, 0.445000f, 0, true);
        // 021: B6: quad-to pMh-p0-p1
        // Shape.QuadTo:
        shape.addVertex(pos0, 0.561000f, 0.459000f, 0, false);
        shape.addVertex(pos0, 0.598000f, 0.459000f, 0, true);
        // 022: B4: quad-to p0-p1-p2h **** MID
        // Shape.QuadTo:
        shape.addVertex(pos0, 0.645000f, 0.459000f, 0, false);
        shape.addVertex(pos0, 0.671000f, 0.436000f, 0, true);
        // 024: B6: quad-to pMh-p0-p1
        // Shape.QuadTo:
        shape.addVertex(pos0, 0.698000f, 0.413000f, 0, false);
        shape.addVertex(pos0, 0.698000f, 0.372000f, 0, true);
        // 025: B4: quad-to p0-p1-p2h **** MID
        // Shape.QuadTo:
        shape.addVertex(pos0, 0.698000f, 0.310000f, 0, false);
        shape.addVertex(pos0, 0.639000f, 0.263000f, 0, true);
        // 027: B6: quad-to pMh-p0-p1
        // Shape.QuadTo:
        shape.addVertex(pos0, 0.579000f, 0.215000f, 0, false);
        shape.addVertex(pos0, 0.470000f, 0.190000f, 0, true);
        // 028: B1: line-to p0-p1
        // Shape.LineTo:
        shape.addVertex(pos0, 0.431000f, 0.181000f, 0, true);
        // 029: B2: quad-to p0-p1-p2
        // Shape.QuadTo:
        shape.addVertex(pos0, 0.426000f, 0.156000f, 0, false);
        shape.addVertex(pos0, 0.426000f, 0.134000f, 0, true);
        // 031: B4: quad-to p0-p1-p2h **** MID
        // Shape.QuadTo:
        shape.addVertex(pos0, 0.426000f, 0.096000f, 0, false);
        shape.addVertex(pos0, 0.444000f, 0.073000f, 0, true);
        // 033: B6: quad-to pMh-p0-p1
        // Shape.QuadTo:
        shape.addVertex(pos0, 0.462000f, 0.050000f, 0, false);
        shape.addVertex(pos0, 0.493000f, 0.050000f, 0, true);
        // 034: B2: quad-to p0-p1-p2
        // Shape.QuadTo:
        shape.addVertex(pos0, 0.565000f, 0.050000f, 0, false); // NOLINT
        shape.addVertex(pos0, 0.616000f, 0.139000f, 0, true);
        // 036: B1: line-to p0-p1
        // Shape.LineTo:
        shape.addVertex(pos0, 0.644000f, 0.122000f, 0, true);
        // 037: B2: quad-to p0-p1-p2
        // Shape.QuadTo:
        shape.addVertex(pos0, 0.578000f, -0.013000f, 0, false); // NOLINT
        shape.addVertex(pos0, 0.450000f, -0.013000f, 0, true);
        jau_PLAIN_PRINT(true, "Glyph05FreeSerifBoldItalic_ae.shape01a.winding_area: %s", jau::math::geom::to_string(shape.windingOfLastOutline()));
        shape.closeLastOutline(false);

        // 039: B0a: move-to p0
        // Shape.MoveTo:
        shape.closeLastOutline(false);
        shape.addEmptyOutline();
        shape.addVertex(pos0, 0.194000f, 0.058000f, 0, true);
        // 039: B4: quad-to p0-p1-p2h **** MID
        // Shape.QuadTo:
        shape.addVertex(pos0, 0.238000f, 0.058000f, 0, false);
        shape.addVertex(pos0, 0.278000f, 0.122000f, 0, true);
        // 041: B5: quad-to pMh-p0-p1h ***** MID
        // Shape.QuadTo:
        shape.addVertex(pos0, 0.319000f, 0.187000f, 0, false); // NOLINT
        shape.addVertex(pos0, 0.338000f, 0.256000f, 0, true);
        // 042: B6: quad-to pMh-p0-p1
        // Shape.QuadTo:
        shape.addVertex(pos0, 0.358000f, 0.326000f, 0, false);
        shape.addVertex(pos0, 0.358000f, 0.363000f, 0, true);
        // 043: B4: quad-to p0-p1-p2h **** MID
        // Shape.QuadTo:
        shape.addVertex(pos0, 0.358000f, 0.387000f, 0, false);
        shape.addVertex(pos0, 0.345000f, 0.403000f, 0, true);
        // 045: B6: quad-to pMh-p0-p1
        // Shape.QuadTo:
        shape.addVertex(pos0, 0.331000f, 0.419000f, 0, false);
        shape.addVertex(pos0, 0.310000f, 0.419000f, 0, true);
        // 046: B4: quad-to p0-p1-p2h **** MID
        // Shape.QuadTo:
        shape.addVertex(pos0, 0.267000f, 0.419000f, 0, false);
        shape.addVertex(pos0, 0.227000f, 0.356000f, 0, true);
        // 048: B5: quad-to pMh-p0-p1h ***** MID
        // Shape.QuadTo:
        shape.addVertex(pos0, 0.187000f, 0.293000f, 0, false);
        shape.addVertex(pos0, 0.167000f, 0.225000f, 0, true);
        // 049: B6: quad-to pMh-p0-p1
        // Shape.QuadTo:
        shape.addVertex(pos0, 0.146000f, 0.156000f, 0, false);
        shape.addVertex(pos0, 0.146000f, 0.119000f, 0, true);
        // 050: B4: quad-to p0-p1-p2h **** MID
        // Shape.QuadTo:
        shape.addVertex(pos0, 0.146000f, 0.092000f, 0, false);
        shape.addVertex(pos0, 0.159000f, 0.075000f, 0, true);
        // 052: B6: quad-to pMh-p0-p1
        // Shape.QuadTo:
        shape.addVertex(pos0, 0.172000f, 0.058000f, 0, false);
        shape.addVertex(pos0, 0.194000f, 0.058000f, 0, true);
        jau_PLAIN_PRINT(true, "Glyph05FreeSerifBoldItalic_ae.shape02a.winding_area: %s", jau::math::geom::to_string(shape.windingOfLastOutline()));
        shape.closeLastOutline(false);

        if( true ) {
            // GlyphShape<168>: offset 0 of 8/61 points
            //  pM[060] P[443/231, on true, end true]
            //  p0[053] P[438/214, on true, end false]
            //  p1[054] P[498/223, on false, end false]
            //  p2[055] P[608/320, on false, end false]
            // 053: B0a: move-to p0
            // Shape.MoveTo:
            shape.closeLastOutline(false);
            shape.addEmptyOutline();
            shape.addVertex(pos0, 0.438000f, 0.214000f, 0, true);
            // 053: B4: quad-to p0-p1-p2h **** MID
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.498000f, 0.223000f, 0, false);
            shape.addVertex(pos0, 0.553000f, 0.271000f, 0, true);
            // GlyphShape<168>: offset 2 of 8/61 points
            //  pM[054] P[498/223, on false, end false]
            //  p0[055] P[608/320, on false, end false]
            //  p1[056] P[608/388, on true, end false]
            //  p2[057] P[608/429, on false, end false]
            // 055: B6: quad-to pMh-p0-p1
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.608000f, 0.320000f, 0, false);
            shape.addVertex(pos0, 0.608000f, 0.388000f, 0, true);
            // GlyphShape<168>: offset 3 of 8/61 points
            //  pM[055] P[608/320, on false, end false]
            //  p0[056] P[608/388, on true, end false]
            //  p1[057] P[608/429, on false, end false]
            //  p2[058] P[575/429, on true, end false]
            // 056: B2: quad-to p0-p1-p2
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.608000f, 0.429000f, 0, false);
            shape.addVertex(pos0, 0.575000f, 0.429000f, 0, true);
            // GlyphShape<168>: offset 5 of 8/61 points
            //  pM[057] P[608/429, on false, end false]
            //  p0[058] P[575/429, on true, end false]
            //  p1[059] P[502/429, on false, end false]
            //  p2[060] P[443/231, on true, end true]
            // 058: B2: quad-to p0-p1-p2
            // Shape.QuadTo:
            shape.addVertex(pos0, 0.502000f, 0.429000f, 0, false);
            shape.addVertex(pos0, 0.443000f, 0.231000f, 0, true);
            // GlyphShape<168>: offset 7 of 8/61 points
            //  pM[059] P[502/429, on false, end false]
            //  p0[060] P[443/231, on true, end true]
            //  p1[053] P[438/214, on true, end false]
            //  p2[054] P[498/223, on false, end false]
            // 060: B1: line-to p0-p1
            // Shape.LineTo:
            shape.addVertex(pos0, 0.438000f, 0.214000f, 0, true);
            jau_PLAIN_PRINT(true, "Glyph05FreeSerifBoldItalic_ae.shape03a.winding_area: %s", jau::math::geom::to_string(shape.windingOfLastOutline()));
            shape.closeLastOutline(false);
        }

        // End Shape for Glyph 168

        shape.setIsQuadraticNurbs();
        shape.setSharpness(OutlineShape::DEFAULT_SHARPNESS);
    }
};

#endif // GAMP_DEMOS_GRAPH_TESTSHAPES_GLYPH05FSBI_AE_HPP_
