//
//  ImpactCompactUnwind.h
//  Impact
//
//  Created by Matt Massicotte on 2019-10-03.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#ifndef ImpactCompactUnwind_h
#define ImpactCompactUnwind_h

#include "ImpactResult.h"
#include "ImpactCPU.h"

#include <sys/types.h>
#include <mach-o/compact_unwind_encoding.h>

#if defined(__x86_64__) || defined(__i386__) || defined(__arm64__)
#define IMPACT_COMPACT_UNWIND_SUPPORTED 1
#else
#define IMPACT_COMPACT_UNWIND_SUPPORTED 0
#endif

typedef struct {
    uintptr_t address;
    uintptr_t imageLoadAddress;
    const struct unwind_info_section_header* header;
} ImpactCompactUnwindTarget;

ImpactResult ImpactCompactUnwindLookupEncoding(ImpactCompactUnwindTarget target, compact_unwind_encoding_t* encoding);

ImpactResult ImpactCompactUnwindStepRegisters(ImpactCompactUnwindTarget target, ImpactCPURegisters* registers, uint32_t* dwarfFDEOffset);
ImpactResult ImpactCompactUnwindStepArchRegisters(ImpactCompactUnwindTarget target, ImpactCPURegisters* registers, compact_unwind_encoding_t encoding, uint32_t* dwarfFDEOffset);


#endif /* ImpactCompactUnwind_h */
