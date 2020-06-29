//
//  ImpactUnwind_arm64.c
//  Impact
//
//  Created by Matt Massicotte on 2019-10-09.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#include "ImpactCompactUnwind.h"
#include "ImpactUtility.h"
#include "ImpactUnwind.h"
#include "ImpactDWARF.h"

#include <mach-o/compact_unwind_encoding.h>

#if defined(__arm64__)

static const uint32_t ImpactCompactUnwindSavedRegisterPairMask = UNWIND_ARM64_FRAME_X19_X20_PAIR &
    UNWIND_ARM64_FRAME_X21_X22_PAIR &
    UNWIND_ARM64_FRAME_X23_X24_PAIR &
    UNWIND_ARM64_FRAME_X25_X26_PAIR &
    UNWIND_ARM64_FRAME_X27_X28_PAIR &
    UNWIND_ARM64_FRAME_D8_D9_PAIR &
    UNWIND_ARM64_FRAME_D10_D11_PAIR &
    UNWIND_ARM64_FRAME_D12_D13_PAIR &
    UNWIND_ARM64_FRAME_D14_D15_PAIR;

#pragma pack(push, 8)
// the ordering here is flipped to account for the stack direction
typedef struct {
    uintptr_t regB;
    uintptr_t regA;
} ImpactCompactUnwindSavedRegisterPair;
#pragma pack(pop)

static ImpactResult ImpactCompactUnwindRestoreFrameRegisterPair(ImpactCPURegisters* registers, ImpactCompactUnwindSavedRegisterPair pair, ImpactCPURegister firstReg, ImpactCPURegister secondReg) {
    ImpactResult result = ImpactCPUSetRegister(registers, firstReg, pair.regA);
    if (result != ImpactResultSuccess) {
        return result;
    }

    return ImpactCPUSetRegister(registers, secondReg, pair.regB);
}

static ImpactResult ImpactCompactUnwindRestoreSavedRegisters(ImpactCPURegisters* registers, uintptr_t address, compact_unwind_encoding_t encoding) {
    ImpactResult result = ImpactResultFailure;

    // Read the saved registers a pair at a time. In theory, we could read the entire stack area containing saved registers
    // in one shot. This would be a performance optimization for the common case. But, stack corruption or other failures would then
    // prevent us from doing any partial restores, which might end up saving the unwind. Overall, probably best to do it
    // simply and on the slower side.

    const ImpactCompactUnwindSavedRegisterPair* pair = (ImpactCompactUnwindSavedRegisterPair*)address;

    if (encoding & UNWIND_ARM64_FRAME_X19_X20_PAIR) {
        result = ImpactCompactUnwindRestoreFrameRegisterPair(registers, *pair, ImpactCPURegister_ARM64_X19, ImpactCPURegister_ARM64_X20);
        if (result != ImpactResultSuccess) {
            return result;
        }

        pair -= 1;
    }

    if (encoding & UNWIND_ARM64_FRAME_X21_X22_PAIR) {
        result = ImpactCompactUnwindRestoreFrameRegisterPair(registers, *pair, ImpactCPURegister_ARM64_X21, ImpactCPURegister_ARM64_X22);
        if (result != ImpactResultSuccess) {
            return result;
        }

        pair -= 1;
    }

    if (encoding & UNWIND_ARM64_FRAME_X23_X24_PAIR) {
        result = ImpactCompactUnwindRestoreFrameRegisterPair(registers, *pair, ImpactCPURegister_ARM64_X23, ImpactCPURegister_ARM64_X24);
        if (result != ImpactResultSuccess) {
            return result;
        }

        pair -= 1;
    }

    if (encoding & UNWIND_ARM64_FRAME_X25_X26_PAIR) {
        result = ImpactCompactUnwindRestoreFrameRegisterPair(registers, *pair, ImpactCPURegister_ARM64_X25, ImpactCPURegister_ARM64_X26);
        if (result != ImpactResultSuccess) {
            return result;
        }

        pair -= 1;
    }

    if (encoding & UNWIND_ARM64_FRAME_X27_X28_PAIR) {
        result = ImpactCompactUnwindRestoreFrameRegisterPair(registers, *pair, ImpactCPURegister_ARM64_X27, ImpactCPURegister_ARM64_X28);
        if (result != ImpactResultSuccess) {
            return result;
        }

        pair -= 1;
    }

    // floating point registers would happen here, but we don't support that

    return ImpactResultSuccess;
}

