#include "ComputeUpdaterParticleUpdate.h"

#include "ShaderStorage.h"
#include "glload/include/glload/gl_4_4.h"

// TODO: header
ComputeUpdaterParticleUpdater::ComputeUpdaterParticleUpdater(unsigned int numParticles, unsigned int numFaces, const std::string &computeShaderKey)
{
	_totalParticleCount = numParticles;
	ShaderStorage &shaderStorageRef = ShaderStorage::GetInstance();

	_unifLocParticleCount = shaderStorageRef.GetUniformLocation(computeShaderKey, "uMaxParticleCount");
	_unifLocPolygonFaceCount = shaderStorageRef.GetUniformLocation(computeShaderKey, "uPolygonFaceCount");
	_unifLocDeltaTimeSec = shaderStorageRef.GetUniformLocation(computeShaderKey, "uDeltaTimeSec");

	_computeProgramId = shaderStorageRef.GetShaderProgram(computeShaderKey);

	glUseProgram(_computeProgramId);

	// the program in which this uniform is located must be bound in order to set the value
	glUniform1ui(_unifLocParticleCount, numParticles);
	glUniform1ui(_unifLocPolygonFaceCount, numFaces);
	// delta time set in Update(...)

	// atomic counter initialization courtesy of geeks3D (and my use of glBufferData(...) 
	// instead of glMapBuffer(...)
	// http://www.geeks3d.com/20120309/opengl-4-2-atomic-counter-demo-rendering-order-of-fragments/

	// particle counter
	glGenBuffers(1, &_acParticleCounterBufferId);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, _acParticleCounterBufferId);
	GLuint atomicCounterResetVal = 0;
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), (void *)&atomicCounterResetVal, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

	// the atomic counter copy buffer follows suit
	glGenBuffers(1, &_acParticleCounterCopyBufferId);
	glBindBuffer(GL_COPY_WRITE_BUFFER, _acParticleCounterCopyBufferId);
	glBufferData(GL_COPY_WRITE_BUFFER, sizeof(GLuint), 0, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

	// cleanup
	glUseProgram(0);

	// don't need to have a program or bound buffer to set the buffer base
	// Note: It seems that atomic counters must be bound where they are declared and cannot be 
	// bound dynamically like the ParticleSsbo and PolygonSsbo.  So remember to use the SAME buffer 
	// binding base as specified in the shader.
	// Also Note: Do not bind a buffer base for this one because it is not in the shader.  It is 
	// instead meant to copy the atomic counter buffer before the copy is mapped to a system 
	// memory pointer.  Doing this with the actual atomic counter caused a horrific performance 
	// drop.  It appeared to completely trash the instruction pipeline.
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, _acParticleCounterBufferId);
}

// TODO: header
ComputeUpdaterParticleUpdater::~ComputeUpdaterParticleUpdater()
{
	glDeleteBuffers(1, &_acParticleCounterBufferId);
	glDeleteBuffers(1, &_acParticleCounterCopyBufferId);
}

// TODO: header
unsigned int ComputeUpdaterParticleUpdater::Update(const float deltaTimeSec) const
{
	// spread out the particles between lots of work items, but keep it 1-dimensional for easy 
	// navigation through a 1-dimensional particle buffer
	GLuint numWorkGroupsX = (_totalParticleCount / 256) + 1;
	GLuint numWorkGroupsY = 1;
	GLuint numWorkGroupsZ = 1;

	glUseProgram(_computeProgramId);

	glUniform1f(_unifLocDeltaTimeSec, deltaTimeSec);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, _acParticleCounterBufferId);
	unsigned int atomicCounterResetValue = 0;
	glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), (void *)&atomicCounterResetValue);
	glDispatchCompute(numWorkGroupsX, numWorkGroupsY, numWorkGroupsZ);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

	// cleanup
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
	glUseProgram(0);

	// now that all active particles have updated, check how many active particles exist 
	// Note: Thanks to this post for prompting me to learn about buffer copying to solve this 
	// "extract atomic counter from compute shader" issue.
	// (http://gamedev.stackexchange.com/questions/93726/what-is-the-fastest-way-of-reading-an-atomic-counter) 
	glBindBuffer(GL_COPY_READ_BUFFER, _acParticleCounterBufferId);
	glBindBuffer(GL_COPY_WRITE_BUFFER, _acParticleCounterCopyBufferId);
	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, sizeof(GLuint));
	unsigned int *ptr = (GLuint*)glMapBufferRange(GL_COPY_WRITE_BUFFER, 0, sizeof(GLuint), GL_MAP_READ_BIT);
	unsigned int numActiveParticles = *ptr;
	glUnmapBuffer(GL_COPY_WRITE_BUFFER);

	// cleanup
	glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
	glBindBuffer(GL_COPY_READ_BUFFER, 0);

	return numActiveParticles;

}
