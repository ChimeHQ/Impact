//
//  CrashInvocation.h
//  ImpactTestMac
//
//  Created by Matt Massicotte on 2019-09-18.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface CrashInvocation : NSObject

- (void)execute;

@property (nonatomic, readonly) NSString* name;

@end

NS_ASSUME_NONNULL_END
