//
//  ImpactCPU.h
//  Impact
//
//  Created by Matt Massicotte on 2019-09-19.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#ifndef ImpactCPU_h
#define ImpactCPU_h

#include <sys/types.h>

#include "ImpactResult.h"
#include "ImpactState.h"

typedef _STRUCT_MCONTEXT ImpactCPURegisters;

#if defined(__x86_64__)
// These register number values are significant. Their definitions come from
//
// https://software.intel.com/sites/default/files/article/402129/mpx-linux64-abi.pdf
// System V Application Binary Interface AMD64 Architecture Processor Supplement Draft Version 0.3
//
// DWARF uses abstract numbers to name registers. That document has a table labelled
// "DWARF Register Number Mapping" which defines them.
//
// Apple's libunwind uses a negative number to represent rip. I'm not yet sure if DWARF ever uses
// that value. For now, we'll just do the same thing.
typedef enum {
    ImpactCPURegister_X86_64_RIP = -1,

    ImpactCPURegister_X86_64_RAX = 0,
    ImpactCPURegister_X86_64_RDX = 1,
    ImpactCPURegister_X86_64_RCX = 2,
    ImpactCPURegister_X86_64_RBX = 3,
    ImpactCPURegister_X86_64_RSI = 4,
    ImpactCPURegister_X86_64_RDI = 5,
    ImpactCPURegister_X86_64_RBP = 6,
    ImpactCPURegister_X86_64_RSP = 7,
    ImpactCPURegister_X86_64_R8  = 8,
    ImpactCPURegister_X86_64_R9  = 9,
    ImpactCPURegister_X86_64_R10 = 10,
    ImpactCPURegister_X86_64_R11 = 11,
    ImpactCPURegister_X86_64_R12 = 12,
    ImpactCPURegister_X86_64_R13 = 13,
    ImpactCPURegister_X86_64_R14 = 14,
    ImpactCPURegister_X86_64_R15 = 15,
} ImpactCPURegister;

static const ImpactCPURegister ImpactCPURegisterStackPointer = ImpactCPURegister_X86_64_RSP;
static const ImpactCPURegister ImpactCPURegisterInstructionPointer = ImpactCPURegister_X86_64_RIP;
static const ImpactCPURegister ImpactCPURegisterFramePointer = ImpactCPURegister_X86_64_RBP;

static const mach_msg_type_number_t ImpactCPUThreadStateCount = x86_THREAD_STATE64_COUNT;
static const thread_state_flavor_t ImpactCPUThreadStateFlavor = x86_THREAD_STATE64;
#elif defined(__i386__)
typedef enum {
    ImpactCPURegister_i386_RIP = -1
} ImpactCPURegister;

static const ImpactCPURegister ImpactCPURegisterStackPointer = ImpactCPURegister_i386_RIP;
static const ImpactCPURegister ImpactCPURegisterInstructionPointer = ImpactCPURegister_i386_RIP;
static const ImpactCPURegister ImpactCPURegisterFramePointer = ImpactCPURegister_i386_RIP;

static const mach_msg_type_number_t ImpactCPUThreadStateCount = x86_THREAD_STATE_COUNT;
static const thread_state_flavor_t ImpactCPUThreadStateFlavor = x86_THREAD_STATE;
#elif defined(__arm64__)
typedef enum {
    ImpactCPURegister_ARM64_RIP = -1
} ImpactCPURegister;

static const ImpactCPURegister ImpactCPURegisterStackPointer = ImpactCPURegister_ARM64_RIP;
static const ImpactCPURegister ImpactCPURegisterInstructionPointer = ImpactCPURegister_ARM64_RIP;
static const ImpactCPURegister ImpactCPURegisterFramePointer = ImpactCPURegister_ARM64_RIP;

static const mach_msg_type_number_t ImpactCPUThreadStateCount = ARM_THREAD_STATE64_COUNT;
static const thread_state_flavor_t ImpactCPUThreadStateFlavor = ARM_THREAD_STATE;

#elif defined(__arm__) && !defined(__arm64__)
typedef enum {
    ImpactCPURegister_ARMv7_RIP = -1
} ImpactCPURegister;

static const ImpactCPURegister ImpactCPURegisterStackPointer = ImpactCPURegister_ARMv7_RIP;
static const ImpactCPURegister ImpactCPURegisterInstructionPointer = ImpactCPURegister_ARMv7_RIP;
static const ImpactCPURegister ImpactCPURegisterFramePointer = ImpactCPURegister_ARMv7_RIP;

static const mach_msg_type_number_t ImpactCPUThreadStateCount = ARM_THREAD_STATE_COUNT;
static const thread_state_flavor_t ImpactCPUThreadStateFlavor = ARM_THREAD_STATE;
#endif


ImpactResult ImpactCPURegistersLog(ImpactState* state, const ImpactCPURegisters* registers);
ImpactResult ImpactCPUGetRegister(const ImpactCPURegisters* registers, ImpactCPURegister num, uintptr_t* value);
ImpactResult ImpactCPUSetRegister(ImpactCPURegisters* registers, ImpactCPURegister num, uintptr_t value);

#endif /* ImpactCPU_h */
