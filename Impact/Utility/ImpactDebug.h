//
//  ImpactDebug.h
//  Impact
//
//  Created by Matt Massicotte on 2019-09-20.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#ifndef ImpactDebug_h
#define ImpactDebug_h

#include "ImpactState.h"

#include <stdio.h>

#if 1

#define ImpactDebugLog(format, ...) do { \
dprintf(GlobalImpactState->constantState.log.fd, format, __VA_ARGS__); \
} while(0)

#else

#define ImpactDebugLog(format, ...) ImpactLog(format, __VA_ARGS__)

#endif

#endif /* ImpactDebug_h */
