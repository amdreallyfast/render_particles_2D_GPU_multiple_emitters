#include "SsboBase.h"

#include "glload/include/glload/gl_4_4.h"

/*-----------------------------------------------------------------------------------------------
Description:
    Gives members default values.
Parameters: None
Returns:    None
Creator: John Cox, 9-20-2016
-----------------------------------------------------------------------------------------------*/
SsboBase::SsboBase() :
    _vaoId(0),
    _bufferId(0),
    _drawStyle(0),
    _numVertices(0)
{

}

/*-----------------------------------------------------------------------------------------------
Description:
    Cleans up the buffer and VAO.  If either is 0, then the glDelete*(...) call silently does 
    nothing.
Parameters: None
Returns:    None
Creator: John Cox, 9-20-2016
-----------------------------------------------------------------------------------------------*/
SsboBase::~SsboBase()
{
    glDeleteVertexArrays(1, &_vaoId);
    glDeleteBuffers(1, &_bufferId);
}

/*-----------------------------------------------------------------------------------------------
Description:
    A simple getter for the vertex array object ID.
Parameters: None
Returns:    
    A copy of the VAO's ID.
Creator: John Cox, 9-20-2016
-----------------------------------------------------------------------------------------------*/
unsigned int SsboBase::VaoId() const
{
    return _vaoId;
}

/*-----------------------------------------------------------------------------------------------
Description:
    A simple getter for the vertex buffer object's ID.
Parameters: None
Returns:
    A copy of the VBO's ID.
Creator: John Cox, 9-20-2016
-----------------------------------------------------------------------------------------------*/
unsigned int SsboBase::BufferId() const
{
    return _bufferId;
}

/*-----------------------------------------------------------------------------------------------
Description:
    A simple getter for the draw style for the contained object (GL_TRIANGLES, GL_LINES, 
    GL_POINTS, etc.)
Parameters: None
Returns:
    A copy of the draw style GLenum.
Creator: John Cox, 9-20-2016
-----------------------------------------------------------------------------------------------*/
unsigned int SsboBase::DrawStyle() const
{
    return _drawStyle;
}

/*-----------------------------------------------------------------------------------------------
Description:
    A simple getter for the number of vertices in the SSBO.  Used in the glDrawArrays(...) call.
Parameters: None
Returns:
    See description.
Creator: John Cox, 9-20-2016
-----------------------------------------------------------------------------------------------*/
unsigned int SsboBase::NumVertices() const
{
    return _numVertices;
}

