//
//  ImpactSignal.h
//  Impact
//
//  Created by Matt Massicotte on 2019-09-18.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#ifndef ImpactSignal_h
#define ImpactSignal_h

#include "ImpactResult.h"
#include "ImpactState.h"

ImpactResult ImpactSignalInitialize(ImpactState* state);
ImpactResult ImpactSignalUninstallHandlers(const ImpactState* state);

#endif /* ImpactSignal_h */
