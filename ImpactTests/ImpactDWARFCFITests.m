//
//  ImpactDWARFCFITests.m
//  ImpactTests
//
//  Created by Matt Massicotte on 2019-10-05.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#import <XCTest/XCTest.h>

#import "ImpactDWARF.h"
#import "ImpactDWARFParser.h"
#import "ImpactState.h"
#import "ImpactCrashHelper.h"

#import <stdio.h>

@interface ImpactDWARFCFITests : XCTestCase

@end

@implementation ImpactDWARFCFITests

- (void)setUp {
    GlobalImpactState = malloc(sizeof(ImpactState));

    GlobalImpactState->constantState.log.fd = STDERR_FILENO;
}

- (void)tearDown {
    free(GlobalImpactState);
}

- (void)testFunctionReadFDEData {
    const ImpactMachODataRegion region = [ImpactCrashHelper regionWithName:@"unwind/libobjc_10_14_4_18E226.eh_frame.x86_64.bin"
                                                               loadAddress:0x785ee0];
    const ImpactDWARFEnvironment env = {
        .pointerWidth = 8
    };

    ImpactDWARFCFIData cfiData = {0};

    // first compact unwind entry with a dwarf offset "_tls_init"
    ImpactResult result = ImpactDWARFReadData(region, env, 0x00001C78, &cfiData);

    XCTAssertEqual(result, ImpactResultSuccess);

    XCTAssertEqual(cfiData.fde.header.length.length32, 28);
    XCTAssertEqual(cfiData.fde.header.length.length64, 0);
    XCTAssertEqual(ImpactDWARFCFIHeaderGetLength(cfiData.fde.header), 28);
    XCTAssertEqual(cfiData.fde.header.CIE_id, 0x7c);

    // this is a complex one, so it's tough. The spec defines this value as relative to the eh_frame data itself. This results
    // in an absolute pointer value that makes sense within a running binary. But, we have a pointer to the eh_frame section
    // only, so we have to do a little math.
    //
    // target_address = functionOffset + imageLoadAddress
    // imageLoadAddress = region.address - region.loadAddress
    XCTAssertEqual(cfiData.fde.target_address, 0x1298 + (region.address - region.loadAddress));
    XCTAssertEqual(cfiData.fde.address_range, 0x18);

    XCTAssertEqual(cfiData.fde.instructions.length, 7);

    XCTAssertEqual(ImpactDWARFCFIHeaderGetLength(cfiData.cie.header), 0x14);
    XCTAssertEqual(cfiData.cie.header.CIE_id, 0);
    XCTAssertEqual(cfiData.cie.version, 1);
    XCTAssertEqualObjects([NSString stringWithUTF8String:cfiData.cie.augmentation], @"zR");
    XCTAssertEqual(cfiData.cie.code_alignment_factor, 1);
    XCTAssertEqual(cfiData.cie.data_alignment_factor, -8);
    XCTAssertEqual(cfiData.cie.return_address_register, 16);

    XCTAssertTrue(cfiData.cie.augmentationData.fdesHaveAugmentationData);
    XCTAssertEqual(cfiData.cie.augmentationData.pointerEncoding, 0x10);

    XCTAssertEqual(cfiData.cie.instructions.length, 7);
}

- (void)testImpactDWARFReadCIEWithPersonality {
    const ImpactMachODataRegion region = [ImpactCrashHelper regionWithName:@"unwind/libobjc_10_14_4_18E226.eh_frame.x86_64.bin"
                                                               loadAddress:0x785ee0];
    const ImpactDWARFEnvironment env = {
        .pointerWidth = 8
    };

    ImpactDataCursor cursor = {0};
    ImpactDWARFCIE cie = {0};

    // 000008e0 => first CIE in this executable with "zPLR" augmentation
    ImpactResult result = ImpactDataCursorInitialize(&cursor, region.address, region.length, 0x000008e0);
    XCTAssertEqual(result, ImpactResultSuccess);

    result = ImpactDWARFReadCIE(&cursor, env, &cie);
    XCTAssertEqual(result, ImpactResultSuccess);

    XCTAssertEqualObjects([NSString stringWithUTF8String:cie.augmentation], @"zPLR");
    XCTAssertEqual(cie.augmentationData.pointerEncoding, 0x10);
    XCTAssertEqual(cie.augmentationData.personalityEncoding, 0x9b);

    // This is a "pre-resolved" pointer, because the encoding contains DW_EH_PE_indirect. I'm not
    // yet certain how decode this successfully. However, since we don't need pesonality functions,
    // it isn't critical.
    XCTAssertEqual(cie.augmentationData.personality, 0x00789040 + (region.address - region.loadAddress));
}

