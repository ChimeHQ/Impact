//
//  ImpactRuntimeException.h
//  Impact
//
//  Created by Matt Massicotte on 2019-09-30.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#ifndef ImpactRuntimeException_h
#define ImpactRuntimeException_h

#include "ImpactState.h"
#include "ImpactResult.h"

@class NSException;

__BEGIN_DECLS

ImpactResult ImpactRuntimeExceptionInitialize(ImpactState * state);

void ImpactRuntimeExceptionLogNSException(NSException* exception);

__END_DECLS


#endif /* ImpactRuntimeException_h */
