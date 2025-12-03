// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(260779903, Redpoint::EOS::Platform::Integration::Meta)
{

extern const FName PlatformNameMeta;

}

namespace Redpoint::EOS::Platform::Integration::Meta
{
REDPOINT_EOS_FILE_NS_EXPORT(260779903, Redpoint::EOS::Platform::Integration::Meta, PlatformNameMeta)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()