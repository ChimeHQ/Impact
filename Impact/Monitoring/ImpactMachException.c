//
//  ImpactMachException.c
//  Impact
//
//  Created by Matt Massicotte on 2019-09-19.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#include "ImpactMachException.h"
#include "ImpactUtility.h"
#include "ImpactLog.h"
#include "ImpactSignal.h"
#include "ImpactState.h"
#include "ImpactCrashHandler.h"

#include <mach/task.h>
#include <mach/mach_port.h>
#include <mach/mach_init.h>
#include <mach/exc.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#ifdef  __MigPackStructs
#pragma pack(push, 4)
#endif

typedef struct {
    mach_msg_header_t Head;
    mach_msg_body_t msgh_body;
    mach_msg_port_descriptor_t thread;
    mach_msg_port_descriptor_t task;
    NDR_record_t NDR;
    exception_type_t exception;
    mach_msg_type_number_t codeCnt;
    int64_t code[2];
} ImpactMachExceptionRaiseRequest;

typedef struct {
    mach_msg_header_t Head;
    NDR_record_t NDR;
    exception_type_t exception;
    mach_msg_type_number_t codeCnt;
    int64_t code[2];
    int flavor;
    mach_msg_type_number_t old_stateCnt;
    natural_t old_state[614];
} ImpactMachExceptionRaiseStateRequest;

typedef struct {
    mach_msg_header_t Head;
    mach_msg_body_t msgh_body;
    mach_msg_port_descriptor_t thread;
    mach_msg_port_descriptor_t task;
    NDR_record_t NDR;
    exception_type_t exception;
    mach_msg_type_number_t codeCnt;
    int64_t code[2];
    int flavor;
    mach_msg_type_number_t old_stateCnt;
    natural_t old_state[614];
} ImpactMachExceptionRaiseStateIdentityRequest;

// Here's an interesting problem. The kernel will reply with MACH_RCV_TOO_LARGE even if
// you register a behavior that doesn't need all the data from the larger replies. This
// union makes it possible to define a structure that can hold all possible replies.
typedef union {
    ImpactMachExceptionRaiseRequest raise;
    ImpactMachExceptionRaiseStateRequest raiseState;
    ImpactMachExceptionRaiseStateIdentityRequest raiseStateIdentity;
} ImpactMachExceptionAllRaiseRequest;

#ifdef __MigPackStructs
#pragma pack(pop)
#endif

static const exception_behavior_t ImpactMachExceptionBehavior = (EXCEPTION_DEFAULT | MACH_EXCEPTION_CODES);

typedef struct {
    exception_mask_t mask;
    exception_handler_t handlerPort;
    exception_behavior_t behavior;
    thread_state_flavor_t flavor;
} ImpactMachExceptionHandler;

static void* ImpactMachExceptionServer(void* ctx);

static ImpactResult ImpactMachExceptionSetupThread(void* ctx) {
    pthread_t thread;
    pthread_attr_t attrs;

    int result = pthread_attr_init(&attrs);
    if (result != 0) {
        return ImpactResultFailure;
    }

    result = pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
    if (result != 0) {
        return ImpactResultFailure;
    }

    result = pthread_create(&thread, &attrs, ImpactMachExceptionServer, ctx);

    pthread_attr_destroy(&attrs);

    return result == 0 ? ImpactResultSuccess : ImpactResultFailure;
}

static ImpactResult ImpactMachExceptionGetHandler(const ImpactState* state, uint i, ImpactMachExceptionHandler* handler) {
    const ImpactMachExceptionHandlers* prexistingHandlers = &state->constantState.preexistingMachExceptionHandlers;

    if (i >= prexistingHandlers->count) {
        return ImpactResultArgumentInvalid;
    }

    handler->behavior = prexistingHandlers->behaviors[i];
    handler->mask = prexistingHandlers->masks[i];
    handler->handlerPort = prexistingHandlers->handlers[i];
    handler->flavor = prexistingHandlers->flavors[i];

    return ImpactResultSuccess;
}

ImpactResult ImpactMachExceptionLogPreexistingHandlers(const ImpactState* state) {
    if (ImpactInvalidPtr(state)) {
        return ImpactResultArgumentInvalid;
    }

    const mach_msg_type_number_t count = state->constantState.preexistingMachExceptionHandlers.count;

    for (mach_msg_type_number_t i = 0; i < count; ++i) {
        ImpactMachExceptionHandler handler = {0};

        const ImpactResult result = ImpactMachExceptionGetHandler(state, i, &handler);
        if (result != ImpactResultSuccess) {
            ImpactDebugLog("[Log:WARN:%s] unable to read preexisting handler %d\n", __func__, result);
            continue;
        }

        if (!MACH_PORT_VALID(handler.handlerPort)) {
            continue;
        }

        ImpactDebugLog("[Log:INFO:%s] preexisting handler %d - %x, %x\n", __func__, i, handler.mask, handler.behavior);
    }

    return ImpactResultSuccess;
}

