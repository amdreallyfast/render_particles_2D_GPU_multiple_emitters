#include "ParticleSsbo.h"

#include <vector>

#include "glload/include/glload/gl_4_4.h"


/*-----------------------------------------------------------------------------------------------
Description:
    Ensures that the object starts object with initialized values.
Parameters: None
Returns:    None
Exception:  Safe
Creator:    John Cox (9-6-2016)
-----------------------------------------------------------------------------------------------*/
ParticleSsbo::ParticleSsbo() :
    SsboBase()
{
}

// TODO: header
ParticleSsbo::~ParticleSsbo()
{
    glDeleteVertexArrays(1, &_vaoId);
    glDeleteBuffers(1, &_bufferId);
}

// TODO: header
// Note: Creating the shader storage buffer does not require the compute shader's program ID and so the compute shader's key is not necessary.  This method only needs the render shader's key because the vertex attrib arrays are determined here.  
// Note: Particles are loaded onto the GPU in this function in their "zero" state, which means that their position and velocity default to 0s and their "is active" flag defaults to false.  It is ok that they are not initialized with any emitters because, as the compute shader runs, it will reset any inactive particles that it comes across, and the particle rendering shader will render inactive particles as black.  This will still look ok.
void ParticleSsbo::Init(unsigned int numParticles, unsigned int renderProgramId)
{
    _drawStyle = GL_POINTS;
    _numItems = numParticles;

    std::vector<Particle> allParticles(numParticles);

    // setting up this buffer does not require a program ID
    _bufferId = 0;
    glGenBuffers(1, &_bufferId);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _bufferId);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Particle) * numParticles, allParticles.data(), 
        GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // set up the VAO
    // now set up the vertex array indices for the drawing shader
    // Note: MUST bind the program beforehand or else the VAO binding will blow up.  It won't 
    // spit out an error but will rather silently bind to whatever program is currently bound, 
    // even if it is the undefined program 0.
    glUseProgram(renderProgramId);
    glGenVertexArrays(1, &_vaoId);
    glBindVertexArray(_vaoId);

    // the vertex array attributes only work on whatever is bound to the array buffer, so bind 
    // shader storage buffer to the array buffer, set up the vertex array attributes, and the 
    // VAO will then use the buffer ID of whatever is bound to it
    glBindBuffer(GL_ARRAY_BUFFER, _bufferId);
    // do NOT call glBufferData(...) because it was called earlier for the shader storage buffer

    // vertex attribute order is same as the structure
    // (1) position
    // (2) velocity
    // (3) "is active" flag
    
    unsigned int vertexArrayIndex = 0;
    unsigned int bufferStartOffset = 0;
    unsigned int bytesPerStep = sizeof(Particle);

    // position
    GLenum itemType = GL_FLOAT;
    unsigned int numItems = sizeof(Particle::_position) / sizeof(float);
    glEnableVertexAttribArray(vertexArrayIndex);
    glVertexAttribPointer(vertexArrayIndex, numItems, itemType, GL_FALSE, bytesPerStep, 
        (void *)bufferStartOffset);

    // velocity
    itemType = GL_FLOAT;
    numItems = sizeof(Particle::_velocity) / sizeof(float);
    bufferStartOffset += sizeof(Particle::_position);
    vertexArrayIndex++;
    glEnableVertexAttribArray(vertexArrayIndex);
    glVertexAttribPointer(vertexArrayIndex, numItems, itemType, GL_FALSE, bytesPerStep, 
        (void *)bufferStartOffset);

    // "is active" flag
    itemType = GL_INT;
    numItems = sizeof(Particle::_isActive) / sizeof(int);
    bufferStartOffset += sizeof(Particle::_velocity);
    vertexArrayIndex++;
    glEnableVertexAttribArray(vertexArrayIndex);
    glVertexAttribPointer(vertexArrayIndex, numItems, itemType, GL_FALSE, bytesPerStep, 
        (void *)bufferStartOffset);

    // cleanup
    glBindVertexArray(0);   // unbind this BEFORE the array
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);    // render program
}
