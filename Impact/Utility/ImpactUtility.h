//
//  ImpactUtility.h
//  Impact
//
//  Created by Matt Massicotte on 2019-09-20.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#ifndef ImpactUtility_h
#define ImpactUtility_h

#include "ImpactPointer.h"
#include "ImpactDebug.h"
#include "ImpactResult.h"

#include <stdbool.h>

bool ImpactDebuggerAttached(void);
ImpactResult ImpactReadMemory(uintptr_t address, size_t size, void* buffer);

#endif /* ImpactUtility_h */
