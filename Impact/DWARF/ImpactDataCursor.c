//
//  ImpactDataCursor.c
//  Impact
//
//  Created by Matt Massicotte on 2019-08-01.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#include "ImpactDataCursor.h"
#include "ImpactUtility.h"

ImpactResult ImpactDataCursorInitialize(ImpactDataCursor* cursor, uintptr_t address, uintptr_t limit, uint64_t offset) {
    if (ImpactInvalidPtr(cursor) || ImpactInvalidPtr((void*)address)) {
        return ImpactResultPointerInvalid;
    }

    cursor->address = address;
    cursor->limit = limit;
    cursor->offset = offset;

    return ImpactResultSuccess;
}

bool ImpactDataCursorIsValid(const ImpactDataCursor* cursor) {
    if (ImpactInvalidPtr(cursor)) {
        return false;
    }

    if (ImpactInvalidPtr((void*)cursor->address)) {
        return false;
    }

    return !ImpactDataCursorAtEnd(cursor);
}

bool ImpactDataCursorAtEnd(const ImpactDataCursor* cursor) {
    if (ImpactInvalidPtr(cursor)) {
        return true;
    }

    return cursor->offset >= cursor->limit;
}

const void* ImpactDataCursorCurrentPointer(const ImpactDataCursor* cursor) {
    if (!ImpactDataCursorIsValid(cursor)) {
        return NULL;
    }

    return (void*)(cursor->address + cursor->offset);
}

ImpactResult ImpactDataCursorReadValue(ImpactDataCursor* cursor, size_t size, void* value) {
    if (ImpactInvalidPtr(cursor) || ImpactInvalidPtr(value)) {
        return ImpactResultPointerInvalid;
    }

    const void* ptr = ImpactDataCursorCurrentPointer(cursor);
    if (ImpactInvalidPtr(ptr)) {
        return ImpactResultStateInvalid;
    }

    switch (size) {
    case 1:
        *(uint8_t*)value = *(const uint8_t*)ptr;
        break;
    case 2:
        *(uint16_t*)value = *(const uint16_t*)ptr;
        break;
    case 4:
        *(uint32_t*)value = *(const uint32_t*)ptr;
        break;
    case 8:
        *(uint64_t*)value = *(const uint64_t*)ptr;
        break;
    default:
        return ImpactResultArgumentInvalid;
    }

    cursor->offset += size;

    return ImpactResultSuccess;
}

ImpactResult ImpactDataCursorReadUint8(ImpactDataCursor* cursor, uint8_t* value) {
    return ImpactDataCursorReadValue(cursor, 1, value);
}

ImpactResult ImpactDataCursorReadUint32(ImpactDataCursor* cursor, uint32_t* value) {
    return ImpactDataCursorReadValue(cursor, 4, value);
}

ImpactResult ImpactDataCursorReadUint64(ImpactDataCursor* cursor, uint64_t* value) {
    return ImpactDataCursorReadValue(cursor, 8, value);
}

ImpactResult ImpactDataCursorReadULEB128(ImpactDataCursor* cursor, uleb128* value) {
    int shift = 0;
    size_t count = 0;

    for (; count <= 8; ++count) {
        uint8_t byte = 0;

        ImpactResult result = ImpactDataCursorReadUint8(cursor, &byte);
        if (result != ImpactResultSuccess) {
            return result;
        }

        *value |= (byte & 0x7f) << shift;
        shift += 7;

        if (!(byte & 0x80)) {
            break;
        }
    }

    return count < 8 ? ImpactResultSuccess : ImpactResultFailure;
}

ImpactResult ImpactDataCursorReadSLEB128(ImpactDataCursor* cursor, sleb128* value) {
    int shift = 0;
    size_t count = 0;
    uint8_t byte = 0;

    for (; count <= 8; ++count) {
        ImpactResult result = ImpactDataCursorReadUint8(cursor, &byte);
        if (result != ImpactResultSuccess) {
            return result;
        }

        *value |= (byte & 0x7f) << shift;
        shift += 7;

        if (!(byte & 0x80)) {
            break;
        }
    }

    if (count >= 8) {
        return ImpactResultFailure;
    }

    if (shift < sizeof(uint64_t) && (byte & 0x40)) {
        *value |= (-1ULL) << shift;
    }

    return ImpactResultSuccess;
}

ImpactResult ImpactDataCursorReadString(ImpactDataCursor* cursor, const char **string, uint32_t *length) {
    if (ImpactInvalidPtr(cursor) || ImpactInvalidPtr(string) || ImpactInvalidPtr(length)) {
        return ImpactResultPointerInvalid;
    }

    *string = ImpactDataCursorCurrentPointer(cursor);

    for (uint32_t i = 0; i < ImpactDataCursorMaxStringLength; ++i) {
        uint8_t c = 0;

        ImpactResult result = ImpactDataCursorReadUint8(cursor, &c);
        if (result != ImpactResultSuccess) {
            return result;
        }

        if (c == 0) {
            break;
        }

        *length += 1;
    }

    return ImpactResultSuccess;
}
