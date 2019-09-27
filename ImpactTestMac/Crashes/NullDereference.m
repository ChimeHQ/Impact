//
//  NullDereference.m
//  ImpactTestMac
//
//  Created by Matt Massicotte on 2019-09-19.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#import "NullDereference.h"

@implementation NullDereference

- (void)execute {
    int *x = NULL;

    *x = 42;
}

- (NSString *)name {
    return @"Null Dereference";
}

@end
