// Copyright (c) 2010-2025 Gothel Software e.K.

#ifndef constants_glsl
#define constants_glsl

#ifdef USE_LIGHT0
    // Defining The Material Colors
    const vec4 matAmbient  = vec4(0.2, 0.2, 0.2, 1.0); // orig default
    const vec4 matDiffuse  = vec4(0.8, 0.8, 0.8, 1.0); // orig default

    // const vec4 matSpecular = vec4(0.0, 0.0, 0.0, 1.0); // orig default
    const vec4 matSpecular = vec4(0.8, 0.8, 0.8, 1.0);

    // const float matShininess = 0.0; // orig default
    const float matShininess = 0.5;
#endif

const float EPSILON =  0.0000001;  // FIXME: determine proper hw-precision

#endif // constants_glsl
