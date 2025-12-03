// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointAvatarLoadingIndicator.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UENUM(BlueprintType, Category = "Redpoint EOS Online Framework")
enum class ERedpointAvatarLoadingIndicator : uint8
{
    /**
     * The avatar widget renders nothing while loading.
     */
    None,

    /**
     * The avatar widget renders a linear throbber while the avatar is loading.
     */
    Throbber,

    /**
     * The avatar widget renders the default texture while the avatar is loading.
     */
    DefaultTexture,
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()
