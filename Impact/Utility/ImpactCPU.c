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

#if defined(__arm64__)
// These functions are all setup to mirror, roughly, what the macros in arm/_structs.h work. And wow
// is it messy.
//
// Note that I feel like the key for LR should be ptrauth_key_return_address and FP should be
// ptrauth_key_frame_pointer. But, they are not defined that way, so this code matches the SDK.
// Filed FB7805236 in case that's wrong. But, I find it pretty confusing either way.

#if !__has_feature(ptrauth_calls)

// We need to use pointer-authentication-aware macros to work correctly on arm64e. However,
// it is possible that we are built arm64, but have to interact with arm64e executables. In that
// case, we have to fallback to using a mask as the supplied macros will be disabled.

#undef ptrauth_strip
#undef ptrauth_sign_unauthenticated

static const uintptr_t ImpactCPURegisterARM64PACMask = 0x0000000fffffffff;

#define ptrauth_strip(value, key) (void*)((uintptr_t)value & ImpactCPURegisterARM64PACMask)
#define ptrauth_sign_unauthenticated(value, key, data) (value)
#endif

#define ImpactCPURegisterResignPointer(value, key, str) ptrauth_sign_unauthenticated(ptrauth_strip(value, key), key, ptrauth_string_discriminator(str))

static uintptr_t ImpactCPURegisterARM64GetFP(_STRUCT_ARM_THREAD_STATE64 threadState) {
    const uintptr_t value = arm_thread_state64_get_fp(threadState);

    return (uintptr_t)ptrauth_strip((void *)value, ptrauth_key_process_independent_data);
}

static void ImpactCPURegisterARM64SetFP(_STRUCT_ARM_THREAD_STATE64* state, uintptr_t value) {
    void* ptr = ImpactCPURegisterResignPointer((void*)value, ptrauth_key_process_independent_data, "fp");
    
    arm_thread_state64_set_fp(*state, ptr);
}

static uintptr_t ImpactCPURegisterARM64GetLR(_STRUCT_ARM_THREAD_STATE64 threadState) {
    const uintptr_t value = arm_thread_state64_get_lr(threadState);

    return (uintptr_t)ptrauth_strip((void *)value, ptrauth_key_process_independent_code);
}

static void ImpactCPURegisterARM64SetLR(_STRUCT_ARM_THREAD_STATE64* state, uintptr_t value) {
    void* ptr = ImpactCPURegisterResignPointer((void*)value, ptrauth_key_process_independent_code, "lr");

    arm_thread_state64_set_lr_fptr(*state, ptr);
}

static uintptr_t ImpactCPURegisterARM64GetSP(_STRUCT_ARM_THREAD_STATE64 threadState) {
    const uintptr_t value = arm_thread_state64_get_sp(threadState);

    return (uintptr_t)ptrauth_strip((void *)value, ptrauth_key_process_independent_data);
}

static void ImpactCPURegisterARM64SetSP(_STRUCT_ARM_THREAD_STATE64* state, uintptr_t value) {
    void* ptr = ImpactCPURegisterResignPointer((void*)value, ptrauth_key_process_independent_data, "sp");

    arm_thread_state64_set_sp(*state, ptr);
}

static uintptr_t ImpactCPURegisterARM64GetPC(_STRUCT_ARM_THREAD_STATE64 threadState) {
    const uintptr_t value = arm_thread_state64_get_pc(threadState);

    return (uintptr_t)ptrauth_strip((void *)value, ptrauth_key_process_independent_code);
}

static void ImpactCPURegisterARM64SetPC(_STRUCT_ARM_THREAD_STATE64* state, uintptr_t value) {
    void* ptr = ImpactCPURegisterResignPointer((void*)value, ptrauth_key_process_independent_code, "pc");

    arm_thread_state64_set_pc_fptr(*state, ptr);
}
#endif

