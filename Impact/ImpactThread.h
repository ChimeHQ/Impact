//
//  ImpactThread.h
//  Impact
//
//  Created by Matt Massicotte on 2019-09-27.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#ifndef ImpactThread_h
#define ImpactThread_h

#include "ImpactResult.h"
#include "ImpactState.h"

#include <mach/task.h>

typedef struct {
    thread_act_array_t threads;
    mach_msg_type_number_t count;

    thread_act_t threadSelf;
} ImpactThreadList;

ImpactResult ImpactThreadListInitialize(ImpactThreadList* list);
ImpactResult ImpactThreadListDeinitialize(ImpactThreadList* list);
ImpactResult ImpactThreadListLog(ImpactState* state, const ImpactThreadList* list);

#endif /* ImpactThread_h */
