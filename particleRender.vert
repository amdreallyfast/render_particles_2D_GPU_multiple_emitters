#version 440

// position in window space (both X and Y on the range [-1,+1])
layout (location = 0) in vec2 pos;  

// velocity also in window space (ex: an X speed of 1.0 would cross the window horizontally in 2 
// seconds)
layout (location = 1) in vec2 vel;  

layout (location = 2) in int isActive;

// must have the same name as its corresponding "in" item in the frag shader
smooth out vec4 particleColor;

// TODO: render inactive particles as black

void main()
{
    // hard code a white particle color
    //particleColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    if (isActive == 0)
    {
        // invisible (alpha = 0), but "fully transparent" does not mean "no color", it merely 
        // means that the color of this thing will be added to the thing behind it (see Z 
        // adjustment later)
        particleColor = vec4(0.0f, 0.0f, 0.0f, 0.1f);
    }
    else
    {
        // active => opaque (alpha = 1)
        particleColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    
    //??why does it win if Zparticle < Zpolygonline??

    // depth range is 0-1, and opaque stuff must be draw first or else the blending will blend an opaque value first and then the transparent value
    // Note: In this demo, the only thing that matters is that the particle is completely black and transparent when it is inactive and that it is white and opaque when active.
	gl_Position = vec4(pos, -0.6f, 1.0f);
}

