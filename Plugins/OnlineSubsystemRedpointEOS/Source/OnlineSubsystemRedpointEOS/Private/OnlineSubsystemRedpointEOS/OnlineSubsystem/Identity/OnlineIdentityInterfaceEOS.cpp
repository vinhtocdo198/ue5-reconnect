// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Identity/OnlineIdentityInterfaceEOS.h"

#include "OnlineSubsystemRedpointEOS/Shared/EOSCommon.h"
#include "OnlineSubsystemRedpointEOS/Shared/EpicGames/OnlineSubsystemRedpointEAS.h"
#include "OnlineSubsystemUtils.h"
#include "RedpointEOSCore/Utils/WorldResolution.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1057129160, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Identity)
{
using namespace ::Redpoint::EOS::Core::Utils;

IOnlineSubsystem *IOnlineIdentityInterfaceEOS::GetNativeSubsystem(
    int32 LocalUserNum,
    bool bObtainRedpointImplementation) const
{
    FName NativeSubsystemName;
    bool bFoundNativeSubsystemName = this->GetNativeSubsystemName(LocalUserNum, NativeSubsystemName);

    if (!bFoundNativeSubsystemName)
    {
        if (bObtainRedpointImplementation)
        {
            return nullptr;
        }
        else
        {
            return IOnlineSubsystem::GetByPlatform();
        }
    }

#if EOS_HAS_AUTHENTICATION
    if (NativeSubsystemName.IsEqual(REDPOINT_EAS_SUBSYSTEM))
    {
        return this->GetEpicGamesOnlineSubsystem().Get();
    }
#endif // #if EOS_HAS_AUTHENTICATION

    TSoftObjectPtr<UWorld> World = FWorldResolution::GetWorld(this->GetInstanceName(), true);
    if (!World.IsValid())
    {
        if (bObtainRedpointImplementation)
        {
            return nullptr;
        }
        else
        {
            return IOnlineSubsystem::GetByPlatform();
        }
    }

    return Online::GetSubsystem(
        World.Get(),
        bObtainRedpointImplementation ? FName(*FString::Printf(TEXT("Redpoint%s"), *NativeSubsystemName.ToString()))
                                      : NativeSubsystemName);
}

IOnlineSubsystem *IOnlineIdentityInterfaceEOS::GetNativeSubsystem(
    const FUniqueNetId &UserIdEOS,
    bool bObtainRedpointImplementation) const
{
    int32 LocalUserNum;
    if (this->GetLocalUserNum(UserIdEOS, LocalUserNum))
    {
        return this->GetNativeSubsystem(LocalUserNum, bObtainRedpointImplementation);
    }
    return nullptr;
}

}

REDPOINT_EOS_CODE_GUARD_END()