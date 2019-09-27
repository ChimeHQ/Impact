//
//  ImpactState.c
//  Impact
//
//  Created by Matt Massicotte on 2019-09-26.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#include "ImpactState.h"
#include "ImpactDebug.h"

#include <unistd.h>

void ImpactStateTransitionCtx(ImpactState* state, const char* logContext, ImpactCrashState expectedState, ImpactCrashState newState) {
    if (atomic_compare_exchange_strong(&state->mutableState.crashState, &expectedState, newState)) {
        ImpactDebugLog("[Log:%s] transition %d -> %d\n", logContext, expectedState, newState);
        return;
    }

    ImpactDebugLog("[Log:%s] state transition failed %d -> %d\n", logContext, expectedState, newState);

    _exit(1);
}

_Noreturn void ImpactStateInvalidCtx(const char* logContext, ImpactCrashState invalidState) {
    ImpactDebugLog("[Log:%s] state invalid %d\n", logContext, invalidState);

    _exit(1);
}
