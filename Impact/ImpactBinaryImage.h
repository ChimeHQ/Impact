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

__BEGIN_DECLS

typedef struct {
    uintptr_t address;
    intptr_t loadAddress;
    uintptr_t length;
} ImpactMachODataRegion;

typedef struct {
    const uint8_t* uuid;
    intptr_t slide;
    ImpactMachODataRegion ehFrameRegion;
    ImpactMachODataRegion unwindInfoRegion;
    uintptr_t loadAddress;
    uintptr_t textSize;
    const char* path;
} ImpactMachOData;

#if __LP64__
typedef struct mach_header_64 ImpactMachOHeader;
typedef struct section_64 ImpactMachOSection;
typedef struct segment_command_64 ImpactSegmentCommand;
typedef struct section_64 ImpactSection;

#define Impact_LC_SEGMENT LC_SEGMENT_64
#else
typedef struct mach_header ImpactMachOHeader;
typedef struct section ImpactMachOSection;
typedef struct segment_command ImpactSegmentCommand;
typedef struct section ImpactSection;

#define Impact_LC_SEGMENT LC_SEGMENT
#endif

ImpactResult ImpactBinaryImageInitialize(ImpactState* state);

ImpactResult ImpactBinaryImageGetData(const ImpactMachOHeader* header, const char* path, ImpactMachOData* data);

ImpactResult ImpactBinaryImageFind(ImpactState* state, uintptr_t address, ImpactMachOData* data);

ImpactResult ImpactBinaryImageLogRemainingImages(ImpactState* state);

__END_DECLS

#endif /* ImpactBinaryImage_h */
