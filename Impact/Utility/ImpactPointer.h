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
#include <stddef.h>
#include <mach/machine/vm_types.h>
#include <mach/vm_page_size.h>

// the upper half of the address space is all kernel stuff, so
// we cannot have a pointer to that region
const static uintptr_t ImpactPointerMax = UINTPTR_MAX / 2;

#define ImpactPointerMin vm_kernel_page_size

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

#endif /* ImpactPointer_h */
