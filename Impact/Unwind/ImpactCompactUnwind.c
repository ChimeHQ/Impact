//
//  ImpactCompactUnwind.c
//  Impact
//
//  Created by Matt Massicotte on 2019-10-03.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#include "ImpactCompactUnwind.h"
#include "ImpactUtility.h"
#include "ImpactUnwind.h"
#include "ImpactState.h"
#include "ImpactLog.h"
#include "ImpactDWARF.h"

#include <mach-o/compact_unwind_encoding.h>

typedef struct unwind_info_section_header CompactUnwindHeader;
typedef struct unwind_info_section_header_index_entry CompactUnwindIndexEntry;
typedef struct unwind_info_compressed_second_level_page_header CompactUnwindCompressedHeader;

static uint32_t ImpactCompactUnwindSearch(const void* ptr, const void* ctx, size_t elementSize, uint32_t count, bool (*comparsionFn) (const void *, const void *));
static bool ImpactCompactUnwindCompareIndexEntry(const void *a, const void *b);
static bool ImpactCompactUnwindCompareCompressedEntry(const void *a, const void *b);

ImpactResult ImpactCompactUnwindLookupFirstLevel(ImpactCompactUnwindTarget target, const struct unwind_info_section_header_index_entry** index) {
    if (ImpactInvalidPtr(index)) {
        return ImpactResultPointerInvalid;
    }

    const uintptr_t imageRelativeAddress = target.address - target.imageLoadAddress;

    *index = ImpactPointerOffset(target.header, target.header->indexSectionOffset);

    const uint32_t count = target.header->indexCount;
    const size_t size = sizeof(struct unwind_info_section_header_index_entry);

    uint32_t idx = ImpactCompactUnwindSearch(*index, &imageRelativeAddress, size, count, ImpactCompactUnwindCompareIndexEntry);
    if (idx == count) {
        // the last index is actually a limit, so we do not need to check
        // if that's actually our match
        *index = NULL;

        return ImpactResultFailure;
    }

    if (idx <= 0) {
        return ImpactResultFailure;
    }

    *index = ImpactPointerOffset(*index, (idx - 1) * size);

    return ImpactResultSuccess;
}

ImpactResult ImpactCompactUnwindLookupSecondLevelRegular(ImpactCompactUnwindTarget target, const CompactUnwindIndexEntry* index, const compact_unwind_encoding_t** encoding) {
    return ImpactResultUnimplemented;
}

ImpactResult ImpactCompactUnwindLookupSecondLevelCompressedEntry(ImpactCompactUnwindTarget target, const CompactUnwindIndexEntry* index, const compact_unwind_encoding_t** encoding) {
    if (ImpactInvalidPtr(index) || ImpactInvalidPtr(encoding)) {
        return ImpactResultPointerInvalid;
    }

    const CompactUnwindCompressedHeader* compressedHeader = ImpactPointerOffset(target.header, index->secondLevelPagesSectionOffset);
    if (compressedHeader->kind != UNWIND_SECOND_LEVEL_COMPRESSED) {
        ImpactDebugLog("[Log:WARN:%s] compressed header kind invalid %d\n", __func__, compressedHeader->kind);
        return ImpactResultInconsistentData;
    }

    const uintptr_t imageRelativeAddress = target.address - target.imageLoadAddress;

    if (imageRelativeAddress < index->functionOffset) {
        ImpactDebugLog("[Log:WARN:%s] address not in range\n", __func__);
        return ImpactResultInconsistentData;
    }

    const uintptr_t targetFunctionOffset = imageRelativeAddress - index->functionOffset;

    *encoding = ImpactPointerOffset(compressedHeader, compressedHeader->entryPageOffset);

    const uint32_t count = compressedHeader->entryCount;
    const size_t size = sizeof(compact_unwind_encoding_t);

    const uint32_t idx = ImpactCompactUnwindSearch(*encoding, &targetFunctionOffset, size, count, ImpactCompactUnwindCompareCompressedEntry);
    if (idx == count) {
        // we have to special-case the last entry, in case that's out match
        const CompactUnwindIndexEntry* nextIndex = index + 1;

        if (imageRelativeAddress >= nextIndex->functionOffset) {
            *encoding = NULL;
            ImpactDebugLog("[Log:WARN:%s] address not in within found function\n", __func__);
            return ImpactResultFailure;
        }
    }

    if (idx <= 0) {
        return ImpactResultFailure;
    }

    // remember, the returned result will be the first *past* our match
    *encoding = ImpactPointerOffset(*encoding, (idx - 1) * size);

    return ImpactResultSuccess;
}

ImpactResult ImpactCompactUnwindLookupSecondLevelEncoding(const CompactUnwindHeader* header, const CompactUnwindCompressedHeader* secondLevelHeader, uint16_t encodingIndex, const compact_unwind_encoding_t** encoding) {
    if (ImpactInvalidPtr(header) || ImpactInvalidPtr(encoding)) {
        return ImpactResultPointerInvalid;
    }

    if (encodingIndex < header->commonEncodingsArrayCount) {
        const uintptr_t offset = encodingIndex * sizeof(compact_unwind_encoding_t);

        *encoding = ImpactPointerOffset(header, header->commonEncodingsArraySectionOffset + offset);

        return ImpactResultSuccess;
    }

    if (ImpactInvalidPtr(secondLevelHeader)) {
        return ImpactResultPointerInvalid;
    }

    const uint16_t index = encodingIndex - header->commonEncodingsArrayCount;
    const uintptr_t offset = index * sizeof(compact_unwind_encoding_t);

    *encoding = ImpactPointerOffset(secondLevelHeader, secondLevelHeader->encodingsPageOffset + offset);

    return ImpactResultSuccess;
}

