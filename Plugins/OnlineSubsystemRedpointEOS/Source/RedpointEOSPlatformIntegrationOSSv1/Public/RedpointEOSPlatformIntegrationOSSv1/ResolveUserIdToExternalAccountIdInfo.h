// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Online/CoreOnline.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSPlatform/Types/ExternalAccountIdInfo.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(364030773, Redpoint::EOS::Platform::Integration::OSSv1)
{
using namespace ::Redpoint::EOS::Platform::Types;

typedef TFunction<TOptional<FExternalAccountIdInfo>(const FUniqueNetId &InUserId)>
    FResolveUserIdToExternalAccountIdInfo;

}; // namespace REDPOINT_EOS_FILE_NS_ID(364030773,Redpoint::EOS::Platform::Integration::OSSv1)

namespace Redpoint::EOS::Platform::Integration::OSSv1
{
REDPOINT_EOS_FILE_NS_EXPORT(
    364030773,
    Redpoint::EOS::Platform::Integration::OSSv1,
    FResolveUserIdToExternalAccountIdInfo)
}

REDPOINT_EOS_CODE_GUARD_END()