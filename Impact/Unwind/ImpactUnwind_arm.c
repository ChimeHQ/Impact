//
//  ImpactUnwind_arm.c
//  Impact
//
//  Created by Matt Massicotte on 2019-10-08.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#include "ImpactCompactUnwind.h"
#include "ImpactUtility.h"
#include "ImpactUnwind.h"

#if defined(__arm__)

ImpactResult ImpactCompactUnwindStepArchRegisters(ImpactCompactUnwindTarget target, ImpactCPURegisters* registers, compact_unwind_encoding_t encoding, uint32_t* dwarfFDEOffset) {
    return ImpactResultFailure;
}

#endif
