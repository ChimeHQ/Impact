//
//  ImpactDWARFCFIInstructions.h
//  Impact
//
//  Created by Matt Massicotte on 2019-10-13.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#ifndef ImpactDWARFCFIInstructions_h
#define ImpactDWARFCFIInstructions_h

#include "ImpactDataCursor.h"
#include "ImpactDWARF.h"

#if IMPACT_DWARF_CFI_SUPPORTED

ImpactResult ImpactDWARFRun_DW_CFA_def_cfa(ImpactDataCursor* cursor, ImpactDWARFCFIState* state);
ImpactResult ImpactDWARFRun_DW_CFA_offset(ImpactDataCursor* cursor, uint8_t operand, const ImpactDWARFCIE* cie, ImpactDWARFCFIState* state);
ImpactResult ImpactDWARFRun_DW_CFA_advance_loc(ImpactDataCursor* cursor, uint8_t operand, const ImpactDWARFCIE* cie, ImpactDWARFCFIState* state, uint32_t* location);
ImpactResult ImpactDWARFRun_DW_CFA_def_cfa_offset(ImpactDataCursor* cursor, ImpactDWARFCFIState* state);

#endif

#endif /* ImpactDWARFCFIInstructions_h */
