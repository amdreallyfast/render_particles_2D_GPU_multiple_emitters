#include "ParticleUpdater.h"

#include "ShaderStorage.h"
#include "ParticleRegionCircle.h"
#include "ParticleRegionPolygon.h"

/*-----------------------------------------------------------------------------------------------
Description:
    Ensures that the object starts object with initialized values.
Parameters: None
Returns:    None
Exception:  Safe
Creator:    John Cox (7-4-2016)
-----------------------------------------------------------------------------------------------*/
ParticleUpdater::ParticleUpdater() //:
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
void ParticleUpdater::Init(const std::string &computeShaderKey)
{
    _computeShaderKey = computeShaderKey;
    
    ShaderStorage &shaderStorageRef = ShaderStorage::GetInstance();
    GLuint computeProgramId = shaderStorageRef.GetShaderProgram(_computeShaderKey);

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

    // generate the storage 
}

/*-----------------------------------------------------------------------------------------------
Description:
    A simple assignment. 
Parameters: 
    pRegion     A pointer to a "particle region" interface.
Returns:    None
Exception:  Safe
Creator:    John Cox (7-4-2016)
-----------------------------------------------------------------------------------------------*/
void ParticleUpdater::SetRegion(const IParticleRegion *pRegion)
{
    //_pRegion = pRegion;
    if (dynamic_cast<const ParticleRegionCircle *>(pRegion) == 0)
    {

    }
    else if (dynamic_cast<const ParticleRegionPolygon *>(pRegion) == 0)
    {

    }
    else
    {
        // bad particle region (??throw a fit??)
    }
}

/*-----------------------------------------------------------------------------------------------
Description:
    A simple assignment.
Parameters: 
    pEmitter    A pointer to a "particle emitter" interface.
    maxParticlesEmittedPerFrame     A restriction on the provided emitter to prevent all 
        particles from being emitted at once.
Returns:    None
Exception:  Safe
Creator:    John Cox (7-4-2016)
-----------------------------------------------------------------------------------------------*/
void ParticleUpdater::AddEmitter(const IParticleEmitter *pEmitter, const int maxParticlesEmittedPerFrame)
{
    if (_emitterCount >= MAX_EMITTERS)
    {
        return;
    }

    _pEmitters[_emitterCount] = pEmitter;
    _maxParticlesEmittedPerFrame[_emitterCount] = maxParticlesEmittedPerFrame;
    _emitterCount++;
}

/*-----------------------------------------------------------------------------------------------
Description:
    Checks if each particle is out of bounds, and if so, tells the emitter to reset it.  If the 
    emitter hasn't reached its quota for emitted particles, then the particle is sent back out 
    again.  Lastly, if the particle is active, then its position is updated with its velocity and
    the provided delta time.
Parameters:
    particleCollection  The particle collection that will be updated.
    startIndex          Used in case the user wanted to adapt the updater to use multiple 
                        emitters and then wanted to split the number of particles between these 
                        emitters.
    numToUpdate         Same idea as "start index".
    deltatimeSec        Self-explanatory
Returns:    None
Exception:  Safe
Creator:    John Cox (7-4-2016)
-----------------------------------------------------------------------------------------------*/
void ParticleUpdater::Update(std::vector<Particle> &particleCollection, 
    const unsigned int startIndex, const unsigned int numToUpdate, const float deltaTimeSec) const
{
    if (_emitterCount == 0 || _pRegion == 0)
    {
        return;
    }

    // for all particles:
    // - if it has gone out of bounds, reset it and deactivate it
    // - if it is inactive and the emitter hasn't used up its quota for emitted particles this frame, reactivate it
    // - if it is active, update its position with its velocity
    // Note: If if() statements are used for each situation, then a particle has a chance to go 
    // out of bounds and get reset, get reactivated, and emit again in the same frame.  If 
    // else-if() statements are used, then only one of those situations will be run per frame.  
    // I did the former, but it doesn't really matter which approach is chosen.

    // simply called "end" because I want to keep using the "< end" notation on the loop end 
    // condition
    unsigned int end = startIndex + numToUpdate;
    if (end > particleCollection.size())
    {
        // if "end" was already == particle collection size, then all is good
        end = particleCollection.size();
    }

    // when using multiple emitters, it looks best to cycle between all emitters one by one, but that is also more difficult to deal with and requires a number of different checks and conditions, and provided the total number of particles to update exceeds the total number of max particles emitted per frame across all emitters, then it will look just as good to "fill up" each emitter one by one, and that is much easier to implement
    unsigned int particleEmitCounter = 0;
    int emitterIndex = 0;

    //for (size_t particleIndex = 0; particleIndex < particleCollection.size(); particleIndex++)
    for (size_t particleIndex = startIndex; particleIndex < end; particleIndex++)
    {
        Particle &pCopy = particleCollection[particleIndex];
        if (_pRegion->OutOfBounds(pCopy))
        {
            pCopy._isActive = false;
        }

        if (pCopy._isActive)
        {
            pCopy._position = pCopy._position + (pCopy._velocity * deltaTimeSec);
        }
        else if (emitterIndex < MAX_EMITTERS)   // also implicitly, "is active" is false
        {
            // if all emitters have put out all they can this frame, then this condition will 
            // not be entered
            _pEmitters[emitterIndex]->ResetParticle(&pCopy);
            pCopy._isActive = true;
            particleCollection[particleIndex] = pCopy;

            particleEmitCounter++;
            if (particleEmitCounter >= _maxParticlesEmittedPerFrame[emitterIndex])
            {
                // get the next emitter
                // Note: This is in a while loop because the number of emitters in this particle updater are unknown and some of the pointers in the emitter array may still be 0, so this has to loop through (potentially) all of them.
                emitterIndex++;
                while (emitterIndex < MAX_EMITTERS)
                {
                    if (_pEmitters[emitterIndex] != 0)
                    {
                        // got another one, so start the particle emission counter over again
                        particleEmitCounter = 0;
                        break;
                    }
                    emitterIndex++;
                }
            }
        }
    }
}

void ParticleUpdater::ResetAllParticles(std::vector<Particle> &particleCollection)
{
    // reset all particles evenly 
    // Note: I could do a weighted fancy algorithm and account for the "particles emitted per frame" for each emitter, but this is just a demo program.
    // Also Note: This integer division could leave a few particles unaffected, but those will quickly be swept up into the flow of things when "update" runs.
    unsigned int particlesPerEmitter = particleCollection.size() / _emitterCount;
    for (size_t emitterIndex = 0; emitterIndex < _emitterCount; emitterIndex++)
    {
        for (size_t particleIndex = 0; particleIndex < particlesPerEmitter; particleIndex++)
        {
            _pEmitters[emitterIndex]->ResetParticle(&particleCollection[particleIndex]);
        }
    }

}

