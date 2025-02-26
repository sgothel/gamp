// Copyright (c) 2010-2025 Gothel Software e.K.

#ifndef varyings_glsl
#define varyings_glsl

varying vec3  gcv_CurveParam;

varying vec2  gcv_FboTexCoord;

#ifdef USE_COLOR_TEXTURE
    varying vec2  gcv_ColorTexCoord;
#endif
#ifdef USE_FRUSTUM_CLIPPING
    varying vec3  gcv_ClipCoord;
#endif

#ifdef USE_COLOR_CHANNEL
    varying vec4  gcv_Color;
#endif

#endif // varyings_glsl

