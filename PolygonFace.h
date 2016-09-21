#pragma once

#include "glm/vec2.hpp"
#include "glm/vec4.hpp"

/*-----------------------------------------------------------------------------------------------
Description:
    This is a simple structure that describes the face of a 2D polygon.  The face begins at P1 
    and ends at P2.  In this demo, the normal will be used to calculate collisions.
Creator:    John Cox (9-8-2016)
-----------------------------------------------------------------------------------------------*/
struct PolygonFace
{
    // TODO: header
    PolygonFace(const glm::vec2 &start, const glm::vec2 &end, const glm::vec2 normal) :
        _start(glm::vec4(start, 0.0f, 1.0f)),
        _end(glm::vec4(end, 0.0f, 1.0f)),
        _normal(glm::vec4(normal, 0.0f, 1.0f))
    {
    }

    // even though this is a 2D program, vec4s were chosen because it is easier than trying to 
    // match GLSL's 16-bytes-per-variable with arrays of dummy floats
    glm::vec4 _start;
    glm::vec4 _end;
    glm::vec4 _normal;
};
