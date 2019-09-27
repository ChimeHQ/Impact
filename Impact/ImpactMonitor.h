//
//  ImpactMonitor.h
//  Impact
//
//  Created by Matt Massicotte on 2019-09-18.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface ImpactMonitor : NSObject

+ (ImpactMonitor *)shared;

- (void)startWithURL:(NSURL *)url identifier:(NSString *)string;

@property (nonatomic) BOOL suppressReportCrash;

@end

NS_ASSUME_NONNULL_END
