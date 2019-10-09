//
//  ImpactBinaryImage.h
//  Impact
//
//  Created by Matt Massicotte on 2019-10-02.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#ifndef ImpactBinaryImage_h
#define ImpactBinaryImage_h

#include "ImpactState.h"
#include "ImpactResult.h"

typedef struct {
    uintptr_t address;
    uintptr_t length;
} ImpactMachODataRegion;

typedef struct {
    const uint8_t* uuid;
    intptr_t slide;
    ImpactMachODataRegion ehFrameRegion;
    ImpactMachODataRegion unwindInfoRegion;
    uintptr_t loadAddress;
    uintptr_t textSize;
} ImpactMachOData;

ImpactResult ImpactBinaryImageInitialize(ImpactState* state);

ImpactResult ImpactBinaryImageFind(const ImpactState* state, uintptr_t address, ImpactMachOData* data);


#endif /* ImpactBinaryImage_h */