ImpactResult ImpactMachExceptionInitialize(ImpactState* state) {
    memset(&state->constantState.preexistingMachExceptionHandlers, 0, sizeof(ImpactMachExceptionHandlers));

    mach_port_t* port = &state->constantState.machExceptionPort;
    const task_t task = mach_task_self();

    kern_return_t kr = mach_port_allocate(task, MACH_PORT_RIGHT_RECEIVE, port);
    if (kr != KERN_SUCCESS) {
        ImpactDebugLog("[Log:%s] unable to allocate port %d\n", __func__, kr);
        return ImpactResultFailure;
    }

    kr = mach_port_insert_right(task, *port, *port, MACH_MSG_TYPE_MAKE_SEND);
    if (kr != KERN_SUCCESS) {
        ImpactDebugLog("[Log:%s] unable to insert port right %d\n", __func__, kr);
        return ImpactResultFailure;
    }

    ImpactResult result = ImpactMachExceptionSetupThread(state);
    if (result != ImpactResultSuccess) {
        ImpactDebugLog("[Log:%s] unable to start thread %d\n", __func__, result);
        return ImpactResultFailure;
    }

    const exception_mask_t mask = EXC_MASK_BAD_ACCESS | EXC_MASK_BAD_INSTRUCTION | EXC_MASK_ARITHMETIC | EXC_MASK_SOFTWARE | EXC_MASK_BREAKPOINT | EXC_MASK_GUARD;

    ImpactMachExceptionHandlers* prexistingHandlers = &state->constantState.preexistingMachExceptionHandlers;

    // on input, max size of thees arrays, on output, actual array count
    prexistingHandlers->count = EXC_TYPES_COUNT;

    kr = task_swap_exception_ports(task,
                                   mask,
                                   *port,
                                   ImpactMachExceptionBehavior,
                                   THREAD_STATE_NONE,
                                   prexistingHandlers->masks,
                                   &prexistingHandlers->count,
                                   prexistingHandlers->handlers,
                                   prexistingHandlers->behaviors,
                                   prexistingHandlers->flavors);

    if (kr != KERN_SUCCESS) {
        ImpactDebugLog("[Log:%s] unable to install handler %d\n", __func__, kr);
        return ImpactResultFailure;
    }

    ImpactMachExceptionLogPreexistingHandlers(state);

    return ImpactResultSuccess;
}

ImpactResult ImpactMachExceptionRestorePreexisting(const ImpactState* state) {
    if (ImpactInvalidPtr(state)) {
        return ImpactResultArgumentInvalid;
    }

    const ImpactMachExceptionHandlers* prexistingHandlers = &state->constantState.preexistingMachExceptionHandlers;

    bool someFailed = false;

    for (mach_msg_type_number_t i = 0; i < prexistingHandlers->count; ++i) {
        // note that per kernel source inspection (osfmk/kern/ipc_tt.c), this function takes a lock :(
        const kern_return_t kr = task_set_exception_ports(mach_task_self(),
                                                          prexistingHandlers->masks[i],
                                                          prexistingHandlers->handlers[i],
                                                          prexistingHandlers->behaviors[i],
                                                          prexistingHandlers->flavors[i]);
        if (kr != KERN_SUCCESS) {
            someFailed = true;
        }
    }

    return someFailed ? ImpactResultFailure : ImpactResultSuccess;
}

static ImpactResult ImpactMachExceptionLog(ImpactState* state, const ImpactMachExceptionRaiseRequest* request) {
    ImpactLogger* log = &state->constantState.log;

    ImpactLogWriteString(log, "hello from the mach exception handler\n");

    thread_t thread = request->thread.name;

    ImpactDebugLog("[Log:%s] request %d %x %llx %llx %x\n", __func__, request->Head.msgh_id, request->exception, request->code[0], request->code[1], thread);
    
    return ImpactResultSuccess;
}

static ImpactResult ImpactMachExceptionSendToHandler(const ImpactMachExceptionRaiseRequest* request, const ImpactMachExceptionHandler* handler) {
    return ImpactResultUnimplemented;
}

