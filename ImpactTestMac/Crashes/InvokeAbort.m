//
//  InvokeAbort.m
//  ImpactTestMac
//
//  Created by Matt Massicotte on 2019-09-18.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#import "InvokeAbort.h"

@implementation InvokeAbort

- (void)execute {
    abort();
}

- (NSString *)name {
    return @"Invoke Abort";
}

@end
