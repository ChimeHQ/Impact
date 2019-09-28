//
//  ImpactCrashHandler.h
//  Impact
//
//  Created by Matt Massicotte on 2019-09-19.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#ifndef CrashHandler_h
#define CrashHandler_h

#include "ImpactResult.h"
#include "ImpactCPU.h"
#include "ImpactState.h"

ImpactResult ImpactCrashHandler(ImpactState* state, thread_act_t crashedThread, const ImpactCPURegisters* registers);

#endif /* CrashHandler_h */
