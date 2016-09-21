#include "ParticlePolygonComputeUpdater.h"

#include "ShaderStorage.h"
#include "ParticleRegionCircle.h"
#include "ParticleRegionPolygon.h"
#include "IParticleEmitter.h"

#include "glload/include/glload/gl_4_4.h"

/*-----------------------------------------------------------------------------------------------
Description:
    Ensures that the object starts object with initialized values.

    ??does anything??
Parameters: None
Returns:    None
Exception:  Safe
Creator:    John Cox (7-4-2016)
-----------------------------------------------------------------------------------------------*/
ParticlePolygonComputeUpdater::ParticlePolygonComputeUpdater() //:
    //_pRegion(0)
{
    //for (size_t emitterIndex = 0; emitterIndex < MAX_EMITTERS; emitterIndex++)
    //{
    //    _pEmitters[emitterIndex] = 0;
    //    _maxParticlesEmittedPerFrame[emitterIndex] = 0;
    //}
    //_emitterCount = 0;
}

// TODO: header
// computeShaderKey: used to access the shader program in the shader storage object
// particles: the collection of Particle data that is to be uploaded to the GPU
void ParticlePolygonComputeUpdater::Init(const std::string &computeShaderKey)
{
    ShaderStorage &shaderStorageRef = ShaderStorage::GetInstance();
    _computeProgramId = shaderStorageRef.GetShaderProgram(computeShaderKey);

    _unifLocPolygonFaceCount = shaderStorageRef.GetUniformLocation(computeShaderKey, "uPolygonFaceCount");
    _unifLocDeltaTimeSec = shaderStorageRef.GetUniformLocation(computeShaderKey, "uDeltaTimeSec");
    _unifLocRadiusSqr = shaderStorageRef.GetUniformLocation(computeShaderKey, "uRadiusSqr");
    _unifLocParticleRegionCircleCenter = shaderStorageRef.GetUniformLocation(computeShaderKey, "uParticleRegionCircleCenter");
    _unifLocPointEmitterCenter = shaderStorageRef.GetUniformLocation(computeShaderKey, "uPointEmitterCenter");

    // TODO: find uniforms and store them in this class
    //uniform uint uParticleCount;
    //uniform uint uPolygonFaceCount;
    //uniform float uDeltaTimeSec;
    // uniform float uRadiusSqr;
    // uniform vec4 uParticleRegionCircleCenter;




    // TODO: query work group counts and size (??move to display code??)


    // TODO:
    // - ParticleStorageGpu.h/cpp -> ParticlePolygonSsbo.h/cpp (need to represent each compute shader's SSBO)
    // - particleMain.comp -> PaticlePolygon.comp
    // - ParticlePolygonComputeUpdater -> ParticlePolygonComputeUpdater.h/cpp (each compute shader will have its own set of uniforms)

    // generate the storage 
}
//
///*-----------------------------------------------------------------------------------------------
//Description:
//    A simple assignment. 
//Parameters: 
//    pRegion     A pointer to a "particle region" interface.
//Returns:    None
//Exception:  Safe
//Creator:    John Cox (7-4-2016)
//-----------------------------------------------------------------------------------------------*/
//void ParticlePolygonComputeUpdater::SetRegion(const IParticleRegion *pRegion)
//{
//    //_pRegion = pRegion;
//    if (dynamic_cast<const ParticleRegionCircle *>(pRegion) == 0)
//    {
//
//    }
//    else if (dynamic_cast<const ParticleRegionPolygon *>(pRegion) == 0)
//    {
//
//    }
//    else
//    {
//        // bad particle region (??throw a fit??)
//    }
//}

/*-----------------------------------------------------------------------------------------------
Description:
    Adds a point emitter to internal storage.  This is used to initialize particles.  If there multiple emitters, then the update will need to perform multiple calls to the compute shader, each with different emitter information.

    If, for some reason, the particle emitter cannot be cast to either a point emitter or a bar emitter, then the emitter will not be added to either particle emitter collection and "false" is returned. 
Parameters:
    pEmitter    A pointer to a "particle emitter" interface.
    maxParticlesEmittedPerFrame     A restriction on the provided emitter to prevent all particles from being emitted at once.
Returns:    
    True if the emitter was added, otherwise false.
Exception:  Safe
Creator:    John Cox (9-18-2016)
-----------------------------------------------------------------------------------------------*/
bool ParticlePolygonComputeUpdater::AddEmitter(const IParticleEmitter *pEmitter,
    const int maxParticlesEmittedPerFrame)
{
    const ParticleEmitterPoint *pointEmitter = 
        dynamic_cast<const ParticleEmitterPoint *>(pEmitter);
    const ParticleEmitterBar *barEmitter =
        dynamic_cast<const ParticleEmitterBar *>(pEmitter);

    if (pointEmitter != 0 || (_pointEmitters.size() < MAX_EMITTERS))
    {
        _pointEmitters.push_back(pointEmitter);
        return true;
    }
    else if (barEmitter != 0 && (_barEmitters.size() < MAX_EMITTERS))
    {
        _barEmitters.push_back(barEmitter);
        return true;
    }

    return false;
}

