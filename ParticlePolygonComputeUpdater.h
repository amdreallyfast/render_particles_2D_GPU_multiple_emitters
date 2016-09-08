#pragma once

#include "Particle.h"
#include "IParticleEmitter.h"
#include "IParticleRegion.h"
#include <string>

/*-----------------------------------------------------------------------------------------------
Description:
    Encapsulates particle updating for the compute shader that checks particles against a 
    polygon region.  The main function is the "update" method.

    Note: When this class goes "poof", it won't delete the given pointers.  This is ensured by
    only using const pointers.
Creator:    John Cox (7-4-2016)
-----------------------------------------------------------------------------------------------*/
class ParticlePolygonComputeUpdater
{
public:
    ParticlePolygonComputeUpdater();
    
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

    // use arrays instead of std::vector<...> for the sake of cache coherency
    unsigned int _emitterCount;
    static const int MAX_EMITTERS = 5;
    //const IParticleEmitter *_pEmitters[MAX_EMITTERS];
    //unsigned int _maxParticlesEmittedPerFrame[MAX_EMITTERS];
};
