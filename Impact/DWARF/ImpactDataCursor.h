//
//  ImpactDataCursor.h
//  Impact
//
//  Created by Matt Massicotte on 2019-08-01.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#ifndef ImpactDataCursor_h
#define ImpactDataCursor_h

#include "ImpactResult.h"
#include "ImpactLEB.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uintptr_t address;
    uintptr_t limit;
    uint64_t offset;
} ImpactDataCursor;

static const uint32_t ImpactDataCursorMaxStringLength = 4096;

ImpactResult ImpactDataCursorInitialize(ImpactDataCursor* cursor, uintptr_t address, uintptr_t limit, uint64_t offset);

const void* ImpactDataCursorCurrentPointer(const ImpactDataCursor* cursor);
bool ImpactDataCursorIsValid(const ImpactDataCursor* cursor);
bool ImpactDataCursorAtEnd(const ImpactDataCursor* cursor);

ImpactResult ImpactDataCursorReadValue(ImpactDataCursor* cursor, size_t size, void* value);
ImpactResult ImpactDataCursorReadUint8(ImpactDataCursor* cursor, uint8_t* value);
ImpactResult ImpactDataCursorReadUint32(ImpactDataCursor* cursor, uint32_t* value);
ImpactResult ImpactDataCursorReadUint64(ImpactDataCursor* cursor, uint64_t* value);
ImpactResult ImpactDataCursorReadULEB128(ImpactDataCursor* cursor, uleb128* value);
ImpactResult ImpactDataCursorReadSLEB128(ImpactDataCursor* cursor, sleb128* value);
ImpactResult ImpactDataCursorReadString(ImpactDataCursor* cursor, const char **string, uint32_t *length);

#endif /* ImpactDataCursor_h */
