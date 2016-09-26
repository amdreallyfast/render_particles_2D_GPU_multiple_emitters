#include "ParticleEmitterBar.h"

#include "RandomToast.h"

/*-----------------------------------------------------------------------------------------------
Description:
    Ensures that the object starts object with initialized values.
Parameters:
    p1      The bar's origin point in window space (XY on range [-1,+1]).
    p2      The bar's end point.
    emitDir     Particles will be launched in this direction evenly along the bar.
    minVel      The minimum velocity for particles being emitted.
    maxVel      The maximum emission velocity.
Returns:    None
Creator:    John Cox (7-2-2016)
-----------------------------------------------------------------------------------------------*/
ParticleEmitterBar::ParticleEmitterBar(const glm::vec2 &p1, const glm::vec2 &p2, 
    const glm::vec2 &emitDir, float minVel, const float maxVel)
{
    // the start and end points should be translatable
    _start = glm::vec4(p1, 0.0f, 1.0f);
    _end = glm::vec4(p2, 0.0f, 1.0f);
    _velocityCalculator.SetMinMaxVelocity(minVel, maxVel);

    // emission direction should not be translatable; like a normal, it should only be rotatable
    _emitDir = glm::vec4(emitDir, 0.0f, 0.0f);
    _velocityCalculator.SetDir(emitDir);
}

/*-----------------------------------------------------------------------------------------------
Description:
    Sets the given particle's starting position and velocity.  The position is set to a random 
    point along the var.
    
    Does NOT alter the "is active" flag.  That flag is altered only by the "particle updater" 
    object.
Parameters:
resetThis   Self-explanatory.
Returns:    None
Creator:    John Cox (7-2-2016)
-----------------------------------------------------------------------------------------------*/
void ParticleEmitterBar::ResetParticle(Particle *resetThis) const
{
    // give it some flavor by making the particles be reset to within a range near the emitter 
    // bar's position instead of exactly on the bar, making it look like a particle hotspot
    glm::vec4 startToEnd = _end - _start;
    resetThis->_position = _start + (RandomOnRange0to1() * startToEnd);
    resetThis->_velocity = _velocityCalculator.GetNew();
}

/*-----------------------------------------------------------------------------------------------
Description:
    A simple getter for the start vertex of the bar. ??use??
Parameters: None
Returns:    
    A vec4.
Creator:    John Cox (9-20-2016)
-----------------------------------------------------------------------------------------------*/
glm::vec4 ParticleEmitterBar::GetBarStart() const
{
    return _start;
}

/*-----------------------------------------------------------------------------------------------
Description:
    A simple getter for the end vertex of the bar.
Parameters: None
Returns:
    A vec4.
Creator:    John Cox (9-20-2016)
-----------------------------------------------------------------------------------------------*/
glm::vec4 ParticleEmitterBar::GetBarEnd() const
{
    return _end;
}

/*-----------------------------------------------------------------------------------------------
Description:
    A simple getter for the emision direction.
Parameters: None
Returns:
    A vec4.
Creator:    John Cox (9-20-2016)
-----------------------------------------------------------------------------------------------*/
glm::vec4 ParticleEmitterBar::GetEmitDir() const
{
    return _emitDir;
}

///*-----------------------------------------------------------------------------------------------
//Description:
//    Applies the transform to the emission direction and to the points that make up the bar.  The 
//    emission direction is only rotated, while the bar points are rotated and translated.
//Parameters:
//    m       A 4x4 transform matrix.  Because glm transform functions only spit out a 4x4.
//Returns:    None
//Exception:  Safe
//Creator:    John Cox (8-26-2016)
//-----------------------------------------------------------------------------------------------*/
//void ParticleEmitterBar::SetTransform(const glm::mat4 &m)
//{
//    _currentBarStart = glm::vec2(m * glm::vec4(_originalBarStart, 0.0f, 1.0f));
//
//    // "bar start to end" is relative to "bar start", so it can be rotated but not translated
//    _currentBarStartToEnd = glm::vec2(m * glm::vec4(_originalBarStartToEnd, 0.0f, 0.0f));
//
//    // ditto for emission direction because it is relative to the bar
//    glm::vec2 newEmissionDir = glm::vec2(m * glm::vec4(_originalEmitDirection, 0.0f, 0.0f));
//    _velocityCalculator.SetDir(newEmissionDir);
//}
