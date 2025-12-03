// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/NetworkingSystem.h"

#include "RedpointEOSNetworking/NetworkingSystemImpl.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1078758172, Redpoint::EOS::Networking)
{
using namespace ::Redpoint::EOS::Networking;

FName INetworkingSystem::GetSystemName()
{
    return FName(TEXT("INetworkingSystem"));
}

TSharedRef<INetworkingSystem> INetworkingSystem::ConstructSystem(
    FPlatformHandle InPlatformHandle,
    FPlatformInstanceEnvironmentRef InPlatformInstanceEnvironment)
{
    return MakeShared<FNetworkingSystemImpl>();
}

}

REDPOINT_EOS_CODE_GUARD_END()