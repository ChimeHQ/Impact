//
//  ImpactDWARFParser.c
//  Impact
//
//  Created by Matt Massicotte on 2019-05-07.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#include "ImpactDWARFParser.h"
#include "ImpactDWARFDefines.h"
#include "ImpactUtility.h"

#if IMPACT_DWARF_CFI_SUPPORTED

ImpactResult ImpactDWARFReadEncodedPointer(ImpactDataCursor* cursor, ImpactDWARFEnvironment env, uint8_t encoding, uint64_t *outValue) {
    if (ImpactInvalidPtr(cursor) || ImpactInvalidPtr(outValue)) {
        return ImpactResultPointerInvalid;
    }

    if (encoding == DW_EH_PE_omit) {
        *outValue = 0;
        return ImpactResultSuccess;
    }

    ImpactResult result = ImpactResultFailure;
    int64_t value = 0;
    const uintptr_t currentAddress = (uintptr_t)ImpactDataCursorCurrentPointer(cursor);

    switch (encoding & DW_EH_PE_type_mask) {
    case DW_EH_PE_ptr:
        result = ImpactDataCursorReadValue(cursor, env.pointerWidth, &value);
        break;
    case DW_EH_PE_uleb128:
        result = ImpactDataCursorReadULEB128(cursor, (uleb128*)&value);
        break;
    case DW_EH_PE_udata2:
        result = ImpactDataCursorReadValue(cursor, 2, &value);
        break;
    case DW_EH_PE_udata4:
        result = ImpactDataCursorReadValue(cursor, 4, &value);
        break;
    case DW_EH_PE_udata8:
        result = ImpactDataCursorReadValue(cursor, 8, &value);
        break;
    case DW_EH_PE_sleb128:
        result = ImpactDataCursorReadSLEB128(cursor, &value);
        break;
    case DW_EH_PE_sdata2:
        result = ImpactDataCursorReadValue(cursor, 2, &value);
        break;
    case DW_EH_PE_sdata4:
        result = ImpactDataCursorReadValue(cursor, 4, &value);
        break;
    case DW_EH_PE_sdata8:
        result = ImpactDataCursorReadValue(cursor, 8, &value);
        break;
    }

    if (result != ImpactResultSuccess) {
        ImpactDebugLog("[Log:WARN] %s unknown pointer encoding %x\n", __func__, encoding);

        return result;
    }

    switch (encoding & DW_EH_PE_offset_mask) {
    case DW_EH_PE_absptr:
        break;
    case DW_EH_PE_pcrel:
        // So, this is strange. This does not actually mean relative to a PC, but relative to
        // the location of this field within the eh_frame section. I don't really know why
        // it would be useful to encode things in such a strange way, but it's very common.
//        value += currentAddress;
        value += currentAddress;
        break;
    default:
        ImpactDebugLog("[Log:WARN] %s unknown pointer encoding %x\n", __func__, encoding);

        return ImpactResultInconsistentData;
    }

    // NOTE: DW_EH_PE_indirect is not handled here. It's expected that ImpactDWARFResolveEncodedPointer be
    // used for that. This helps us defer, and possibly avoid memory accesses.

    *outValue = value;

    return ImpactResultSuccess;
}

ImpactResult ImpactDWARFReadLength(ImpactDataCursor* cursor, ImpactDWARFCFILength* length) {
    uint32_t length32 = 0;

    ImpactResult result = ImpactDataCursorReadUint32(cursor, &length32);
    if (result != ImpactResultSuccess) {
        return result;
    }

    if (length32 == 0) {
        return ImpactResultEndOfData;
    }

    length->length32 = length32;

    if (length32 != 0xffffffff) {
        return ImpactResultSuccess;
    }

    uint64_t length64 = 0;

    result = ImpactDataCursorReadUint64(cursor, &length64);
    if (result != ImpactResultSuccess) {
        return result;
    }

    length->length64 = length64;

    return ImpactResultSuccess;
}

