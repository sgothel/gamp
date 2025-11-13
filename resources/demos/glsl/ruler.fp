//Copyright (c) 2010-2025 Gothel Software e.K.

uniform vec3 gcu_RulerColor;
uniform vec2 gcu_RulerPixFreq;

const vec2 onev2 = vec2(1.0, 1.0);

void main (void)
{
  vec2 c = step( onev2, mod(gl_FragCoord.xy, gcu_RulerPixFreq) );
  if( c.s == 0.0 || c.t == 0.0 ) {
    mgl_FragColor = vec4(gcu_RulerColor, 1.0);
  } else {
    discard;
  }
}
