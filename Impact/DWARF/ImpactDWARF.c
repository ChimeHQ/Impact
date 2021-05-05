//
//  ImpactDWARF.c
//  Impact
//
//  Created by Matt Massicotte on 2019-05-06.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#include "ImpactDWARF.h"
#include "ImpactDWARFParser.h"
#include "ImpactDWARFCFIInstructions.h"
#include "ImpactDataCursor.h"
#include "ImpactDWARFDefines.h"
#include "ImpactUtility.h"

#if IMPACT_DWARF_CFI_SUPPORTED

ImpactResult ImpactDWARFRunCFIInstructions(const ImpactDWARFCFIInstructions* instructions, const ImpactDWARFCIE* cie, uintptr_t pcOffset, ImpactDWARFCFIState* state) {
    ImpactDataCursor cursor = {0};

    ImpactResult result = ImpactDataCursorInitialize(&cursor, (uintptr_t)instructions->data, instructions->length, 0);
    if (result != ImpactResultSuccess) {
        ImpactDebugLog("[Log:WARN] %s failed to initialize data cursor %d\n", __func__, result);
        return result;
    }

    uint32_t location = 0;

    // ignore the location for CIE instructions, because those are run for all
    // functions, regardless of our relative position within the function
    const bool considerLocation = &cie->instructions != instructions;

    ImpactDebugLog("[Log:INFO] running CFI instructions with offset 0x%lx\n", pcOffset);

    while (!ImpactDataCursorAtEnd(&cursor)) {
        if (considerLocation && (location >= pcOffset)) {
            // we have advanced past the last location described by this entry
            break;
        }

        uint8_t opcode = 0;

        result = ImpactDataCursorReadUint8(&cursor, &opcode);
        if (result != ImpactResultSuccess) {
            ImpactDebugLog("[Log:WARN] failed to read CFI opcode %d\n", result);
            return result;
        }

        result = ImpactResultFailure;
        const uint8_t operand = opcode & 0x3F;

        switch (opcode) {
        case DW_CFA_nop:
            ImpactDebugLog("[Log:INFO] DW_CFA_nop\n");
            result = ImpactResultSuccess;
            break;
        case DW_CFA_def_cfa:
            result = ImpactDWARFRun_DW_CFA_def_cfa(&cursor, state);
            break;
        case DW_CFA_def_cfa_offset:
            result = ImpactDWARFRun_DW_CFA_def_cfa_offset(&cursor, state);
            break;
        default:
            switch (opcode & 0xC0) {
                case DW_CFA_offset:
                    result = ImpactDWARFRun_DW_CFA_offset(&cursor, operand, cie, state);
                    break;
                case DW_CFA_advance_loc:
                    result = ImpactDWARFRun_DW_CFA_advance_loc(&cursor, operand, cie, state, &location);
                    break;
                default:
                    ImpactDebugLog("[Log:WARN] %s unhandled opcode %x\n", __func__, opcode);
                    return ImpactResultFailure;
            }
                break;
        }

        // Looking through all of /usr/lib on a number of macOS versions, I've only found these other opcodes in use
        // DW_CFA_def_cfa_register
        // DW_CFA_advance_loc1
        // DW_CFA_advance_loc4
        // DW_CFA_remember_state
        // DW_CFA_restore
        // DW_CFA_restore_state
        //
        // libc++abi.dylib contains DWARF instructions for ARM64. Interestingly, the only time DW_CFA_register
        // has come up so for is in the hand-written CFI code in CrashProbe.

        if (result != ImpactResultSuccess) {
            return result;
        }
    }
    
    return ImpactResultSuccess;
}

ImpactResult ImpactDWARFRunInstructions(const ImpactDWARFCFIData* data, ImpactDWARFTarget target, ImpactDWARFCFIState* state) {
    // We need to run all CIE instructions, regardless of PC. So, pass in zero here.
    ImpactResult result = ImpactDWARFRunCFIInstructions(&data->cie.instructions, &data->cie, 0, state);
    if (result != ImpactResultSuccess) {
        return result;
    }

    // Now, this FDE can cover a range that extends past the current PC. So, compute
    // limit.
    uint64_t targetAddress = 0;

    result = ImpactDWARFResolveEncodedPointer(data->cie.augmentationData.pointerEncoding, data->fde.target_address, &targetAddress);
    if (result != ImpactResultSuccess) {
        return result;
    }

    ImpactDebugLog("[Log:INFO] pc=%lx target=%llx\n", target.pc, targetAddress);

    const uintptr_t pcOffset = target.pc - targetAddress;

    result = ImpactDWARFRunCFIInstructions(&data->fde.instructions, &data->cie, pcOffset, state);
    if (result != ImpactResultSuccess) {
        return result;
    }

    return ImpactResultSuccess;
}

ImpactResult ImpactDWARFReadData(ImpactMachODataRegion cfiRegion, ImpactDWARFEnvironment env, uint32_t offset, ImpactDWARFCFIData* data) {
    ImpactDataCursor cursor = {0};

    ImpactResult result = ImpactDataCursorInitialize(&cursor, cfiRegion.address, cfiRegion.length, offset);
    if (result != ImpactResultSuccess) {
        ImpactDebugLog("[Log:WARN] %s failed to initialize data cursor %d\n", __func__, result);
        return result;
    }

    result = ImpactDWARFReadCFI(&cursor, env, data);
    if (result != ImpactResultSuccess) {
        ImpactDebugLog("[Log:WARN] %s parsing CFI failed %d\n", __func__, result);
        return result;
    }

    return ImpactResultSuccess;
}

