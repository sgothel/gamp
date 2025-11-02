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

#ifdef USE_NORMAL_CHANNEL
    varying vec3 gcv_Normal;
#endif

#ifdef USE_LIGHT0
    varying vec4 gcv_position;
    varying vec3 gcv_cameraDir;
    varying vec3 gcv_light0Dir;
    varying float gcv_light0Attenuation;
#endif

#endif // varyings_glsl

