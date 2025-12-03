// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSRooms/Features/VoiceChatRoomFeature.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1823375277, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Features;
using namespace ::Redpoint::EOS::API;

class FVoiceChatRoomFeatureImpl : public IVoiceChatRoomFeature
{
public:
    typedef TDelegate<FString()> FGetRTCRoomNameDelegate;

    FGetRTCRoomNameDelegate GetRTCRoomNameDelegate;
    RTC::EJoinRoomOptions::Type JoinFlagsValue;
    bool bUseManualAudioInputValue;
    bool bUseManualAudioOutputValue;
    bool bLocalAudioDeviceInputStartsMutedValue;

    FVoiceChatRoomFeatureImpl(const FGetRTCRoomNameDelegate &InGetRTCRoomNameDelegate);

    virtual FString GetRTCRoomName() const override;
    virtual RTC::EJoinRoomOptions::Type JoinFlags() const override;
    virtual bool UseManualAudioInput() const override;
    virtual bool UseManualAudioOutput() const override;
    virtual bool LocalAudioDeviceInputStartsMuted() const override;
    virtual TSharedPtr<IRoomFeatureRequestBase> GetFeatureRequestForUpdate() const override;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(1823375277, Redpoint::EOS::Rooms::Features, FVoiceChatRoomFeatureImpl)
}

REDPOINT_EOS_CODE_GUARD_END()