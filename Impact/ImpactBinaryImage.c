//
//  ImpactBinaryImage.c
//  Impact
//
//  Created by Matt Massicotte on 2019-10-02.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

#include "ImpactBinaryImage.h"
#include "ImpactCPU.h"
#include "ImpactUtility.h"
#include "ImpactLog.h"

#include <mach-o/dyld.h>
#include <mach-o/getsect.h>
#include <mach-o/dyld_images.h>
#include <dlfcn.h>
#include <mach-o/loader.h>
#include <TargetConditionals.h>

#include <string.h>

//static void ImpactBinaryImageAdded(const struct mach_header* mh, intptr_t vmaddr_slide);
//static void ImpactBinaryImageRemoved(const struct mach_header* mh, intptr_t vmaddr_slide);

static uint32_t ImpactBinaryImageNotFoundFlag = ~0;

static ImpactResult ImpactBinaryImageFindDyldInfo(struct task_dyld_info* info);

ImpactResult ImpactBinaryImageInitialize(ImpactState* state) {
//    _dyld_register_func_for_add_image(ImpactBinaryImageAdded);
//    _dyld_register_func_for_remove_image(ImpactBinaryImageRemoved);

    state->mutableState.images.lastFoundIndex = ImpactBinaryImageNotFoundFlag;
    state->mutableState.images.writtenIndex = ImpactBinaryImageNotFoundFlag;

    return ImpactBinaryImageFindDyldInfo(&state->mutableState.images.dyldInfo);
}

ImpactResult ImpactBinaryImageGetSectionData(const ImpactSegmentCommand* segCommand, ImpactMachOData* data, intptr_t slide) {
    if (ImpactInvalidPtr(segCommand) || ImpactInvalidPtr(data)) {
        return ImpactResultPointerInvalid;
    }

    const uint8_t *ptr = (uint8_t *)segCommand + sizeof(ImpactSegmentCommand);
    uint32_t foundSections = 0;

    for (uint32_t i = 0; i < segCommand->nsects && foundSections < 2; ++i) {
        const ImpactSection* const section = (ImpactSection*)ptr;

        if (strncmp(section->sectname, "__unwind_info", 13) == 0) {
            data->unwindInfoRegion.address = section->addr + slide;
            data->unwindInfoRegion.loadAddress = section->addr;
            data->unwindInfoRegion.length = section->size;

            foundSections++;
        } else if (strncmp(section->sectname, "__eh_frame", 10) == 0) {
            data->ehFrameRegion.address = section->addr + slide;
            data->ehFrameRegion.loadAddress = section->addr;
            data->ehFrameRegion.length = section->size;

            foundSections++;
        }

        ptr += sizeof(ImpactSection);
    }

    return ImpactResultSuccess;
}

ImpactResult ImpactBinaryImageGetData(const ImpactMachOHeader* header, const char* path, ImpactMachOData* data) {
    if (ImpactInvalidPtr(header) || ImpactInvalidPtr(data)) {
        return ImpactResultPointerInvalid;
    }

    const uint8_t *ptr = (uint8_t *)header + sizeof(ImpactMachOHeader);

    data->loadAddress = (uintptr_t)header;
    data->path = path;

    for (uint32_t i = 0; i < header->ncmds; ++i) {
        const struct load_command* const lcmd = (struct load_command*)ptr;
        const uint32_t cmdCode = lcmd->cmd & ~LC_REQ_DYLD;

        switch (cmdCode) {
            case LC_UUID:
                data->uuid = ((struct uuid_command*)ptr)->uuid;

                break;
            case Impact_LC_SEGMENT: {
                const ImpactSegmentCommand* segCommand = (const ImpactSegmentCommand*)lcmd;

                if (strncmp(segCommand->segname, "__TEXT", 6) != 0) {
                    break;
                }

                data->slide = (uintptr_t)header - segCommand->vmaddr;
                data->textSize = segCommand->vmsize;

                ImpactBinaryImageGetSectionData(segCommand, data, data->slide);
            } break;
            default:
                break;
        }

        ptr += lcmd->cmdsize;
    }

    return ImpactResultSuccess;
}

static ImpactResult ImpactBinaryImageLogPath(ImpactLogger* log, const char* path, bool last) {
#if TARGET_OS_OSX
    // This strips out the username, should it be present in the path
    if (strncmp(path, "/Users/", 7) == 0) {
        const char *sanitizedPath = path + 7; // advance the string past the initial "/Users/" path component

        sanitizedPath = strchr(sanitizedPath, '/');
        if (sanitizedPath != NULL) {
            ImpactLogWriteString(log, "path: /Users/USER");
            ImpactLogWriteString(log, sanitizedPath);

            return ImpactLogWriteString(log, last ? "\n" : ", ");
        }
    }
#endif

    return ImpactLogWriteKeyString(log, "path", path, last);
}