ImpactResult ImpactCPURegistersLog(ImpactState* state, const ImpactCPURegisters* registers) {
    if (ImpactInvalidPtr(state) || ImpactInvalidPtr(registers)) {
        return ImpactResultPointerInvalid;
    }

    ImpactLogger* log = &state->constantState.log;

    ImpactLogWriteString(log, "[Thread:State] ");

#if defined(__x86_64__)
    ImpactLogWriteKeyInteger(log, "rax", registers->__ss.__rax, false);
    ImpactLogWriteKeyInteger(log, "rbx", registers->__ss.__rbx, false);
    ImpactLogWriteKeyInteger(log, "rcx", registers->__ss.__rcx, false);
    ImpactLogWriteKeyInteger(log, "rdx", registers->__ss.__rdx, false);
    ImpactLogWriteKeyInteger(log, "rdi", registers->__ss.__rdi, false);
    ImpactLogWriteKeyInteger(log, "rsi", registers->__ss.__rsi, false);
    ImpactLogWriteKeyInteger(log, "rbp", registers->__ss.__rbp, false);
    ImpactLogWriteKeyInteger(log, "rsp", registers->__ss.__rsp, false);
    ImpactLogWriteKeyInteger(log, "r8", registers->__ss.__r8, false);
    ImpactLogWriteKeyInteger(log, "r9", registers->__ss.__r9, false);
    ImpactLogWriteKeyInteger(log, "r10", registers->__ss.__r10, false);
    ImpactLogWriteKeyInteger(log, "r11", registers->__ss.__r11, false);
    ImpactLogWriteKeyInteger(log, "r12", registers->__ss.__r12, false);
    ImpactLogWriteKeyInteger(log, "r13", registers->__ss.__r13, false);
    ImpactLogWriteKeyInteger(log, "r14", registers->__ss.__r14, false);
    ImpactLogWriteKeyInteger(log, "r15", registers->__ss.__r15, false);
    ImpactLogWriteKeyInteger(log, "rip", registers->__ss.__rip, false);
    ImpactLogWriteKeyInteger(log, "rflags", registers->__ss.__rflags, false);
    ImpactLogWriteKeyInteger(log, "cs", registers->__ss.__cs, false);
    ImpactLogWriteKeyInteger(log, "fs", registers->__ss.__fs, false);
    ImpactLogWriteKeyInteger(log, "gs", registers->__ss.__gs, true);
#elif defined(__arm64__)
    ImpactLogWriteKeyInteger(log, "x0", registers->__ss.__x[0], false);
    ImpactLogWriteKeyInteger(log, "x1", registers->__ss.__x[1], false);
    ImpactLogWriteKeyInteger(log, "x2", registers->__ss.__x[2], false);
    ImpactLogWriteKeyInteger(log, "x3", registers->__ss.__x[3], false);
    ImpactLogWriteKeyInteger(log, "x4", registers->__ss.__x[4], false);
    ImpactLogWriteKeyInteger(log, "x5", registers->__ss.__x[5], false);
    ImpactLogWriteKeyInteger(log, "x6", registers->__ss.__x[6], false);
    ImpactLogWriteKeyInteger(log, "x7", registers->__ss.__x[7], false);
    ImpactLogWriteKeyInteger(log, "x8", registers->__ss.__x[8], false);
    ImpactLogWriteKeyInteger(log, "x9", registers->__ss.__x[9], false);
    ImpactLogWriteKeyInteger(log, "x10", registers->__ss.__x[10], false);
    ImpactLogWriteKeyInteger(log, "x11", registers->__ss.__x[11], false);
    ImpactLogWriteKeyInteger(log, "x12", registers->__ss.__x[12], false);
    ImpactLogWriteKeyInteger(log, "x13", registers->__ss.__x[13], false);
    ImpactLogWriteKeyInteger(log, "x14", registers->__ss.__x[14], false);
    ImpactLogWriteKeyInteger(log, "x15", registers->__ss.__x[15], false);
    ImpactLogWriteKeyInteger(log, "x16", registers->__ss.__x[16], false);
    ImpactLogWriteKeyInteger(log, "x17", registers->__ss.__x[17], false);
    ImpactLogWriteKeyInteger(log, "x18", registers->__ss.__x[18], false);
    ImpactLogWriteKeyInteger(log, "x19", registers->__ss.__x[19], false);
    ImpactLogWriteKeyInteger(log, "x20", registers->__ss.__x[20], false);
    ImpactLogWriteKeyInteger(log, "x21", registers->__ss.__x[21], false);
    ImpactLogWriteKeyInteger(log, "x22", registers->__ss.__x[22], false);
    ImpactLogWriteKeyInteger(log, "x23", registers->__ss.__x[23], false);
    ImpactLogWriteKeyInteger(log, "x24", registers->__ss.__x[24], false);
    ImpactLogWriteKeyInteger(log, "x25", registers->__ss.__x[25], false);
    ImpactLogWriteKeyInteger(log, "x26", registers->__ss.__x[26], false);
    ImpactLogWriteKeyInteger(log, "x27", registers->__ss.__x[27], false);
    ImpactLogWriteKeyInteger(log, "x28", registers->__ss.__x[28], false);

    // These values must be accessed via the handy pointer-authentication-aware macros to
    // to work correctly on arm64e
    ImpactLogWriteKeyInteger(log, "fp", ImpactCPURegisterARM64GetFP(registers->__ss), false);
    ImpactLogWriteKeyInteger(log, "lr", ImpactCPURegisterARM64GetLR(registers->__ss), false);
    ImpactLogWriteKeyInteger(log, "sp", ImpactCPURegisterARM64GetSP(registers->__ss), false);
    ImpactLogWriteKeyInteger(log, "pc", ImpactCPURegisterARM64GetPC(registers->__ss), true);
#endif

    return ImpactResultSuccess;
}

