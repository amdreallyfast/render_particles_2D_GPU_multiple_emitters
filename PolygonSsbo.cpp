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
    SsboBase()
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
    Given a collection of polygon faces, this allocates an SSBO, dumps the face data into them, 
    and sets up the vertex attribtues for the render shader.

    Note: The compute shader ID is not necessary to set up the SSBO.
Parameters: 
    faceCollection  Self-explanatory.
    renderProgramId The rendering shader that will be drawing this polygon.
Returns:    None
Creator: John Cox, 9-25-2016
-----------------------------------------------------------------------------------------------*/
void PolygonSsbo::Init(const std::vector<PolygonFace> &faceCollection, unsigned int renderProgramId)
{
    _drawStyle = GL_LINES;

    // two vertices per face
    _numVertices = faceCollection.size() * 2;

    ////glm::vec2 p1(-0.25f, -0.5f);
    ////glm::vec2 p2(+0.25f, -0.5f);
    ////glm::vec2 p3(+0.5f, +0.25f);
    ////glm::vec2 p4(-0.5f, +0.25f);
    ////PolygonFace face1(p1, p2, glm::vec2(1.0f, 0.0f));
    ////PolygonFace face2(p2, p3, glm::vec2(1.0f, 0.0f));
    ////PolygonFace face3(p3, p4, glm::vec2(1.0f, 0.0f));
    ////PolygonFace face4(p4, p1, glm::vec2(1.0f, 0.0f));
    ////PolygonFace faceArr[4] = { face1, face2, face3, face4 };

    //glm::vec4 p1(-0.25f, -0.5f, 0.0f, 1.0f);
    //glm::vec4 p2(+0.25f, -0.5f, 0.0f, 1.0f);
    //glm::vec4 p3(+0.5f, +0.25f, 0.0f, 1.0f);
    //glm::vec4 p4(-0.5f, +0.25f, 0.0f, 1.0f);

    //glm::vec4 n1(+1.00f, +0.4f, 0.0f, 1.0f);
    //glm::vec4 n2(+0.75f, -0.4f, 0.0f, 1.0f);
    //glm::vec4 n3(+0.50f, +0.8f, 0.0f, 1.0f);
    //glm::vec4 n4(+0.25f, -0.8f, 0.0f, 1.0f);

    //glm::vec4 vertexArr[] = { p1, p2, n1, p2, p3, n2, p3, p4, n3, p4, p1, n4 };
    ////glm::vec4 vertexArr[] = { p1, p2, n1, p3, p4, n2 };

    // the compute shader program is not required for buffer creation
    glGenBuffers(1, &_bufferId);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _bufferId);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(PolygonFace) * faceCollection.size(), faceCollection.data(), GL_STATIC_DRAW);
    //glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(vertexArr), vertexArr, GL_STATIC_DRAW);
    //glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _bufferId);
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

    //// face normal
    //itemType = GL_FLOAT;
    //numItems = sizeof(PolygonFace::_normal) / sizeof(float);
    //bufferStartOffset += sizeof(PolygonFace::_end);
    //vertexArrayIndex++;
    //glEnableVertexAttribArray(vertexArrayIndex);
    //glVertexAttribPointer(vertexArrayIndex, numItems, itemType, GL_FALSE, bytesPerStep, (void *)bufferStartOffset);

    // cleanup
    glBindVertexArray(0);   // unbind this BEFORE the array
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);    // render program
}

