// Copyright (c) 2010-2025 Gothel Software e.K.

#ifndef uniforms_glsl
#define uniforms_glsl

uniform mat4    gcu_PMVMatrix01[3]; // P, Mv, and Mvi
uniform vec4    gcu_ColorStatic;
uniform float   gcu_Weight;

#ifdef USE_COLOR_TEXTURE
    uniform vec2  gcu_ColorTexBBox[3]; // box-min[2], box-max[2] and tex-size[2]
    uniform vec4  gcu_ColorTexClearCol; // clear color for gcu_ColorTexBBox clipping
#endif
#ifdef USE_FRUSTUM_CLIPPING
    uniform vec4  gcu_ClipFrustum[6]; // L, R, B, T, N, F each {n.x, n.y, n.z, d}
#endif

uniform mat4    gcu_PMVMatrix02[3]; // P, Mv, and Mvi
uniform sampler2D  gcu_FboTexUnit;

/**
 * .x .y : texture-, fbo- or screen-size
 */
uniform vec2   gcu_FboTexSize;

#endif // uniforms_glsl
