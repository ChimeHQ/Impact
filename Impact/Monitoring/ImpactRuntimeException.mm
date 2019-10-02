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

ImpactResult ImpactRuntimeExceptionInitialize(ImpactState * state) {
    return ImpactResultSuccess;
}

void ImpactRuntimeExceptionLogNSException(NSException* exception) {
    ImpactState* state = GlobalImpactState;

    if (ImpactInvalidPtr(state)) {
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
