//
//  ImpactDWARF.c
//  Impact
//
//  Created by Matt Massicotte on 2019-05-06.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#include "ImpactDWARF.h"
#include "ImpactDWARFParser.h"
#include "ImpactDataCursor.h"
#include "ImpactDWARFDefines.h"
#include "ImpactUtility.h"

static ImpactResult ImpactDWARFRun_DW_CFA_def_cfa(ImpactDataCursor* cursor, ImpactDWARFCFIState* state) {
    if (ImpactInvalidPtr(state)) {
        return ImpactResultPointerInvalid;
    }

    ImpactResult result = ImpactDataCursorReadULEB128(cursor, &state->cfaRegister);
    if (result != ImpactResultSuccess) {
        return result;
    }

    result = ImpactDataCursorReadULEB128(cursor, (uleb128*)&state->cfaRegisterOffset);
    if (result != ImpactResultSuccess) {
        return result;
    }

    return ImpactResultSuccess;
}

static ImpactResult ImpactDWARFRun_DW_CFA_offset(ImpactDataCursor* cursor, uint8_t operand, const ImpactDWARFCIE* cie, ImpactDWARFCFIState* state) {
    if (ImpactInvalidPtr(cie) || ImpactInvalidPtr(state)) {
        return ImpactResultPointerInvalid;
    }

    if (operand >= ImpactDWARFRegisterCount) {
        return ImpactResultArgumentInvalid;
    }

    uint64_t value = 0;

    ImpactResult result = ImpactDataCursorReadULEB128(cursor, &value);
    if (result != ImpactResultSuccess) {
        return result;
    }

    const uint64_t offset = value * cie->data_alignment_factor;

    state->registers[operand].location = ImpactDWARFRegsterInCFA;
    state->registers[operand].value = offset;

    return ImpactResultSuccess;
}

static ImpactResult ImpactDWARFRun_DW_CFA_def_cfa_offset(ImpactDataCursor* cursor, ImpactDWARFCFIState* state) {
    if (ImpactInvalidPtr(state)) {
        return ImpactResultPointerInvalid;
    }

    ImpactResult result = ImpactDataCursorReadULEB128(cursor, (uleb128*)&state->cfaRegisterOffset);
    if (result != ImpactResultSuccess) {
        return result;
    }

    return ImpactResultSuccess;
}

ImpactResult ImpactDWARFRunCFIInstructions(const ImpactDWARFCFIInstructions* instructions, const ImpactDWARFCIE* cie, uintptr_t pcOffset, ImpactDWARFCFIState* state) {
    ImpactDataCursor cursor = {0};

    ImpactResult result = ImpactDataCursorInitialize(&cursor, (uintptr_t)instructions->data, instructions->length, 0);
    if (result != ImpactResultSuccess) {
        return result;
    }

    while (!ImpactDataCursorAtEnd(&cursor)) {
        uint8_t opcode = 0;

        result = ImpactDataCursorReadUint8(&cursor, &opcode);
        if (result != ImpactResultSuccess) {
            ImpactDebugLog("[Log:%s] failed to read opcode %d\n", __func__, result);
            return result;
        }

        result = ImpactResultFailure;
        const uint8_t operand = opcode & 0x3F;

        switch (opcode) {
        case DW_CFA_nop:
            ImpactDebugLog("[Log:%s] DW_CFA_nop\n", __func__);
            result = ImpactResultSuccess;
            break;
        case DW_CFA_def_cfa:
            ImpactDebugLog("[Log:%s] DW_CFA_def_cfa\n", __func__);
            result = ImpactDWARFRun_DW_CFA_def_cfa(&cursor, state);
            break;
        case DW_CFA_def_cfa_offset:
            ImpactDebugLog("[Log:%s] DW_CFA_def_cfa_offset\n", __func__);
            result = ImpactDWARFRun_DW_CFA_def_cfa_offset(&cursor, state);
            break;
        default:
            switch (opcode & 0xC0) {
                case DW_CFA_offset:
                    ImpactDebugLog("[Log:%s] DW_CFA_offset\n", __func__);
                    result = ImpactDWARFRun_DW_CFA_offset(&cursor, operand, cie, state);
                    break;
                case DW_CFA_advance_loc:
                    ImpactDebugLog("[Log:%s] DW_CFA_advance_loc\n", __func__);
                    result = ImpactResultSuccess;
                    break;
                default:
                    ImpactDebugLog("[Log:%s] unhandled opcode %x\n", __func__, opcode);
                    return ImpactResultFailure;
            }
                break;
        }

        if (result != ImpactResultSuccess) {
            return result;
        }
    }
    
    return ImpactResultSuccess;
}

ImpactResult ImpactDWARFRunInstructions(const ImpactDWARFCFIData* data, ImpactDWARFCFIState* state) {
    uintptr_t pc = 0;

    ImpactResult result = ImpactDWARFRunCFIInstructions(&data->cie.instructions, &data->cie, pc, state);
    if (result != ImpactResultSuccess) {
        return result;
    }

    result = ImpactDWARFRunCFIInstructions(&data->fde.instructions, &data->cie, pc, state);
    if (result != ImpactResultSuccess) {
        return result;
    }

    return ImpactResultSuccess;
}

ImpactResult ImpactDWARFReadData(ImpactMachODataRegion cfiRegion, uint32_t offset, ImpactDWARFCFIData* data) {
    ImpactDataCursor cursor = {0};

    ImpactResult result = ImpactDataCursorInitialize(&cursor, cfiRegion.address, cfiRegion.length, offset);
    if (result != ImpactResultSuccess) {
        return result;
    }

    result = ImpactDWARFReadCFI(&cursor, data);
    if (result != ImpactResultSuccess) {
        ImpactDebugLog("[Log:%s] parsing CFI failed %d\n", __func__, result);
        return result;
    }

    return ImpactResultSuccess;
}

ImpactResult ImpactDWARFGetCFAValue(const ImpactDWARFCFIState* state, const ImpactCPURegisters* registers, uintptr_t *value) {
    if (ImpactInvalidPtr(state) || ImpactInvalidPtr(registers) || ImpactInvalidPtr(value)) {
        return ImpactResultPointerInvalid;
    }

    if (state->cfaRegister != 0) {
        ImpactResult result = ImpactCPUGetRegister(registers, (ImpactCPURegister)state->cfaRegister, value);
        if (result != ImpactResultSuccess) {
            return result;
        }

        *value += state->cfaRegisterOffset;

        return ImpactResultSuccess;
    }

    return ImpactResultUnimplemented;
}
