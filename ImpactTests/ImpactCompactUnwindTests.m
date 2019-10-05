//
//  ImpactCompactUnwindTests.m
//  ImpactTests
//
//  Created by Matt Massicotte on 2019-10-03.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#import <XCTest/XCTest.h>

#import "ImpactCompactUnwind.h"

@interface ImpactCompactUnwindTests : XCTestCase

@end

@implementation ImpactCompactUnwindTests

- (void)testFunctionLookupInMiddleOfCompressedPage {
    NSString* path = [[NSBundle bundleForClass:[self class]] pathForResource:@"unwind" ofType:nil];
    NSString* filePath = [path stringByAppendingPathComponent:@"libobjc_10_14_4_18E226.unwind_info.x86_64.bin"];

    NSData* data = [NSData dataWithContentsOfFile:filePath];

    const void* bytes = [data bytes];

    ImpactCompactUnwindTarget target = {0x0000A3ED + 2, bytes, {0, 0}};
    compact_unwind_encoding_t encoding = 0;

    ImpactResult result = ImpactCompactUnwindLookupEncoding(target, &encoding);

    XCTAssertEqual(result, ImpactResultSuccess);
    XCTAssertEqual(encoding, 0x010558D1);
}

- (void)testFunctionLookupAtFirstEntryOfCompressedPage {
    NSString* path = [[NSBundle bundleForClass:[self class]] pathForResource:@"unwind" ofType:nil];
    NSString* filePath = [path stringByAppendingPathComponent:@"libobjc_10_14_4_18E226.unwind_info.x86_64.bin"];

    NSData* data = [NSData dataWithContentsOfFile:filePath];

    const void* bytes = [data bytes];

    ImpactCompactUnwindTarget target = {0x00000F01, bytes, {0, 0}};
    compact_unwind_encoding_t encoding = 0;

    ImpactResult result = ImpactCompactUnwindLookupEncoding(target, &encoding);

    XCTAssertEqual(result, ImpactResultSuccess);
    XCTAssertEqual(encoding, 0x01030161);
}

- (void)testFunctionLookupLastEntryOfLastCompressedPage {
    NSString* path = [[NSBundle bundleForClass:[self class]] pathForResource:@"unwind" ofType:nil];
    NSString* filePath = [path stringByAppendingPathComponent:@"libobjc_10_14_4_18E226.unwind_info.x86_64.bin"];

    NSData* data = [NSData dataWithContentsOfFile:filePath];

    const void* bytes = [data bytes];

    ImpactCompactUnwindTarget target = {0x00021A28 + 1, bytes, {0, 0}};
    compact_unwind_encoding_t encoding = 0;

    ImpactResult result = ImpactCompactUnwindLookupEncoding(target, &encoding);

    XCTAssertEqual(result, ImpactResultSuccess);
    XCTAssertEqual(encoding, 0x01010001);
}

- (void)testFunctionLookupEntryWithoutCommonEncoding {
    NSString* path = [[NSBundle bundleForClass:[self class]] pathForResource:@"unwind" ofType:nil];
    NSString* filePath = [path stringByAppendingPathComponent:@"libobjc_10_14_4_18E226.unwind_info.x86_64.bin"];

    NSData* data = [NSData dataWithContentsOfFile:filePath];

    const void* bytes = [data bytes];

    ImpactCompactUnwindTarget target = {0x0001995D + 2, bytes, {0, 0}};
    compact_unwind_encoding_t encoding = 0;

    ImpactResult result = ImpactCompactUnwindLookupEncoding(target, &encoding);

    XCTAssertEqual(result, ImpactResultSuccess);
    XCTAssertEqual(encoding, 0x040013E0);
}

@end
