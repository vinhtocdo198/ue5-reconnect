// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "VoiceChat.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1380048048, Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat)
{

class ONLINESUBSYSTEMREDPOINTEOS_API FEOSVoiceManagerDevice : public FVoiceChatDeviceInfo
{
public:
    bool bIsDefaultDevice;

    FEOSVoiceManagerDevice()
        : FVoiceChatDeviceInfo()
        , bIsDefaultDevice(false)
    {
        this->Id = FString();
        this->DisplayName = FString();
    }

    FEOSVoiceManagerDevice(const FString &InId, const FString &InDisplayName, bool bInIsDefaultDevice)
        : FVoiceChatDeviceInfo()
        , bIsDefaultDevice(bInIsDefaultDevice)
    {
        this->Id = InId;
        this->DisplayName = InDisplayName;
    }
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat
{
REDPOINT_EOS_FILE_NS_EXPORT(1380048048, Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat, FEOSVoiceManagerDevice)
}

#endif // #if !REDPOINT_EOS_IS_DEDICATED_SERVER

REDPOINT_EOS_CODE_GUARD_END()