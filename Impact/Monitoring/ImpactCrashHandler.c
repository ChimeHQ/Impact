//
//  ImpactCrashHandler.c
//  Impact
//
//  Created by Matt Massicotte on 2019-09-19.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#include "ImpactCrashHandler.h"
#include "ImpactUtility.h"
#include "ImpactThread.h"

#include <unistd.h>

ImpactResult ImpactCrashHandler(ImpactState* state, const ImpactCPURegisters* registers) {
    if (ImpactInvalidPtr(state)) {
        return ImpactResultArgumentInvalid;
    }

    ImpactDebugLog("[Log:%s] entering the crash handler\n", __func__);

    ImpactThreadList list = {0};

    ImpactResult result = ImpactThreadListInitialize(&list);
    if (result != ImpactResultSuccess) {
        ImpactDebugLog("[Log:%s] unable to initialize thread list %d\n", __func__, result);
        return result;
    }

    result = ImpactThreadListLog(state, &list);
    if (result != ImpactResultSuccess) {
        ImpactDebugLog("[Log:%s] unable to log thread list %d\n", __func__, result);
        return result;
    }

    ImpactDebugLog("[Log:%s] exiting the crash handler\n", __func__);

    if (state->constantState.suppressReportCrash) {
        // There are lots of exit functions. Only _exit is listed
        // as safe to call from a signal handler.

        ImpactDebugLog("[Log:%s] suppressing ReportCrash\n", __func__);

        _exit(0);
        return ImpactResultSuccess;
    }

    return ImpactResultSuccess;
}
