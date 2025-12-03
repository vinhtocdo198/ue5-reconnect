// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "VoiceChat.h"

namespace REDPOINT_EOS_FILE_NS_ID(495223541, Redpoint::EOS::Tests::VoiceChat)
{

struct FVoiceChatUserHolder
{
private:
    IVoiceChatUser *VoiceChatUser;

public:
    FVoiceChatUserHolder();
    FVoiceChatUserHolder(IVoiceChatUser *InVoiceChatUser);
    UE_NONCOPYABLE(FVoiceChatUserHolder);
    ~FVoiceChatUserHolder();
    FVoiceChatUserHolder &operator=(IVoiceChatUser *InVoiceChatUser);
    IVoiceChatUser *operator->();
};

}

namespace Redpoint::EOS::Tests::VoiceChat
{
REDPOINT_EOS_FILE_NS_EXPORT(495223541, Redpoint::EOS::Tests::VoiceChat, FVoiceChatUserHolder)
}