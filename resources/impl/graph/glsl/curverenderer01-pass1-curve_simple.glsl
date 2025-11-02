// Copyright (c) 2010-2025 Gothel Software e.K.

    vec4 graphColor;

#ifdef USE_FRUSTUM_CLIPPING
    if( isOutsideMvFrustum(gcv_ClipCoord) ) {
        #if USE_DISCARD
            discard; // discard freezes NV tegra2 compiler
        #else
            graphColor = vec4(0);
        #endif
    } else
#endif

    if( gcv_CurveParam.x == 0.0 && gcv_CurveParam.y == 0.0 ) {
        // pass-1: Lines
#if defined(USE_COLOR_TEXTURE) && defined(USE_COLOR_CHANNEL)
        vec4 t = clip_coord(gcuTexture2D(gcu_ColorTexUnit, gcv_ColorTexCoord.st), gcu_ColorTexClearCol, gcv_ColorTexCoord, vec2(0), gcu_ColorTexBBox[2]);

        graphColor = vec4( mix( t.rgb * gcu_StaticColor.rgb, gcv_Color.rgb, gcv_Color.a ),
                           mix( t.a * gcu_StaticColor.a, 1, gcv_Color.a) );
#elif defined(USE_COLOR_TEXTURE)
        graphColor = clip_coord(gcuTexture2D(gcu_ColorTexUnit, gcv_ColorTexCoord.st), gcu_ColorTexClearCol, gcv_ColorTexCoord, vec2(0), gcu_ColorTexBBox[2])
                       * gcu_StaticColor;
#elif defined(USE_COLOR_CHANNEL)
        graphColor = gcv_Color * gcu_StaticColor;
#else
        graphColor = gcu_StaticColor;
#endif
    } else {
        // pass-1: curves
        vec2 rtex = vec2( abs(gcv_CurveParam.x), abs(gcv_CurveParam.y) - 0.1 );

        vec2 dtx = dFdx(rtex);
        vec2 dty = dFdy(rtex);

        vec2 f = vec2((dtx.y - dtx.x + 2.0*rtex.x*dtx.x), (dty.y - dty.x + 2.0*rtex.x*dty.x));
        float position = rtex.y - (rtex.x * (1.0 - rtex.x));
        float a = clamp(0.5 - ( position/length(f) ) * sign(gcv_CurveParam.y), 0.0, 1.0);

        if( a <= EPSILON ) {
            #ifdef USE_DISCARD
                discard; // freezes NV tegra2 compiler
            #else
                graphColor = vec4(0);
            #endif
        } else {
#if defined(USE_COLOR_TEXTURE) && defined(USE_COLOR_CHANNEL)
            vec4 t = clip_coord(gcuTexture2D(gcu_ColorTexUnit, gcv_ColorTexCoord.st), gcu_ColorTexClearCol, gcv_ColorTexCoord, vec2(0), gcu_ColorTexBBox[2]);

            graphColor = vec4( mix( t.rgb * gcu_StaticColor.rgb, gcv_Color.rgb, gcv_Color.a ),
                               a * mix( t.a * gcu_StaticColor.a, 1, gcv_Color.a) );
#elif defined(USE_COLOR_TEXTURE)
            vec4 t = clip_coord(gcuTexture2D(gcu_ColorTexUnit, gcv_ColorTexCoord.st), gcu_ColorTexClearCol, gcv_ColorTexCoord, vec2(0), gcu_ColorTexBBox[2]);

            graphColor = vec4(t.rgb * gcu_StaticColor.rgb, t.a * gcu_StaticColor.a * a);
#elif defined(USE_COLOR_CHANNEL)
            graphColor = vec4(gcv_Color.rgb * gcu_StaticColor.rgb, gcv_Color.a * gcu_StaticColor.a * a);
#else
            graphColor = vec4(gcu_StaticColor.rgb, gcu_StaticColor.a * a);
#endif
        }
    }

#ifdef USE_LIGHT0
    vec4 ambient = graphColor * matAmbient;
    vec4 specular = vec4(0.0);

    float lambertTerm = dot(gcv_Normal, gcv_light0Dir);
    vec4 diffuse = graphColor * lambertTerm *  gcv_light0Attenuation * matDiffuse;
    if (lambertTerm > 0.0) {
        float NdotHV;
        /*
        vec3 halfDir = normalize (gcv_light0Dir + gcv_cameraDir);
        NdotHV   = max(0.0, dot(gcv_Normal, halfDir));
        */
        vec3 E = normalize(-gcv_position.xyz);
        vec3 R = reflect(-gcv_light0Dir, gcv_Normal);
        NdotHV   = max(0.0, dot(R, E));

        specular += graphColor * pow(NdotHV, matShininess) * gcv_light0Attenuation * matSpecular;
    }
    mgl_FragColor = ambient + diffuse + specular ;
#else
    mgl_FragColor = graphColor;
#endif

