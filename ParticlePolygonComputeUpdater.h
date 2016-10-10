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

    bool AddEmitter(const IParticleEmitter *pEmitter);
    void InitParticleCollection(std::vector<Particle> &initThis);

    void Update(const float deltaTimeSec, const glm::mat4 &windowSpaceTransform) const;

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
    int _unifLocWindowSpaceRegionTransform;
    int _unifLocWindowSpaceEmitterTransform;
    unsigned int _atomicCounterBuffer;


    // all the updating heavy lifting goes on in the compute shader, so CPU cache coherency is 
    // not a concern for emitter storage on the CPU side and a std::vector<...> is acceptable
    // Note: The compute shader has no concept of inheritance.  Rather than store a single 
    // collection of IParticleEmitter pointers and cast them to either point or bar emitters on 
    // every update, just store them separately.
    static const int MAX_EMITTERS = 4;
    std::vector<const ParticleEmitterPoint *> _pointEmitters;
    std::vector<const ParticleEmitterBar *> _barEmitters;
};
