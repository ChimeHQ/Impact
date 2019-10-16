//
//  ImpactCrashHelper.m
//  ImpactTests
//
//  Created by Matt Massicotte on 2019-10-14.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#import "ImpactCrashHelper.h"

@implementation ImpactCrashHelper

+ (ImpactMachODataRegion)regionWithName:(NSString *)name loadAddress:(uint64_t)addr {
    NSBundle* bundle = [NSBundle bundleForClass:[self class]];
    NSString* path = [bundle pathForResource:name ofType:nil];
    NSData* data = [NSData dataWithContentsOfFile:path];

    const void* bytes = [data bytes];

    ImpactMachODataRegion region = {
        .address = (uintptr_t)bytes,
        .loadAddress = addr,
        .length = (uint32_t)data.length
    };

    return region;
}

@end
