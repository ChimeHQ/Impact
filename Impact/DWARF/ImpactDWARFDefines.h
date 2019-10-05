//
//  ImpactDWARFDefines.h
//  Impact
//
//  Created by Matt Massicotte on 2019-07-27.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#ifndef ImpactDWARFDefines_h
#define ImpactDWARFDefines_h

// Pointer Encoding
#define DW_EH_PE_type_mask (0x0F)
#define DW_EH_PE_offset_mask (0x70)

enum {
    DW_EH_PE_ptr       = 0x00,
    DW_EH_PE_uleb128   = 0x01,
    DW_EH_PE_udata2    = 0x02,
    DW_EH_PE_udata4    = 0x03,
    DW_EH_PE_udata8    = 0x04,
    DW_EH_PE_signed    = 0x08,
    DW_EH_PE_sleb128   = 0x09,
    DW_EH_PE_sdata2    = 0x0A,
    DW_EH_PE_sdata4    = 0x0B,
    DW_EH_PE_sdata8    = 0x0C,
    DW_EH_PE_absptr    = 0x00,
    DW_EH_PE_pcrel     = 0x10,
    DW_EH_PE_textrel   = 0x20,
    DW_EH_PE_datarel   = 0x30,
    DW_EH_PE_funcrel   = 0x40,
    DW_EH_PE_aligned   = 0x50,
    DW_EH_PE_indirect  = 0x80,
    DW_EH_PE_omit      = 0xFF
};

enum {
    DW_CFA_nop                 = 0x0,
    DW_CFA_set_loc             = 0x1,
    DW_CFA_advance_loc1        = 0x2,
    DW_CFA_advance_loc2        = 0x3,
    DW_CFA_advance_loc4        = 0x4,
    DW_CFA_offset_extended     = 0x5,
    DW_CFA_restore_extended    = 0x6,
    DW_CFA_undefined           = 0x7,
    DW_CFA_same_value          = 0x8,
    DW_CFA_register            = 0x9,
    DW_CFA_remember_state      = 0xA,
    DW_CFA_restore_state       = 0xB,
    DW_CFA_def_cfa             = 0xC,
    DW_CFA_def_cfa_register    = 0xD,
    DW_CFA_def_cfa_offset      = 0xE,
    DW_CFA_def_cfa_expression  = 0xF,
    DW_CFA_expression         = 0x10,
    DW_CFA_offset_extended_sf = 0x11,
    DW_CFA_def_cfa_sf         = 0x12,
    DW_CFA_def_cfa_offset_sf  = 0x13,
    DW_CFA_val_offset         = 0x14,
    DW_CFA_val_offset_sf      = 0x15,
    DW_CFA_val_expression     = 0x16,
    DW_CFA_advance_loc        = 0x40, // high 2 bits are 0x1, lower 6 bits are delta
    DW_CFA_offset             = 0x80, // high 2 bits are 0x2, lower 6 bits are register
    DW_CFA_restore            = 0xC0, // high 2 bits are 0x3, lower 6 bits are register

    // GNU extensions
    DW_CFA_GNU_window_save              = 0x2D,
    DW_CFA_GNU_args_size                = 0x2E,
    DW_CFA_GNU_negative_offset_extended = 0x2F
};

#endif /* ImpactDWARFDefines_h */
