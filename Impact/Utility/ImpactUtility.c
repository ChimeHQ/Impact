//
//  ImpactUtility.c
//  Impact
//
//  Created by Matt Massicotte on 2019-10-06.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#include "ImpactUtility.h"

#include <unistd.h>
#include <sys/sysctl.h>

// code from:
// https://developer.apple.com/library/archive/qa/qa1361/_index.html
bool ImpactDebuggerAttached(void) {
    int mib[4] = {0};
    struct kinfo_proc info = {0};

    info.kp_proc.p_flag = 0;

    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PID;
    mib[3] = getpid();

    size_t size = sizeof(info);
    const int ret = sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, NULL, 0);
    if (ret != 0) {
        return true;
    }

    return ( (info.kp_proc.p_flag & P_TRACED) != 0 );
}

ImpactResult ImpactReadMemory(uintptr_t address, size_t size, void* buffer) {
    if (ImpactInvalidPtr((void*)address)) {
        return ImpactResultPointerInvalid;
    }

    if (ImpactInvalidPtr(buffer)) {
        return ImpactResultPointerInvalid;
    }

    *(uintptr_t *)buffer = *(uintptr_t*)address;

    return ImpactResultSuccess;
}
