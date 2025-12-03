// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/Map.h"
#include "Online/CoreOnline.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/UniqueNetIdEOS.h"
#include "Templates/Tuple.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

class FOnlineSubsystemEOS;

namespace REDPOINT_EOS_FILE_NS_ID(3149312102, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem)
{

/**
 * @note: This API should be removed once the voice chat system uses FPlatformHandle instead of
 * calling back into the OSSv1 module.
 */
class ONLINESUBSYSTEMREDPOINTEOS_API FPlatformUserIdManager
{
public:
    typedef TTuple<TWeakPtr<FOnlineSubsystemEOS>, TSharedRef<const FUniqueNetIdEOS>> FValue;

    static TMap<FPlatformUserId, FValue> AllocatedIds;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem
{
REDPOINT_EOS_FILE_NS_EXPORT(
    3149312102,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem,
    FPlatformUserIdManager)
}

REDPOINT_EOS_CODE_GUARD_END()