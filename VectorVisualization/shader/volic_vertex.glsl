void main(void)
{
    vec4 v = gl_ModelViewProjectionMatrix * gl_Vertex;

    //vec4 texcoord = vec4(0.5*(v.xy + v.w) * vec2(viewport.zw), 0.0, v.w);

    gl_TexCoord[0] = gl_MultiTexCoord0;
    //gl_TexCoord[1] = texcoord;
    gl_TexCoord[5] = gl_MultiTexCoord5;

    gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;
    gl_FrontColor = gl_Color;
    gl_Position = v;
}