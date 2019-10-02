//
//  NonMainThreadUncaughtNSException.m
//  ImpactTestMac
//
//  Created by Matt Massicotte on 2019-10-01.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#import "NonMainThreadUncaughtNSException.h"

@implementation NonMainThreadUncaughtNSException

- (void)execute {
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        [NSException raise:@"AnException" format:@"something bad happened"];
    });
}

- (NSString *)name {
    return @"Non-Main Thread Uncaught NSException";
}

@end
