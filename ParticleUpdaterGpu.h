#pragma once

#include "Particle.h"
#include "IParticleEmitter.h"
#include "IParticleRegion.h"
#include <string>

/*-----------------------------------------------------------------------------------------------
Description:
    Encapsulates particle updating with a given emitter and region.  The main function is the 
    "update" method.

    Note: When this class goes "poof", it won't delete the given pointers.  This is ensured by
    only using const pointers.
Creator:    John Cox (7-4-2016)
-----------------------------------------------------------------------------------------------*/
class ParticleUpdaterGpu
{
public:
    ParticleUpdaterGpu();
    
    void Init(const std::string &computeShaderKey);
    void Update(const float deltaTimeSec) const;

    void SetRegion(const IParticleRegion *pRegion);
    void AddEmitter(const IParticleEmitter *pEmitter, const int maxParticlesEmittedPerFrame);
    // no "remove emitter" method because this is just a demo


    //void Update(std::vector<Particle> &particleCollection, const unsigned int startIndex, 
    //    const unsigned int numToUpdate, const float deltaTimeSec) const;
    //void ResetAllParticles(std::vector<Particle> &particleCollection);

private:
    std::string _computeShaderKey;
    unsigned int _unifLocPolygonFaceCount;
    unsigned int _unifLocDeltaTimeSec;
    unsigned int _unifLocRadiusSqr;
    unsigned int _unifLocParticleRegionCircleCenter;
    unsigned int _unifLocPointEmitterCenter;

    // the form "const something *" means that it is a pointer to a const something, so the 
    // pointer can be changed for a new region or emitter, but the region or emitter itself 
    // can't be altered

    //const IParticleRegion *_pRegion;

    // use arrays instead of std::vector<...> for the sake of cache coherency
    unsigned int _emitterCount;
    static const int MAX_EMITTERS = 5;
    //const IParticleEmitter *_pEmitters[MAX_EMITTERS];
    //unsigned int _maxParticlesEmittedPerFrame[MAX_EMITTERS];
};
