// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/VoiceChat/EOSVoiceManagerLocalUserRemoteUser.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3314572764, Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat)
{

FEOSVoiceManagerLocalUserRemoteUser::FEOSVoiceManagerLocalUserRemoteUser(
    const TSharedRef<const FUniqueNetIdEOS> &InUserId)
    : UserId(InUserId)
    , bIsTalking(false)
    , bIsMuted(false)
{
}

}

#endif // #if !REDPOINT_EOS_IS_DEDICATED_SERVER

REDPOINT_EOS_CODE_GUARD_END()