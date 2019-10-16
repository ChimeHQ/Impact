//
//  ImpactDWARF.h
//  Impact
//
//  Created by Matt Massicotte on 2019-05-06.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#ifndef ImpactDWARF_h
#define ImpactDWARF_h

#include <stdint.h>

#include "ImpactBinaryImage.h"
#include "ImpactLEB.h"
#include "ImpactCPU.h"
#include "ImpactResult.h"

#if defined(__x86_64__) || defined(__i386__) || defined(__arm64__)
#define IMPACT_DWARF_CFI_SUPPORTED 1
#else
#define IMPACT_DWARF_CFI_SUPPORTED 0
#endif

#if IMPACT_DWARF_CFI_SUPPORTED

typedef struct {
    uint32_t length32;
    uint64_t length64;
} ImpactDWARFCFILength;

typedef struct {
    ImpactDWARFCFILength length;
    uint64_t CIE_id;
} ImpactDWARFCFIHeader;

static bool ImpactDWARFCFILengthHas64BitMarker(ImpactDWARFCFILength length) {
    return length.length32 == 0xffffffff;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
// these are useful for test targets
static uint64_t ImpactDWARFCFILengthGetValue(ImpactDWARFCFILength length) {
    if (ImpactDWARFCFILengthHas64BitMarker(length)) {
        return length.length64;
    }

    return length.length32;
}

static uint64_t ImpactDWARFCFIHeaderGetLength(ImpactDWARFCFIHeader header) {
    return ImpactDWARFCFILengthGetValue(header.length);
}

static uint64_t ImpactDWARFCFIHeaderLengthFieldSize(ImpactDWARFCFIHeader header) {
    if (ImpactDWARFCFILengthHas64BitMarker(header.length)) {
        return 4 + 8;
    } else {
        return 4;
    }
}

static uint64_t ImpactDWARFCFIGetTotalLength(ImpactDWARFCFIHeader header) {
    // the length field of a CFI entry does not include itself
    return ImpactDWARFCFIHeaderGetLength(header) + ImpactDWARFCFIHeaderLengthFieldSize(header);
}

#pragma clang diagnostic pop

typedef struct {
    const void* data;
    uint8_t length;
} ImpactDWARFCFIInstructions;

typedef struct {
    ImpactDWARFCFIHeader header;
    uintptr_t initial_location;
    uintptr_t segment_selector;
    uint64_t target_address;
    uint64_t address_range;
    ImpactDWARFCFIInstructions instructions;
} ImpactDWARFFDE;

typedef struct {
    uleb128 length;
    bool fdesHaveAugmentationData;
    uint8_t personalityEncoding;
    uint64_t personality;
    uint8_t lsdaEncoding;
    uint8_t pointerEncoding;
    bool isSignalFrame;
} ImpactDWARFCIEAppleAugmentationData;

typedef struct {
    ImpactDWARFCFIHeader header;
    uint8_t version;
    const char* augmentation;
    uint8_t address_size;
    uint8_t segment_size;
    uleb128 code_alignment_factor;
    sleb128 data_alignment_factor;
    uleb128 return_address_register;
    ImpactDWARFCIEAppleAugmentationData augmentationData;
    ImpactDWARFCFIInstructions instructions;
} ImpactDWARFCIE;

typedef struct {
    ImpactDWARFCIE cie;
    ImpactDWARFFDE fde;
} ImpactDWARFCFIData;

typedef enum {
    ImpactDWARFCFADefinitionRuleUndefined = 0,
    ImpactDWARFCFADefinitionRuleRegisterOffset = 1,
    ImpactDWARFCFADefinitionRuleExpressiom = 2
} ImpactDWARFCFARegisterRule;

typedef enum {
    ImpactDWARFCFIRegisterRuleUnused = 0,
    ImpactDWARFCFIRegisterRuleOffsetFromCFA = 1
} ImpactDWARFCFIRegisterRule;

typedef struct {
    ImpactDWARFCFIRegisterRule rule;
    int64_t value;
} ImpactDWARFRegister;

typedef struct {
    ImpactDWARFCFARegisterRule rule;
    uint32_t registerNum;
    int64_t value;
} ImpactDWARFCFADefinition;

typedef struct {
    ImpactDWARFCFADefinition cfaDefinition;
    ImpactDWARFRegister registerRules[ImpactCPUDWARFRegisterCount];
} ImpactDWARFCFIState;

typedef struct {
    uint8_t pointerWidth;
} ImpactDWARFEnvironment;

typedef struct {
    uintptr_t pc;
    ImpactMachODataRegion ehFrameRegion;
    ImpactDWARFEnvironment environment;
} ImpactDWARFTarget;

ImpactResult ImpactDWARFRunInstructions(const ImpactDWARFCFIData* data, ImpactDWARFTarget target, ImpactDWARFCFIState* state);
ImpactResult ImpactDWARFReadData(ImpactMachODataRegion cfiRegion, ImpactDWARFEnvironment env, uint32_t offset, ImpactDWARFCFIData* data);
ImpactResult ImpactDWARFResolveEncodedPointer(uint8_t encoding, uint64_t value, uint64_t *resolvedValue);

ImpactResult ImpactDWARFGetCFAValue(const ImpactDWARFCFIState* state, const ImpactCPURegisters* registers, uintptr_t *value);
ImpactResult ImpactDWARFGetRegisterValue(const ImpactDWARFCFIState* state, uintptr_t cfa, ImpactDWARFRegister dwarfRegister, uintptr_t* value);

ImpactResult ImpactDWARFStepRegisters(const ImpactDWARFCFIData* cfiData, ImpactDWARFTarget target, ImpactCPURegisters* registers);

#endif

#endif /* ImpactDWARF_h */
