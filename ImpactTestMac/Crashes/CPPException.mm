//
//  CPPException.m
//  ImpactTestMac
//
//  Created by Matt Massicotte on 2019-10-10.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#import "CPPException.h"

#include <stdexcept>

@implementation CPPException

- (void)execute {
    throw std::runtime_error("boom");
}

- (NSString *)name {
    return @"Uncaught C++ Exception";
}

@end
