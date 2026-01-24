// Copyright (c) 2010-2025 Gothel Software e.K.
// Single Per Pixel Lighting (Example: GearsES2)

uniform vec4 gcu_StaticColor;

varying vec3 gcv_normal;
varying vec3 gcv_cameraDir;
varying vec4 gcv_position;

varying vec3 gcv_light0Dir;
varying float gcv_light0Attenuation;

// Defining The Material Colors
const vec4 matAmbient  = vec4(0.2, 0.2, 0.2, 1.0); // orig default
const vec4 matDiffuse  = vec4(0.8, 0.8, 0.8, 1.0); // orig default
// const vec4 matSpecular = vec4(0.0, 0.0, 0.0, 1.0); // orig default
const vec4 matSpecular = vec4(0.8, 0.8, 0.8, 1.0);
// const float matShininess = 0.0; // orig default
const float matShininess = 0.5;

void main()
{
    vec4 ambient = gcu_StaticColor * matAmbient;
    vec4 specular = vec4(0.0);

    float lambertTerm = dot(gcv_normal, gcv_light0Dir);
    vec4 diffuse = gcu_StaticColor * lambertTerm *  gcv_light0Attenuation * matDiffuse;
    if (lambertTerm > 0.0) {
        /*
        vec3 halfDir = normalize (gcv_light0Dir + gcv_cameraDir);
        float NdotHV   = max(0.0, dot(gcv_normal, halfDir));
        */
        vec3 E = normalize(-gcv_position.xyz);
        vec3 R = reflect(-gcv_light0Dir, gcv_normal);
        float NdotHV = max(0.0, dot(R, E));

        specular += gcu_StaticColor * pow(NdotHV, matShininess) * gcv_light0Attenuation * matSpecular;
    }

    // mgl_FragColor = ambient + max(diffuse, vec4(0.0f)) + specular;
    mgl_FragColor = max(ambient*2.0f, ambient + max(diffuse, vec4(0.0f)) + specular);
}
