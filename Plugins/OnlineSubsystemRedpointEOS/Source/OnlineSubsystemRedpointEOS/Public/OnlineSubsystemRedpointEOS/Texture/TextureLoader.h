// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Engine/Texture.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::Texture
{

class ONLINESUBSYSTEMREDPOINTEOS_API FTextureLoader
{
public:
    static UTexture *LoadTextureFromHttpResponse(const FHttpResponsePtr &Response);
};

}

REDPOINT_EOS_CODE_GUARD_END()