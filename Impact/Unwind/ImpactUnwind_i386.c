//
//  ImpactUnwind_i386.c
//  Impact
//
//  Created by Matt Massicotte on 2019-10-09.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#include "ImpactCompactUnwind.h"
#include "ImpactUtility.h"
#include "ImpactUnwind.h"
#include "ImpactDWARF.h"

#include <mach-o/compact_unwind_encoding.h>

#if defined(__i386__)

ImpactResult ImpactCompactUnwindStepArchRegisters(ImpactCompactUnwindTarget target, ImpactCPURegisters* registers, compact_unwind_encoding_t encoding, bool* finished) {
    return ImpactResultFailure;
}

#endif
