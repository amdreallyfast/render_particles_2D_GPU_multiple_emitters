#include "ParticleSsbo.h"

#include <vector>

#include "glload/include/glload/gl_4_4.h"
#include "ShaderStorage.h"

/*-----------------------------------------------------------------------------------------------
Description:
    Ensures that the object starts object with initialized values.
Parameters: None
Returns:    None
Creator: John Cox, 9-6-2016
-----------------------------------------------------------------------------------------------*/
ParticleSsbo::ParticleSsbo() :
    SsboBase()
{
}

/*-----------------------------------------------------------------------------------------------
Description:
    Does nothing.  Exists to be declared virtual so that the base class' destructor is called
    upon object death.
Parameters: None
Returns:    None
Creator: John Cox, 9-6-2016
-----------------------------------------------------------------------------------------------*/
ParticleSsbo::~ParticleSsbo()
{
	glDeleteVertexArrays(1, &_vaoId);
}

// TODO: header
// The buffer is initialized in a separate Init(...) call from the compute and render shader initialization because of the following restrictions:
// (1) OpenGL buffers cannot be generated until the OpenGL context is established, which means that the buffers cannot be generated in the constructor.
// (2) The buffer's size and contents (glBufferData(...)) should be established only once.  It is ok for it to be initialized the same way twice, but I consider that wasteful.
// (3) There is more than one shader that will use the same buffer, so the buffer needs to be available when initializing either shader with it.
// So it makes sense to generate the buffer and set its size and contents in one place, and then to configure each shader separately.
void ParticleSsbo::Init(const std::vector<Particle> &allParticles)
{
	if (_bufferId == 0)
	{
		glGenBuffers(1, &_bufferId);

		// only let the buffer size be set once
		_numVertices = allParticles.size();
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, _bufferId);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Particle) * allParticles.size(),
			allParticles.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	if (_vaoId == 0)
	{
		glGenVertexArrays(1, &_vaoId);
	}

	_hasBeenInitialized = true;
}

// TODO: header
void ParticleSsbo::ConfigureCompute(unsigned int computeProgramId)
{
	if (!_hasBeenInitialized)
	{
		fprintf(stderr, "ParticleSsbo::ConfigureCompute(...) error: SSBO has not been initialized\n");
		return;
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, _bufferId);

	// binding requires some setup
	// Note: The shader and the just-created buffer need to talk to each other.  
	// (1) Search the compute shader for a shader storage block (indicated by the keyword 
	//  "buffer") called "ParticleBuffer".
	// (2) Tell the compute shader that the storage block called "ParticleBuffer" will be 
	//  assigned to a particular "binding" location.  This can be that be 0, or 1, or 2, or 43, 
	//  or 80, or anywhere from 0 to GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS.  As long as it 
	//  DOESN'T INTERFERE WITH ANY OTHER BINDING IN THIS SHADER, then it should be fine.  If 
	//  this SSBO has binding point 1 and the geometry render shader uses, say, a vec4 for 
	//  binding point 1, this is ok because they are different shaders.
	// (3) Tell OpenGL that the shader storage block object (this is the CPU side) 
	//  will use the same binding location.  If we tell OpenGL that the compute shader's storage 
	//  block called "ParticleBuffer" will use, say, binding point 3, and then we tell OpenGL 
	//  that the just-created SSBO will use binding point 67, then the compute shader will see 
	//  an unallocated buffer and not be able to do anything.  The render shader will then 
	//  happily keep rendering the particles at their starting location and the particles appear 
	//  to go nowhere.  So get the bindings straight :).
	// Also Note: Thanks to geeks3.com for the "how to".  
	//  http://www.geeks3d.com/20140704/tutorial-introduction-to-opengl-4-3-shader-storage-buffers-objects-ssbo-demo/
	GLuint ssboBindingPointIndex = 3;   // or 1, or 5, or 17, or wherever IS UNUSED
	GLuint storageBlockIndex = glGetProgramResourceIndex(computeProgramId, GL_SHADER_STORAGE_BLOCK, "ParticleBuffer");
	glShaderStorageBlockBinding(computeProgramId, storageBlockIndex, ssboBindingPointIndex);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ssboBindingPointIndex, _bufferId);

	// cleanup
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