/*-----------------------------------------------------------------------------------------------
Description:
    Summons the compute shader.

    TODO: spread out the particles between multiple emitters.
Parameters:
    numParticles    Used to compute the number of work groups.
    deltatimeSec        Self-explanatory
Returns:    None
Exception:  Safe
Creator:    John Cox (7-4-2016)
-----------------------------------------------------------------------------------------------*/
void ParticlePolygonComputeUpdater::Update(unsigned int numParticles, 
    const float deltaTimeSec) const
{
    if (_pointEmitters.empty() && _barEmitters.empty())
    {
        // nothing to do
        return;
    }

    // TODO: spread out the particles between multiple emitters.

//    uPointEmitterCenter

    glUseProgram(_computeProgramId);
    glUniform1f(_unifLocDeltaTimeSec, deltaTimeSec);

    // an array will not assume data order in glm's vec4
    glm::vec4 emitterPos = _pointEmitters[0]->GetPos();
    float emittertPosArr[4] = { emitterPos.x, emitterPos.y, emitterPos.z, emitterPos.w };
    glUniform4fv(_unifLocPointEmitterCenter, 1, emittertPosArr);

    // read the explanation in particlePolygonRegion.comp for why these are computed this way
    GLuint numWorkGroupsX = (numParticles / 256) + 1;
    GLuint numWorkGroupsY = 1;
    GLuint numWorkGroupsZ = 1;
    glDispatchCompute(numWorkGroupsX, numWorkGroupsY, numWorkGroupsZ);

    // tell the GPU:
    // (1) Accesses to the shader buffer after this call will reflect writes prior to the 
    // barrier.  This is only available in OpenGL 4.3 or higher.
    // (2) Vertex data sourced from buffer objects after the barrier will reflect data written 
    // by shaders prior to the barrier.  The affected buffer(s) is determined by the buffers 
    // that were bound for the vertex attributes.  In this case, that means GL_ARRAY_BUFFER.
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

    // cleanup
    glUseProgram(0);

    //// when using multiple emitters, it looks best to cycle between all emitters one by one, but that is also more difficult to deal with and requires a number of different checks and conditions, and provided the total number of particles to update exceeds the total number of max particles emitted per frame across all emitters, then it will look just as good to "fill up" each emitter one by one, and that is much easier to implement
    //unsigned int particleEmitCounter = 0;
    //int emitterIndex = 0;

    ////for (size_t particleIndex = 0; particleIndex < particleCollection.size(); particleIndex++)
    //for (size_t particleIndex = startIndex; particleIndex < end; particleIndex++)
    //{
    //    Particle &pCopy = particleCollection[particleIndex];
    //    if (_pRegion->OutOfBounds(pCopy))
    //    {
    //        pCopy._isActive = false;
    //    }

    //    if (pCopy._isActive)
    //    {
    //        pCopy._position = pCopy._position + (pCopy._velocity * deltaTimeSec);
    //    }
    //    else if (emitterIndex < MAX_EMITTERS)   // also implicitly, "is active" is false
    //    {
    //        // if all emitters have put out all they can this frame, then this condition will 
    //        // not be entered
    //        _pEmitters[emitterIndex]->ResetParticle(&pCopy);
    //        pCopy._isActive = true;
    //        particleCollection[particleIndex] = pCopy;

    //        particleEmitCounter++;
    //        if (particleEmitCounter >= _maxParticlesEmittedPerFrame[emitterIndex])
    //        {
    //            // get the next emitter
    //            // Note: This is in a while loop because the number of emitters in this particle updater are unknown and some of the pointers in the emitter array may still be 0, so this has to loop through (potentially) all of them.
    //            emitterIndex++;
    //            while (emitterIndex < MAX_EMITTERS)
    //            {
    //                if (_pEmitters[emitterIndex] != 0)
    //                {
    //                    // got another one, so start the particle emission counter over again
    //                    particleEmitCounter = 0;
    //                    break;
    //                }
    //                emitterIndex++;
    //            }
    //        }
    //    }
    //}
}

//
//// TODO: header
//// While the first run through the update will reset all particles that are out of bounds, it is possible that some particles will start within bounds but not at the emitter's origin, which will look weird.  This function solves that by giving them initial values.
//void ParticlePolygonComputeUpdater::ResetAllParticles(std::vector<Particle> &particleCollection)
//{
//    // reset all particles evenly 
//    // Note: I could do a weighted fancy algorithm and account for the "particles emitted per frame" for each emitter, but this is just a demo program.
//    // Also Note: This integer division could leave a few particles unaffected, but those will quickly be swept up into the flow of things when "update" runs.
//    unsigned int particlesPerEmitter = particleCollection.size() / (_pointEmitters.size() + _barEmitters.size());
//    unsigned int particlesResetSoFar = 0;
//
//    // point emitters
//    for (size_t emitterIndex = 0; emitterIndex < _pointEmitters.size(); emitterIndex++)
//    {
//        for (size_t particleCounter = 0; particleCounter < particlesPerEmitter; particleCounter++)
//        {
//            unsigned int particleIndex = particlesResetSoFar + particleCounter;
//            _pointEmitters[emitterIndex]->ResetParticle(&particleCollection[particleIndex]);
//            particlesResetSoFar++;
//        }
//    }
//
//    // bar emitters
//    for (size_t emitterIndex = 0; emitterIndex < _barEmitters.size(); emitterIndex++)
//    {
//        for (size_t particleCounter = 0; particleCounter < particlesPerEmitter; particleCounter++)
//        {
//            unsigned int particleIndex = particlesResetSoFar + particleCounter;
//            _barEmitters[emitterIndex]->ResetParticle(&particleCollection[particleIndex]);
//            particlesResetSoFar++;
//        }
//    }
//    //for (size_t emitterIndex = 0; emitterIndex < _emitterCount; emitterIndex++)
//    //{
//    //    for (size_t particleIndex = 0; particleIndex < particlesPerEmitter; particleIndex++)
//    //    {
//    //        _pEmitters[emitterIndex]->ResetParticle(&particleCollection[particleIndex]);
//    //    }
//    //}
//
//}
//
