// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Engine/Texture.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1029236471, Redpoint::EOS::Core::Utils)
{

class REDPOINTEOSCORE_API FTextureLoader
{
public:
    FTextureLoader() = default;
    UE_NONCOPYABLE(FTextureLoader);
    ~FTextureLoader() = default;

    typedef TDelegate<void(const TSoftObjectPtr<UTexture> &Texture)> FOnTextureLoadComplete;

    static TSoftObjectPtr<UTexture> LoadTextureFromHttpResponse(const FHttpResponseRef &Response);
    static void LoadTextureFromHttpUrl(const FString &Url, const FOnTextureLoadComplete &OnComplete);
};

}

namespace Redpoint::EOS::Core::Utils
{
REDPOINT_EOS_FILE_NS_EXPORT(1029236471, Redpoint::EOS::Core::Utils, FTextureLoader)
}

REDPOINT_EOS_CODE_GUARD_END()