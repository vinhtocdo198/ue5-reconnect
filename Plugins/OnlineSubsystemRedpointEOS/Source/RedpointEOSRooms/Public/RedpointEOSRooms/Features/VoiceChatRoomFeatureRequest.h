// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/RTC/JoinRoomOptions.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/RoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/VoiceChatRoomFeature.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1444549138, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;
using namespace ::Redpoint::EOS::API::RTC;

class REDPOINTEOSROOMS_API FVoiceChatRoomFeatureRequest : public IRoomFeatureRequest<IVoiceChatRoomFeature>
{
private:
    EJoinRoomOptions::Type JoinFlagsValue;
    bool bUseManualAudioInputValue;
    bool bUseManualAudioOutputValue;
    bool bLocalAudioDeviceInputStartsMutedValue;

public:
    FVoiceChatRoomFeatureRequest(
        EJoinRoomOptions::Type JoinFlags,
        bool bUseManualAudioInput,
        bool bUseManualAudioOutput,
        bool bLocalAudioDeviceInputStartsMuted);
    UE_NONCOPYABLE(FVoiceChatRoomFeatureRequest);
    virtual ~FVoiceChatRoomFeatureRequest() override = default;

    EJoinRoomOptions::Type JoinFlags() const;
    bool UseManualAudioInput() const;
    bool UseManualAudioOutput() const;
    bool LocalAudioDeviceInputStartsMuted() const;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(1444549138, Redpoint::EOS::Rooms::Features, FVoiceChatRoomFeatureRequest)
}

REDPOINT_EOS_CODE_GUARD_END()