- (void)testDWARFCFIRunInstructions {
    const ImpactMachODataRegion region = [ImpactCrashHelper regionWithName:@"unwind/libobjc_10_14_4_18E226.eh_frame.x86_64.bin"
                                                               loadAddress:0x785ee0];
    const ImpactDWARFEnvironment env = {
        .pointerWidth = 8
    };

    ImpactDWARFCFIData cfiData = {0};

    // first compact unwind entry with a dwarf offset "_tls_init"
    ImpactResult result = ImpactDWARFReadData(region, env, 0x00000C88, &cfiData);
    XCTAssertEqual(result, ImpactResultSuccess);

//    00000bc8 0000001c ffffffff CIE
//      Version:               1
//      Augmentation:          "zPLR"
//      Code alignment factor: 1
//      Data alignment factor: -8
//      Return address column: 16
//      Personality Address: 0000258d
//      Augmentation data:     9B 8D 25 00 00 10 10
//
//      DW_CFA_def_cfa: reg7 +8
//      DW_CFA_offset: reg16 -8
//      DW_CFA_nop:
//      DW_CFA_nop:
//
//    00000c88 0000001c 0000017c FDE cie=0000017c pc=ff88fcb0...ff88fcbb
//      DW_CFA_advance_loc: 1
//      DW_CFA_def_cfa_offset: +16
//      DW_CFA_nop:
//      DW_CFA_nop:
//      DW_CFA_nop:
//      DW_CFA_nop:

    const uint64_t stack[] = {
        0x0,
        0x11223344, // RIP (-8)
        0x0, // CFA (RSP + 16)
        0x0
    };
    const uint64_t rip = 0x0;
    const uint64_t rbp = 0xccaabb;
    const uint64_t rsp = (uint64_t)stack;

    const ImpactDWARFTarget target = {
        .pc = rip,
        .ehFrameRegion = region,
        .environment = env
    };

    ImpactCPURegisters registers = {0};

    result = ImpactCPUSetRegister(&registers, ImpactCPURegister_X86_64_RIP, rip);
    XCTAssertEqual(result, ImpactResultSuccess);

    result = ImpactCPUSetRegister(&registers, ImpactCPURegister_X86_64_RBP, rbp);
    XCTAssertEqual(result, ImpactResultSuccess);

    result = ImpactCPUSetRegister(&registers, ImpactCPURegister_X86_64_RSP, rsp);
    XCTAssertEqual(result, ImpactResultSuccess);

    result = ImpactDWARFStepRegisters(&cfiData, target, &registers);
    XCTAssertEqual(result, ImpactResultSuccess);

    // now, verify that all the registers are mutated as expected
    uintptr_t value = 0;

    result = ImpactCPUGetRegister(&registers, ImpactCPURegister_X86_64_RIP, &value);
    XCTAssertEqual(result, ImpactResultSuccess);
    XCTAssertEqual(value, 0x11223344);

    result = ImpactCPUGetRegister(&registers, ImpactCPURegister_X86_64_RBP, &value);
    XCTAssertEqual(result, ImpactResultSuccess);
    XCTAssertEqual(value, 0xccaabb);

    result = ImpactCPUGetRegister(&registers, ImpactCPURegister_X86_64_RSP, &value);
    XCTAssertEqual(result, ImpactResultSuccess);
    XCTAssertEqual(value, (uint64_t)stack + 16);
}

@end
