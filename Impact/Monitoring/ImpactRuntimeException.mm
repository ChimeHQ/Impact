//
//  ImpactRuntimeException.c
//  Impact
//
//  Created by Matt Massicotte on 2019-09-30.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#include "ImpactRuntimeException.h"
#include "ImpactLog.h"
#include "ImpactUtility.h"
#include "ImpactSignal.h"

#import <Foundation/Foundation.h>

static void ImpactNSUncaughtExceptionExceptionHandler(NSException *exception) {
    ImpactDebugLog("[Log:INFO] uncaught exception\n");
    ImpactRuntimeExceptionLogNSException(exception);

    ImpactState* state = GlobalImpactState;

    if (ImpactInvalidPtr(state)) {
        return;
    }

    NSUncaughtExceptionHandler* existing = (NSUncaughtExceptionHandler*)state->constantState.preexistingNSExceptionHandler;

    if (ImpactInvalidPtr((void*)existing)) {
        ImpactDebugLog("[Log:WARN] invalid existing NSUncaughtExceptionHandler\n");
        return;
    }

    existing(exception);
}

ImpactResult ImpactRuntimeExceptionInitialize(ImpactState* state) {
    atomic_store(&state->mutableState.exceptionCount, 1);

    state->constantState.preexistingNSExceptionHandler = (void*)NSGetUncaughtExceptionHandler();

    NSSetUncaughtExceptionHandler(ImpactNSUncaughtExceptionExceptionHandler);

    return ImpactResultSuccess;
}

void ImpactRuntimeExceptionLogNSException(NSException* exception) {
    ImpactState* state = GlobalImpactState;

    if (ImpactInvalidPtr(state)) {
        return;
    }

    if (atomic_fetch_add(&state->mutableState.exceptionCount, 1) > 1) {
        ImpactDebugLog("[Log:WARN] subsequent invocation of ImpactRuntimeExceptionLogNSException ignored\n");
        return;
    }

    ImpactLogger* log = &state->constantState.log;

    if (!ImpactLogIsValid(log)) {
        return;
    }

    ImpactLogWriteString(log, "[Exception] ");
    ImpactLogWriteKeyString(log, "type", "objc", false);
    ImpactLogWriteKeyStringObject(log, "name", exception.name, false);
    ImpactLogWriteKeyStringObject(log, "message", exception.reason, true);

    for (NSNumber *address in exception.callStackReturnAddresses) {
        ImpactLogWriteString(log, "[Exception:Frame] ");
        ImpactLogWriteKeyInteger(log, "ip", address.unsignedIntegerValue, true);
    }
}
