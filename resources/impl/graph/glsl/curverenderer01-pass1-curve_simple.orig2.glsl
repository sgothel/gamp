
    vec2 rtex = vec2( abs(gcv_CurveParam.x), abs(gcv_CurveParam.y) );

    if( gcv_CurveParam.x == 0.0 && gcv_CurveParam.y == 0.0 ) {
        // pass-1: Lines
        // mgl_FragColor = vec4(0, 0, 1.0, 0.5); // blue
        mgl_FragColor = gcu_StaticColor;
    } else if ( gcv_CurveParam.x > 0.0 && ( rtex.y > 0.0 || rtex.x == 1.0 ) ) {
        // pass-1: curves
        rtex.y -= 0.1;

        if(rtex.y < 0.0 && gcv_CurveParam.y < 0.0) {
            #if USE_DISCARD
                discard; // freezes NV tegra2 compiler
            #else
                mgl_FragColor = vec4(0, 1.0, 0.0, 0.5); // green
            #endif
        } else {
            rtex.y = max(rtex.y, 0.0); // always >= 0

            vec2 dtx = dFdx(rtex);
            vec2 dty = dFdy(rtex);

            vec2 f = vec2((dtx.y - dtx.x + 2.0*rtex.x*dtx.x), (dty.y - dty.x + 2.0*rtex.x*dty.x));
            float position = rtex.y - (rtex.x * (1.0 - rtex.x));

            float a = clamp(0.5 - ( position/length(f) ) * sign(gcv_CurveParam.y), 0.0, 1.0);
            if( 0.0 == a ) {
                #if USE_DISCARD
                    discard; // freezes NV tegra2 compiler
                #else
                    mgl_FragColor = vec4(1.0, 0.0, 0.0, 0.5); // red
                #endif
            } else {
                // mgl_FragColor = vec4(1.0, 1.0, 0.0, a); // yellow
                mgl_FragColor = vec4(gcu_StaticColor.rgb, gcu_StaticColor.a * a);
            }
        }
    } else {
        mgl_FragColor = vec4(1.0, 0.0, 1.0, 0.5); //
    }

