//
//  ImpactCompactUnwindTests.m
//  ImpactTests
//
//  Created by Matt Massicotte on 2019-10-03.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#import <XCTest/XCTest.h>

#import "ImpactCompactUnwind.h"
#import "ImpactCrashHelper.h"

@interface ImpactCompactUnwindTests : XCTestCase

@end

@implementation ImpactCompactUnwindTests

- (void)testFunctionLookupInMiddleOfCompressedPage {
    ImpactMachODataRegion region = [ImpactCrashHelper regionWithName:@"unwind/libobjc_10_14_4_18E226.unwind_info.x86_64.bin"
                                                         loadAddress:0x784688];

    const uintptr_t imageAddress = region.address - region.loadAddress;

    const ImpactCompactUnwindTarget target = {
        .address = (0x0000A3ED + 2) + imageAddress,
        .imageLoadAddress = imageAddress,
        .header = (void*)region.address
    };
    compact_unwind_encoding_t encoding = 0;

    ImpactResult result = ImpactCompactUnwindLookupEncoding(target, &encoding);

    XCTAssertEqual(result, ImpactResultSuccess);
    XCTAssertEqual(encoding, 0x010558D1);
}

- (void)testFunctionLookupAtFirstEntryOfCompressedPage {
    ImpactMachODataRegion region = [ImpactCrashHelper regionWithName:@"unwind/libobjc_10_14_4_18E226.unwind_info.x86_64.bin"
                                                         loadAddress:0x784688];

    const uintptr_t imageAddress = region.address - region.loadAddress;

    const ImpactCompactUnwindTarget target = {
        .address = 0x00000F01 + imageAddress,
        .imageLoadAddress = imageAddress,
        .header = (void*)region.address
    };
    compact_unwind_encoding_t encoding = 0;

    ImpactResult result = ImpactCompactUnwindLookupEncoding(target, &encoding);

    XCTAssertEqual(result, ImpactResultSuccess);
    XCTAssertEqual(encoding, 0x01030161);
}

- (void)testFunctionLookupLastEntryOfLastCompressedPage {
    ImpactMachODataRegion region = [ImpactCrashHelper regionWithName:@"unwind/libobjc_10_14_4_18E226.unwind_info.x86_64.bin"
                                                         loadAddress:0x784688];

    const uintptr_t imageAddress = region.address - region.loadAddress;

    const ImpactCompactUnwindTarget target = {
        .address = 0x00021A28 + 1 + imageAddress,
        .imageLoadAddress = imageAddress,
        .header = (void*)region.address
    };
    compact_unwind_encoding_t encoding = 0;

    ImpactResult result = ImpactCompactUnwindLookupEncoding(target, &encoding);

    XCTAssertEqual(result, ImpactResultSuccess);
    XCTAssertEqual(encoding, 0x01010001);
}

- (void)testFunctionLookupEntryWithoutCommonEncoding {
    ImpactMachODataRegion region = [ImpactCrashHelper regionWithName:@"unwind/libobjc_10_14_4_18E226.unwind_info.x86_64.bin"
                                                         loadAddress:0x784688];

    const uintptr_t imageAddress = region.address - region.loadAddress;

    const ImpactCompactUnwindTarget target = {
        .address = 0x0001995D + 2 + imageAddress,
        .imageLoadAddress = imageAddress,
        .header = (void*)region.address
    };
    compact_unwind_encoding_t encoding = 0;

    ImpactResult result = ImpactCompactUnwindLookupEncoding(target, &encoding);

    XCTAssertEqual(result, ImpactResultSuccess);
    XCTAssertEqual(encoding, 0x040013E0);
}

@end
