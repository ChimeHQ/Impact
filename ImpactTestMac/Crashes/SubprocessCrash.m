//
//  SubprocessCrash.m
//  ImpactTestMac
//
//  Created by Matthew Massicotte on 2021-07-20.
//  Copyright © 2021 Chime Systems Inc. All rights reserved.
//

#import "SubprocessCrash.h"

@implementation SubprocessCrash

- (void)execute {
    NSURL* crasherURL = [[NSBundle mainBundle] URLForAuxiliaryExecutable:@"crasher"];

    NSLog(@"url: %@", crasherURL);

    NSError* error = nil;
    NSTask* task = [NSTask launchedTaskWithExecutableURL:crasherURL arguments:@[] error:&error terminationHandler:^(NSTask *tsk) {

    }];

    [task waitUntilExit];
}

- (NSString *)name {
    return @"Subprocess Crash";
}

@end
