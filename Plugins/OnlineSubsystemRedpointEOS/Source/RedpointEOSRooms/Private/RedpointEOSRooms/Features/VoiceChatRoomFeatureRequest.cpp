// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Features/VoiceChatRoomFeatureRequest.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSRooms/Features/VoiceChatRoomFeature.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1444549138, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;
using namespace ::Redpoint::EOS::API::RTC;

FVoiceChatRoomFeatureRequest::FVoiceChatRoomFeatureRequest(
    EJoinRoomOptions::Type JoinFlags,
    bool bUseManualAudioInput,
    bool bUseManualAudioOutput,
    bool bLocalAudioDeviceInputStartsMuted)
    : JoinFlagsValue(JoinFlags)
    , bUseManualAudioInputValue(bUseManualAudioInput)
    , bUseManualAudioOutputValue(bUseManualAudioOutput)
    , bLocalAudioDeviceInputStartsMutedValue(bLocalAudioDeviceInputStartsMuted)
{
}

EJoinRoomOptions::Type FVoiceChatRoomFeatureRequest::JoinFlags() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->JoinFlagsValue;
}

bool FVoiceChatRoomFeatureRequest::UseManualAudioInput() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->bUseManualAudioInputValue;
}

bool FVoiceChatRoomFeatureRequest::UseManualAudioOutput() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->bUseManualAudioOutputValue;
}

bool FVoiceChatRoomFeatureRequest::LocalAudioDeviceInputStartsMuted() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->bLocalAudioDeviceInputStartsMutedValue;
}

}

REDPOINT_EOS_CODE_GUARD_END()