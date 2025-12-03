// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/RTC/JoinRoomOptions.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/RoomFeature.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(727565754, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;
using namespace ::Redpoint::EOS::API;

class REDPOINTEOSROOMS_API IVoiceChatRoomFeature : public IRoomFeature
{
public:
    static FName GetFeatureName();

    virtual FString GetRTCRoomName() const = 0;
    virtual RTC::EJoinRoomOptions::Type JoinFlags() const = 0;
    virtual bool UseManualAudioInput() const = 0;
    virtual bool UseManualAudioOutput() const = 0;
    virtual bool LocalAudioDeviceInputStartsMuted() const = 0;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(727565754, Redpoint::EOS::Rooms::Features, IVoiceChatRoomFeature)
}

REDPOINT_EOS_CODE_GUARD_END()