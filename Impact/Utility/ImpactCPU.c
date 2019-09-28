//
//  ImpactCPU.c
//  Impact
//
//  Created by Matt Massicotte on 2019-09-19.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#include "ImpactCPU.h"
#include "ImpactUtility.h"
#include "ImpactLog.h"

ImpactResult ImpactCPURegistersLog(ImpactState* state, const ImpactCPURegisters* registers) {
    ImpactLogger* log = &state->constantState.log;

    ImpactLogWriteString(log, "[Thread:State] ");

#if defined(__x86_64__)
    ImpactLogWriteKeyInteger(log, "rax", registers->__ss.__rax);
    ImpactLogWriteKeyInteger(log, "rbx", registers->__ss.__rbx);
    ImpactLogWriteKeyInteger(log, "rcx", registers->__ss.__rcx);
    ImpactLogWriteKeyInteger(log, "rdx", registers->__ss.__rdx);
    ImpactLogWriteKeyInteger(log, "rdi", registers->__ss.__rdi);
    ImpactLogWriteKeyInteger(log, "rsi", registers->__ss.__rsi);
    ImpactLogWriteKeyInteger(log, "rbp", registers->__ss.__rbp);
    ImpactLogWriteKeyInteger(log, "rsp", registers->__ss.__rsp);
    ImpactLogWriteKeyInteger(log, "r8", registers->__ss.__r8);
    ImpactLogWriteKeyInteger(log, "r9", registers->__ss.__r9);
    ImpactLogWriteKeyInteger(log, "r10", registers->__ss.__r10);
    ImpactLogWriteKeyInteger(log, "r11", registers->__ss.__r11);
    ImpactLogWriteKeyInteger(log, "r12", registers->__ss.__r12);
    ImpactLogWriteKeyInteger(log, "r13", registers->__ss.__r13);
    ImpactLogWriteKeyInteger(log, "r14", registers->__ss.__r14);
    ImpactLogWriteKeyInteger(log, "r15", registers->__ss.__r15);
    ImpactLogWriteKeyInteger(log, "rip", registers->__ss.__rip);
    ImpactLogWriteKeyInteger(log, "rflags", registers->__ss.__rflags);
    ImpactLogWriteKeyInteger(log, "cs", registers->__ss.__cs);
    ImpactLogWriteKeyInteger(log, "fs", registers->__ss.__fs);
    ImpactLogWriteKeyInteger(log, "gs", registers->__ss.__gs);
#else
    #error "Architecture not supported"
#endif

    ImpactLogWriteString(log, "\n");


    return ImpactResultSuccess;
}
