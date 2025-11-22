// Copyright (c) 2010-2025 Gothel Software e.K.

uniform mat4 gcu_PMVMatrix[2]; // P, Mv

layout(std140) struct SolInSpace {
    vec4 winCenter; // center in win-coord
    vec4 coreColor;
    vec4 haloColor;
    vec4 bgColor;
    float winRadius;  // radius in win-coord
    float coreRadius; // normalized [0..1]
    float seam;       // normalized [0..1]
};
uniform SolInSpace gcu_solInSpace;

void main (void)
{
    float w_cdist = abs(distance(gl_FragCoord.xyz, gcu_solInSpace.winCenter.xyz)); // distance to center in win-coord
    float n_rdist = (gcu_solInSpace.winRadius - w_cdist)/gcu_solInSpace.winRadius; // normalized dist to radius
    float seam = gcu_solInSpace.seam;
    if( n_rdist >= 0.0 ) {
        float coreRadius = gcu_solInSpace.coreRadius;
        float n_cdist = 1.0 - n_rdist;
        if( n_rdist <= seam ) {
            mgl_FragColor =  vec4(gcu_solInSpace.haloColor.rgb, 0.5);
        } else if( n_cdist < coreRadius ) {
            mgl_FragColor = gcu_solInSpace.coreColor;
            // mgl_FragColor = vec4(0, 0, 0.5, 0.5); // gcu_solInSpace.coreColor;
        } else {
            float l = 1.0 - coreRadius;
            float n_rdist2 = n_rdist / l;
            vec4 hot =  gcu_solInSpace.haloColor;
            vec4 cold = vec4(gcu_solInSpace.bgColor.rgb, 0.5);
            mgl_FragColor = mix(cold, hot, n_rdist2);
        }
    } else {
        discard;
    }
}
