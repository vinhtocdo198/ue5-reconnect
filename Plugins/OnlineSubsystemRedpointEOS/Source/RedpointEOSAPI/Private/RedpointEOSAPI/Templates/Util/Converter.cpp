// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Templates/Util/ApiCallNativeConverter.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Templates::Util
{

FString FApiCallNativeConverter::FromAnsi(const char *InString) const
{
    REDPOINT_EOS_USE_LLM_TAG(API);

    if (InString == nullptr)
    {
        return FString();
    }

    return ANSI_TO_TCHAR(InString);
}

FString FApiCallNativeConverter::FromUtf8(const char *InString) const
{
    REDPOINT_EOS_USE_LLM_TAG(API);

    if (InString == nullptr)
    {
        return FString();
    }

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Whidden-reinterpret-cast"
#endif
    return UTF8_TO_TCHAR((const UTF8CHAR *)InString);
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
}

}

REDPOINT_EOS_CODE_GUARD_END()