//
//  ImpactMachException.h
//  Impact
//
//  Created by Matt Massicotte on 2019-09-19.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#ifndef ImpactMachException_h
#define ImpactMachException_h

#include "ImpactResult.h"
#include "ImpactState.h"

#include <TargetConditionals.h>

#define IMPACT_MACH_EXCEPTION_SUPPORTED  (TARGET_OS_OSX || TARGET_OS_IOS)

#if IMPACT_MACH_EXCEPTION_SUPPORTED

ImpactResult ImpactMachExceptionInitialize(ImpactState* state);

#endif

#endif /* ImpactMachException_h */
