// Copyright (c) 2010-2025 Gothel Software e.K.

varying  vec2          mgl_texCoord;

uniform sampler2D      mgl_Texture0;

void main (void)
{
  mgl_FragColor = texture2D(mgl_Texture0, mgl_texCoord);
}

