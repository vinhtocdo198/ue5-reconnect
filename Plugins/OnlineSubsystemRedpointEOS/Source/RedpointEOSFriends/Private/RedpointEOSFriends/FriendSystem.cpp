// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSFriends/FriendSystem.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSCore/Utils/MakeSharedWithEvents.h"
#include "RedpointEOSFriends/FriendSystemImpl.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(4025721429, Redpoint::EOS::Friends)
{
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Friends;

FName IFriendSystem::GetSystemName()
{
    return FName(TEXT("IFriendSystem"));
}

TSharedRef<IFriendSystem> IFriendSystem::ConstructSystem(
    FPlatformHandle InPlatformHandle,
    FPlatformInstanceEnvironmentRef InPlatformInstanceEnvironment)
{
    return MakeSharedWithEvents<FFriendSystemImpl>(InPlatformHandle);
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()