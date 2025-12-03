// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if PLATFORM_ANDROID

namespace REDPOINT_EOS_FILE_NS_ID(1687685427, Redpoint::EOS::Platform::Android)
{

class REDPOINTEOSPLATFORM_API FAndroidEnvironment
{
public:
    static bool IsMetaQuest();
};

}

namespace Redpoint::EOS::Platform::Android
{
REDPOINT_EOS_FILE_NS_EXPORT(1687685427, Redpoint::EOS::Platform::Android, FAndroidEnvironment)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()