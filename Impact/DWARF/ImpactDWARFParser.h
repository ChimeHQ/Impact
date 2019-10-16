//
//  ImpactDWARFParser.h
//  Impact
//
//  Created by Matt Massicotte on 2019-05-07.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#ifndef ImpactDWARFParser_h
#define ImpactDWARFParser_h

#include "ImpactDWARF.h"
#include "ImpactDataCursor.h"

#if IMPACT_DWARF_CFI_SUPPORTED

ImpactResult ImpactDWARFReadCIE(ImpactDataCursor* cursor, ImpactDWARFEnvironment env, ImpactDWARFCIE* cie);
ImpactResult ImpactDWARFReadCFI(ImpactDataCursor* cursor, ImpactDWARFEnvironment env, ImpactDWARFCFIData* cfiData);

#endif

#endif /* ImpactDWARFParser_h */
