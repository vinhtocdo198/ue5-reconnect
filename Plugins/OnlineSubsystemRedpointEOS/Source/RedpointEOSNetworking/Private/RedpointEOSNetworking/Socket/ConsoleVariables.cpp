// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/Socket/ConsoleVariables.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && !(defined(UE_BUILD_SHIPPING) && UE_BUILD_SHIPPING)

namespace REDPOINT_EOS_FILE_NS_ID(3821566149, Redpoint::EOS::Networking::Socket)
{

TAutoConsoleVariable<bool> CVarOnDemandPacketDispatch(
    TEXT("t.EOSOnDemandPacketDispatch"),
    false,
    TEXT("When on, EOS calls EOS_P2P_ReceivePacket every time a socket has RecvFrom or HasPendingData called on it, "
         "instead of only receiving packets at the start of a frame."));

TAutoConsoleVariable<bool> CVarEnablePacketTiming(
    TEXT("t.EOSEnablePacketTiming"),
    false,
    TEXT("When on, EOS includes timestamps as packets make their way through the system. This uses the tick/nanosecond "
         "measurement of the local computer, so reported values will only be correct when using play-in-editor. "
         "Connections to different computers will not be accurate as the clocks will not be in alignment."));

}

#endif

REDPOINT_EOS_CODE_GUARD_END()