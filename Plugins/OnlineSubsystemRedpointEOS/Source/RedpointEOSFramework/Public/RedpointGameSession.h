// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "GameFramework/GameSession.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointGameSession.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS(Blueprintable, Category = "Redpoint EOS Online Framework")
class REDPOINTEOSFRAMEWORK_API ARedpointGameSession : public AGameSession
{
    GENERATED_BODY()

public:
    virtual void RegisterPlayer(APlayerController *NewPlayer, const FUniqueNetIdRepl &UniqueId, bool bWasFromInvite)
        override;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()