static ImpactResult ImpactMachExceptionForward(const ImpactState* state, const ImpactMachExceptionRaiseRequest* request, bool* forwarded) {
    if (ImpactInvalidPtr(state) || ImpactInvalidPtr(request) || ImpactInvalidPtr(forwarded)) {
        return ImpactResultArgumentInvalid;
    }

    const ImpactMachExceptionHandlers* prexistingHandlers = &state->constantState.preexistingMachExceptionHandlers;
    uint32_t failCount = 0;
    uint32_t handlerCount = 0;
    exception_mask_t forwardedMasks = 0;

    for (mach_msg_type_number_t i = 0; i < prexistingHandlers->count; ++i) {
        ImpactMachExceptionHandler handler = {0};
        ImpactResult result = ImpactMachExceptionGetHandler(state, i, &handler);
        if (result != ImpactResultSuccess) {
            failCount++;
            continue;
        }

        // not a real handler
        if (!MACH_PORT_VALID(handler.handlerPort)) {
            continue;
        }

        handlerCount++;

        if ((forwardedMasks & handler.mask) != 0) {
            ImpactDebugLog("[Log:WARN:%s] duplicate forwarding mask found %x\n", __func__, handler.mask);
            continue;
        }

        forwardedMasks |= handler.mask;

        result = ImpactMachExceptionSendToHandler(request, &handler);
        if (result != ImpactResultSuccess) {
            ImpactDebugLog("[Log:WARN:%s] failed forwarding to %x %d\n", __func__, handler.mask, result);
            failCount++;
        }
    }
    
    if (handlerCount > 0 && failCount < handlerCount) {
        *forwarded = true;
    }

    return failCount == 0 ? ImpactResultSuccess : ImpactResultFailure;
}

static ImpactResult ImpactMachExceptionReply(const ImpactMachExceptionRaiseRequest* request, kern_return_t code) {
    __Reply__exception_raise_t reply = {0};

    reply.Head.msgh_bits = MACH_MSGH_BITS(MACH_MSGH_BITS_REMOTE(request->Head.msgh_bits), 0);
    reply.Head.msgh_remote_port = request->Head.msgh_remote_port;
    reply.Head.msgh_local_port = MACH_PORT_NULL;
    reply.Head.msgh_size = sizeof(reply);
    reply.Head.msgh_id = request->Head.msgh_id + 100;
    reply.NDR = request->NDR;
    reply.RetCode = code;

    ImpactDebugLog("[Log:INFO:%s] sending reply message %d\n", __func__, reply.Head.msgh_id);

    const mach_msg_return_t mr = mach_msg(&reply.Head,
                                          MACH_SEND_MSG,
                                          reply.Head.msgh_size,
                                          0,
                                          MACH_PORT_NULL,
                                          MACH_MSG_TIMEOUT_NONE,
                                          MACH_PORT_NULL);
    if (mr != MACH_MSG_SUCCESS) {
        ImpactDebugLog("[Log:ERROR:%s] failed to reply to exception %x\n", __func__, mr);
        return ImpactResultFailure;
    }

    return ImpactResultSuccess;
}

static ImpactResult ImpactMachExceptionReadException(ImpactMachExceptionAllRaiseRequest* request) {
    if (ImpactInvalidPtr(request)) {
        return ImpactResultArgumentInvalid;
    }

    ImpactDebugLog("[Log:INFO:%s] waiting on exception %x\n", __func__, request->raise.Head.msgh_size);

    const mach_msg_return_t mr = mach_msg(&request->raise.Head,
                                          MACH_RCV_MSG | MACH_RCV_LARGE,
                                          0,
                                          request->raise.Head.msgh_size,
                                          request->raise.Head.msgh_local_port,
                                          MACH_MSG_TIMEOUT_NONE,
                                          MACH_PORT_NULL);
    if (mr != MACH_MSG_SUCCESS) {
        ImpactDebugLog("[Log:ERROR:%s] unable to read exception %x %x\n", __func__, mr, request->raise.Head.msgh_size);
        return ImpactResultFailure;
    }

    return ImpactResultSuccess;
}

static ImpactResult ImpactMachExceptionProcess(ImpactState* state, const ImpactMachExceptionRaiseRequest* request, bool* forwarded) {
    ImpactResult result = ImpactMachExceptionLog(state, request);
    if (result != ImpactResultSuccess) {
        ImpactDebugLog("[Log:ERROR:%s] unable to log exception %d\n", __func__, result);
        return result;
    }

    const thread_act_t thread = request->thread.name;

    result = ImpactCrashHandler(state, thread, NULL);

    result = ImpactMachExceptionForward(state, request, forwarded);
    if (result != ImpactResultSuccess) {
        ImpactDebugLog("[Log:WARN:%s] failed to forward exception %d\n", __func__, result);
    }

    return result;
}

