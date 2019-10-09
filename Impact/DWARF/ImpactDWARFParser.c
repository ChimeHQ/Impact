//
//  ImpactDWARFParser.c
//  Impact
//
//  Created by Matt Massicotte on 2019-05-07.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#include "ImpactDataCursor.h"
#include "ImpactDWARF.h"
#include "ImpactDWARFDefines.h"
#include "ImpactUtility.h"

ImpactResult ImpactDWARFReadEncodedPointer(ImpactDataCursor* cursor, uint8_t encoding, intptr_t *value) {
    if (ImpactInvalidPtr(cursor) || ImpactInvalidPtr(value)) {
        return ImpactResultPointerInvalid;
    }

    if (encoding == DW_EH_PE_omit) {
        *value = 0;
        return ImpactResultSuccess;
    }

    ImpactResult result = ImpactResultFailure;
    uint64_t readValue = 0;
    const uintptr_t address = (uintptr_t)ImpactDataCursorCurrentPointer(cursor);

    switch (encoding & DW_EH_PE_type_mask) {
    case DW_EH_PE_ptr:
        result = ImpactDataCursorReadValue(cursor, sizeof(void*), &readValue);
        break;
    case DW_EH_PE_uleb128:
        result = ImpactDataCursorReadULEB128(cursor, &readValue);
        break;
    case DW_EH_PE_udata2:
        result = ImpactDataCursorReadValue(cursor, 2, &readValue);
        break;
    case DW_EH_PE_udata4:
        result = ImpactDataCursorReadValue(cursor, 4, &readValue);
        break;
    case DW_EH_PE_udata8:
        result = ImpactDataCursorReadValue(cursor, 8, &readValue);
        break;
    case DW_EH_PE_sleb128:
        result = ImpactDataCursorReadSLEB128(cursor, (int64_t *)&readValue);
        break;
    case DW_EH_PE_sdata2:
        result = ImpactDataCursorReadValue(cursor, 2, &readValue);
        break;
    case DW_EH_PE_sdata4:
        result = ImpactDataCursorReadValue(cursor, 4, &readValue);
        break;
    case DW_EH_PE_sdata8:
        result = ImpactDataCursorReadValue(cursor, 8, &readValue);
        break;
    }

    if (result != ImpactResultSuccess) {
        ImpactDebugLog("[Log:WARN:%s] unknown pointer encoding %x\n", __func__, encoding);

        return result;
    }

    switch (encoding & DW_EH_PE_offset_mask) {
    case DW_EH_PE_absptr:
        break;
    case DW_EH_PE_pcrel:
        // so, this is strange. The documentation says this is relative to the program counter,
        // but all implementations I can find use this to offset relative to *this* entry
        // in the dwarf data, which seems pretty weird...
        readValue += address;
        break;
    default:
        ImpactDebugLog("[Log:WARN:%s] unknown pointer encoding %x\n", __func__, encoding);

        return ImpactResultFailure;
    }

    if (encoding & DW_EH_PE_indirect) {
        if (ImpactInvalidPtr((void *)readValue)) {
            ImpactDebugLog("[Log:WARN:%s] indirect value invalid %x\n", __func__, encoding);
            return ImpactResultPointerInvalid;
        }

        readValue = *(uint64_t*)readValue;
    }

    *value = (intptr_t)readValue;

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

    result = ImpactResultFailure;

    if (ImpactDWARFCFILengthHas64BitMarker(header->length)) {
        result = ImpactDataCursorReadUint64(cursor, &header->CIE_id);
    } else {
        result = ImpactDataCursorReadUint32(cursor, (uint32_t*)&header->CIE_id);
    }

    return result;
}

ImpactResult ImpactDWARFReadCIEAugmentation(ImpactDataCursor* cursor, const char* augmentationString, ImpactDWARFCIEAppleAugmentationData* data) {
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

            result = ImpactDWARFReadEncodedPointer(cursor, data->personalityEncoding, (intptr_t*)&data->personality);
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

ImpactResult ImpactDWARFReadCIE(ImpactDataCursor* cursor, ImpactDWARFCIE* cie) {
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

    result = ImpactDWARFReadCIEAugmentation(cursor, cie->augmentation, &cie->augmentationData);
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

ImpactResult ImpactDWARFReadCFI(ImpactDataCursor* cursor, ImpactDWARFCFIData* cfiData) {
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

    result = ImpactDWARFReadCIE(&cieCursor, &cfiData->cie);
    if (result != ImpactResultSuccess) {
        return result;
    }

    // now, finally, continue reading the FDE

    // really don't know what's going on here. This doesn't seem to be covered by the
    // DWARF spec...
    const uint8_t encoding = cfiData->cie.augmentationData.pointerEncoding;

    result = ImpactDWARFReadEncodedPointer(cursor, encoding, (intptr_t*)&cfiData->fde.target_address);
    if (result != ImpactResultSuccess) {
        return result;
    }

    result = ImpactDWARFReadEncodedPointer(cursor, encoding & 0x0F, (intptr_t*)&cfiData->fde.address_range);
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
