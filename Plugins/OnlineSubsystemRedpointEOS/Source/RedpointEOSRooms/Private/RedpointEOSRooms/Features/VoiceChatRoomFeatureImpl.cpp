// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Features/VoiceChatRoomFeatureImpl.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSRooms/Features/VoiceChatRoomFeatureRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1823375277, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;
using namespace ::Redpoint::EOS::API;

FVoiceChatRoomFeatureImpl::FVoiceChatRoomFeatureImpl(const FGetRTCRoomNameDelegate &InGetRTCRoomNameDelegate)
    : GetRTCRoomNameDelegate(InGetRTCRoomNameDelegate)
    , JoinFlagsValue()
    , bUseManualAudioInputValue()
    , bUseManualAudioOutputValue()
    , bLocalAudioDeviceInputStartsMutedValue()
{
}

FString FVoiceChatRoomFeatureImpl::GetRTCRoomName() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    if (this->GetRTCRoomNameDelegate.IsBound())
    {
        return this->GetRTCRoomNameDelegate.Execute();
    }
    return TEXT("");
}

TSharedPtr<IRoomFeatureRequestBase> FVoiceChatRoomFeatureImpl::GetFeatureRequestForUpdate() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return MakeShared<FVoiceChatRoomFeatureRequest>(
        this->JoinFlagsValue,
        this->bUseManualAudioInputValue,
        this->bUseManualAudioOutputValue,
        this->bLocalAudioDeviceInputStartsMutedValue);
}

RTC::EJoinRoomOptions::Type FVoiceChatRoomFeatureImpl::JoinFlags() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->JoinFlagsValue;
}

bool FVoiceChatRoomFeatureImpl::UseManualAudioInput() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->bUseManualAudioInputValue;
}

bool FVoiceChatRoomFeatureImpl::UseManualAudioOutput() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->bUseManualAudioOutputValue;
}

bool FVoiceChatRoomFeatureImpl::LocalAudioDeviceInputStartsMuted() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->bLocalAudioDeviceInputStartsMutedValue;
}

}

REDPOINT_EOS_CODE_GUARD_END()