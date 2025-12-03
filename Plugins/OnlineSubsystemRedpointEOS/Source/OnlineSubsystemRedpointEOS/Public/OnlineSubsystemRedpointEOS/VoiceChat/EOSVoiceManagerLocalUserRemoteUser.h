// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/UniqueNetIdEOS.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3314572764, Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat)
{

class ONLINESUBSYSTEMREDPOINTEOS_API FEOSVoiceManagerLocalUserRemoteUser
{
public:
    FEOSVoiceManagerLocalUserRemoteUser(const TSharedRef<const FUniqueNetIdEOS> &InUserId);
    UE_NONCOPYABLE(FEOSVoiceManagerLocalUserRemoteUser);
    virtual ~FEOSVoiceManagerLocalUserRemoteUser() = default;

    const TSharedRef<const FUniqueNetIdEOS> UserId;
    bool bIsTalking;
    bool bIsMuted;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat
{
REDPOINT_EOS_FILE_NS_EXPORT(
    3314572764,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat,
    FEOSVoiceManagerLocalUserRemoteUser)
}

#endif // #if !REDPOINT_EOS_IS_DEDICATED_SERVER

REDPOINT_EOS_CODE_GUARD_END()