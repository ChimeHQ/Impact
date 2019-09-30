//
//  ImpactUnwind.c
//  Impact
//
//  Created by Matt Massicotte on 2019-09-28.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#include "ImpactUnwind.h"
#include "ImpactUtility.h"

ImpactResult ImpactUnwindStepRegistersWithFramePointer(ImpactCPURegisters* registers, bool* finished) {
    if (ImpactInvalidPtr(registers) || ImpactInvalidPtr(finished)) {
        return ImpactResultPointerInvalid;
    }

    uintptr_t* fp = 0;
    *finished = true;

    ImpactResult result = ImpactCPUGetRegister(registers, ImpactCPURegisterFramePointer, (uintptr_t *)&fp);
    if (result != ImpactResultSuccess) {
        ImpactDebugLog("[Log:Error] %s failed to get frame pointer %d\n", __func__, result);
        return result;
    }

    if (ImpactInvalidPtr(fp)) {
        ImpactDebugLog("[Log:Error] %s current frame pointer invalid %p\n", __func__, fp);
        return ImpactResultFailure;
    }

    if (fp[0] == 0) {
        // we've reached the end of the stack
        return ImpactResultSuccess;
    }

    if (ImpactInvalidPtr((void*)fp[0])) {
        ImpactDebugLog("[Log:Error] %s new frame pointer invalid %lx\n", __func__, fp[0]);
        return ImpactResultFailure;
    }

    result = ImpactCPUSetRegister(registers, ImpactCPURegisterFramePointer, fp[0]);
    if (result != ImpactResultSuccess) {
        ImpactDebugLog("[Log:Error] %s unable to set fp %lx\n", __func__, fp[0]);
        return result;
    }

    result = ImpactCPUSetRegister(registers, ImpactCPURegisterInstructionPointer, fp[1]);
    if (result != ImpactResultSuccess) {
        return result;
    }

    result = ImpactCPUSetRegister(registers, ImpactCPURegisterStackPointer, (uintptr_t)fp + 2);
    if (result != ImpactResultSuccess) {
        return result;
    }

    *finished = false;

    return ImpactResultSuccess;
}

ImpactResult ImpactUnwindStepRegisters(ImpactCPURegisters* registers, bool* finished) {
    return ImpactUnwindStepRegistersWithFramePointer(registers, finished);
}
