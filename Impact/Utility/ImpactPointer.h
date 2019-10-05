//
//  ImpactPointer.h
//  Impact
//
//  Created by Matt Massicotte on 2019-09-18.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#ifndef ImpactPointer_h
#define ImpactPointer_h

#include <sys/types.h>
#include <stdbool.h>
#include <mach/machine/vm_types.h>

// the upper half of the address space is all kernel stuff, so
// we cannot have a pointer to that region
const static uintptr_t ImpactPointerMax = UINTPTR_MAX / 2;
const static uintptr_t ImpactPointerMin = PAGE_SIZE;

static inline bool ImpactInvalidPtr(const void * const ptr) {
    return ((uintptr_t)ptr < ImpactPointerMin) || ((uintptr_t)ptr > ImpactPointerMax);
}

static inline void* ImpactPointerOffset(const void* ptr, uintptr_t offset) {
    void* result = (void*)((uintptr_t)ptr + offset);

    if (ImpactInvalidPtr(result)) {
        return NULL;
    }

    return result;
}

static inline bool ImpactReadMemory(uintptr_t address, uintptr_t* value) {
    if (ImpactInvalidPtr((void*)address)) {
        return false;
    }

    if (ImpactInvalidPtr(value)) {
        return false;
    }

    *value = *(uintptr_t*)address;

    return true;
}

#endif /* ImpactPointer_h */