ImpactResult ImpactDWARFResolveEncodedPointer(uint8_t encoding, uint64_t value, uint64_t *resolvedValue) {
    if (ImpactInvalidPtr(resolvedValue)) {
        return ImpactResultPointerInvalid;
    }

    if (encoding & DW_EH_PE_indirect) {
        // read the pointer here
        return ImpactResultUnimplemented;
    }

    *resolvedValue = value;

    return ImpactResultSuccess;
}

ImpactResult ImpactDWARFGetCFAValue(const ImpactDWARFCFIState* state, const ImpactCPURegisters* registers, uintptr_t *value) {
    if (ImpactInvalidPtr(state) || ImpactInvalidPtr(registers) || ImpactInvalidPtr(value)) {
        return ImpactResultPointerInvalid;
    }

    switch (state->cfaDefinition.rule) {
        case ImpactDWARFCFADefinitionRuleRegisterOffset: {
            const ImpactCPURegister regNum = state->cfaDefinition.registerNum;

            ImpactResult result = ImpactCPUGetRegister(registers, regNum, value);
            if (result != ImpactResultSuccess) {
                return result;
            }

            *value += state->cfaDefinition.value;

            return ImpactResultSuccess;
        }
        default:
            break;

    }

    ImpactDebugLog("[Log:WARN] unsupported CFA definition %d \n", state->cfaDefinition.rule);

    return ImpactResultUnimplemented;
}

ImpactResult ImpactDWARFGetRegisterValue(const ImpactDWARFCFIState* state, uintptr_t cfa, ImpactDWARFRegister dwarfRegister, uintptr_t* value) {
    if (ImpactInvalidPtr(state) || ImpactInvalidPtr(value)) {
        ImpactDebugLog("[Log:WARN] %s pointer argument invalid\n", __func__);
        return ImpactResultPointerInvalid;
    }

    switch (dwarfRegister.rule) {
        case ImpactDWARFCFIRegisterRuleOffsetFromCFA: {
            const uintptr_t addr = cfa + dwarfRegister.value;

            return ImpactReadMemory(addr, sizeof(void*), value);
        }
        default:
            ImpactDebugLog("[Log:WARN] unsupported DWARF register rule %d\n", dwarfRegister.rule);
            break;
    }

    return ImpactResultUnimplemented;
}

ImpactResult ImpactDWARFStepRegisters(const ImpactDWARFCFIData* cfiData, ImpactDWARFTarget target, ImpactCPURegisters* registers) {
    if (ImpactInvalidPtr(cfiData) || ImpactInvalidPtr(registers)) {
        return ImpactResultPointerInvalid;
    }

    ImpactDWARFCFIState state = {0};

    ImpactResult result = ImpactDWARFRunInstructions(cfiData, target, &state);
    if (result != ImpactResultSuccess) {
        return result;
    }

    uintptr_t cfaValue = 0;

    result = ImpactDWARFGetCFAValue(&state, registers, &cfaValue);
    if (result != ImpactResultSuccess) {
        ImpactDebugLog("[Log:WARN] %s failed to get CFA %d\n", __func__, result);
        return result;
    }

    ImpactDebugLog("[Log:INFO] CFA=%lx, RA=%lld\n", cfaValue, cfiData->cie.return_address_register);
    
    if (ImpactInvalidPtr((void*)cfaValue)) {
        ImpactDebugLog("[Log:WARN] CFA invalid\n");
        return ImpactResultFailure;
    }

    ImpactCPURegisters updatedRegisters = *registers;

    if (cfiData->cie.augmentationData.signedWithBKey) {
        ImpactDebugLog("[Log:WARN] addresses are signed\n");
    }

    for (uint32_t i = 0; i < ImpactCPUDWARFRegisterCount; ++i) {
        const ImpactDWARFRegister reg = state.registerRules[i];

        if (reg.rule == ImpactDWARFCFIRegisterRuleUnused) {
            continue;
        }

        uintptr_t regValue = 0;
        result = ImpactDWARFGetRegisterValue(&state, cfaValue, reg, &regValue);
        if (result != ImpactResultSuccess) {
            return result;
        }

        ImpactDebugLog("[Log:INFO] restoring register %d with 0x%lx\n", i, regValue);

        if (i == cfiData->cie.return_address_register) {
            result = ImpactCPUSetRegister(&updatedRegisters, ImpactCPURegisterInstructionPointer, regValue);

            if (result != ImpactResultSuccess) {
                return result;
            }
        }

        result = ImpactCPUSetRegister(&updatedRegisters, i, regValue);
        if (result != ImpactResultSuccess) {
            return result;
        }
    }

    result = ImpactCPUSetRegister(&updatedRegisters, ImpactCPURegisterStackPointer, cfaValue);
    if (result != ImpactResultSuccess) {
        return result;
    }

    *registers = updatedRegisters;

    return ImpactResultSuccess;
}

#endif
