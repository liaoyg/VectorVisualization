#extension GL_ARB_texture_rectangle : enable

uniform vec4 viewport;
uniform sampler2DRect imageFBOSampler;


void main(void)
{
    vec4 bgColor = vec4(1.0, 1.0, 1.0, 1.0);

    // look up color from previous render pass
    vec4 dest = texture2DRect(imageFBOSampler, gl_FragCoord.xy*viewport.xy);

    // blend with background color
    //dest = vec4(clamp((1.0-dest.a)*bgColor + dest, 0.0, 1.0).rgb, 1.0);
    dest = clamp((1.0-dest.a)*bgColor + dest, 0.0, 1.0);
    //dest = vec4(clamp((dest.a)*bgColor + dest, 0.0, 1.0).rgb, 1.0);

    gl_FragColor = vec4(dest);
}

