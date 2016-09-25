#include "SsboBase.h"

#include "glload/include/glload/gl_4_4.h"

// TODO: header
// gives members default values
SsboBase::SsboBase() :
    _vaoId(0),
    _bufferId(0),
    _drawStyle(0),
    _numItems(0)
{

}

// TODO: header
// cleans up the buffer IDs
// if the buffer ID is 0, then the glDelete*(...) calls silently do nothing
SsboBase::~SsboBase()
{
    glDeleteVertexArrays(1, &_vaoId);
    glDeleteBuffers(1, &_bufferId);
}

// TODO: header
unsigned int SsboBase::VaoId() const
{
    return _vaoId;
}

// TODO: header
unsigned int SsboBase::BufferId() const
{
    return _bufferId;
}

// TODO: header
// GL_TRIANGLES, GL_LINES, etc.
unsigned int SsboBase::DrawStyle() const
{
    return _drawStyle;
}

// TODO: header
unsigned int SsboBase::NumItems() const
{
    return _numItems;
}

