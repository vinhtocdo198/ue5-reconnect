// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatform/RuntimePlatform.h"

namespace REDPOINT_EOS_FILE_NS_ID(3334144570, Redpoint::EOS::Platform)
{

EOS_Platform_RTCOptions *IRuntimePlatform::GetRTCOptions()
{
    return nullptr;
}

FString IRuntimePlatform::GetAntiCheatPlatformName() const
{
    return TEXT("Unknown");
};

bool IRuntimePlatform::UseCrossPlatformFriendStorage() const
{
    return false;
};

void IRuntimePlatform::RegisterLifecycleHandlers(const TWeakPtr<ILifecycleManager> &InLifecycleManager)
{
}

void IRuntimePlatform::SetLifecycleForNewPlatformInstance(
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    API::FPlatformHandle InPlatform)
{
}

bool IRuntimePlatform::ShouldPollLifecycleApplicationStatus() const
{
    return false;
}

bool IRuntimePlatform::ShouldPollLifecycleNetworkStatus() const
{
    return false;
}

bool IRuntimePlatform::PollLifecycleApplicationStatus(EOS_EApplicationStatus &OutApplicationStatus) const
{
    OutApplicationStatus = EOS_EApplicationStatus::EOS_AS_Foreground;
    return true;
};

bool IRuntimePlatform::PollLifecycleNetworkStatus(EOS_ENetworkStatus &OutNetworkStatus) const
{
    OutNetworkStatus = EOS_ENetworkStatus::EOS_NS_Online;
    return true;
};

bool IRuntimePlatform::IsDelegatedSubsystemForciblyExcluded(FName InSubsystemName) const
{
    return false;
}

}