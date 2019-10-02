//
//  UncaughtNSException.m
//  ImpactTestMac
//
//  Created by Matt Massicotte on 2019-09-30.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#import "UncaughtNSException.h"

@implementation UncaughtNSException

- (void)execute {
    [NSException raise:@"AnException" format:@"something bad happened"];
}

- (NSString *)name {
    return @"Uncaught NSException";
}

@end
