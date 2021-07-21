//
//  ImpactLog.c
//  Impact
//
//  Created by Matt Massicotte on 2019-09-18.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#include "ImpactLog.h"
#include "ImpactPointer.h"

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

ImpactResult ImpactLogInitialize(ImpactState* state, const char* _Nonnull path) {
    if (ImpactInvalidPtr(state) || ImpactInvalidPtr(path)) {
        return ImpactResultPointerInvalid;
    }

    int fd = open(path, O_WRONLY | O_APPEND | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        return ImpactResultFailure;
    }

    state->mutableState.log.fd = fd;

    state->mutableState.log.bufferCount = 0;
    memset(state->mutableState.log.buffer, 0, ImpactLogBufferSize);

    return ImpactResultSuccess;
}

ImpactResult ImpactLogDeinitialize(ImpactLogger* _Nonnull log) {
    if (ImpactInvalidPtr(log)) {
        return ImpactResultPointerInvalid;
    }

    return ImpactResultFailure;
}

bool ImpactLogIsValid(const ImpactLogger* log) {
    if (ImpactInvalidPtr(log)) {
        return false;
    }

    if (log->fd <= 0) {
        return false;
    }

    return log->bufferCount <= ImpactLogBufferSize;
}

ImpactResult ImpactLogWriteData(ImpactLogger* log, const char* data, size_t length) {
    if (length == 0) {
        return ImpactResultSuccess;
    }

    for(uint32_t i = 0; i < 100; ++i) {
        const ssize_t spaceRemaining = ImpactLogBufferSize - log->bufferCount;
        ssize_t chunkSize = MIN(length, spaceRemaining);
        if (chunkSize <= 0) {
            const ImpactResult result = ImpactLogFlush(log);
            if (result != ImpactResultSuccess) {
                return result;
            }

            chunkSize = MIN(length, ImpactLogBufferSize);
        }

        void* buffer = log->buffer + log->bufferCount;

        memcpy(buffer, data, chunkSize);

        log->bufferCount += chunkSize;
        length -= chunkSize;
        data += chunkSize;

        if (length == 0) {
            return ImpactResultSuccess;
        }
    }

    return ImpactResultTooManyIterations;
}

ImpactResult ImpactLogFlush(ImpactLogger* log) {
    size_t writeCount = log->bufferCount;
    const void* data = log->buffer;

    while (writeCount > 0) {
        const ssize_t count = write(log->fd, data, writeCount);

        if (count == -1) {
            return ImpactResultCallFailed;
        }

        // this accounts for a partial write
        writeCount -= count;
        data = log->buffer + count;
    }

    log->bufferCount = 0;

    return ImpactResultSuccess;
}

static char ImpactLogValueToHexChar(uint8_t value) {
    uint8_t maskedValue = value & 0x0F;

    if (maskedValue >= 10) {
        return maskedValue - 10 + 'a';
    } else {
        return maskedValue + '0';
    }
}

ImpactResult ImpactLogWriteString(ImpactLogger* log, const char* string) {
    if (!ImpactLogIsValid(log)) {
        return ImpactResultArgumentInvalid;
    }

    if (ImpactInvalidPtr(string)) {
        return ImpactResultArgumentInvalid;
    }

    return ImpactLogWriteData(log, string, strlen(string));
}

ImpactResult ImpactLogWriteInteger(ImpactLogger* log, uintptr_t number) {
    if (!ImpactLogIsValid(log)) {
        return ImpactResultArgumentInvalid;
    }

    char buffer[16];
    char *ptr = buffer + sizeof(buffer) - 1;

    memset(buffer, '0', sizeof(buffer));

    while (number > 0) {
        int modResult = number % 16;

        *ptr = ImpactLogValueToHexChar(modResult);

        number = number / 16;

        if (number > 0) {
            ptr -= 1;
        }
    }

    ImpactLogWriteString(log, "0x");

    const size_t length = sizeof(buffer) - (ptr - buffer);

    return ImpactLogWriteData(log, ptr, length);
}

ImpactResult ImpactLogWriteHexData(ImpactLogger* log, const uint8_t* data, size_t length) {
    char buffer[2] = {0};

    for (size_t i = 0; i < length; ++i) {
        buffer[1] = ImpactLogValueToHexChar(data[i]);
        buffer[0] = ImpactLogValueToHexChar(data[i] >> 4);

        ImpactLogWriteData(log, buffer, 2);
    }

    return ImpactResultSuccess;
}

ImpactResult ImpactLogWriteSeparator(ImpactLogger* log, bool ending) {
    const ImpactResult result = ImpactLogWriteString(log, ending ? "\n" : ", ");
    if (result != ImpactResultSuccess) {
        return result;
    }

    if (ending) {
        return ImpactLogFlush(log);
    }

    return ImpactResultSuccess;
}

ImpactResult ImpactLogWriteKeyInteger(ImpactLogger* log, const char* key, uintptr_t number, bool last) {
    ImpactLogWriteString(log, key);
    ImpactLogWriteString(log, ": ");
    ImpactLogWriteInteger(log, number);

    return ImpactLogWriteSeparator(log, last);
}

ImpactResult ImpactLogWriteKeyPointer(ImpactLogger* log, const char* key, const void* ptr, bool last) {
    return ImpactLogWriteKeyInteger(log, key, (uintptr_t)ptr, last);
}

ImpactResult ImpactLogWriteKeyString(ImpactLogger* log, const char* key, const char* string, bool last) {
    ImpactLogWriteString(log, key);
    ImpactLogWriteString(log, ": ");
    ImpactLogWriteString(log, string);

    return ImpactLogWriteSeparator(log, last);
}

ImpactResult ImpactLogWriteKeyStringObject(ImpactLogger* log, const char* key, NSString* string, bool last) {
    if (string.length == 0) {
        return ImpactLogWriteKeyString(log, key, "<none>", last);
    }

    NSString* encodedString = [[string dataUsingEncoding:NSUTF8StringEncoding] base64EncodedStringWithOptions:0];

    return ImpactLogWriteKeyString(log, key, encodedString.UTF8String, last);
}

ImpactResult ImpactLogWriteKeyHexData(ImpactLogger* log, const char* key, const uint8_t* _Nullable data, size_t length, bool last) {
    ImpactLogWriteString(log, key);
    ImpactLogWriteString(log, ": ");
    ImpactLogWriteHexData(log, data, length);

    return ImpactLogWriteSeparator(log, last);
}

ImpactResult ImpactLogWriteTime(ImpactLogger* log, const char* key, bool last) {
    const uint64_t epochTimeMS = time(NULL) * 1000;

    return ImpactLogWriteKeyInteger(log, key, epochTimeMS, last);
}
