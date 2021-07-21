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
#include "ImpactBinaryImage.h"
#include "ImpactUtility.h"
#include "ImpactCPU.h"
#include "ImpactRuntimeException.h"

#include <sys/sysctl.h>
#import <sys/utsname.h>

ImpactState* GlobalImpactState = NULL;

#if TARGET_OS_OSX
const char* ImpactPlatformName = "macOS";
#elif TARGET_OS_IOS
const char* ImpactPlatformName = "iOS";
#elif TARGET_OS_TV
const char* ImpactPlatformName = "tvOS";
#elif TARGET_OS_WATCH
const char* ImpactPlatformName = "watchOS";
#else
#error("Unsupported platform")
#endif

@implementation ImpactMonitor

+ (ImpactMonitor *)shared {
    static ImpactMonitor *sharedMyManager = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedMyManager = [[self alloc] init];
    });
    return sharedMyManager;
}

+ (NSInteger)buildNumber {
    return CURRENT_PROJECT_VERSION;
}

+(NSString *)platform {
    return [NSString stringWithUTF8String:ImpactPlatformName];
}

- (instancetype)init {
    self = [super init];
    if (self) {
        _suppressReportCrash = NO;
    }

    return self;
}

- (void)startWithURL:(NSURL *)url identifier:(NSUUID *)uuid {
    if (ImpactDebuggerAttached()) {
        NSLog(@"[Impact] Debugger attached, monitoring disabled");
        return;
    }

    GlobalImpactState = malloc(sizeof(ImpactState));

    GlobalImpactState->constantState.suppressReportCrash = self.suppressReportCrash == YES;

    atomic_store(&GlobalImpactState->mutableState.crashState, ImpactCrashStateUninitialized);

    ImpactResult result;

    NSLog(@"[Impact] trying to start with: %s", url.fileSystemRepresentation);
    
    result = ImpactLogInitialize(GlobalImpactState, url.fileSystemRepresentation);
    if (result != ImpactResultSuccess) {
        NSLog(@"[Impact] Unable to initialize log %d", result);
        return;
    }

    [self logExecutableData:GlobalImpactState];
    [self logEnvironmentDataWithId:uuid state:GlobalImpactState];

    result = ImpactSignalInitialize(GlobalImpactState);
    if (result != ImpactResultSuccess) {
        NSLog(@"[Impact] Unable to initialize signal %d", result);
    }

    result = ImpactBinaryImageInitialize(GlobalImpactState);
    if (result != ImpactResultSuccess) {
        NSLog(@"[Impact] Unable to initialize binary images %d", result);
        return;
    }

    result = ImpactRuntimeExceptionInitialize(GlobalImpactState);
    if (result != ImpactResultSuccess) {
        NSLog(@"[Impact] Unable to initialize run time exceptions %d", result);
    }

#if IMPACT_MACH_EXCEPTION_SUPPORTED
    result = ImpactMachExceptionInitialize(GlobalImpactState);
    if (result != ImpactResultSuccess) {
        NSLog(@"[Impact] Unable to initialize mach exceptions %d", result);
    }
#endif
    
    atomic_store(&GlobalImpactState->mutableState.crashState, ImpactCrashStateInitialized);

    ImpactDebugLog("[Log:INFO] finished initialization\n");
}

- (NSString *)OSVersionString {
    NSOperatingSystemVersion version = [[NSProcessInfo processInfo] operatingSystemVersion];

    return [NSString stringWithFormat:@"%d.%d.%d", (int)version.majorVersion, (int)version.minorVersion, (int)version.patchVersion];
}

- (NSString *)hardwareModel {
#if TARGET_OS_OSX
    char str[128] = {0};
    size_t size = sizeof(str);

    const int result = sysctlbyname("hw.model", str, &size, NULL, 0);
    if (result != 0) {
        return nil;
    }

    return [NSString stringWithUTF8String:str];
#else
    struct utsname systemInfo = {0};

    const int result = uname(&systemInfo);
    if (result != 0) {
        return nil;
    }

    return [NSString stringWithUTF8String:systemInfo.machine];
#endif
}

- (NSString *)simulatorModel {
    return NSProcessInfo.processInfo.environment[@"SIMULATOR_MODEL_IDENTIFIER"];
}

- (const char *)targetTranslated {
    int ret = 0;
    size_t size = sizeof(ret);

    if (sysctlbyname("sysctl.proc_translated", &ret, &size, NULL, 0) == -1) {
        if (errno == ENOENT) {
            return "false";
        }

        ImpactDebugLog("[Log:WARN] failed to determine if process is being translated\n");

        return "<unknown>";
    }

    return ret == 1 ? "true" : "false";
}

- (void)logExecutableData:(ImpactState *)state {
    ImpactLogger* log = ImpactStateGetLog(state);

    NSBundle *mainBundle = [NSBundle mainBundle];

    ImpactLogWriteString(log, "[Application] ");

    if (self.applicationIdentifier.length != 0) {
        ImpactLogWriteKeyStringObject(log, "id", self.applicationIdentifier, false);
    } else {
        ImpactLogWriteKeyStringObject(log, "id", [mainBundle bundleIdentifier], false);
    }

    ImpactLogWriteKeyStringObject(log, "org_id", self.organizationIdentifier, false);

    ImpactLogWriteKeyStringObject(log, "version", [mainBundle objectForInfoDictionaryKey:@"CFBundleVersion"], false);
    ImpactLogWriteKeyStringObject(log, "short_version", [mainBundle objectForInfoDictionaryKey:@"CFBundleShortVersionString"], true);
}

- (void)logEnvironmentDataWithId:(NSUUID *)identifier state:(ImpactState *)state {
    ImpactLogger* log = ImpactStateGetLog(state);

    ImpactLogWriteString(log, "[Environment] ");

    ImpactLogWriteKeyString(log, "platform", ImpactPlatformName, false);

    ImpactLogWriteKeyString(log, "arch", ImpactCPUArchitectureName, false);
    
    NSString* reportId = [[[identifier UUIDString] stringByReplacingOccurrencesOfString:@"-" withString:@""] lowercaseString];

    ImpactLogWriteKeyString(log, "report_id", reportId.UTF8String, false);
    ImpactLogWriteKeyStringObject(log, "install_id", self.installIdentifier, false);

    char str[256] = {0};
    size_t size = sizeof(str);

    int result = 0;

    result = sysctlbyname("kern.osversion", str, &size, NULL, 0);
    if (result == 0) {
        ImpactLogWriteKeyString(log, "os_build", str, false);
    } else {
        ImpactLogWriteKeyString(log, "os_build", "<unknown>", false);
    }

    ImpactLogWriteKeyStringObject(log, "model", self.hardwareModel , false);

    ImpactLogWriteKeyStringObject(log, "simulated_model", self.simulatorModel, false);

    ImpactLogWriteKeyString(log, "os_version", [self OSVersionString].UTF8String, false);

    ImpactLogWriteKeyString(log, "translated", self.targetTranslated, false);

    ImpactLogWriteKeyString(log, "region", [NSLocale currentLocale].countryCode.UTF8String, false);

    ImpactLogWriteKeyInteger(log, "pid", getpid(), false);
    ImpactLogWriteKeyInteger(log, "ppid", getppid(), false);

    ImpactLogWriteTime(log, "time", false);

    ImpactLogWriteKeyInteger(log, "impact_version", [self.class buildNumber], true);
}

@end
