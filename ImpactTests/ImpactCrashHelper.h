//
//  ImpactCrashHelper.h
//  ImpactTests
//
//  Created by Matt Massicotte on 2019-10-14.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "ImpactBinaryImage.h"

NS_ASSUME_NONNULL_BEGIN

@interface ImpactCrashHelper : NSObject

+ (ImpactMachODataRegion)regionWithName:(NSString *)name loadAddress:(uint64_t)addr;

@end

NS_ASSUME_NONNULL_END
