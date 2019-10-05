//
//  ImpactCompactUnwind.h
//  Impact
//
//  Created by Matt Massicotte on 2019-10-03.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#ifndef ImpactCompactUnwind_h
#define ImpactCompactUnwind_h

#include <sys/types.h>
#include <mach-o/compact_unwind_encoding.h>

#include "ImpactResult.h"
#include "ImpactCPU.h"
#include "ImpactBinaryImage.h"

struct unwind_info_section_header;

typedef struct {
    uintptr_t address;
    const struct unwind_info_section_header* header;
    ImpactMachODataRegion ehFrameRegion;
} ImpactCompactUnwindTarget;

ImpactResult ImpactCompactUnwindLookupEncoding(ImpactCompactUnwindTarget target, compact_unwind_encoding_t* encoding);

ImpactResult ImpactCompactUnwindStepRegisters(ImpactCompactUnwindTarget target, ImpactCPURegisters* registers, bool* finished);

ImpactResult ImpactCompactUnwindStepArchRegisters(ImpactCompactUnwindTarget target, ImpactCPURegisters* registers, compact_unwind_encoding_t encoding, bool* finished);


#endif /* ImpactCompactUnwind_h */
