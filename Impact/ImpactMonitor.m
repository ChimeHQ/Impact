//
//  ImpactMonitor.m
//  Impact
//
//  Created by Matt Massicotte on 2019-09-18.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#import "ImpactMonitor.h"
#include "ImpactState.h"
#include "ImpactLog.h"
#include "ImpactSignal.h"
#include "ImpactMachException.h"

ImpactState* GlobalImpactState = NULL;

@implementation ImpactMonitor

+ (ImpactMonitor *)shared {
    static ImpactMonitor *sharedMyManager = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedMyManager = [[self alloc] init];
    });
    return sharedMyManager;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        _suppressReportCrash = NO;
    }

    return self;
}

- (void)startWithURL:(NSURL *)url identifier:(NSString *)string {
    assert(GlobalImpactState == NULL);

    NSURL* parent = url.URLByDeletingLastPathComponent;

    NSError* error = nil;
    if (![NSFileManager.defaultManager createDirectoryAtURL:parent withIntermediateDirectories:YES attributes:nil error:&error]) {
        NSLog(@"[Impact] Unable to create directory for log %@", error);
        return;
    }

    GlobalImpactState = malloc(sizeof(ImpactState));

    GlobalImpactState->constantState.suppressReportCrash = self.suppressReportCrash == YES;

    atomic_store(&GlobalImpactState->mutableState.crashState, ImpactCrashStateUninitialized);

    ImpactResult result;

    NSLog(@"trying to start with: %s", url.fileSystemRepresentation);
    
    result = ImpactLogInitialize(GlobalImpactState, url.fileSystemRepresentation);
    if (result != ImpactResultSuccess) {
        NSLog(@"[Impact] Unable to initialize log %d", result);
        return;
    }

    result = ImpactSignalInitialize(GlobalImpactState);
    if (result != ImpactResultSuccess) {
        NSLog(@"[Impact] Unable to initialize signal %d", result);
        return;
    }

    result = ImpactMachExceptionInitialize(GlobalImpactState);
    if (result != ImpactResultSuccess) {
        NSLog(@"[Impact] Unable to initialize mach exceptions %d", result);
        return;
    }

    atomic_store(&GlobalImpactState->mutableState.crashState, ImpactCrashStateInitialized);
}

@end
