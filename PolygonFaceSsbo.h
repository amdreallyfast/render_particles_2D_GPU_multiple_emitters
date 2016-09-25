#pragma once

#include "SsboBase.h"
#include "PolygonFace.h"
#include <vector>

// TODO: header
class PolygonFaceSsbo : public SsboBase
{
public:
    PolygonFaceSsbo();
    virtual ~PolygonFaceSsbo();

    void Init(const std::vector<PolygonFace> &faceCollection, unsigned int renderProgramId);
};

