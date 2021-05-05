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
// "DWARF Register Number Mapping" which defines them. There are 130.
//
// Apple's libunwind uses a negative number to represent rip. DWARF defines the CFA in terms
// of a uleb, (ie unsigned), so I believe this is a safe sentinel.
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

    ImpactCPURegister_X86_64_RA = 16

    // there are 130 of these defined, I haven't found a need to define them all yet
} ImpactCPURegister;

enum {
    ImpactCPUDWARFRegisterCount = 17
};

static const ImpactCPURegister ImpactCPURegisterStackPointer = ImpactCPURegister_X86_64_RSP;
static const ImpactCPURegister ImpactCPURegisterInstructionPointer = ImpactCPURegister_X86_64_RIP;
static const ImpactCPURegister ImpactCPURegisterFramePointer = ImpactCPURegister_X86_64_RBP;

static const mach_msg_type_number_t ImpactCPUThreadStateCount = x86_THREAD_STATE64_COUNT;
static const thread_state_flavor_t ImpactCPUThreadStateFlavor = x86_THREAD_STATE64;

static const char* ImpactCPUArchitectureName = "x86_64";
#elif defined(__i386__)
typedef enum {
    ImpactCPURegister_i386_RIP = -1
} ImpactCPURegister;

enum {
    ImpactCPUDWARFRegisterCount = 0
};

static const ImpactCPURegister ImpactCPURegisterStackPointer = ImpactCPURegister_i386_RIP;
static const ImpactCPURegister ImpactCPURegisterInstructionPointer = ImpactCPURegister_i386_RIP;
static const ImpactCPURegister ImpactCPURegisterFramePointer = ImpactCPURegister_i386_RIP;

static const mach_msg_type_number_t ImpactCPUThreadStateCount = x86_THREAD_STATE_COUNT;
static const thread_state_flavor_t ImpactCPUThreadStateFlavor = x86_THREAD_STATE;

static const char* ImpactCPUArchitectureName = "i386";
#elif defined(__arm64__)
// Aarch64 is documented here:
// https://developer.arm.com/docs/ihi0057/c/dwarf-for-the-arm-64-bit-architecture-aarch64-abi-2018q4

typedef enum {
    ImpactCPURegister_ARM64_RIP = -1,

    ImpactCPURegister_ARM64_X0  = 0,
    ImpactCPURegister_ARM64_X1  = 1,
    ImpactCPURegister_ARM64_X2  = 2,
    ImpactCPURegister_ARM64_X3  = 3,
    ImpactCPURegister_ARM64_X4  = 4,
    ImpactCPURegister_ARM64_X5  = 5,
    ImpactCPURegister_ARM64_X6  = 6,
    ImpactCPURegister_ARM64_X7  = 7,
    ImpactCPURegister_ARM64_X8  = 8,
    ImpactCPURegister_ARM64_X9  = 9,
    ImpactCPURegister_ARM64_X10 = 10,
    ImpactCPURegister_ARM64_X11 = 11,
    ImpactCPURegister_ARM64_X12 = 12,
    ImpactCPURegister_ARM64_X13 = 13,
    ImpactCPURegister_ARM64_X14 = 14,
    ImpactCPURegister_ARM64_X15 = 15,
    ImpactCPURegister_ARM64_X16 = 16,
    ImpactCPURegister_ARM64_X17 = 17,
    ImpactCPURegister_ARM64_X18 = 18,
    ImpactCPURegister_ARM64_X19 = 19,
    ImpactCPURegister_ARM64_X20 = 20,
    ImpactCPURegister_ARM64_X21 = 21,
    ImpactCPURegister_ARM64_X22 = 22,
    ImpactCPURegister_ARM64_X23 = 23,
    ImpactCPURegister_ARM64_X24 = 24,
    ImpactCPURegister_ARM64_X25 = 25,
    ImpactCPURegister_ARM64_X26 = 26,
    ImpactCPURegister_ARM64_X27 = 27,
    ImpactCPURegister_ARM64_X28 = 28,
    ImpactCPURegister_ARM64_X29 = 29,
    ImpactCPURegister_ARM64_X30 = 30,
    ImpactCPURegister_ARM64_X31 = 31,

    ImpactCPURegister_ARM64_ELR_mode = 33,
    ImpactCPURegister_ARM64_RA_SIGN_STATE = 34,

    // lots more vector stuff here
} ImpactCPURegister;

enum {
    ImpactCPUDWARFRegisterCount = 35
};

static const ImpactCPURegister ImpactCPURegisterStackPointer = ImpactCPURegister_ARM64_X31;
static const ImpactCPURegister ImpactCPURegisterInstructionPointer = ImpactCPURegister_ARM64_RIP;
static const ImpactCPURegister ImpactCPURegisterFramePointer = ImpactCPURegister_ARM64_X29;
static const ImpactCPURegister ImpactCPURegisterLinkRegister = ImpactCPURegister_ARM64_X30;

static const mach_msg_type_number_t ImpactCPUThreadStateCount = ARM_THREAD_STATE64_COUNT;
static const thread_state_flavor_t ImpactCPUThreadStateFlavor = ARM_THREAD_STATE64;

#if defined(__arm64e__)
static const char* ImpactCPUArchitectureName = "arm64e";
#else
static const char* ImpactCPUArchitectureName = "arm64";
#endif

#elif defined(__arm__) && !defined(__arm64__)
typedef enum {
    ImpactCPURegister_ARMv7_RIP = -1
} ImpactCPURegister;

static const ImpactCPURegister ImpactCPURegisterStackPointer = ImpactCPURegister_ARMv7_RIP;
static const ImpactCPURegister ImpactCPURegisterInstructionPointer = ImpactCPURegister_ARMv7_RIP;
static const ImpactCPURegister ImpactCPURegisterFramePointer = ImpactCPURegister_ARMv7_RIP;

static const mach_msg_type_number_t ImpactCPUThreadStateCount = ARM_THREAD_STATE_COUNT;
static const thread_state_flavor_t ImpactCPUThreadStateFlavor = ARM_THREAD_STATE;

static const char* ImpactCPUArchitectureName = "armv7";
#endif


ImpactResult ImpactCPURegistersLog(ImpactState* state, const ImpactCPURegisters* registers);
ImpactResult ImpactCPUGetRegister(const ImpactCPURegisters* registers, ImpactCPURegister num, uintptr_t* value);
ImpactResult ImpactCPUSetRegister(ImpactCPURegisters* registers, ImpactCPURegister num, uintptr_t value);

#endif /* ImpactCPU_h */
