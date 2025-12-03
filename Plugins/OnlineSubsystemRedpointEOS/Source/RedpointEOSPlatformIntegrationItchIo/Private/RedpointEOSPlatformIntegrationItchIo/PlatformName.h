// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_ITCH_IO_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(1779587970, Redpoint::EOS::Platform::Integration::ItchIo)
{

extern const FName PlatformNameItchIo;

}

namespace Redpoint::EOS::Platform::Integration::ItchIo
{
REDPOINT_EOS_FILE_NS_EXPORT(1779587970, Redpoint::EOS::Platform::Integration::ItchIo, PlatformNameItchIo)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()