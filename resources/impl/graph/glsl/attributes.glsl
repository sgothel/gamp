// Copyright (c) 2010-2025 Gothel Software e.K.

#ifndef attributes_glsl
#define attributes_glsl

attribute vec4    gca_Vertex;

/**
 * CDTriangulator2D.extractBoundaryTriangles(..):
 *     AA line (exp)   : z > 0
 *     line            : x ==   0, y == 0
 *     hole or holeLike: 0 > y
 *     !hole           : 0 < y
 *
 *     0   == gca_CurveParam.x : vertex-0 of triangle
 *     0.5 == gca_CurveParam.x : vertex-1 of triangle
 *     1   == gca_CurveParam.x : vertex-2 of triangle
 */
attribute vec3 gca_CurveParam;

attribute vec4 gca_FboVertex;
attribute vec2 gca_FboTexCoord;

#ifdef USE_COLOR_CHANNEL
    attribute vec4 gca_Color;
#endif

#ifdef USE_NORMAL_CHANNEL
    attribute vec3 gca_Normal;
#endif

#endif // attributes_glsl
