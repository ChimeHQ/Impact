//
//  ImpactDWARFCFIInstructions.c
//  Impact
//
//  Created by Matt Massicotte on 2019-10-13.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#include "ImpactDWARFCFIInstructions.h"
#include "ImpactUtility.h"

#if IMPACT_DWARF_CFI_SUPPORTED

ImpactResult ImpactDWARFRun_DW_CFA_def_cfa(ImpactDataCursor* cursor, ImpactDWARFCFIState* state) {
    if (ImpactInvalidPtr(state)) {
        return ImpactResultPointerInvalid;
    }

    uleb128 value = 0;

    ImpactResult result = ImpactDataCursorReadULEB128(cursor, &value);
    if (result != ImpactResultSuccess) {
        return result;
    }

    state->cfaDefinition.rule = ImpactDWARFCFADefinitionRuleRegisterOffset;

    state->cfaDefinition.registerNum = (int32_t)value;

    value = 0;
    result = ImpactDataCursorReadULEB128(cursor, &value);
    if (result != ImpactResultSuccess) {
        return result;
    }

    state->cfaDefinition.value = value;

    ImpactDebugLog("[Log:INFO] DW_CFA_def_cfa reg %d offset %lld\n", state->cfaDefinition.registerNum, state->cfaDefinition.value);

    return ImpactResultSuccess;
}

ImpactResult ImpactDWARFRun_DW_CFA_offset(ImpactDataCursor* cursor, uint8_t operand, const ImpactDWARFCIE* cie, ImpactDWARFCFIState* state) {
    if (ImpactInvalidPtr(cie) || ImpactInvalidPtr(state)) {
        return ImpactResultPointerInvalid;
    }

    if (operand >= ImpactCPUDWARFRegisterCount) {
        ImpactDebugLog("[Log:WARN] DW_CFA_offset register out of range %d\n", operand);
        return ImpactResultInconsistentData;
    }

    uint64_t value = 0;

    ImpactResult result = ImpactDataCursorReadULEB128(cursor, &value);
    if (result != ImpactResultSuccess) {
        ImpactDebugLog("[Log:WARN] DW_CFA_offset failed to read uleb128 %d\n", result);
        return result;
    }

    const int64_t offset = value * cie->data_alignment_factor;

    state->registerRules[operand].rule = ImpactDWARFCFIRegisterRuleOffsetFromCFA;
    state->registerRules[operand].value = offset;

    ImpactDebugLog("[Log:INFO] DW_CFA_offset reg=%d offset=%lld\n", operand, offset);

    return ImpactResultSuccess;
}

ImpactResult ImpactDWARFRun_DW_CFA_advance_loc(ImpactDataCursor* cursor, uint8_t operand, const ImpactDWARFCIE* cie, ImpactDWARFCFIState* state, uint32_t* location) {
    if (ImpactInvalidPtr(cie) || ImpactInvalidPtr(state) || ImpactInvalidPtr(location)) {
        return ImpactResultPointerInvalid;
    }

    // should be a little more careful about checking bounds here
    const int32_t delta = operand * (int32_t)cie->code_alignment_factor;

    ImpactDebugLog("[Log:INFO] DW_CFA_advance_loc delta=%d\n", delta);

    *location += delta;

    return ImpactResultSuccess;
}

ImpactResult ImpactDWARFRun_DW_CFA_def_cfa_offset(ImpactDataCursor* cursor, ImpactDWARFCFIState* state) {
    if (ImpactInvalidPtr(state)) {
        return ImpactResultPointerInvalid;
    }

    uleb128 value = 0;
    ImpactResult result = ImpactDataCursorReadULEB128(cursor, &value);
    if (result != ImpactResultSuccess) {
        return result;
    }

    // per spec, only defined for register/offset rules
    if (state->cfaDefinition.rule != ImpactDWARFCFIRegisterRuleOffsetFromCFA) {
        ImpactDebugLog("[Log:WARN] invalid for current CFA rule %d\n", state->cfaDefinition.rule);
        return ImpactResultInconsistentData;
    }

    ImpactDebugLog("[Log:INFO] DW_CFA_def_cfa_offset offset=%llu\n", value);

    state->cfaDefinition.value = value;

    return ImpactResultSuccess;
}

#endif