ImpactResult ImpactCPUGetRegister(const ImpactCPURegisters* registers, ImpactCPURegister num, uintptr_t* value) {
    if (ImpactInvalidPtr(registers) || ImpactInvalidPtr(value)) {
        ImpactDebugLog("[Log:WARN] %s pointer argument invalid\n", __func__);
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
            ImpactDebugLog("[Log:WARN] %s register number unsupported %d\n", __func__, num);
            return ImpactResultArgumentInvalid;
    }
#elif defined(__arm64__)
    switch (num) {
        case ImpactCPURegister_ARM64_X0:  *value = registers->__ss.__x[0]; break;
        case ImpactCPURegister_ARM64_X1:  *value = registers->__ss.__x[1]; break;
        case ImpactCPURegister_ARM64_X2:  *value = registers->__ss.__x[2]; break;
        case ImpactCPURegister_ARM64_X3:  *value = registers->__ss.__x[3]; break;
        case ImpactCPURegister_ARM64_X4:  *value = registers->__ss.__x[4]; break;
        case ImpactCPURegister_ARM64_X5:  *value = registers->__ss.__x[5]; break;
        case ImpactCPURegister_ARM64_X6:  *value = registers->__ss.__x[6]; break;
        case ImpactCPURegister_ARM64_X7:  *value = registers->__ss.__x[7]; break;
        case ImpactCPURegister_ARM64_X8:  *value = registers->__ss.__x[8]; break;
        case ImpactCPURegister_ARM64_X9:  *value = registers->__ss.__x[9]; break;
        case ImpactCPURegister_ARM64_X10: *value = registers->__ss.__x[10]; break;
        case ImpactCPURegister_ARM64_X11: *value = registers->__ss.__x[11]; break;
        case ImpactCPURegister_ARM64_X12: *value = registers->__ss.__x[12]; break;
        case ImpactCPURegister_ARM64_X13: *value = registers->__ss.__x[13]; break;
        case ImpactCPURegister_ARM64_X14: *value = registers->__ss.__x[14]; break;
        case ImpactCPURegister_ARM64_X15: *value = registers->__ss.__x[15]; break;
        case ImpactCPURegister_ARM64_X16: *value = registers->__ss.__x[16]; break;
        case ImpactCPURegister_ARM64_X17: *value = registers->__ss.__x[17]; break;
        case ImpactCPURegister_ARM64_X18: *value = registers->__ss.__x[18]; break;
        case ImpactCPURegister_ARM64_X19: *value = registers->__ss.__x[19]; break;
        case ImpactCPURegister_ARM64_X20: *value = registers->__ss.__x[20]; break;
        case ImpactCPURegister_ARM64_X21: *value = registers->__ss.__x[21]; break;
        case ImpactCPURegister_ARM64_X22: *value = registers->__ss.__x[22]; break;
        case ImpactCPURegister_ARM64_X23: *value = registers->__ss.__x[23]; break;
        case ImpactCPURegister_ARM64_X24: *value = registers->__ss.__x[24]; break;
        case ImpactCPURegister_ARM64_X25: *value = registers->__ss.__x[25]; break;
        case ImpactCPURegister_ARM64_X26: *value = registers->__ss.__x[26]; break;
        case ImpactCPURegister_ARM64_X27: *value = registers->__ss.__x[27]; break;
        case ImpactCPURegister_ARM64_X28: *value = registers->__ss.__x[28]; break;

        // see note on register accesss in ImpactCPURegistersLog
        case ImpactCPURegister_ARM64_X29: *value = ImpactCPURegisterARM64GetFP(registers->__ss); break;
        case ImpactCPURegister_ARM64_X30: *value = ImpactCPURegisterARM64GetLR(registers->__ss); break;
        case ImpactCPURegister_ARM64_X31:  *value = ImpactCPURegisterARM64GetSP(registers->__ss); break;
        case ImpactCPURegister_ARM64_RIP: *value = ImpactCPURegisterARM64GetPC(registers->__ss); break;
        default:
            ImpactDebugLog("[Log:WARN] %s register number unsupported %30d\n", __func__, num);
            return ImpactResultArgumentInvalid;
    }
#endif

    return ImpactResultSuccess;
}

