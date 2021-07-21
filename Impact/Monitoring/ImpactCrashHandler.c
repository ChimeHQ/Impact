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
#include "ImpactBinaryImage.h"

#include <unistd.h>

ImpactResult ImpactCrashHandler(ImpactState* state, thread_act_t crashedThread, const ImpactCPURegisters* registers) {
    if (ImpactInvalidPtr(state)) {
        return ImpactResultArgumentInvalid;
    }

    ImpactDebugLog("[Log:INFO] entering the crash handler\n");

    ImpactThreadList list = {0};

    ImpactResult result = ImpactThreadListInitialize(&list, crashedThread, registers);
    if (result != ImpactResultSuccess) {
        ImpactDebugLog("[Log:ERROR:%s] unable to initialize thread list %d\n", __func__, result);
        return result;
    }

    result = ImpactThreadListLog(state, &list);
    if (result != ImpactResultSuccess) {
        ImpactDebugLog("[Log:ERROR:%s] unable to log thread list %d\n", __func__, result);
        return result;
    }

    result = ImpactBinaryImageLogRemainingImages(state);
    if (result != ImpactResultSuccess) {
        ImpactDebugLog("[Log:ERROR] unable to log remaining images %d\n", result);
        return result;
    }

    ImpactDebugLog("[Log:INFO] exiting the crash handler\n");

    if (state->constantState.suppressReportCrash) {
        // There are lots of exit functions. Only _exit is listed
        // as safe to call from a signal handler.

        ImpactDebugLog("[Log:WARN:%s] suppressing ReportCrash\n", __func__);

        _exit(0);
        return ImpactResultSuccess;
    }

    return ImpactResultSuccess;
}
