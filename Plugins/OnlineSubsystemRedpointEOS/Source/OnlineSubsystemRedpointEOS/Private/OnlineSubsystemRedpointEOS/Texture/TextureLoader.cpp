// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/Texture/TextureLoader.h"

#include "RedpointEOSCore/Utils/TextureLoader.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::Texture
{

UTexture *FTextureLoader::LoadTextureFromHttpResponse(const FHttpResponsePtr &Response)
{
    if (!Response.IsValid())
    {
        return nullptr;
    }

    return ::Redpoint::EOS::Core::Utils::FTextureLoader::LoadTextureFromHttpResponse(Response.ToSharedRef()).Get();
}

}

REDPOINT_EOS_CODE_GUARD_END()