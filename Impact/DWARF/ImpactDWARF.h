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
    uint32_t initial_location;
    uint32_t segment_selector;
    uint32_t target_address;
    uint32_t address_range;
    ImpactDWARFCFIInstructions instructions;
} ImpactDWARFFDE;

typedef struct {
    uleb128 length;
    bool fdesHaveAugmentationData;
    uint8_t personalityEncoding;
    uintptr_t personality;
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

typedef struct {
    ImpactMachODataRegion cfiRegion;
    uintptr_t address;
} ImpactDWARFTarget;

typedef enum {
    ImpactDWARFRegsterUndefined = 0,
    ImpactDWARFRegsterUnused = 1,
    ImpactDWARFRegsterInCFA = 2,
    ImpactDWARFRegsterFromCFA = 3,
    ImpactDWARFRegsterInRegister = 4,
    ImpactDWARFRegsterAtExpression = 5,
    ImpactDWARFRegsterIsExpresssion = 6
} ImpactDWARFRegsterLocation;

typedef struct {
    ImpactDWARFRegsterLocation location;
    int64_t value;
} ImpactDWARFRegister;

enum { ImpactDWARFRegisterCount = 10 };

typedef struct {
    uint64_t cfaRegister;
    int64_t cfaRegisterOffset;
    ImpactDWARFRegister registers[ImpactDWARFRegisterCount];
} ImpactDWARFCFIState;

ImpactResult ImpactDWARFRunInstructions(const ImpactDWARFCFIData* data, ImpactDWARFCFIState* state);
ImpactResult ImpactDWARFReadData(ImpactMachODataRegion cfiRegion, uint32_t offset, ImpactDWARFCFIData* data);

ImpactResult ImpactDWARFGetCFAValue(const ImpactDWARFCFIState* state, const ImpactCPURegisters* registers, uintptr_t *value);

#endif /* ImpactDWARF_h */
