#ifndef ImpactState_h
#define ImpactState_h

#include "ImpactDebug.h"

#include <signal.h>
#include <stdatomic.h>
#include <mach/exc.h>
#include <mach-o/dyld_images.h>
#include <stdbool.h>

enum { ImpactLogBufferSize = 256 };

typedef struct {
    int fd;
    uint32_t bufferCount;
    char buffer[ImpactLogBufferSize];
} ImpactLogger;

enum { ImpactSignalCount = 5 };

typedef struct {
    exception_mask_t masks[EXC_TYPES_COUNT];
    exception_handler_t handlers[EXC_TYPES_COUNT];
    exception_behavior_t behaviors[EXC_TYPES_COUNT];
    thread_state_flavor_t flavors[EXC_TYPES_COUNT];

    mach_msg_type_number_t count;
} ImpactMachExceptionHandlers;

typedef enum {
    ImpactCrashStateUninitialized = 0,
    ImpactCrashStateInitialized,
    ImpactCrashStateMachException,
    ImpactCrashStateMachExceptionReplied,
    ImpactCrashStateSignal,
    ImpactCrashStateSignalHandled,
    ImpactCrashStateSignalAfterMachException,
    ImpactCrashStateSignalHandledAfterMachException,
    ImpactCrashStateSignalAfterMachExceptionReplied,
    ImpactCrashStateMachExceptionAfterSignal,
    ImpactCrashStateMachExceptionAfterSignalHandled,
    ImpactCrashStateSecondMachException,
    ImpactCrashStateSecondMachExceptionReplied,
    ImpactCrashStateSecondSignal,
    ImpactCrashStateSecondSignalHandled
} ImpactCrashState;

typedef struct {
    struct task_dyld_info dyldInfo;
    uint32_t writtenIndex;
    uint32_t lastFoundIndex;
} ImpactBinaryImages;

typedef struct {
    // signals
    struct sigaction preexistingActions[ImpactSignalCount];

    // mach exception
    ImpactMachExceptionHandlers preexistingMachExceptionHandlers;
    mach_port_t machExceptionPort;

    // general configuration
    bool suppressReportCrash;

    void* preexistingNSExceptionHandler;
} ImpactConstantState;

typedef struct {
    ImpactLogger log;
    ImpactBinaryImages images;

    _Atomic ImpactCrashState crashState;
    _Atomic uint32_t exceptionCount;
} ImpactMutableState;

typedef struct {
    ImpactConstantState constantState;
    ImpactMutableState mutableState;
} ImpactState;

extern ImpactState* GlobalImpactState;

static inline ImpactLogger* ImpactStateGetLog(ImpactState* state) {
    return &state->mutableState.log;
}

#include <unistd.h>

void ImpactStateTransitionCtx(ImpactState* state, const char* logContext, ImpactCrashState expectedState, ImpactCrashState newState);
_Noreturn void ImpactStateInvalidCtx(const char* logContext, ImpactCrashState invalidState);

#define ImpactStateTransition(state, expectedState, newState) ImpactStateTransitionCtx(state, __func__, expectedState, newState)
#define ImpactStateInvalid(invalidState) ImpactStateInvalidCtx(__func__, invalidState)

#endif /* ImpactState_h */
