//
//  ImpactThread.c
//  Impact
//
//  Created by Matt Massicotte on 2019-09-27.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#include "ImpactThread.h"
#include "ImpactUtility.h"
#include "ImpactLog.h"
#include "ImpactCPU.h"
#include "ImpactUnwind.h"

#include <mach/mach_init.h>
#include <mach/mach_port.h>
#include <mach/vm_map.h>
#include <mach/thread_act.h>

ImpactResult ImpactThreadListInitialize(ImpactThreadList* list, thread_act_t crashedThread, const ImpactCPURegisters* crashedThreadRegisters) {
    if (ImpactInvalidPtr(list)) {
        return ImpactResultArgumentInvalid;
    }

    const kern_return_t kr = task_threads(mach_task_self(), &list->threads, &list->count);
    if (kr != KERN_SUCCESS) {
        ImpactDebugLog("[Log:%s] unable to get threads %d\n", __func__, kr);
        return ImpactResultFailure;
    }

    list->threadSelf = mach_thread_self();

    if (crashedThread == ImpactThreadAssumeSelfCrashed) {
        list->crashedThread = list->threadSelf;
    } else {
        list->crashedThread = crashedThread;
    }

    list->crashedThreadRegisters = crashedThreadRegisters;

    return ImpactResultSuccess;
}

ImpactResult ImpactThreadListDeinitialize(ImpactThreadList* list) {
    const task_t task = mach_task_self();
    
    for (mach_msg_type_number_t i = 0; i < list->count; ++i) {
        const kern_return_t kr = mach_port_deallocate(task, list->threads[i]);
        if (kr != KERN_SUCCESS) {
            ImpactDebugLog("[Log:%s] unable to dealloc thread port %d\n", __func__, kr);
        }

        list->threads[i] = MACH_PORT_NULL;
    }

    const vm_size_t size = sizeof(thread_t) * list->count;

    kern_return_t kr = vm_deallocate(task, (vm_address_t)list->threads, size);
    if (kr != KERN_SUCCESS) {
        ImpactDebugLog("[Log:%s] unable to dealloc thread storage %d\n", __func__, kr);
    }

    kr = mach_port_deallocate(task, list->threadSelf);
    if (kr != KERN_SUCCESS) {
        ImpactDebugLog("[Log:%s] unable to dealloc self thread port %d\n", __func__, kr);
    }

    return ImpactResultSuccess;
}

ImpactResult ImpactThreadGetState(const ImpactThreadList* list, thread_act_t thread, ImpactCPURegisters* registers) {
    if (ImpactInvalidPtr(list) || ImpactInvalidPtr(registers)) {
        return ImpactResultArgumentInvalid;
    }

    if (MACH_PORT_VALID(list->crashedThread) && thread == list->crashedThread && !ImpactInvalidPtr(list->crashedThreadRegisters)) {
        *registers = *list->crashedThreadRegisters;
        return ImpactResultSuccess;
    }

#if IMPACT_THREADS_SUPPORTED
    mach_msg_type_number_t count = ImpactCPUThreadStateCount;
    thread_state_t state = (thread_state_t)&registers->__ss;

    const kern_return_t kr = thread_get_state(thread, ImpactCPUThreadStateFlavor, state, &count);
    if (kr != KERN_SUCCESS) {
        ImpactDebugLog("[Log:%s] unable to read thread state %d\n", __func__, kr);
        return ImpactResultFailure;
    }

    return ImpactResultSuccess;
#else
    return ImpactResultFailure;
#endif
}

#if IMPACT_THREADS_SUPPORTED
static ImpactResult ImpactThreadListSuspendAllExceptForCurrent(const ImpactThreadList* list) {
    if (ImpactInvalidPtr(list)) {
        return ImpactResultArgumentInvalid;
    }

    for (mach_msg_type_number_t i = 0; i < list->count; ++i) {
        const thread_act_t thread = list->threads[i];

        if (thread == list->threadSelf) {
            continue;
        }

        const kern_return_t kr = thread_suspend(thread);
        if (kr != KERN_SUCCESS) {
            ImpactDebugLog("[Log:%s] failed to suspend thread %d\n", __func__, kr);
        }
    }

    return ImpactResultSuccess;
}

static ImpactResult ImpactThreadListResumeAllExceptForCurrent(const ImpactThreadList* list) {
    if (ImpactInvalidPtr(list)) {
        return ImpactResultArgumentInvalid;
    }

    for (mach_msg_type_number_t i = 0; i < list->count; ++i) {
        const thread_act_t thread = list->threads[i];

        if (thread == list->threadSelf) {
            continue;
        }

        const kern_return_t kr = thread_resume(thread);
        if (kr != KERN_SUCCESS) {
            ImpactDebugLog("[Log:%s] failed to suspend thread %d\n", __func__, kr);
        }
    }

    return ImpactResultSuccess;
}
#endif

