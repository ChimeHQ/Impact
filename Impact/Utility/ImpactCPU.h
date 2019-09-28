//
//  ImpactCPU.h
//  Impact
//
//  Created by Matt Massicotte on 2019-09-19.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#ifndef ImpactCPU_h
#define ImpactCPU_h

#include <sys/types.h>

#include "ImpactResult.h"
#include "ImpactState.h"

typedef _STRUCT_MCONTEXT ImpactCPURegisters;

ImpactResult ImpactCPURegistersLog(ImpactState* state, const ImpactCPURegisters* registers);

#endif /* ImpactCPU_h */
