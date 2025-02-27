// Copyright (c) 2010-2025 Gothel Software e.K.

#ifndef functions_glsl
#define functions_glsl

/** Returns product of components. */
float v_mul( vec2 v ) {
    return v.x * v.y;
}
/** Returns product of components. */
float v_mul( vec3 v ) {
    return v.x * v.y * v.z;
}
/** Returns component wise logical 'and' as float '0' or '1' using the components product and clamp. */
float v_and( vec2 v ) {
    return clamp(v.x * v.y, 0, 1);
}
/** Returns component wise logical 'and' as float '0' or '1' using the components product and clamp. */
float v_and( vec3 v ) {
    return clamp(v.x * v.y * v.z, 0, 1);
}

/** Returns sum of components. */
float v_sum( vec2 v ) {
    return v.x + v.y;
}
/** Returns sum of components. */
float v_sum( vec3 v ) {
    return v.x + v.y + v.z;
}
/** Returns component wise logical 'or' as float '0' or '1' using the components sum  and clamp. */
float v_or( vec2 v ) {
    return clamp(v.x + v.y, 0, 1);
}
/** Returns component wise logical 'or' as float '0' or '1' using the components sum  and clamp. */
float v_or( vec3 v ) {
    return clamp(v.x + v.y + v.z, 0, 1);
}

/** 32-bit float Epsilon value */
const float EPSILON = 1.1920929E-7f;

/**
 * Branch-less clipping color selection.
 * <p>
 * Returns either 'col_in' if the 'coord' is within ['low'..'high'] range,
 * otherwise 'col_ex'.
 * </p>
 * <p>
 * This is achieved via the build-in 'step' and 'mix' function
 * as well as our own 'v_mul' and v_or' function,
 * which flattens a 'vec2' to one float suitable to be used as the 'mix' criteria.
 * </p>
 */
vec4 clip_coord(vec4 col_in, vec4 col_ex, vec2 coord, vec2 low, vec2 high) {
    vec4 c = mix( col_ex, col_in, v_mul( step(low,  coord) ));
    return   mix( c,      col_ex,  v_or( step(high+EPSILON, coord) ));
}
/** Branch-less clipping color selection using vec3 coordinates and low/high clipping. */
vec4 clip_coord(vec4 col_in, vec4 col_ex, vec3 coord, vec3 low, vec3 high) {
    vec4 c = mix( col_ex, col_in, v_mul( step(low,  coord) ));
    return   mix( c,      col_ex,  v_or( step(high+EPSILON, coord) ));
}

/**
 * Branch-less clipping test.
 * <p>
 * Returns either '1' if the 'coord' is within ['low'..'high'] range,
 * otherwise '0'.
 * </p>
 * <p>
 * This is achieved via the build-in 'step' and 'mix' function
 * as well as our own 'v_mul' and v_or' function,
 * which flattens a 'vec2' to one float suitable to be used as the 'mix' criteria.
 * </p>
 */
float is_inside(vec2 coord, vec2 low, vec2 high) {
    return v_mul( step(low-EPSILON, coord) ) * ( 1 - v_or( step(high+EPSILON, coord) ) );
}
/** Branch-less clipping test using vec3 coordinates and low/high clipping. */
float is_inside(vec3 coord, vec3 low, vec3 high) {
    return v_mul( step(low-EPSILON, coord) ) * ( 1 - v_or( step(high+EPSILON, coord) ) );
}

/** Return distance of plane {n, d} to given point p. */
float planeDistance(vec3 n, float d, vec3 p) {
    return dot(n, p) + d;
}

#ifdef USE_FRUSTUM_CLIPPING

bool isOutsideMvFrustum(vec3 p) {
    return planeDistance(gcu_ClipFrustum[0].xyz, gcu_ClipFrustum[0].w, p) < 0 ||
           planeDistance(gcu_ClipFrustum[1].xyz, gcu_ClipFrustum[1].w, p) < 0 ||
           planeDistance(gcu_ClipFrustum[2].xyz, gcu_ClipFrustum[2].w, p) < 0 ||
           planeDistance(gcu_ClipFrustum[3].xyz, gcu_ClipFrustum[3].w, p) < 0 ||
           planeDistance(gcu_ClipFrustum[4].xyz, gcu_ClipFrustum[4].w, p) < 0 ||
           planeDistance(gcu_ClipFrustum[5].xyz, gcu_ClipFrustum[5].w, p) < 0;
}

#endif // USE_FRUSTUM_CLIPPING

#endif // functions_glsl

