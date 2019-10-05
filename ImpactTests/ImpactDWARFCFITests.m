//
//  ImpactDWARFCFITests.m
//  ImpactTests
//
//  Created by Matt Massicotte on 2019-10-05.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#import <XCTest/XCTest.h>

#import "ImpactDWARF.h"

@interface ImpactDWARFCFITests : XCTestCase

@end

@implementation ImpactDWARFCFITests

- (NSData *)regionDataWithName:(NSString *)name {
    NSString* path = [[NSBundle bundleForClass:[self class]] pathForResource:@"unwind" ofType:nil];
    NSString* filePath = [path stringByAppendingPathComponent:name];

    return [NSData dataWithContentsOfFile:filePath];
}

- (ImpactMachODataRegion)regionWithName:(NSString *)name {
    NSData* data = [self regionDataWithName:name];

    const void* bytes = [data bytes];

    ImpactMachODataRegion region = {0};
    region.address = (uintptr_t)bytes;
    region.length = (uint32_t)data.length;

    return region;
}

- (void)testFunctionReadFDEData {
    ImpactMachODataRegion region = [self regionWithName:@"libobjc_10_14_4_18E226.eh_frame.x86_64.bin"];

    ImpactDWARFCFIData cfiData = {0};

    // first compact unwind entry with a dwarf offset "_tls_init"
    ImpactResult result = ImpactDWARFReadData(region, 0x00001C78, &cfiData);

    XCTAssertEqual(result, ImpactResultSuccess);

    XCTAssertEqual(cfiData.fde.header.length.length32, 28);
    XCTAssertEqual(cfiData.fde.header.length.length64, 0);
    XCTAssertEqual(ImpactDWARFCFIHeaderGetLength(cfiData.fde.header), 28);
    XCTAssertEqual(cfiData.fde.header.CIE_id, 0x7c);
    // the correct value for target_address cannot be easily computed
    XCTAssertEqual(cfiData.fde.address_range, 0x18);

    XCTAssertEqual(cfiData.fde.instructions.length, 7);

    XCTAssertEqual(ImpactDWARFCFIHeaderGetLength(cfiData.cie.header), 0x14);
    XCTAssertEqual(cfiData.cie.header.CIE_id, 0);
    XCTAssertEqual(cfiData.cie.version, 1);
    XCTAssertTrue(strcmp(cfiData.cie.augmentation, "zR") == 0);
    XCTAssertEqual(cfiData.cie.code_alignment_factor, 1);
    XCTAssertEqual(cfiData.cie.data_alignment_factor, -8);
    XCTAssertEqual(cfiData.cie.return_address_register, 16);

    XCTAssertTrue(cfiData.cie.augmentationData.fdesHaveAugmentationData);
    XCTAssertEqual(cfiData.cie.augmentationData.pointerEncoding, 0x10);

    XCTAssertEqual(cfiData.cie.instructions.length, 7);
}

@end
