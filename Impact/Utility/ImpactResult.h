//
//  ImpactResult.h
//  Impact
//
//  Created by Matt Massicotte on 2019-09-18.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#ifndef ImpactResult_h
#define ImpactResult_h

typedef enum {
    ImpactResultSuccess = 0,
    ImpactResultFailure,
    ImpactResultPointerInvalid,
    ImpactResultArgumentInvalid,
    ImpactResultCallFailed,
    ImpactResultInconsistentData,
    ImpactResultMemoryReadFailed,
    ImpactResultUnimplemented
} ImpactResult;

#endif /* ImpactResult_h */
