// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationItchIo/PlatformName.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_ITCH_IO_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(1779587970, Redpoint::EOS::Platform::Integration::ItchIo)
{

const FName PlatformNameItchIo = FName(TEXT("ItchIo"));

}

#endif

REDPOINT_EOS_CODE_GUARD_END()