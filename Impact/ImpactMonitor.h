//
//  ImpactMonitor.h
//  Impact
//
//  Created by Matt Massicotte on 2019-09-18.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

FOUNDATION_EXTERN const char* ImpactPlatformName;

@interface ImpactMonitor : NSObject

+ (ImpactMonitor *)shared;

@property (class, nonatomic, assign, readonly) NSInteger buildNumber;

- (void)startWithURL:(NSURL *)url identifier:(NSUUID *)uuid;

@property (nonatomic) BOOL suppressReportCrash;

@property (nonatomic, nullable) NSString *applicationIdentifier;
@property (nonatomic, nullable) NSString *organizationIdentifier;
@property (nonatomic, nullable) NSString *installIdentifier;

@end

NS_ASSUME_NONNULL_END
