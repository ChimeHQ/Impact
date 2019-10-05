//
//  ImpactUnwind_x86_64.c
//  Impact
//
//  Created by Matt Massicotte on 2019-10-03.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#include "ImpactCompactUnwind.h"
#include "ImpactUtility.h"
#include "ImpactUnwind.h"
#include "ImpactDWARF.h"

#include <mach-o/compact_unwind_encoding.h>

#define UNWIND_X86_64_RBP_FRAME_REG_MASK 0x7

#if defined(__x86_64__)

static const uint32_t ImpactCompactUnwindRBPRegisterCount = 5;

static ImpactResult ImpactCompactUnwindUpdateRBPFrameRegister(ImpactCPURegisters* registers, uint32_t identifier, uintptr_t address) {
    if (identifier == UNWIND_X86_64_REG_NONE) {
        return ImpactResultSuccess;
    }

    uintptr_t value = 0;

    if (!ImpactReadMemory(address, &value)) {
        return ImpactResultMemoryReadFailed;
    }

    switch (identifier) {
        case UNWIND_X86_64_REG_RBX:
            registers->__ss.__rbx = value;
            break;
        case UNWIND_X86_64_REG_R12:
            registers->__ss.__r12 = value;
            break;
        case UNWIND_X86_64_REG_R13:
            registers->__ss.__r13 = value;
            break;
        case UNWIND_X86_64_REG_R14:
            registers->__ss.__r14 = value;
            break;
        case UNWIND_X86_64_REG_R15:
            registers->__ss.__r15 = value;
            break;
        case UNWIND_X86_64_REG_RBP:
            registers->__ss.__rbp = value;
            break;
        default:
            return ImpactResultArgumentInvalid;
    }

    return ImpactResultSuccess;
}

static ImpactResult ImpactCompactUnwindStepRBPFrame(ImpactCPURegisters* registers, compact_unwind_encoding_t encoding, bool* finished) {
    const uint32_t registersOffset = (encoding & UNWIND_X86_64_RBP_FRAME_OFFSET) >> 16;
    uint32_t registerIdenfifiers = encoding & UNWIND_X86_64_RBP_FRAME_REGISTERS;

    uintptr_t registerEntry = registers->__ss.__rbp - sizeof(uintptr_t) * registersOffset;

    for (uint32_t i = 0; i < ImpactCompactUnwindRBPRegisterCount; ++i) {
        const uint32_t regIdentifier = registerIdenfifiers & UNWIND_X86_64_RBP_FRAME_REG_MASK;

        ImpactResult updateResult = ImpactCompactUnwindUpdateRBPFrameRegister(registers, regIdentifier, registerEntry);
        if (updateResult != ImpactResultSuccess) {
            return updateResult;
        }

        // look at the next identifier entry and corresponding stack address
        registerIdenfifiers = registerIdenfifiers >> 3;
        registerEntry += sizeof(uintptr_t);
    }

    ImpactResult frameResult = ImpactUnwindStepRegistersWithFramePointer(registers, finished);
    if (frameResult != ImpactResultSuccess) {
        ImpactDebugLog("[Log:WARN:%s] frame pointer unwind failed %x\n", __func__, frameResult);

        return ImpactResultFailure;
    }

    return ImpactResultSuccess;
}

static ImpactResult ImpactCompactUnwindStepDWARFFrame(ImpactCompactUnwindTarget target, ImpactCPURegisters* registers, compact_unwind_encoding_t encoding) {
    if (ImpactInvalidPtr(registers)) {
        return ImpactResultPointerInvalid;
    }

    const uint32_t fdeOffset = encoding & UNWIND_X86_64_DWARF_SECTION_OFFSET;

    ImpactDebugLog("[Log:INFO:%s] looking up FDE with offset %x\n", __func__, fdeOffset);

    ImpactDWARFCFIData cfiData = {0};

    ImpactResult result = ImpactDWARFReadData(target.ehFrameRegion, fdeOffset, &cfiData);
    if (result != ImpactResultSuccess) {
        ImpactDebugLog("[Log:WARN:%s] failed to parse CFI data %d\n", __func__, result);
        return result;
    }

    ImpactDWARFCFIState state = {0};

    result = ImpactDWARFRunInstructions(&cfiData, &state);
    if (result != ImpactResultSuccess) {
        ImpactDebugLog("[Log:%s] failed to run CFI instructions %d\n", __func__, result);
        return result;
    }

    uintptr_t cfaValue = 0;

    result = ImpactDWARFGetCFAValue(&state, registers, &cfaValue);
    if (result != ImpactResultSuccess) {
        ImpactDebugLog("[Log:%s] failed to get CFA %d\n", __func__, result);
        return result;
    }

    return ImpactResultUnimplemented;

}

ImpactResult ImpactCompactUnwindStepArchRegisters(ImpactCompactUnwindTarget target, ImpactCPURegisters* registers, compact_unwind_encoding_t encoding, bool* finished) {
    const uint32_t mode = encoding & UNWIND_X86_64_MODE_MASK;
    switch (mode) {
        case UNWIND_X86_64_MODE_RBP_FRAME:
            return ImpactCompactUnwindStepRBPFrame(registers, encoding, finished);
        case UNWIND_X86_64_MODE_STACK_IMMD:
            return ImpactResultUnimplemented;
        case UNWIND_X86_64_MODE_STACK_IND:
            return ImpactResultUnimplemented;
        case UNWIND_X86_64_MODE_DWARF:
            return ImpactCompactUnwindStepDWARFFrame(target, registers, encoding);
    }

    return ImpactResultArgumentInvalid;
}

#endif