static ImpactResult ImpactThreadLogFrame(ImpactState* state, const ImpactCPURegisters* registers) {
    if (ImpactInvalidPtr(state) || ImpactInvalidPtr(registers)) {
        return ImpactResultArgumentInvalid;
    }

    ImpactLogger* log = &state->constantState.log;

    ImpactLogWriteString(log, "[Thread:Frame] ");

    uintptr_t value = 0;
    ImpactResult result;

    result = ImpactCPUGetRegister(registers, ImpactCPURegisterInstructionPointer, &value);
    if (result != ImpactResultSuccess) {
        return result;
    }

    ImpactLogWriteKeyInteger(log, "ip", value, false);

    result = ImpactCPUGetRegister(registers, ImpactCPURegisterStackPointer, &value);
    if (result != ImpactResultSuccess) {
        return result;
    }

    ImpactLogWriteKeyInteger(log, "sp", value, false);

    result = ImpactCPUGetRegister(registers, ImpactCPURegisterFramePointer, &value);
    if (result != ImpactResultSuccess) {
        return result;
    }

    ImpactLogWriteKeyInteger(log, "fp", value, true);

    return ImpactResultSuccess;
}

static ImpactResult ImpactThreadLogStacktrace(ImpactState* state, const ImpactCPURegisters* registers) {
    if (ImpactInvalidPtr(state) || ImpactInvalidPtr(registers)) {
        return ImpactResultArgumentInvalid;
    }

    ImpactCPURegisters unwindRegisters = *registers;

    // for now, impose a limit on how many frames we write out
    for (uint32_t i = 0; i < 512; ++i) {
        ImpactResult result = ImpactThreadLogFrame(state, &unwindRegisters);
        if (result != ImpactResultSuccess) {
            ImpactDebugLog("[Log:%s] failed to write frame %x\n", __func__, result);
        }

        result = ImpactUnwindStepRegisters(state, &unwindRegisters);
        switch (result) {
            case ImpactResultEndOfStack:
                return ImpactResultSuccess;
            case ImpactResultSuccess:
                break;
            default:
                ImpactDebugLog("[Log:WARN] failed to step registers %x\n", result);
                return result;
        }
    }

    ImpactDebugLog("[Log:%s] exceeded maximum number of frames\n", __func__);

    return ImpactResultFailure;
}

ImpactResult ImpactThreadLog(ImpactState* state, const ImpactThreadList* list, thread_act_t thread) {
    if (ImpactInvalidPtr(state) || ImpactInvalidPtr(list)) {
        return ImpactResultArgumentInvalid;
    }

    ImpactCPURegisters registers = {0};

    ImpactResult result = ImpactThreadGetState(list, thread, &registers);
    if (result != ImpactResultSuccess) {
        ImpactDebugLog("[Log:%s] failed to get thread state %d\n", __func__, result);
        return result;
    }

    result = ImpactCPURegistersLog(state, &registers);
    if (result != ImpactResultSuccess) {
        ImpactDebugLog("[Log:%s] failed to log thread state %d\n", __func__, result);
    }

    if (thread == list->crashedThread && MACH_PORT_VALID(thread)) {
        ImpactLogger* log = &state->constantState.log;

        ImpactLogWriteString(log, "[Thread:Crashed]\n");
    }

    result = ImpactThreadLogStacktrace(state, &registers);
    if (result != ImpactResultSuccess) {
        ImpactDebugLog("[Log:%s] failed to log thread stack trace %d\n", __func__, result);
    }

    return ImpactResultSuccess;
}

ImpactResult ImpactThreadListLog(ImpactState* state, const ImpactThreadList* list) {
    ImpactResult result = ImpactResultFailure;

#if IMPACT_THREADS_SUPPORTED
    result = ImpactThreadListSuspendAllExceptForCurrent(list);
#endif

    for (mach_msg_type_number_t i = 0; i < list->count; ++i) {
        const thread_act_t thread = list->threads[i];

        result = ImpactThreadLog(state, list, thread);
        if (result != ImpactResultSuccess) {
            ImpactDebugLog("[Log:%s] failed to log thread %d %d\n", __func__, i, result);
        }
    }

#if IMPACT_THREADS_SUPPORTED
    result = ImpactThreadListResumeAllExceptForCurrent(list);
#endif

    return ImpactResultSuccess;
}
