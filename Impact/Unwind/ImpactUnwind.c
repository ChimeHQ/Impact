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
#include "ImpactDWARF.h"

#include <ptrauth.h>

// this structure is only intended for use with x86_64. However, it should work in practice for any ABI that follows similar conventions
#pragma pack(push, 8)
typedef struct {
    struct ImpactStackFrame* previous;
    uintptr_t returnAddress;
} ImpactStackFrameEntry;
#pragma pack(pop)

_Static_assert(sizeof(ImpactStackFrameEntry) == (sizeof(void*) * 2), "Frame entries must be exactly two pointers in size");

ImpactResult ImpactUnwindStepRegistersWithFramePointer(ImpactCPURegisters* registers) {
    if (ImpactInvalidPtr(registers)) {
        return ImpactResultPointerInvalid;
    }

    const ImpactStackFrameEntry* frame = NULL;

    ImpactResult result = ImpactCPUGetRegister(registers, ImpactCPURegisterFramePointer, (uintptr_t *)&frame);
    if (result != ImpactResultSuccess) {
        return result;
    }

    if (ImpactInvalidPtr(frame)) {
        ImpactDebugLog("[Log:ERROR] frame pointer invalid %p\n", frame);
        return ImpactResultFailure;
    }

    if (frame->previous == NULL) {
        return ImpactResultEndOfStack;
    }

    result = ImpactCPUSetRegister(registers, ImpactCPURegisterFramePointer, (uintptr_t)frame->previous);
    if (result != ImpactResultSuccess) {
        return result;
    }

    result = ImpactCPUSetRegister(registers, ImpactCPURegisterInstructionPointer, (uintptr_t)frame->returnAddress);
    if (result != ImpactResultSuccess) {
        return result;
    }

    // assumes that stack grows towards lower memory (so adding moves towards the calling function)
    const uintptr_t newSP = (uintptr_t)frame + sizeof(ImpactStackFrameEntry);

    return ImpactCPUSetRegister(registers, ImpactCPURegisterStackPointer, newSP);
}

#if IMPACT_DWARF_CFI_SUPPORTED
static ImpactResult ImpactUnwindDWARFCFIStepRegisters(ImpactMachODataRegion ehFrameRegion, uintptr_t pc, ImpactCPURegisters* registers, uint32_t fdeOffset) {
    if (ImpactInvalidPtr(registers)) {
        return ImpactResultPointerInvalid;
    }

    ImpactDWARFCFIData cfiData = {0};
    const ImpactDWARFEnvironment env = {
        .pointerWidth = sizeof(void*)
    };

    ImpactResult result = ImpactDWARFReadData(ehFrameRegion, env, fdeOffset, &cfiData);
    if (result != ImpactResultSuccess) {
        ImpactDebugLog("[Log:WARN] %s failed to parse CFI data %d\n", __func__, result);
        return result;
    }

    const ImpactDWARFTarget dwarfTarget = {
        .pc = pc,
        .ehFrameRegion = ehFrameRegion,
        .environment = env
    };

    return ImpactDWARFStepRegisters(&cfiData, dwarfTarget, registers);
}
#endif

static ImpactResult ImpactUnwindCompactUnwindStepRegisters(ImpactMachOData* imageData, uintptr_t pc, ImpactCPURegisters* registers, uint32_t* dwarfFDEOFfset) {
    const ImpactCompactUnwindTarget target = {
        .address = pc,
        .imageLoadAddress = imageData->loadAddress,
        .header = (const struct unwind_info_section_header*)imageData->unwindInfoRegion.address
    };

    return ImpactCompactUnwindStepRegisters(target, registers, dwarfFDEOFfset);
}

ImpactResult ImpactUnwindStepRegisters(const ImpactState* state, ImpactCPURegisters* registers) {
    uintptr_t pc = 0;

    ImpactResult result = ImpactCPUGetRegister(registers, ImpactCPURegisterInstructionPointer, &pc);
    if (result != ImpactResultSuccess) {
        return result;
    }

    ImpactMachOData imageData = {0};

    result = ImpactBinaryImageFind(state, pc, &imageData);
    if (result != ImpactResultSuccess) {
        ImpactDebugLog("[Log:WARN] unable to find binary image %d\n", result);

        return result;
    }

    ImpactDebugLog("[Log:INFO] found image at %p\n", (void*)imageData.loadAddress);

    if (pc <= imageData.loadAddress) {
        ImpactDebugLog("[Log:WARN] pc not within image range\n");

        return ImpactUnwindStepRegistersWithFramePointer(registers);
    }

    uint32_t dwarfFDEOFfset = 0;
    result = ImpactUnwindCompactUnwindStepRegisters(&imageData, pc, registers, &dwarfFDEOFfset);
    if (result == ImpactResultEndOfStack) {
        return ImpactResultEndOfStack;
    }

    if (result == ImpactResultMissingUnwindInfo) {
        // this is a weirdly common situation, because some apple libs are missing unwind_info section entries
        return ImpactUnwindStepRegistersWithFramePointer(registers);
    }


    if (result != ImpactResultSuccess) {
        ImpactDebugLog("[Log:WARN] compact unwind failed %d\n", result);

        // fall back to the frame pointer
        return ImpactUnwindStepRegistersWithFramePointer(registers);
    }

    if (dwarfFDEOFfset == 0) {
        // compact unwind has done the step
        return ImpactResultSuccess;
    }

#if IMPACT_DWARF_CFI_SUPPORTED
    ImpactDebugLog("[Log:INFO] using DWARF CFI with FDE offset 0x%x\n", dwarfFDEOFfset);

    result = ImpactUnwindDWARFCFIStepRegisters(imageData.ehFrameRegion, pc, registers, dwarfFDEOFfset);
    if (result == ImpactResultSuccess || result == ImpactResultEndOfStack) {
        return result;
    }

    ImpactDebugLog("[Log:WARN] DWARF CFI unwind failed %d\n", result);
#endif
    
    return ImpactUnwindStepRegistersWithFramePointer(registers);
}
