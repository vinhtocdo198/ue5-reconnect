// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/Socket/SocketTracing.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSNetworking/Logging.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3942471015, Redpoint::EOS::Networking::Socket)
{
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Networking;

void LogSocketPacket(const FSendPacket::Options &Opts)
{
#if REDPOINT_EOS_ENABLE_SOCKET_LEVEL_NETWORK_TRACING
    UE_LOG(
        LogRedpointEOSSocket,
        Verbose,
        TEXT("%s:%d: '%s' -> '%s': %s"),
        **Opts.SocketId,
        *Opts.Channel,
        *GetUserIdString(GetAccountId(*Opts.LocalUserId)),
        *GetUserIdString(GetAccountId(*Opts.RemoteUserId)),
        *FString::FromBlob(static_cast<const uint8 *>(*Opts.Data), *Opts.DataLengthBytes));
#endif
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()