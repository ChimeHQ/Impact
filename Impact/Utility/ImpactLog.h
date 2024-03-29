//
//  ImpactLog.h
//  Impact
//
//  Created by Matt Massicotte on 2019-09-18.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#ifndef ImpactLog_h
#define ImpactLog_h

#include "ImpactResult.h"
#include "ImpactState.h"

#include <sys/types.h>
#include <stdbool.h>

#if __OBJC__
#import <Foundation/Foundation.h>
#endif

_Pragma("clang assume_nonnull begin")
__BEGIN_DECLS

ImpactResult ImpactLogInitialize(ImpactState* state, const char* path);
ImpactResult ImpactLogDeinitialize(ImpactLogger* log);
bool ImpactLogIsValid(const ImpactLogger* log);

ImpactResult ImpactLog(const char * __restrict format, ...) __printflike(1, 2);

ImpactResult ImpactLogFlush(ImpactLogger* log);
ImpactResult ImpactLogWriteData(ImpactLogger* log, const char* data, size_t length);
ImpactResult ImpactLogWriteString(ImpactLogger* log, const char* string);
ImpactResult ImpactLogWriteInteger(ImpactLogger* log, uintptr_t number);

ImpactResult ImpactLogWriteKeyInteger(ImpactLogger* log, const char* key, uintptr_t number, bool last);
ImpactResult ImpactLogWriteKeyPointer(ImpactLogger* log, const char* key, const void* _Nullable ptr, bool last);
ImpactResult ImpactLogWriteKeyString(ImpactLogger* log, const char* key, const char* string, bool last);

#if __OBJC__
ImpactResult ImpactLogWriteKeyStringObject(ImpactLogger* log, const char* key, NSString* string, bool last);
#endif

ImpactResult ImpactLogWriteKeyHexData(ImpactLogger* log, const char* key, const uint8_t* _Nullable data, size_t length, bool last);
ImpactResult ImpactLogWriteTime(ImpactLogger* log, const char* key, bool last);

__END_DECLS
_Pragma("clang assume_nonnull end")

#endif /* ImpactLog_h */