ImpactResult ImpactCPUSetRegister(ImpactCPURegisters* registers, ImpactCPURegister num, uintptr_t value) {
    if (ImpactInvalidPtr(registers)) {
        ImpactDebugLog("[Log:WARN] %s pointer argument invalid\n", __func__);
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
        case ImpactCPURegister_X86_64_RA:  registers->__ss.__rip = value; break;
        default:
            ImpactDebugLog("[Log:WARN] %s register number unsupported %d\n", __func__, num);
            return ImpactResultArgumentInvalid;
    }
#elif defined(__arm64__)
    switch (num) {
        case ImpactCPURegister_ARM64_X0:  registers->__ss.__x[0]  = value; break;
        case ImpactCPURegister_ARM64_X1:  registers->__ss.__x[1]  = value; break;
        case ImpactCPURegister_ARM64_X2:  registers->__ss.__x[2]  = value; break;
        case ImpactCPURegister_ARM64_X3:  registers->__ss.__x[3]  = value; break;
        case ImpactCPURegister_ARM64_X4:  registers->__ss.__x[4]  = value; break;
        case ImpactCPURegister_ARM64_X5:  registers->__ss.__x[5]  = value; break;
        case ImpactCPURegister_ARM64_X6:  registers->__ss.__x[6]  = value; break;
        case ImpactCPURegister_ARM64_X7:  registers->__ss.__x[7]  = value; break;
        case ImpactCPURegister_ARM64_X8:  registers->__ss.__x[8]  = value; break;
        case ImpactCPURegister_ARM64_X9:  registers->__ss.__x[9]  = value; break;
        case ImpactCPURegister_ARM64_X10: registers->__ss.__x[10] = value; break;
        case ImpactCPURegister_ARM64_X11: registers->__ss.__x[11] = value; break;
        case ImpactCPURegister_ARM64_X12: registers->__ss.__x[12] = value; break;
        case ImpactCPURegister_ARM64_X13: registers->__ss.__x[13] = value; break;
        case ImpactCPURegister_ARM64_X14: registers->__ss.__x[14] = value; break;
        case ImpactCPURegister_ARM64_X15: registers->__ss.__x[15] = value; break;
        case ImpactCPURegister_ARM64_X16: registers->__ss.__x[16] = value; break;
        case ImpactCPURegister_ARM64_X17: registers->__ss.__x[17] = value; break;
        case ImpactCPURegister_ARM64_X18: registers->__ss.__x[18] = value; break;
        case ImpactCPURegister_ARM64_X19: registers->__ss.__x[19] = value; break;
        case ImpactCPURegister_ARM64_X20: registers->__ss.__x[20] = value; break;
        case ImpactCPURegister_ARM64_X21: registers->__ss.__x[21] = value; break;
        case ImpactCPURegister_ARM64_X22: registers->__ss.__x[22] = value; break;
        case ImpactCPURegister_ARM64_X23: registers->__ss.__x[23] = value; break;
        case ImpactCPURegister_ARM64_X24: registers->__ss.__x[24] = value; break;
        case ImpactCPURegister_ARM64_X25: registers->__ss.__x[25] = value; break;
        case ImpactCPURegister_ARM64_X26: registers->__ss.__x[26] = value; break;
        case ImpactCPURegister_ARM64_X27: registers->__ss.__x[27] = value; break;
        case ImpactCPURegister_ARM64_X28: registers->__ss.__x[28] = value; break;

            // see note on register accesss in ImpactCPURegistersLog
        case ImpactCPURegister_ARM64_X29: ImpactCPURegisterARM64SetFP(&registers->__ss, value); break;
        case ImpactCPURegister_ARM64_X30: ImpactCPURegisterARM64SetLR(&registers->__ss, value); break;
        case ImpactCPURegister_ARM64_X31: ImpactCPURegisterARM64SetSP(&registers->__ss, value); break;
        case ImpactCPURegister_ARM64_RIP: ImpactCPURegisterARM64SetPC(&registers->__ss, value); break;
        default:
            ImpactDebugLog("[Log:WARN] %s register number unsupported %d\n", __func__, num);
            return ImpactResultArgumentInvalid;
    }
#endif

    return ImpactResultSuccess;
}
