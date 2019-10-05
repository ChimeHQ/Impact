//
//  ImpactUnwind.c
//  Impact
//
//  Created by Matt Massicotte on 2019-09-28.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#include "ImpactUnwind.h"
#include "ImpactUtility.h"
#include "ImpactBinaryImage.h"
#include "ImpactCompactUnwind.h"

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

ImpactResult ImpactUnwindStepRegisters(const ImpactState* state, ImpactCPURegisters* registers, bool* finished) {
    uintptr_t pc = 0;

    ImpactResult result = ImpactCPUGetRegister(registers, ImpactCPURegisterInstructionPointer, &pc);
    if (result != ImpactResultSuccess) {
        ImpactDebugLog("[Log:WARN:%s] unable to get PC %d\n", __func__, result);

        return result;
    }

    ImpactMachOData imageData = {0};

    result = ImpactBinaryImageFind(state, pc, &imageData);
    if (result != ImpactResultSuccess) {
        ImpactDebugLog("[Log:WARN:%s] unable to find binary image %d\n", __func__, result);

        return result;
    }

    ImpactDebugLog("[Log:INFO:%s] found image address %p\n", __func__, (void*)imageData.loadAddress);

    ImpactCompactUnwindTarget target = {0};

    if (pc <= imageData.loadAddress) {
        ImpactDebugLog("[Log:WARN:%s] pc not within image range\n", __func__);

        return ImpactUnwindStepRegistersWithFramePointer(registers, finished);
    }

    target.address = pc - imageData.loadAddress;
    target.header = (struct unwind_info_section_header*)imageData.unwindInfoRegion.address;
    target.ehFrameRegion = imageData.ehFrameRegion;

    result = ImpactCompactUnwindStepRegisters(target, registers, finished);
    if (result == ImpactResultSuccess) {
        return ImpactResultSuccess;
    }

    ImpactDebugLog("[Log:WARN:%s] compact unwind failed %d\n", __func__, result);

    return ImpactUnwindStepRegistersWithFramePointer(registers, finished);
}