ImpactResult ImpactCompactUnwindLookupSecondLevelCompressed(ImpactCompactUnwindTarget target, const CompactUnwindIndexEntry* index, const compact_unwind_encoding_t** encoding) {
    ImpactResult result = ImpactCompactUnwindLookupSecondLevelCompressedEntry(target, index, encoding);
    if (result != ImpactResultSuccess) {
        return result;
    }

    if (ImpactInvalidPtr(encoding) || ImpactInvalidPtr(target.header) || ImpactInvalidPtr(index)) {
        return ImpactResultPointerInvalid;
    }

    uint16_t encodingIndex = UNWIND_INFO_COMPRESSED_ENTRY_ENCODING_INDEX(**encoding);
    const CompactUnwindCompressedHeader* compressedHeader = ImpactPointerOffset(target.header, index->secondLevelPagesSectionOffset);

    return ImpactCompactUnwindLookupSecondLevelEncoding(target.header, compressedHeader, encodingIndex, encoding);
}

ImpactResult ImpactCompactUnwindLookupSecondLevel(ImpactCompactUnwindTarget target, const CompactUnwindIndexEntry* index, const compact_unwind_encoding_t** encoding) {
    const struct unwind_info_regular_second_level_page_header* secondLevelHeader = ImpactPointerOffset(target.header, index->secondLevelPagesSectionOffset);

    switch (secondLevelHeader->kind) {
        case UNWIND_SECOND_LEVEL_REGULAR:
            return ImpactCompactUnwindLookupSecondLevelRegular(target, index, encoding);
        case UNWIND_SECOND_LEVEL_COMPRESSED:
            return ImpactCompactUnwindLookupSecondLevelCompressed(target, index, encoding);
        default:
            break;
    }

    return ImpactResultInconsistentData;
}

ImpactResult ImpactCompactUnwindLookupEncoding(ImpactCompactUnwindTarget target, compact_unwind_encoding_t* encoding) {
    if (ImpactInvalidPtr(target.header) || ImpactInvalidPtr(encoding)) {
        return ImpactResultPointerInvalid;
    }

    if (target.header->version != UNWIND_SECTION_VERSION) {
        ImpactDebugLog("[Log:INFO:%s] compact unwind version invalid %d\n", __func__, target.header->version);

        return ImpactResultInconsistentData;
    }

    const CompactUnwindIndexEntry *firstLevelEntry = NULL;
    ImpactResult result;

    result = ImpactCompactUnwindLookupFirstLevel(target, &firstLevelEntry);
    if (result != ImpactResultSuccess) {
        ImpactDebugLog("[Log:INFO:%s] failed to lookup first level encoding %d\n", __func__, result);
        return result;
    }

    const compact_unwind_encoding_t* encodingPtr = NULL;
    result = ImpactCompactUnwindLookupSecondLevel(target, firstLevelEntry, &encodingPtr);
    if (result != ImpactResultSuccess) {
        ImpactDebugLog("[Log:INFO:%s] failed to lookup second level encoding %d\n", __func__, result);
        return result;
    }

    *encoding = *encodingPtr;

    return ImpactResultSuccess;
}

ImpactResult ImpactCompactUnwindStepRegisters(ImpactCompactUnwindTarget target, ImpactCPURegisters* registers, uint32_t* dwarfFDEOffset) {
    compact_unwind_encoding_t encoding = 0;

    ImpactResult result = ImpactCompactUnwindLookupEncoding(target, &encoding);
    if (result != ImpactResultSuccess) {
        ImpactDebugLog("[Log:WARN] failed to look up compact unwind encoding %d\n", result);
        return result;
    }

    if (encoding == 0) {
        ImpactDebugLog("[Log:INFO] no unwind info available\n");

        return ImpactResultMissingUnwindInfo;
    }

    ImpactDebugLog("[Log:INFO] found compact unwind encoding 0x%x\n", encoding);

    return ImpactCompactUnwindStepArchRegisters(target, registers, encoding, dwarfFDEOffset);
}

static uint32_t ImpactCompactUnwindSearch(const void* ptr, const void* ctx, size_t elementSize, uint32_t count, bool (*comparsionFn) (const void *, const void *)) {
    uint32_t low = 0;
    uint32_t high = count;

    while (low < high) {
        const uint32_t mid = (low + high) / 2;

        const void *entry = ImpactPointerOffset(ptr, mid * elementSize);

        if (comparsionFn(ctx, entry)) {
            high = mid;
        } else {
            low = mid + 1;
        }
    }

    if (low > high) {
        return count;
    }

    return low;
}

static bool ImpactCompactUnwindCompareIndexEntry(const void *a, const void *b) {
    const uintptr_t targetOffset = *(uintptr_t *)a;
    const struct unwind_info_section_header_index_entry* index = b;

    const uint32_t functionOffset = index->functionOffset;

    return targetOffset < functionOffset;
}

static bool ImpactCompactUnwindCompareCompressedEntry(const void *a, const void *b) {
    const uintptr_t targetOffset = *(uintptr_t *)a;
    const compact_unwind_encoding_t* encoding = b;

    const uint32_t functionOffset = UNWIND_INFO_COMPRESSED_ENTRY_FUNC_OFFSET(*encoding);

    return targetOffset < functionOffset;
}
