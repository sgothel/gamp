//Copyright (c) 2010-2025 Gothel Software e.K.

#if __VERSION__ >= 130
  #define varying in
  out vec4 mgl_FragData[2];
  #define texture2D texture
#else
  #define mgl_FragData gl_FragData
#endif

uniform sampler2D gcu_TexUnit0;
uniform sampler2D gcu_TexUnit1;

varying vec4    gcv_frontColor;
varying vec2    gcv_texCoord;

void main (void)
{
  vec2 rg = texture2D(gcu_TexUnit0, gcv_texCoord).rg + texture2D(gcu_TexUnit1, gcv_texCoord).rg;
  float b = gcv_frontColor.b - length(rg);
  mgl_FragData[0] = vec4( rg, b, 1.0 );
}
