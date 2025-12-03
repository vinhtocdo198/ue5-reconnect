// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/VoiceChat/VoiceChatUserHolder.h"

namespace REDPOINT_EOS_FILE_NS_ID(495223541, Redpoint::EOS::Tests::VoiceChat)
{
using namespace ::Redpoint::EOS::Tests::VoiceChat;

FVoiceChatUserHolder::FVoiceChatUserHolder()
    : VoiceChatUser(nullptr)
{
}

FVoiceChatUserHolder::FVoiceChatUserHolder(IVoiceChatUser *InVoiceChatUser)
    : VoiceChatUser(InVoiceChatUser)
{
}

FVoiceChatUserHolder::~FVoiceChatUserHolder()
{
    if (VoiceChatUser != nullptr)
    {
        IVoiceChat *VoiceChat = IVoiceChat::Get();
        VoiceChat->ReleaseUser(VoiceChatUser);
    }
}

FVoiceChatUserHolder &FVoiceChatUserHolder::operator=(IVoiceChatUser *InVoiceChatUser)
{
    if (VoiceChatUser != nullptr)
    {
        IVoiceChat *VoiceChat = IVoiceChat::Get();
        VoiceChat->ReleaseUser(VoiceChatUser);
    }
    VoiceChatUser = InVoiceChatUser;
    return *this;
}

IVoiceChatUser *FVoiceChatUserHolder::operator->()
{
    return VoiceChatUser;
}

}