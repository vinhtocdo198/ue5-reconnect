// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Engine/NetConnection.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAntiCheat/AntiCheatSession.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSNetworking/NetDriverRole.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2294846777, Redpoint::EOS::Networking::Auth)
{
using namespace ::Redpoint::EOS::Networking;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::AntiCheat;
using namespace ::Redpoint::EOS::Config;

class FNetworkHelpers
{
public:
    static const IConfig *GetConfig(UNetConnection *Connection);
    static ENetDriverRole GetRole(UNetConnection *Connection);
    static void GetPlatformHandle(UNetConnection *Connection, TSharedPtr<FPlatformInstance> &OutPlatformHandle);
    static void GetAntiCheat(
        UNetConnection *Connection,
        TSharedPtr<FPlatformInstance> &OutPlatformHandle,
        TSharedPtr<FAntiCheatSession> &OutAntiCheatSession,
        bool &OutIsBeacon);
    static ISocketSubsystem *GetSocketSubsystem(UNetConnection *Connection);
};

}

namespace Redpoint::EOS::Networking::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(2294846777, Redpoint::EOS::Networking::Auth, FNetworkHelpers)
}

REDPOINT_EOS_CODE_GUARD_END()