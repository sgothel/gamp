// Copyright (c) 2010-2025 Gothel Software e.K.

varying  vec2          mgl_texCoord;
varying  vec4          frontColor;

uniform sampler2D      mgl_ActiveTexture;

void main (void)
{
  vec4 texColor;
  if(0.0 <= mgl_texCoord.t && mgl_texCoord.t<=1.0) {
    texColor = texture2D(mgl_ActiveTexture, mgl_texCoord);
  } else {
    discard;
  }

  // mix frontColor with texture ..  pre-multiplying texture alpha
  mgl_FragColor = vec4( mix( frontColor.rgb, texColor.rgb, texColor.a ), frontColor.a );
}

