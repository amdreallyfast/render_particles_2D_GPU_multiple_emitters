#include "ParticlePolygonRegion.h"


/*-----------------------------------------------------------------------------------------------
Description:
    Ensures that the object starts object with initialized values.  
Parameters:
    faces   A collection of PolygonFace structures.  This object will keep a copy of the 
            originals and any transformed version.
Returns:    None
Exception:  Safe
Creator:    John Cox (10-10-2016)
-----------------------------------------------------------------------------------------------*/
ParticleRegionPolygon::ParticleRegionPolygon(const std::vector<PolygonFace> faces)
{
    _originalFaces = faces;

    // the transformed variants begin equal to the original points, then diverge after 
    // SetTransform(...) is called
    _transformedFaces = faces;
}

/*-----------------------------------------------------------------------------------------------
Description:
    Why transform this for every emission of every particle when I can do it once before
    particle updating and be done with it for the rest of the frame?
Parameters:
regionTransform     Transform the faces' points and normals with this.
Returns:    None
Exception:  Safe
Creator:    John Cox (10-10-2016)
-----------------------------------------------------------------------------------------------*/
void ParticleRegionPolygon::SetTransform(const glm::mat4 &regionTransform)
{
    for (size_t faceIndex = 0; faceIndex < _originalFaces.size(); faceIndex++)
    {
        PolygonFace faceCopy = _originalFaces[faceIndex];
        faceCopy._start._position = regionTransform * faceCopy._start._position;
        faceCopy._start._normal = regionTransform * faceCopy._start._normal;
        faceCopy._end._position = regionTransform * faceCopy._end._position;
        faceCopy._end._normal = regionTransform * faceCopy._end._normal;

        _transformedFaces[faceIndex] = faceCopy;
    }
}

// TODO: header
const std::vector<PolygonFace> &ParticleRegionPolygon::GetFaces() const
{
    return _transformedFaces;
}