static void ImpactMachExceptionHandlerEntranceAdjustState(ImpactState* state, ImpactCrashState *currentState) {
    *currentState = atomic_load(&state->mutableState.crashState);

    switch (*currentState) {
        case ImpactCrashStateInitialized:
            ImpactStateTransition(state, *currentState, ImpactCrashStateMachException);
            break;
        case ImpactCrashStateSignal:
            ImpactStateTransition(state, *currentState, ImpactCrashStateMachExceptionAfterSignal);
            break;
        case ImpactCrashStateSignalHandled:
            ImpactStateTransition(state, *currentState, ImpactCrashStateMachExceptionAfterSignalHandled);
            break;
        case ImpactCrashStateMachExceptionReplied:
            ImpactStateTransition(state, *currentState, ImpactCrashStateSecondMachException);
            break;
        case ImpactCrashStateSignalAfterMachException:
        case ImpactCrashStateSignalAfterMachExceptionReplied:
            ImpactStateTransition(state, *currentState, ImpactCrashStateSecondMachException);
            break;
        default:
            ImpactStateInvalid(*currentState);
            break;
    }
}

static void ImpactMachExceptionHandlerExitAdjustState(ImpactState* state) {
    const ImpactCrashState currentState = atomic_load(&state->mutableState.crashState);
    switch (currentState) {
        case ImpactCrashStateMachException:
            ImpactStateTransition(state, currentState, ImpactCrashStateMachExceptionReplied);
            break;
        case ImpactCrashStateMachExceptionAfterSignal:
        case ImpactCrashStateMachExceptionAfterSignalHandled:
            ImpactStateTransition(state, currentState, ImpactCrashStateMachExceptionReplied);
            break;
        case ImpactCrashStateSecondMachException:
            ImpactStateTransition(state, currentState, ImpactCrashStateSecondMachExceptionReplied);
            break;
        default:
            ImpactStateInvalid(currentState);
            break;
    }

}

static ImpactResult ImpactMachExceptionHandle(ImpactState* state) {
    if (ImpactInvalidPtr(state)) {
        return ImpactResultArgumentInvalid;
    }

    ImpactMachExceptionAllRaiseRequest request = {0};

    // It is not strictly necessary to populate these fields, but its
    // not wrong to do so. And, its a convenient way to pass the needed
    // info into the next function.
    request.raise.Head.msgh_size = sizeof(ImpactMachExceptionAllRaiseRequest);
    request.raise.Head.msgh_local_port = state->constantState.machExceptionPort;

    ImpactResult result = ImpactMachExceptionReadException(&request);
    if (result != ImpactResultSuccess) {
        return result;
    }

    ImpactCrashState currentState = ImpactCrashStateUninitialized;
    ImpactMachExceptionHandlerEntranceAdjustState(state, &currentState);

    result = ImpactSignalUninstallHandlers(state);
    if (result != ImpactResultSuccess) {
        ImpactDebugLog("[Log:ERROR:%s] failed to restore signal handlers %d\n", __func__, result);
    }

    if (currentState == ImpactCrashStateInitialized) {
        bool forwarded = false;
        result = ImpactMachExceptionProcess(state, &request.raise, &forwarded);
        if (result != ImpactResultSuccess) {
            ImpactDebugLog("[Log:WARN:%s] failed to process exception %d\n", __func__, result);
        }

        if (forwarded) {
            ImpactDebugLog("[Log:INFO:%s] forwarded message to preexisting handler\n", __func__);
            return result;
        }
    }

    // 1 - we failed to forward the exception
    // 2 - there were no previous handlers
    // 3 - we've been invoked more than once
    //
    // In all these cases, we tell the kernel that we were unable to handle this
    // exception, and it should continue the termination process.

    ImpactDebugLog("[Log:INFO:%s] replying directly with KERN_FAILURE\n", __func__);

    result = ImpactMachExceptionReply(&request.raise, KERN_FAILURE);
    if (result != ImpactResultSuccess) {
        ImpactDebugLog("[Log:ERROR:%s] direct reply failed %d\n", __func__, result);
        return result;
    }

    ImpactMachExceptionHandlerExitAdjustState(state);

    return ImpactResultSuccess;
}

static void* ImpactMachExceptionServer(void* ctx) {
    // cap the maximum number of times we attempt this to avoid spinning
    // the CPU. We'll still hang if we ever return

    for (int i = 0; i < 5; ++i) {
        const ImpactResult result = ImpactMachExceptionHandle(ctx);
        if (result != ImpactResultSuccess) {
            return NULL;
        }
    }

    ImpactDebugLog("[Log:INFO:%s] exception handling thread exiting\n", __func__);

    return NULL;
}
