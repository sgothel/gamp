// Copyright (c) 2010-2025 Gothel Software e.K.

varying vec4 gcv_frontColor;

void main (void)
{
    mgl_FragColor = vec4(0.0, gcv_frontColor.g, gcv_frontColor.b, 1.0);
}