// TODO: header
void ParticleSsbo::ConfigureRender(unsigned int renderProgramId)
{
	if (!_hasBeenInitialized)
	{
		fprintf(stderr, "ParticleSsbo::ConfigureRender(...) error: SSBO has not been initialized\n");
		return;
	}

	// set up the VAO
	// now set up the vertex array indices for the drawing shader
	// Note: MUST bind the program beforehand or else the VAO binding will blow up.  It won't 
	// spit out an error but will rather silently bind to whatever program is currently bound, 
	// even if it is the undefined program 0.
	glUseProgram(renderProgramId);
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



///*-----------------------------------------------------------------------------------------------
//Description:
//    Given a desired number of polygons, generates a collection of that many particles with 
//    default values, allocates an SSBO for them, dumps the data into them, and sets up the 
//    vertex attribtues for the render shader.
//
//    Note: The compute shader ID is not necessary to set up the SSBO.
//    Also Note: Particles are loaded onto the GPU in this function in their "zero" state, which 
//    means that their position and velocity default to 0s and their "is active" flag defaults to 
//    false.  It is ok that they are not initialized with any emitters because, as the compute 
//    shader runs, it will reset any inactive particles that it comes across, and the particle 
//    rendering shader will render inactive particles as black.  This will still look ok.
//Parameters: 
//    numParticles        Determines the size of the SSBO.
//    computeProgramId    Required for binding the compute shader's particle buffer to the SSBO.
//    renderProgramId     The rendering shader that will be drawing this polygon.
//Returns:    None
//Creator: John Cox, 9-6-2016
//-----------------------------------------------------------------------------------------------*/
//void ParticleSsbo::Init(const std::vector<Particle> &allParticles, 
//    unsigned int computeProgramId, unsigned int renderProgramId)
//{
//    _drawStyle = GL_POINTS;
//    _numVertices = allParticles.size();
//
//    // unlike the VAOs, the compute shader program is not required for buffer creation, but it 
//    // is required for buffer binding
//    _bufferId = 0;
//    glGenBuffers(1, &_bufferId);
//    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _bufferId);
//    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Particle) * allParticles.size(), allParticles.data(), 
//        GL_STATIC_DRAW);
//
//    // binding requires some setup
//    // Note: The shader and the just-created buffer need to talk to each other.  
//    // (1) Search the compute shader for a shader storage block (indicated by the keyword 
//    //  "buffer") called "ParticleBuffer".
//    // (2) Tell the compute shader that the storage block called "ParticleBuffer" will be 
//    //  assigned to a particular "binding" location.  This can be that be 0, or 1, or 2, or 43, 
//    //  or 80, or anywhere from 0 to GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS.  As long as it 
//    //  DOESN'T INTERFERE WITH ANY OTHER BINDING IN THIS SHADER, then it should be fine.  If 
//    //  this SSBO has binding point 1 and the geometry render shader uses, say, a vec4 for 
//    //  binding point 1, this is ok because they are different shaders.
//    // (3) Tell OpenGL that the just-created shader storage block object (this is the CPU side) 
//    //  will use the same binding location.  If we tell OpenGL that the compute shader's storage 
//    //  block called "ParticleBuffer" will use, say, binding point 3, and then we tell OpenGL 
//    //  that the just-created SSBO will use binding point 67, then the compute shader will see 
//    //  an unallocated buffer and not be able to do anything.  The render shader will then 
//    //  happily keep rendering the particles at their starting location and the particles appear 
//    //  to go nowhere.  So get the bindings straight :).
//    // Also Note: Thanks to geeks3.com for the "how to".  
//    //  http://www.geeks3d.com/20140704/tutorial-introduction-to-opengl-4-3-shader-storage-buffers-objects-ssbo-demo/
//    GLuint ssboBindingPointIndex = 3;   // or 1, or 5, or 17, or wherever IS UNUSED
//    GLuint storageBlockIndex = glGetProgramResourceIndex(computeProgramId, GL_SHADER_STORAGE_BLOCK, "ParticleBuffer");
//    glShaderStorageBlockBinding(computeProgramId, storageBlockIndex, ssboBindingPointIndex);
//    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ssboBindingPointIndex, _bufferId);
//
//    // cleanup
//    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
//
//    // set up the VAO
//    // now set up the vertex array indices for the drawing shader
//    // Note: MUST bind the program beforehand or else the VAO binding will blow up.  It won't 
//    // spit out an error but will rather silently bind to whatever program is currently bound, 
//    // even if it is the undefined program 0.
//    glUseProgram(renderProgramId);
//    glGenVertexArrays(1, &_vaoId);
//    glBindVertexArray(_vaoId);
//
//    // the vertex array attributes only work on whatever is bound to the array buffer, so bind 
//    // shader storage buffer to the array buffer, set up the vertex array attributes, and the 
//    // VAO will then use the buffer ID of whatever is bound to it
//    glBindBuffer(GL_ARRAY_BUFFER, _bufferId);
//    // do NOT call glBufferData(...) because it was called earlier for the shader storage buffer
//
//    // vertex attribute order is same as the structure
//    // (1) position
//    // (2) velocity
//    // (3) "is active" flag
//    
//    unsigned int vertexArrayIndex = 0;
//    unsigned int bufferStartOffset = 0;
//    unsigned int bytesPerStep = sizeof(Particle);
//
//    // position
//    GLenum itemType = GL_FLOAT;
//    unsigned int numItems = sizeof(Particle::_position) / sizeof(float);
//    glEnableVertexAttribArray(vertexArrayIndex);
//    glVertexAttribPointer(vertexArrayIndex, numItems, itemType, GL_FALSE, bytesPerStep, 
//        (void *)bufferStartOffset);
//
//    // velocity
//    itemType = GL_FLOAT;
//    numItems = sizeof(Particle::_velocity) / sizeof(float);
//    bufferStartOffset += sizeof(Particle::_position);
//    vertexArrayIndex++;
//    glEnableVertexAttribArray(vertexArrayIndex);
//    glVertexAttribPointer(vertexArrayIndex, numItems, itemType, GL_FALSE, bytesPerStep, 
//        (void *)bufferStartOffset);
//
//    // "is active" flag
//    itemType = GL_INT;
//    numItems = sizeof(Particle::_isActive) / sizeof(int);
//    bufferStartOffset += sizeof(Particle::_velocity);
//    vertexArrayIndex++;
//    glEnableVertexAttribArray(vertexArrayIndex);
//    glVertexAttribPointer(vertexArrayIndex, numItems, itemType, GL_FALSE, bytesPerStep, 
//        (void *)bufferStartOffset);
//
//    // cleanup
//    glBindVertexArray(0);   // unbind this BEFORE the array
//    glBindBuffer(GL_ARRAY_BUFFER, 0);
//    glUseProgram(0);    // render program
//}
