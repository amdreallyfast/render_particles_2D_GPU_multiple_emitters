#pragma once

#include "Particle.h"
#include <string>

/*-----------------------------------------------------------------------------------------------
Description:
    Generates a chunk of space on the GPU that is big enough to store the requested number of 
    particles, plus determines the vertex attribute pointers of that buffer.  Upon deletion, the 
    buffer and VAO are deleted.

    This is a struct because I can't think of a good reason to make it private.  The OpenGL data 
    should definitely be restricted, but the particle collection needs to be accessible by the 
    ParticleUpdater class, so I could jump through some hoops to make it accessible through a 
    friend interface or something, or I could just make them public and be nice and not mess 
    them up.
Creator:    John Cox (9-3-2016)
-----------------------------------------------------------------------------------------------*/
struct ParticleStorageGpu
{
public:
    ParticleStorageGpu();
    ~ParticleStorageGpu();
    
    void Init(unsigned int numParticles, const std::string &renderShaderKey);

    // save on the large header inclusion of OpenGL and write out these primitive types instead 
    // of using the OpenGL typedefs
    // Note: IDs are GLuint (unsigned int), draw style is GLenum (unsigned int), GLushort is 
    // unsigned short.
    unsigned int _vaoId;
    unsigned int _bufferId;
    unsigned int _drawStyle;    // GL_TRIANGLES, GL_LINES, etc.
};

