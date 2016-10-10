#version 440

layout (location = 0) in vec2 pos;

uniform mat4 transformMatrixWindowSpace;

void main()
{
    // nothing special here except the Z value
    // Note: See explanation for alpha blending in Init() in main.cpp.
    vec4 p = vec4(pos, -0.8f, 1.0f);
    //gl_Position = transformMatrixWindowSpace * p;
    gl_Position = p;
}

