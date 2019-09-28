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

ImpactResult ImpactLogInitialize(ImpactState* state, const char* _Nonnull path) {
    if (ImpactInvalidPtr(state) || ImpactInvalidPtr(path)) {
        return ImpactResultArgumentNull;
    }

    int fd = open(path, O_WRONLY | O_APPEND | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        return ImpactResultFailure;
    }

    state->constantState.log.fd = fd;

    return ImpactResultSuccess;
}

ImpactResult ImpactLogDeinitialize(ImpactLogger* _Nonnull log) {
    if (ImpactInvalidPtr(log)) {
        return ImpactResultArgumentNull;
    }

    return ImpactResultFailure;
}

bool ImpactLogIsValid(const ImpactLogger* log) {
    if (ImpactInvalidPtr(log)) {
        return false;
    }

    return log->fd > 0;
}

ImpactResult ImpactLogWriteData(const ImpactLogger* log, const char* data, size_t length) {
    write(log->fd, data, length);

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

ImpactResult ImpactLogWriteString(const ImpactLogger* log, const char* string) {
    if (!ImpactLogIsValid(log)) {
        return ImpactResultArgumentInvalid;
    }

    if (ImpactInvalidPtr(string)) {
        return ImpactResultArgumentInvalid;
    }

    return ImpactLogWriteData(log, string, strlen(string));
}

ImpactResult ImpactLogWriteInteger(const ImpactLogger* log, uintptr_t number) {
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

ImpactResult ImpactLogWriteKeyInteger(const ImpactLogger* log, const char* key, uintptr_t number) {
    ImpactLogWriteString(log, key);
    ImpactLogWriteString(log, ": ");
    ImpactLogWriteInteger(log, number);
    ImpactLogWriteString(log, ", ");

    return ImpactResultSuccess;
}
