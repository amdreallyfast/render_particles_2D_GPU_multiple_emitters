#include "PolygonFaceSsbo.h"

#include "glload/include/glload/gl_4_4.h"

// TODO: header
PolygonSsbo::PolygonSsbo() :
    _vaoId(0),
    _bufferId(0),
    _drawStyle(0)
{

}

// TODO: header
PolygonSsbo::~PolygonSsbo()
{
    glDeleteVertexArrays(1, &_vaoId);
    glDeleteBuffers(1, &_bufferId);
}

// TODO: header
void PolygonSsbo::Init(const std::vector<PolygonFace> &faceCollection, unsigned int renderProgramId)
{
    _drawStyle = GL_LINES;

    // the compute shader program is not required for buffer creation
    glGenBuffers(1, &_bufferId);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _bufferId);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(PolygonFace) * faceCollection.size(), faceCollection.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // the render program is required for vertex attribute initialization or else the program WILL crash at runtime
    glUseProgram(renderProgramId);
    glGenVertexArrays(1, &_vaoId);
    glBindVertexArray(_vaoId);

    // the vertex array attributes only work on whatever is bound to the array buffer, so bind 
    // shader storage buffer to the array buffer, set up the vertex array attributes, and the 
    // VAO will then use the buffer ID of whatever is bound to it
    glBindBuffer(GL_ARRAY_BUFFER, _bufferId);
    // do NOT call glBufferData(...) because it was called earlier for the shader storage buffer

    unsigned int vertexArrayIndex = 0;
    unsigned int bufferStartOffset = 0;
    unsigned int bytesPerStep = sizeof(PolygonFace);

    // vertex attribute order is same as the structure
    // (1) point 1
    // (2) point 2
    // (3) face normal

    // P1
    GLenum itemType = GL_FLOAT;
    unsigned int numItems = sizeof(PolygonFace::_p1) / sizeof(float);
    glEnableVertexAttribArray(vertexArrayIndex);
    glVertexAttribPointer(vertexArrayIndex, numItems, itemType, GL_FALSE, bytesPerStep, (void *)bufferStartOffset);

    // P2
    itemType = GL_FLOAT;
    numItems = sizeof(PolygonFace::_p2) / sizeof(float);
    bufferStartOffset += sizeof(PolygonFace::_p1);
    vertexArrayIndex++;
    glEnableVertexAttribArray(vertexArrayIndex);
    glVertexAttribPointer(vertexArrayIndex, numItems, itemType, GL_FALSE, bytesPerStep, (void *)bufferStartOffset);

    // face normal
    itemType = GL_FLOAT;
    numItems = sizeof(PolygonFace::_normal) / sizeof(float);
    bufferStartOffset += sizeof(PolygonFace::_p2);
    vertexArrayIndex++;
    glEnableVertexAttribArray(vertexArrayIndex);
    glVertexAttribPointer(vertexArrayIndex, numItems, itemType, GL_FALSE, bytesPerStep, (void *)bufferStartOffset);

    // cleanup
    glBindVertexArray(0);   // unbind this BEFORE the array
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);    // render program
}

// TODO: header
unsigned int PolygonSsbo::VaoId() const
{
    return _vaoId;
}

// TODO: header
unsigned int PolygonSsbo::BufferId() const
{
    return _bufferId;
}

// TODO: header
unsigned int PolygonSsbo::DrawStyle() const
{
    return _drawStyle;
}

