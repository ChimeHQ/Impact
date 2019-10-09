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

#include <string.h>

#if __LP64__
typedef struct mach_header_64 ImpactMachOHeader;
typedef struct section_64 ImpactMachOSection;
typedef struct segment_command_64 ImpactSegmentCommand;
typedef struct section_64 ImpactSection;

#define Impact_LC_SEGMENT LC_SEGMENT_64
#else
typedef struct mach_header ImpactMachOHeader;
typedef struct section ImpactMachOSection;
typedef struct segment_command ImpactSegmentCommand;
typedef struct section ImpactSection;

#define Impact_LC_SEGMENT LC_SEGMENT
#endif

static void ImpactBinaryImageAdded(const struct mach_header* mh, intptr_t vmaddr_slide);
static void ImpactBinaryImageRemoved(const struct mach_header* mh, intptr_t vmaddr_slide);

static ImpactResult ImpactBinaryImageFindDyldInfo(struct task_dyld_info* info);

ImpactResult ImpactBinaryImageInitialize(ImpactState* state) {
    _dyld_register_func_for_add_image(ImpactBinaryImageAdded);
    _dyld_register_func_for_remove_image(ImpactBinaryImageRemoved);

    return ImpactBinaryImageFindDyldInfo(&state->constantState.dyldInfo);
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
            data->unwindInfoRegion.length = section->size;

            foundSections++;
        } else if (strncmp(section->sectname, "__eh_frame", 10) == 0) {
            data->ehFrameRegion.address = section->addr + slide;
            data->ehFrameRegion.length = section->size;

            foundSections++;
        }

        ptr += sizeof(ImpactSection);
    }

    return ImpactResultSuccess;
}

ImpactResult ImpactBinaryImageGetData(const ImpactMachOHeader* header, ImpactMachOData* data) {
    if (ImpactInvalidPtr(header) || ImpactInvalidPtr(data)) {
        return ImpactResultPointerInvalid;
    }

    const uint8_t *ptr = (uint8_t *)header + sizeof(ImpactMachOHeader);

    data->loadAddress = (uintptr_t)header;

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

static ImpactResult ImpactBinaryImageLog(ImpactState* state, const ImpactMachOData* imageData, const char* path) {
    if (ImpactInvalidPtr(state) || ImpactInvalidPtr(imageData) || ImpactInvalidPtr(path)) {
        return ImpactResultPointerInvalid;
    }

    ImpactLogger* log = &state->constantState.log;

    ImpactLogWriteString(log, "[Binary:Load] ");

    ImpactLogWriteKeyString(log, "path", path, false);
    ImpactLogWriteKeyInteger(log, "address", imageData->loadAddress, false);
    ImpactLogWriteKeyInteger(log, "size", imageData->textSize, false);
    ImpactLogWriteKeyInteger(log, "slide", imageData->slide, false);
    ImpactLogWriteKeyHexData(log, "uuid", imageData->uuid, 16, true);

    return ImpactResultSuccess;
}

static void ImpactBinaryImageAdded(const struct mach_header* mh, intptr_t vmaddr_slide) {
    const ImpactMachOHeader* const header = (void *)mh;

    Dl_info info;

    int result = dladdr(mh, &info);
    if (result == 0) {
        ImpactDebugLog("[Log:ERROR:%s] unable to lookup %p\n", __func__, mh);
        return;
    }

    ImpactMachOData imageData = {0};

    ImpactBinaryImageGetData(header, &imageData);

    if (vmaddr_slide != imageData.slide) {
        ImpactDebugLog("[Log:WARN:%s] slides unequal\n", __func__);
    }

    ImpactBinaryImageLog(GlobalImpactState, &imageData, info.dli_fname);
}

static void ImpactBinaryImageRemoved(const struct mach_header* mh, intptr_t vmaddr_slide) {

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

ImpactResult ImpactBinaryImageFind(const ImpactState* state, uintptr_t address, ImpactMachOData* data) {
    if (ImpactInvalidPtr(state) || ImpactInvalidPtr(data)) {
        return ImpactResultPointerInvalid;
    }

    const struct dyld_all_image_infos* imagesInfo = (void *)state->constantState.dyldInfo.all_image_info_addr;
    const size_t imageCount = imagesInfo->infoArrayCount;

    for (int i = 0; i < imageCount; ++i) {
        const struct dyld_image_info* imageInfo = imagesInfo->infoArray + i;
        const ImpactMachOHeader* const header = (ImpactMachOHeader*)imageInfo->imageLoadAddress;

        ImpactMachOData imageData = {0};

        if (ImpactBinaryImageGetData(header, &imageData) != ImpactResultSuccess) {
            continue;
        }

        const uintptr_t upperAddress = imageData.loadAddress + imageData.textSize;

        if (address >= imageData.loadAddress && address < upperAddress) {
            *data = imageData;
            return ImpactResultSuccess;
        }
    }

    return ImpactResultFailure;
}

