varying vec3 varLightVec;
varying vec3 varViewVec;
varying vec3 varNormal;


void main(void)
{
    // compute camera position
    vec4 camera = gl_ModelViewMatrixInverse[3];
    vec4 pos = gl_ModelViewProjectionMatrix * gl_Vertex;
    vec3 posWorld = (gl_ModelViewMatrix * gl_Vertex).xyz;

    //varLightVec = gl_LightSource[1].position.xyz - posWorld;
    varLightVec = vec3(5.0, 10.0, 20.0) - posWorld;
    varViewVec = camera.xyz - posWorld;

    varNormal = (gl_ModelViewMatrixInverseTranspose * vec4(gl_Normal, 0.0)).xyz;

    gl_TexCoord[0] = gl_MultiTexCoord0;

    gl_FrontColor = gl_Color;
    gl_Position = pos;
}