static ImpactResult ImpactCompactUnwindStepFrame(ImpactCPURegisters* registers, compact_unwind_encoding_t encoding) {
    uintptr_t savedRegisterLocation = 0;
    ImpactResult result = ImpactCPUGetRegister(registers, ImpactCPURegisterFramePointer, &savedRegisterLocation);
    if (result != ImpactResultSuccess) {
        return result;
    }

    result = ImpactCompactUnwindRestoreSavedRegisters(registers, savedRegisterLocation, encoding);
    if (result != ImpactResultSuccess) {
        return result;
    }

    return ImpactUnwindStepRegistersWithFramePointer(registers);
}

static ImpactResult ImpactCompactUnwindStepFrameless(ImpactCPURegisters* registers, compact_unwind_encoding_t encoding) {
    uintptr_t stackPointer = 0;
    ImpactResult result = ImpactCPUGetRegister(registers, ImpactCPURegisterStackPointer, &stackPointer);
    if (result != ImpactResultSuccess) {
        return result;
    }

    // compute the stack size, and use that to offset into the stack to find where the registers
    // are saved
    const uint32_t stackSize = ((encoding & UNWIND_ARM64_FRAMELESS_STACK_SIZE_MASK) >> 12) * 16;

    const uintptr_t savedRegisterLocation = stackPointer + stackSize;

    result = ImpactCompactUnwindRestoreSavedRegisters(registers, savedRegisterLocation, encoding);
    if (result != ImpactResultSuccess) {
        return result;
    }

    // adjust the stack pointer by counting the number of registers saved, and subtracting that value

    const uint32_t savedRegisterCount = __builtin_popcount(encoding & ImpactCompactUnwindSavedRegisterPairMask) * 2;
    const uintptr_t previousStackPointer = stackPointer - savedRegisterCount * sizeof(uintptr_t);

    result = ImpactCPUSetRegister(registers, ImpactCPURegisterStackPointer, previousStackPointer);
    if (result != ImpactResultSuccess) {
        return result;
    }

    uintptr_t registerValue = 0;

    // move LR -> PC
    result = ImpactCPUGetRegister(registers, ImpactCPURegisterLinkRegister, &registerValue);
    if (result != ImpactResultSuccess) {
        return result;
    }

    result = ImpactCPUSetRegister(registers, ImpactCPURegisterInstructionPointer, registerValue);
    if (result != ImpactResultSuccess) {
        return result;
    }

    return ImpactUnwindStepRegistersWithFramePointer(registers);
}

ImpactResult ImpactCompactUnwindStepArchRegisters(ImpactCompactUnwindTarget target, ImpactCPURegisters* registers, compact_unwind_encoding_t encoding, uint32_t* dwarfFDEOffset) {
    const uint32_t mode = encoding & UNWIND_ARM64_MODE_MASK;
    switch (mode) {
        case UNWIND_ARM64_MODE_FRAME:
            return ImpactCompactUnwindStepFrame(registers, encoding);
        case UNWIND_ARM64_MODE_FRAMELESS:
            return ImpactCompactUnwindStepFrameless(registers, encoding);
        case UNWIND_ARM64_MODE_DWARF: {
            *dwarfFDEOffset = encoding & UNWIND_ARM64_DWARF_SECTION_OFFSET;
            return ImpactResultSuccess;
        }
    }

    return ImpactResultArgumentInvalid;
}

#endif

