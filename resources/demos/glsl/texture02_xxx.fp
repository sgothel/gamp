// Copyright (c) 2010-2025 Gothel Software e.K.

varying  vec2          mgl_texCoord;
varying  vec4          frontColor;

uniform sampler2D      mgl_Texture0;
uniform sampler2D      mgl_Texture1;

const vec4 One = vec4(1.0, 1.0, 1.0, 1.0);

void main (void)
{
  vec4 texColor0 = texture2D(mgl_Texture0, mgl_texCoord);
  vec4 texColor1 = texture2D(mgl_Texture1, mgl_texCoord);

  // mgl_FragColor = ( ( texColor0 + texColor1 ) / 2.0 ) * frontColor;
  // mgl_FragColor = mix(texColor0, texColor1, One/2.0) * frontColor;
  mgl_FragColor = min(One, mix(texColor0, texColor1, One/2.0) * 1.6) * frontColor;
}

