//
//  ImpactUnwind.h
//  Impact
//
//  Created by Matt Massicotte on 2019-09-28.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#ifndef ImpactUnwind_h
#define ImpactUnwind_h

#include "ImpactResult.h"
#include "ImpactCPU.h"
#include "ImpactState.h"

ImpactResult ImpactUnwindStepRegistersWithFramePointer(ImpactCPURegisters* registers);
ImpactResult ImpactUnwindStepRegisters(ImpactState* state, ImpactCPURegisters* registers);

#endif /* ImpactUnwind_h */
