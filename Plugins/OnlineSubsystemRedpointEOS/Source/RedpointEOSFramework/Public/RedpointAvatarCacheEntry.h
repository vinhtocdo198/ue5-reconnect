// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Engine/Texture.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointAvatarCacheEntry.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

USTRUCT(Category = "Redpoint EOS Online Framework")
struct REDPOINTEOSFRAMEWORK_API FRedpointAvatarCacheEntry
{
    GENERATED_BODY()

public:
    UPROPERTY()
    TObjectPtr<UTexture> Avatar;

    UPROPERTY()
    FDateTime ExpiresAt;

    UPROPERTY()
    bool bFetchInProgress = false;

    UPROPERTY()
    bool bFailed = false;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()
