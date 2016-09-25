#pragma once

#include "Particle.h"
#include "IParticleEmitter.h"
#include "IParticleRegion.h"
#include "ParticleEmitterPoint.h"
#include "ParticleEmitterBar.h"
#include <string>
#include <vector>

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
    ParticlePolygonComputeUpdater();
    
    void Init(unsigned int numParticles, const std::string &computeShaderKey);
    bool AddEmitter(const IParticleEmitter *pEmitter, const int maxParticlesEmittedPerFrame);

    void Update(unsigned int numParticles, const float deltaTimeSec) const;

    //void AddEmitter(const ParticleEmitterPoint *pEmitter, const int maxParticlesEmittedPerFrame);
    //void AddEmitter(const ParticleEmitterBar *pEmitter, const int maxParticlesEmittedPerFrame);
    // no "remove emitter" method because this is just a demo


    //void Update(std::vector<Particle> &particleCollection, const unsigned int startIndex, 
    //    const unsigned int numToUpdate, const float deltaTimeSec) const;
    
    
    //void ResetAllParticles(std::vector<Particle> &particleCollection);

    // no "reset all particles" method necessary here because all particles are initialized with their "is active" flag set to 0 (false), so they will be reset on the first run through the compute shader

private:
    // program ID is a GLuint
    unsigned int _computeProgramId;

    // uniform locations are GLint
    int _unifLocParticleCount;
    int _unifLocPolygonFaceCount;
    int _unifLocDeltaTimeSec;
    int _unifLocPointEmitterCenter;
    int _unifLocBarP1;
    int _unifLocBarP2;
    int _unifLocWindowSpaceRegionTransform;
    int _unifLocWindowSpaceEmitterTransform;


    // all the updating heavy lifting goes on in the compute shader, so CPU cache coherency is not a concern for emitter storage on the CPU side and a std::vector<...> is acceptable
    // Note: The compute shader has no concept of inheritance.  Rather than store a single collection of IParticleEmitter pointers and cast them to either point or bar emitters on every update, just store them separately.
    static const int MAX_EMITTERS = 2;
    std::vector<const ParticleEmitterPoint *> _pointEmitters;
    std::vector<const ParticleEmitterBar *> _barEmitters;
    //const IParticleEmitter *_pEmitters[MAX_EMITTERS];
    //unsigned int _maxParticlesEmittedPerFrame[MAX_EMITTERS];
};
