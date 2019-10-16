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
#include "ImpactCPU.h"

#include <mach-o/compact_unwind_encoding.h>

#define UNWIND_X86_64_RBP_FRAME_REG_MASK 0x7

#if defined(__x86_64__)

static const uint32_t ImpactCompactUnwindRBPRegisterCount = 5;

static ImpactResult ImpactCompactUnwindUpdateRBPFrameRegister(ImpactCPURegisters* registers, uint32_t identifier, uintptr_t address) {
    if (identifier == UNWIND_X86_64_REG_NONE) {
        return ImpactResultSuccess;
    }

    uintptr_t value = 0;

    ImpactResult result = ImpactReadMemory(address, sizeof(uintptr_t), &value);
    if (result != ImpactResultSuccess) {
        return result;
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

static ImpactResult ImpactCompactUnwindRestoreRBPFrameRegisters(ImpactCPURegisters* registers, compact_unwind_encoding_t encoding) {
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

    return ImpactResultSuccess;
}

static ImpactResult ImpactCompactUnwindStepRBPFrame(ImpactCPURegisters* registers, compact_unwind_encoding_t encoding) {
    ImpactResult result = ImpactCompactUnwindRestoreRBPFrameRegisters(registers, encoding);
    if (result != ImpactResultSuccess) {
        return result;
    }

    return ImpactUnwindStepRegistersWithFramePointer(registers);
}

ImpactResult ImpactCompactUnwindStepArchRegisters(ImpactCompactUnwindTarget target, ImpactCPURegisters* registers, compact_unwind_encoding_t encoding, uint32_t* dwarfFDEOffset) {
    const uint32_t mode = encoding & UNWIND_X86_64_MODE_MASK;
    switch (mode) {
        case UNWIND_X86_64_MODE_RBP_FRAME:
            return ImpactCompactUnwindStepRBPFrame(registers, encoding);
        case UNWIND_X86_64_MODE_STACK_IMMD:
            return ImpactResultUnimplemented;
        case UNWIND_X86_64_MODE_STACK_IND:
            return ImpactResultUnimplemented;
        case UNWIND_X86_64_MODE_DWARF: {
            *dwarfFDEOffset = encoding & UNWIND_X86_64_DWARF_SECTION_OFFSET;
            return ImpactResultSuccess;
        }
    }

    return ImpactResultArgumentInvalid;
}

#endif
