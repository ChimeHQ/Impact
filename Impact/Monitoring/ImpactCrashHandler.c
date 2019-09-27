//
//  ImpactCrashHandler.c
//  Impact
//
//  Created by Matt Massicotte on 2019-09-19.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#include "ImpactCrashHandler.h"
#include "ImpactPointer.h"

#include <unistd.h>

ImpactResult ImpactCrashHandler(ImpactState* state, const ImpactCPURegisters* registers) {
    if (ImpactInvalidPtr(state) || ImpactInvalidPtr(registers)) {
        return ImpactResultArgumentInvalid;
    }

    // do stuff?

    if (state->constantState.suppressReportCrash) {
        // There are lots of exit functions. Only _exit is listed
        // as safe to call from a signal handler.
        _exit(0);
        return ImpactResultSuccess;
    }

    return ImpactResultSuccess;
}
