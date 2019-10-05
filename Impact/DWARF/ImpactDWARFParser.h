//
//  ImpactDWARFParser.h
//  Impact
//
//  Created by Matt Massicotte on 2019-05-07.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#ifndef ImpactDWARFParser_h
#define ImpactDWARFParser_h

#include <stdint.h>
#include "ImpactDWARF.h"
#include "ImpactDataCursor.h"

ImpactResult ImpactDWARFReadCFI(ImpactDataCursor* cursor, ImpactDWARFCFIData* cfiData);

#endif /* ImpactDWARFParser_h */
