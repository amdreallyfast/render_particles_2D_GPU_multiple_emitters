#pragma once

#include "glm/vec2.hpp"
#include "glm/vec4.hpp"

/*-----------------------------------------------------------------------------------------------
Description:
    Stores all info necessary to draw a single vertex.
Creator: John Cox (6-12-2016)
-----------------------------------------------------------------------------------------------*/
struct MyVertex
{
    MyVertex(const glm::vec2 &pos, const glm::vec2 &normal) :
        _position(pos, 0.0f, 1.0f),
        _normal(normal, 0.0f, 1.0f)
    {

    }

    // even though this is a 2D program, vec4s were chosen because it is easier than trying to 
    // match GLSL's 16-bytes-per-variable with arrays of dummy floats
    glm::vec4 _position;
    glm::vec4 _normal;
};

