// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationMeta/PlatformName.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(260779903, Redpoint::EOS::Platform::Integration::Meta)
{

const FName PlatformNameMeta = FName(TEXT("Meta"));

}

#endif

REDPOINT_EOS_CODE_GUARD_END()