// Copyright June Rhodes. All Rights Reserved.

#include "RedpointGameSession.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "RedpointNotifyUniqueIdInterface.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

void ARedpointGameSession::RegisterPlayer(
    APlayerController *NewPlayer,
    const FUniqueNetIdRepl &UniqueId,
    bool bWasFromInvite)
{
    Super::RegisterPlayer(NewPlayer, UniqueId, bWasFromInvite);

    if (NewPlayer != nullptr && NewPlayer->PlayerState != nullptr)
    {
        for (UActorComponent *Component :
             NewPlayer->PlayerState->GetComponentsByInterface(URedpointNotifyUniqueIdInterface::StaticClass()))
        {
            IRedpointNotifyUniqueIdInterface::Execute_NotifyUniqueNetIdSet(
                Component,
                NewPlayer->PlayerState,
                NewPlayer->PlayerState->GetUniqueId());
        }
    }
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()