ImpactResult ImpactDWARFReadHeader(ImpactDataCursor* cursor, ImpactDWARFCFIHeader* header) {
    if (ImpactInvalidPtr(header)) {
        return ImpactResultPointerInvalid;
    }

    ImpactResult result = ImpactDWARFReadLength(cursor, &header->length);
    if (result != ImpactResultSuccess) {
        return result;
    }

    if (ImpactDWARFCFILengthHas64BitMarker(header->length)) {
        result = ImpactDataCursorReadUint64(cursor, &header->CIE_id);
    } else {
        result = ImpactDataCursorReadUint32(cursor, (uint32_t*)&header->CIE_id);
    }

    return result;
}

ImpactResult ImpactDWARFReadCIEAugmentation(ImpactDataCursor* cursor, ImpactDWARFEnvironment env, const char* augmentationString, ImpactDWARFCIEAppleAugmentationData* data) {
    if (ImpactInvalidPtr(cursor) || ImpactInvalidPtr(augmentationString) || ImpactInvalidPtr(data)) {
        return ImpactResultPointerInvalid;
    }

    // I'm unsure if this is actually a valid string, but
    // this is how apple's implemenation works...
    if (*augmentationString != 'z') {
        return ImpactResultSuccess;
    }

    ImpactResult result = ImpactDataCursorReadULEB128(cursor, &data->length);
    if (result != ImpactResultSuccess) {
        return result;
    }

    for (const char* c = augmentationString; *c != '\0'; ++c) {
        switch (*c) {
        case 'z':
            data->fdesHaveAugmentationData = true;
            break;
        case 'P':
            result = ImpactDataCursorReadUint8(cursor, &data->personalityEncoding);
            if (result != ImpactResultSuccess) {
                return result;
            }

            result = ImpactDWARFReadEncodedPointer(cursor, env, data->personalityEncoding, &data->personality);
            if (result != ImpactResultSuccess) {
                return result;
            }
            break;
        case 'L':
            result = ImpactDataCursorReadUint8(cursor, &data->lsdaEncoding);
            if (result != ImpactResultSuccess) {
                return result;
            }
            break;
        case 'R':
            result = ImpactDataCursorReadUint8(cursor, &data->pointerEncoding);
            if (result != ImpactResultSuccess) {
                return result;
            }
            break;
        case 'S':
            data->isSignalFrame = true;
            break;
        default:
            return ImpactResultUnexpectedData;
        }
    }

    return ImpactResultSuccess;
}

ImpactResult ImpactDWARFReadCIE(ImpactDataCursor* cursor, ImpactDWARFEnvironment env, ImpactDWARFCIE* cie) {
    if (!ImpactDataCursorIsValid(cursor)) {
        return ImpactResultPointerInvalid;
    }

    uintptr_t startingOffset = cursor->offset;

    ImpactResult result = ImpactDWARFReadHeader(cursor, &cie->header);
    if (result != ImpactResultSuccess) {
        return result;
    }

    result = ImpactDataCursorReadUint8(cursor, &cie->version);
    if (result != ImpactResultSuccess) {
        return result;
    }

    uint32_t augmentationStringLength = 0;

    result = ImpactDataCursorReadString(cursor, &cie->augmentation, &augmentationStringLength);
    if (result != ImpactResultSuccess) {
        return result;
    }

    // Apple's implemenation seems to skip the standard-defined "address_size" and "segment_size" fields
    result = ImpactDataCursorReadULEB128(cursor, &cie->code_alignment_factor);
    if (result != ImpactResultSuccess) {
        return result;
    }

    result = ImpactDataCursorReadSLEB128(cursor, &cie->data_alignment_factor);
    if (result != ImpactResultSuccess) {
        return result;
    }

    result = ImpactDataCursorReadULEB128(cursor, &cie->return_address_register);
    if (result != ImpactResultSuccess) {
        return result;
    }

    result = ImpactDWARFReadCIEAugmentation(cursor, env, cie->augmentation, &cie->augmentationData);
    if (result != ImpactResultSuccess) {
        return result;
    }

    cie->instructions.data = ImpactDataCursorCurrentPointer(cursor);
    cie->instructions.length = ImpactDWARFCFIGetTotalLength(cie->header) - (cursor->offset - startingOffset);

    return ImpactResultSuccess;
}

