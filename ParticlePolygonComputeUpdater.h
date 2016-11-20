#pragma once

#include "Particle.h"
#include "IParticleEmitter.h"
#include "ParticleEmitterPoint.h"
#include "ParticleEmitterBar.h"
#include <string>
#include <vector>
#include "glm/mat4x4.hpp"

/*-----------------------------------------------------------------------------------------------
Description:
    Encapsulates particle updating for the compute shader.  This is only concerned with 
    (1) setting the compute shader's uniforms to use a particlar emitter
    (2) summoning the shader

    Unlike the "particles 2D basic GPU" project, the polygon particle region is handled in an 
    SSBO, so it is not used here.

    Note: When this class goes "poof", it won't delete the given pointers.  This is ensured by
    only using const pointers.
Creator:    John Cox (9-18-2016)
-----------------------------------------------------------------------------------------------*/
class ParticlePolygonComputeUpdater
{
public:
    ParticlePolygonComputeUpdater(unsigned int numParticles, unsigned int numFaces, 
        const std::string &computeShaderKey);
    ~ParticlePolygonComputeUpdater();

    bool AddEmitter(const IParticleEmitter *pEmitter);

    unsigned int Update(const float deltaTimeSec, const glm::mat4 &windowSpaceTransform) const;

private:
    unsigned int _totalParticleCount;

    // program ID is a GLuint
    unsigned int _computeProgramId;

    // unlike most OpeGL IDs, uniform locations are GLint
    int _unifLocParticleCount;
    int _unifLocPolygonFaceCount;
    int _unifLocDeltaTimeSec;
    int _unifLocMaxParticleEmitCount;
    int _unifLocUsePointEmitter;
    int _unifLocOnlyResetParticles;     // as opposed to updating position
    int _unifLocPointEmitterCenter;
    int _unifLocMinParticleVelocity;
    int _unifLocDeltaParticleVelocity;
    int _unifLocBarP1;
    int _unifLocBarP2;
    int _unifLocBarEmitDir;

    // just leave these in case I need them in a future program
    //int _unifLocWindowSpaceRegionTransform;
    //int _unifLocWindowSpaceEmitterTransform;
    
    // one atomic counter is used to 
    // (1) enforce the number of emitted particles per emitter per frame and 
    // (2) count how many particles are currently active
    // Note: The atomic counter, as fast as it is, doesn't seem to fully enforce the number of 
    // emitted particles per emitter per frame.  During experiments, when the emit restriction 
    // was 6, debugging showed that the first emitter spit out 92.  This varies by experiment, 
    // of course, but the point is that compute shaders are so fast that many processors can get 
    // to the same atomic counter check at the same time.  So this only sort of enforces the 
    // limit.
    // Also Note: The copy buffer is necessary to avoid trashing OpenGL's beautifully 
    // synchronized pipeline.  Experiments showed that, after particle updating, mapping a 
    // pointer to the atomic counter dropped frame rates from ~60fps -> ~3fps.  Ouch.  But now 
    // I've learned about buffer copying, so now the buffer mapping happens on a buffer that is 
    // not part of the compute shader's pipeline, and frame rates are back up to ~60fps.  
    // Lovely :)
    unsigned int _atomicCounterBuffer;
    unsigned int _atomicCounterCopyBuffer;

	// another atomic counter is used as a seed for the random hash
	// Note: Rather than needing to seed the position and velocity as I did in the 
	// "single emitter" project, use an atomic counter.  If it reaches maximum unsigned int, 
	// that is ok.  The value will wrap around to 0 and begin again.  
	unsigned int _atomicCounterRandSeed;

    // all the updating heavy lifting goes on in the compute shader, so CPU cache coherency is 
    // not a concern for emitter storage on the CPU side and a std::vector<...> is acceptable
    // Note: The compute shader has no concept of inheritance.  Rather than store a single 
    // collection of IParticleEmitter pointers and cast them to either point or bar emitters on 
    // every update, just store them separately.
    static const int MAX_EMITTERS = 4;
    std::vector<const ParticleEmitterPoint *> _pointEmitters;
    std::vector<const ParticleEmitterBar *> _barEmitters;
};
