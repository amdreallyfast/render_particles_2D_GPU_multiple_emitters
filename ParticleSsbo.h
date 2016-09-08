#pragma once

#include "Particle.h"
#include <string>

/*-----------------------------------------------------------------------------------------------
Description:
    Encapsulates the SSBO that stores Particles.  It generates a chunk of space on the GPU that 
    is big enough to store the requested number of particles, and since this buffer will be used 
    a drawing shader as well as a compute shader, this class will also up the VAO and the vertex 
    attributes.
Creator:    John Cox (9-3-2016)
-----------------------------------------------------------------------------------------------*/
class ParticleSsbo
{
public:
    ParticleSsbo();
    ~ParticleSsbo();
    
    void Init(unsigned int numParticles, unsigned int renderProgramId);

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

// for the drawing
// - buffer ID
// - VAO ID
// - draw style

// for the compute shader
// - buffer ID
// - shader binding point

