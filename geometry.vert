#version 440

layout (location = 0) in vec2 pos;

uniform mat4 transformMatrixWindowSpace;

void main()
{
    // nothing special here
    // Note: Just giving the 2D vectors all the same Z value (-1 was arbitrarily chosen; nothing 
    // special) and giving them a 1.0f w value so that they can be translated by a 4D matrix.
	
    
    
    // to make alpha blending work, the opaque geometry must draw behind (have a lower Z value than) the possibly transparent particle
    // Note: I have no idea why inactive particles (completely black and alpha = 0) that draw behind the polygon fragments win out in the frame buffer.


    // Note: Due to the way that OpenGL alpha blending works, only a fraction of the fragment color (the alpha channel being that 0-1 fraction) will be added on top of whatever was already drawn.  If a completely black particle (inactive particles draw black with alpha = 0) were drawn behind the polygon geometry, then a completely white fragment for the polygon geometry (all white with alpha = 1) will 
    // or else inactive particles (draw black with alpha = 0) will take priority and make the 
    vec4 p = vec4(pos, -0.8f, 1.0f);
    gl_Position = transformMatrixWindowSpace * p;
    //gl_Position = transformMatrixWindowSpace * pos;
}

