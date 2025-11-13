// Copyright (c) 2010-2025 Gothel Software e.K.

//
// 2-pass shader w/o weight
//

#define GetSample(texUnit, texCoord, psize, cx, cy, offX, offY) texture2D(texUnit, texCoord + psize *  vec2(cx+offX, cy+offY))