ImpactResult ImpactDWARFReadFDEAugmentation(ImpactDataCursor* cursor, ImpactDWARFCFIData* cfiData) {
    if (!cfiData->cie.augmentationData.fdesHaveAugmentationData) {
        return ImpactResultSuccess;
    }

    uint64_t fdeAugmentationLength = 0;

    // this length value *does* include itself
    ImpactResult result = ImpactDataCursorReadULEB128(cursor, &fdeAugmentationLength);
    if (result != ImpactResultSuccess) {
        return result;
    }

    // manually set the offset past the augmentation data to skip it for now
    cursor->offset = cursor->offset + (uintptr_t)fdeAugmentationLength;

    return ImpactResultSuccess;
}

uintptr_t ImpactDWARFCFECIEOffsetDelta(ImpactDWARFFDE* fde) {
    // two annoying problems:
    //
    // first, the dwarf spec seems to indicate that this pointer should be an offset relative to the
    // start of the eh_frame section. But, it appears in practice it is relative to the FDE
    //
    // second, it seems relative not to the start of the FDE, but to the start of the CIE_id entry
    bool is64Bit = ImpactDWARFCFILengthHas64BitMarker(fde->header.length);
    uintptr_t delta = (uintptr_t)fde->header.CIE_id + (is64Bit ? 8 : 4);

    return delta;
}

ImpactResult ImpactDWARFReadCFI(ImpactDataCursor* cursor, ImpactDWARFEnvironment env, ImpactDWARFCFIData* cfiData) {
    if (!ImpactDataCursorIsValid(cursor)) {
        return ImpactResultPointerInvalid ;
    }

    const uintptr_t startingOffset = cursor->offset;

    ImpactResult result = ImpactDWARFReadHeader(cursor, &cfiData->fde.header);
    if (result != ImpactResultSuccess) {
        return result;
    }

    if (cfiData->fde.header.CIE_id == 0) {
        return ImpactResultInconsistentData;
    }

    const uintptr_t delta = ImpactDWARFCFECIEOffsetDelta(&cfiData->fde);
    const uintptr_t cieOffset = cursor->offset - delta;

    ImpactDataCursor cieCursor = {0};
    result = ImpactDataCursorInitialize(&cieCursor, cursor->address, cursor->limit, cieOffset);
    if (result != ImpactResultSuccess) {
        return result;
    }

    result = ImpactDWARFReadCIE(&cieCursor, env, &cfiData->cie);
    if (result != ImpactResultSuccess) {
        return result;
    }

    // now, finally, continue reading the FDE

    // This pointer encoding business does not appear in the DWARF spec. It appears
    // to be compiler and ABI-specific. And, since it doesn't come from a spec, it is
    // weird.
    const uint8_t encoding = cfiData->cie.augmentationData.pointerEncoding;

    result = ImpactDWARFReadEncodedPointer(cursor, env, encoding, &cfiData->fde.target_address);
    if (result != ImpactResultSuccess) {
        return result;
    }

    // This mask needs to be applied to calculate the correct range. Because the range is stored as an encoded pointer,
    // but isn't actually a pointer. Who makes this stuff?
    const uint8_t rangeEncoding = encoding & DW_EH_PE_type_mask;

    result = ImpactDWARFReadEncodedPointer(cursor, env, rangeEncoding, &cfiData->fde.address_range);
    if (result != ImpactResultSuccess) {
        return result;
    }

    result = ImpactDWARFReadFDEAugmentation(cursor, cfiData);
    if (result != ImpactResultSuccess) {
        return result;
    }

    cfiData->fde.instructions.data = ImpactDataCursorCurrentPointer(cursor);
    cfiData->fde.instructions.length = ImpactDWARFCFIGetTotalLength(cfiData->fde.header) - (cursor->offset - startingOffset);

    return ImpactResultSuccess;
}

#endif
