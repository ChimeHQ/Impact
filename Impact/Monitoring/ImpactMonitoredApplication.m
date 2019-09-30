//
//  ImpactMonitoredApplication.m
//  Impact
//
//  Created by Matt Massicotte on 2019-09-30.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#import "ImpactMonitoredApplication.h"

@implementation ImpactMonitoredApplication

// AppKit is a real pain when it comes to exceptions. It wraps many calls in @try/@catch
// and silently ignores exceptions by default. This is not good.

- (void)sendEvent:(NSEvent *)event {
    @try {
        [super sendEvent:event];
    } @catch(NSException *e) {
        [self reportException:e];
        @throw e;
    }
}

- (void)reportException:(NSException *)exception {
    // Here, we'll be able to write the NSException details out to a file

    [super reportException:exception];
}


@end
