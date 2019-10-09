//
//  ImpactMonitoredApplication.h
//  Impact
//
//  Created by Matt Massicotte on 2019-09-30.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#import <TargetConditionals.h>

#if TARGET_OS_OSX
#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@interface ImpactMonitoredApplication : NSApplication

@end

NS_ASSUME_NONNULL_END

#endif