static ImpactResult ImpactBinaryImageLog(ImpactLogger* log, const ImpactMachOData* imageData) {
    if (ImpactInvalidPtr(log) || ImpactInvalidPtr(imageData)) {
        return ImpactResultPointerInvalid;
    }

    ImpactLogWriteString(log, "[Binary:Found] ");

    ImpactBinaryImageLogPath(log, imageData->path, false);
    ImpactLogWriteKeyInteger(log, "address", imageData->loadAddress, false);
    ImpactLogWriteKeyInteger(log, "size", imageData->textSize, false);
    ImpactLogWriteKeyHexData(log, "uuid", imageData->uuid, 16, true);

    return ImpactResultSuccess;
}

static ImpactResult ImpactBinaryImageFindDyldInfo(struct task_dyld_info* info) {
    mach_msg_type_number_t count = TASK_DYLD_INFO_COUNT;

    const kern_return_t result = task_info(mach_task_self(), TASK_DYLD_INFO, (task_info_t)info, &count);
    if (result != KERN_SUCCESS) {
        ImpactDebugLog("[Log:ERROR:%s] unable to lookup task dyld info %d\n", __func__, result);
        return ImpactResultCallFailed;
    }

    return ImpactResultSuccess;
}

ImpactResult ImpactBinaryImageGetDyldImageData(const struct dyld_all_image_infos* imagesInfo, const int index, ImpactMachOData* data) {
    if (index > imagesInfo->infoArrayCount) {
        return ImpactResultArgumentInvalid;
    }

    const struct dyld_image_info* imageInfo = imagesInfo->infoArray + index;
    const ImpactMachOHeader* const header = (ImpactMachOHeader*)imageInfo->imageLoadAddress;
    const char* path = imageInfo->imageFilePath;

    return ImpactBinaryImageGetData(header, path, data);
}

static bool ImpactMachODataContainsAddress(const ImpactMachOData* data, uintptr_t address) {
    if (ImpactInvalidPtr(data)) {
        return false;
    }

    const uintptr_t upperAddress = data->loadAddress + data->textSize;

    return address >= data->loadAddress && address < upperAddress;
}

ImpactResult ImpactBinaryImageFind(ImpactState* state, uintptr_t address, ImpactMachOData* data) {
    if (ImpactInvalidPtr(state) || ImpactInvalidPtr(data)) {
        return ImpactResultPointerInvalid;
    }

    if (ImpactInvalidPtr((const void*)address)) {
        return ImpactResultArgumentInvalid;
    }

    ImpactLogger* logger = ImpactStateGetLog(state);

    ImpactBinaryImages* images = &state->mutableState.images;
    const struct dyld_all_image_infos* imagesInfo = (void *)images->dyldInfo.all_image_info_addr;
    const size_t imageCount = imagesInfo->infoArrayCount;

    // first, check the last match, as it's likely these repeat
    ImpactMachOData imageData = {0};

    if (images->lastFoundIndex != ImpactBinaryImageNotFoundFlag) {
        const ImpactResult result = ImpactBinaryImageGetDyldImageData(imagesInfo, images->lastFoundIndex, &imageData);
        if (result != ImpactResultSuccess) {
            return result;
        }

        if (ImpactMachODataContainsAddress(&imageData, address)) {
            *data = imageData;
            return ImpactResultSuccess;
        }
    }

    for (int i = 0; i < imageCount; ++i) {
        const ImpactResult result = ImpactBinaryImageGetDyldImageData(imagesInfo, i, &imageData);
        if (result != ImpactResultSuccess) {
            return result;
        }

        if (i > images->writtenIndex || images->writtenIndex == ImpactBinaryImageNotFoundFlag) {
            // There isn't an obvious action to take on error here, so we can just ignore
            // for now.
            ImpactBinaryImageLog(logger, &imageData);

            images->writtenIndex = i;
        }

        if (ImpactMachODataContainsAddress(&imageData, address)) {
            *data = imageData;
            images->lastFoundIndex = i;
            return ImpactResultSuccess;
        }
    }

    return ImpactResultFailure;
}

ImpactResult ImpactBinaryImageLogRemainingImages(ImpactState* state) {
    if (ImpactInvalidPtr(state)) {
        return ImpactResultPointerInvalid;
    }

    ImpactLogger* logger = ImpactStateGetLog(state);

    ImpactBinaryImages* images = &state->mutableState.images;
    const struct dyld_all_image_infos* imagesInfo = (void *)images->dyldInfo.all_image_info_addr;
    const size_t imageCount = imagesInfo->infoArrayCount;

    ImpactMachOData imageData = {0};

    for (int i = images->writtenIndex + 1; i < imageCount; ++i) {
        ImpactResult result = ImpactBinaryImageGetDyldImageData(imagesInfo, i, &imageData);
        if (result != ImpactResultSuccess) {
            return result;
        }

        result = ImpactBinaryImageLog(logger, &imageData);
        if (result != ImpactResultSuccess) {
            return result;
        }
    }

    return ImpactResultSuccess;
}
