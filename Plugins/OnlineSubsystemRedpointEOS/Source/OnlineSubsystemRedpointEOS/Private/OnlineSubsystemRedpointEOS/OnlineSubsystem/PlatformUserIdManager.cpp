// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/PlatformUserIdManager.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3149312102, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem)
{

TMap<FPlatformUserId, TTuple<TWeakPtr<FOnlineSubsystemEOS>, TSharedRef<const FUniqueNetIdEOS>>>
    FPlatformUserIdManager::AllocatedIds =
        TMap<FPlatformUserId, TTuple<TWeakPtr<FOnlineSubsystemEOS>, TSharedRef<const FUniqueNetIdEOS>>>();

}

REDPOINT_EOS_CODE_GUARD_END()