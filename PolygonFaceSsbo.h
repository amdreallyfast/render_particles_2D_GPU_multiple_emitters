#pragma once

#include "PolygonFace.h"
#include <vector>

// TODO: header
class PolygonSsbo
{
public:
    PolygonSsbo();
    ~PolygonSsbo();

    void Init(const std::vector<PolygonFace> &faceCollection, unsigned int renderProgramId);

    unsigned int VaoId() const;
    unsigned int BufferId() const;
    unsigned int DrawStyle() const;

private:
    // save on the large header inclusion of OpenGL and write out these primitive types instead 
    // of using the OpenGL typedefs
    // Note: IDs are GLuint (unsigned int), draw style is GLenum (unsigned int), GLushort is 
    // unsigned short.
    unsigned int _vaoId;
    unsigned int _bufferId;
    unsigned int _drawStyle;    // GL_TRIANGLES, GL_LINES, etc.
};