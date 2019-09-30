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
    if (ImpactInvalidPtr(state) || ImpactInvalidPtr(registers)) {
        return ImpactResultPointerInvalid;
    }

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
#endif

    ImpactLogWriteString(log, "\n");


    return ImpactResultSuccess;
}

ImpactResult ImpactCPUGetRegister(const ImpactCPURegisters* registers, ImpactCPURegister num, uintptr_t* value) {
    if (ImpactInvalidPtr(registers) || ImpactInvalidPtr(value)) {
        return ImpactResultPointerInvalid;
    }
    
#if defined(__x86_64__)
    switch (num) {
        case ImpactCPURegister_X86_64_RAX: *value = registers->__ss.__rax; break;
        case ImpactCPURegister_X86_64_RDX: *value = registers->__ss.__rdx; break;
        case ImpactCPURegister_X86_64_RCX: *value = registers->__ss.__rcx; break;
        case ImpactCPURegister_X86_64_RBX: *value = registers->__ss.__rbx; break;
        case ImpactCPURegister_X86_64_RSI: *value = registers->__ss.__rsi; break;
        case ImpactCPURegister_X86_64_RDI: *value = registers->__ss.__rdi; break;
        case ImpactCPURegister_X86_64_RBP: *value = registers->__ss.__rbp; break;
        case ImpactCPURegister_X86_64_RSP: *value = registers->__ss.__rsp; break;
        case ImpactCPURegister_X86_64_R8:  *value = registers->__ss.__r8;  break;
        case ImpactCPURegister_X86_64_R9:  *value = registers->__ss.__r9;  break;
        case ImpactCPURegister_X86_64_R10: *value = registers->__ss.__r10; break;
        case ImpactCPURegister_X86_64_R11: *value = registers->__ss.__r11; break;
        case ImpactCPURegister_X86_64_R12: *value = registers->__ss.__r12; break;
        case ImpactCPURegister_X86_64_R13: *value = registers->__ss.__r13; break;
        case ImpactCPURegister_X86_64_R14: *value = registers->__ss.__r14; break;
        case ImpactCPURegister_X86_64_R15: *value = registers->__ss.__r15; break;
        case ImpactCPURegister_X86_64_RIP: *value = registers->__ss.__rip; break;
        default:
            return ImpactResultArgumentInvalid;
    }
#endif

    return ImpactResultSuccess;
}

ImpactResult ImpactCPUSetRegister(ImpactCPURegisters* registers, ImpactCPURegister num, uintptr_t value) {
    if (ImpactInvalidPtr(registers)) {
        return ImpactResultPointerInvalid;
    }

#if defined(__x86_64__)
    switch (num) {
        case ImpactCPURegister_X86_64_RAX: registers->__ss.__rax = value; break;
        case ImpactCPURegister_X86_64_RDX: registers->__ss.__rdx = value; break;
        case ImpactCPURegister_X86_64_RCX: registers->__ss.__rcx = value; break;
        case ImpactCPURegister_X86_64_RBX: registers->__ss.__rbx = value; break;
        case ImpactCPURegister_X86_64_RSI: registers->__ss.__rsi = value; break;
        case ImpactCPURegister_X86_64_RDI: registers->__ss.__rdi = value; break;
        case ImpactCPURegister_X86_64_RBP: registers->__ss.__rbp = value; break;
        case ImpactCPURegister_X86_64_RSP: registers->__ss.__rsp = value; break;
        case ImpactCPURegister_X86_64_R8:  registers->__ss.__r8  = value; break;
        case ImpactCPURegister_X86_64_R9:  registers->__ss.__r9  = value; break;
        case ImpactCPURegister_X86_64_R10: registers->__ss.__r10 = value; break;
        case ImpactCPURegister_X86_64_R11: registers->__ss.__r11 = value; break;
        case ImpactCPURegister_X86_64_R12: registers->__ss.__r12 = value; break;
        case ImpactCPURegister_X86_64_R13: registers->__ss.__r13 = value; break;
        case ImpactCPURegister_X86_64_R14: registers->__ss.__r14 = value; break;
        case ImpactCPURegister_X86_64_R15: registers->__ss.__r15 = value; break;
        case ImpactCPURegister_X86_64_RIP: registers->__ss.__rip = value; break;
        default:
            return ImpactResultArgumentInvalid;
    }
#endif

    return ImpactResultSuccess;
}
