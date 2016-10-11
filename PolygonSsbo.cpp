#include "PolygonSsbo.h"

#include "glload/include/glload/gl_4_4.h"

/*-----------------------------------------------------------------------------------------------
Description:
    Calls the base class to give members initial values (zeros).
Parameters: None
Returns:    None
Creator: John Cox, 9-8-2016
-----------------------------------------------------------------------------------------------*/
PolygonSsbo::PolygonSsbo() :
    SsboBase(),
    _bufferSizeBytes(0)
{
}

/*-----------------------------------------------------------------------------------------------
Description:
    Does nothing.  Exists to be declared virtual so that the base class' destructor is called 
    upon object death.
Parameters: None
Returns:    None
Creator: John Cox, 9-8-2016
-----------------------------------------------------------------------------------------------*/
PolygonSsbo::~PolygonSsbo()
{
}

/*-----------------------------------------------------------------------------------------------
Description:
    Generates the SSBO, binds it in the computer shader, and generates vertex attributes for 
    the geometry shader.

    Does NOT allocate space for the buffer or initialize buffer data.  That is done in 
    UpdateValues(...).
Parameters: 
    computeProgramId    Required for binding the compute shader's face buffer to the SSBO.
    renderProgramId The rendering shader that will be drawing this polygon.
Returns:    None
Creator: John Cox, 10-10-2016
-----------------------------------------------------------------------------------------------*/
void PolygonSsbo::Init(unsigned int computeProgramId, unsigned int renderProgramId)
{
    _drawStyle = GL_LINES;

    // unlike the VAOs, the compute shader program is not required for buffer creation, but it 
    // is required for buffer binding
    glGenBuffers(1, &_bufferId);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _bufferId);
    
    // see the corresponding area in ParticleSsbo::Init(...) for explanation
    GLuint ssboBindingPointIndex = 13;   // or 1, or 5, or 17, or wherever IS UNUSED
    GLuint storageBlockIndex = glGetProgramResourceIndex(computeProgramId, GL_SHADER_STORAGE_BLOCK, "FaceBuffer");
    glShaderStorageBlockBinding(computeProgramId, storageBlockIndex, ssboBindingPointIndex);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ssboBindingPointIndex, _bufferId);

    // cleanup
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

    // each face is made up of two vertices, so set the attribtues for the vertices
    unsigned int vertexArrayIndex = 0;
    unsigned int bufferStartOffset = 0;
    unsigned int bytesPerStep = sizeof(MyVertex);

    // position
    GLenum itemType = GL_FLOAT;
    unsigned int numItems = sizeof(MyVertex::_position) / sizeof(float);
    glEnableVertexAttribArray(vertexArrayIndex);
    glVertexAttribPointer(vertexArrayIndex, numItems, itemType, GL_FALSE, bytesPerStep, (void *)bufferStartOffset);

    // normal
    itemType = GL_FLOAT;
    numItems = sizeof(MyVertex::_normal) / sizeof(float);
    bufferStartOffset += sizeof(MyVertex::_position);
    vertexArrayIndex++;
    glEnableVertexAttribArray(vertexArrayIndex);
    glVertexAttribPointer(vertexArrayIndex, numItems, itemType, GL_FALSE, bytesPerStep, (void *)bufferStartOffset);

    // cleanup
    glBindVertexArray(0);   // unbind this BEFORE the array
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);    // render program
}

/*-----------------------------------------------------------------------------------------------
Description:
    Dumps the given collection of particle faces into the SSBO that is managed by this object.

    Note: This SSBO object has no concept of the compute shader's contents, so it does not 
    update the compute shader's "num faces" uniform.
Parameters:
    faceCollection  Self-explanatory
Returns:    None
Creator: John Cox, 10-10-2016
-----------------------------------------------------------------------------------------------*/
void PolygonSsbo::UpdateValues(const std::vector<PolygonFace> &faceCollection)
{
    // two vertices per face (used with glDrawArrays)
    _numVertices = faceCollection.size() * 2;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _bufferId);
    unsigned int byteCounter = sizeof(PolygonFace) * faceCollection.size();
    if (byteCounter > _bufferSizeBytes)
    {
        // re-allocate more space (yes, this is a crude resize, but its a demo)
        _bufferSizeBytes = byteCounter;
        glBufferData(GL_SHADER_STORAGE_BUFFER, _bufferSizeBytes, faceCollection.data(), GL_STATIC_DRAW);
    }
    else
    {
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, _bufferSizeBytes, faceCollection.data());
    }

    // cleanup
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}