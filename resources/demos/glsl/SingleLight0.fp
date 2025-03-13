// Copyright (c) 2010-2025 Gothel Software e.K.
// Details see GearsES2.java

#if __VERSION__ >= 130
  #define varying in
  out vec4 mgl_FragColor;
#else
  #define mgl_FragColor gl_FragColor
#endif

uniform vec4 mgl_StaticColor;

varying vec3 normal;
varying vec4 position;
varying vec3 lightDir;
varying float attenuation;
varying vec3 cameraDir;

// Defining The Material Colors
const vec4 matAmbient  = vec4(0.2, 0.2, 0.2, 1.0); // orig default
const vec4 matDiffuse  = vec4(0.8, 0.8, 0.8, 1.0); // orig default
// const vec4 matSpecular = vec4(0.0, 0.0, 0.0, 1.0); // orig default
const vec4 matSpecular = vec4(0.8, 0.8, 0.8, 1.0);
// const float matShininess = 0.0; // orig default
const float matShininess = 0.5;

void main()
{
    vec4 ambient = mgl_StaticColor * matAmbient;
    vec4 specular = vec4(0.0);

    float lambertTerm = dot(normal, lightDir);
    vec4 diffuse = mgl_StaticColor * lambertTerm *  attenuation * matDiffuse;
    if (lambertTerm > 0.0) {
        float NdotHV;
        /*
        vec3 halfDir = normalize (lightDir + cameraDir);
        NdotHV   = max(0.0, dot(normal, halfDir));
        */
        vec3 E = normalize(-position.xyz);
        vec3 R = reflect(-lightDir, normal);
        NdotHV   = max(0.0, dot(R, E));

        specular += mgl_StaticColor * pow(NdotHV, matShininess) * attenuation * matSpecular;
    }

    mgl_FragColor = ambient + diffuse + specular ;
}
