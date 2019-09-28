//
//  ImpactLogTests.m
//  ImpactTests
//
//  Created by Matt Massicotte on 2019-09-27.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#import <XCTest/XCTest.h>
#import "ImpactLog.h"

@interface ImpactLogTests : XCTestCase

@end

@implementation ImpactLogTests

- (NSString *)logContentsWithBlock:(void (^)(ImpactLogger*))block {
    NSString* path = [NSString stringWithUTF8String:"/tmp/test.log"];
    ImpactState state = {0};

    ImpactResult result = ImpactLogInitialize(&state, path.UTF8String);

    if (result != ImpactResultSuccess) {
        return nil;
    }

    block(&state.constantState.log);

    return [NSString stringWithContentsOfFile:path
                                     encoding:NSUTF8StringEncoding
                                        error:nil];
}

- (void)testLogZeroInteger {
    NSString* contents = [self logContentsWithBlock:^(ImpactLogger* log) {
        ImpactLogWriteInteger(log, 0);
    }];

    XCTAssertEqualObjects(contents, @"0x0");
}

- (void)testLogOneDigitInteger {
    NSString* contents = [self logContentsWithBlock:^(ImpactLogger* log) {
        ImpactLogWriteInteger(log, 9);
    }];

    XCTAssertEqualObjects(contents, @"0x9");
}

- (void)testLogFirstHexInteger {
    NSString* contents = [self logContentsWithBlock:^(ImpactLogger* log) {
        ImpactLogWriteInteger(log, 10);
    }];

    XCTAssertEqualObjects(contents, @"0xa");
}

- (void)testLogMaxInteger {
    NSString* contents = [self logContentsWithBlock:^(ImpactLogger* log) {
        ImpactLogWriteInteger(log, UINTPTR_MAX);
    }];

    XCTAssertEqualObjects(contents, @"0xffffffffffffffff");
}

@end
