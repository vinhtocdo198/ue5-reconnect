// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "OnlineSubsystemRedpointEOS/VoiceChat/Tasks/VoiceTask.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(4117700320, Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat::Tasks)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat::Tasks;

class FSetAudioInputVolumeVoiceTaskData
{
public:
    struct FRoomNameWithTransmitStatus
    {
        FString RoomName;
        bool bIsTransmitting;
    };
    TSharedPtr<FPlatformInstance> PlatformHandle;
    EOS_ProductUserId LocalUserId;
    TArray<FRoomNameWithTransmitStatus> RoomNamesWithTransmitStatus;
    bool bUserInputMuted;
    float UserInputVolume;
};

class FSetAudioInputVolumeVoiceTask : public FVoiceTask, public TSharedFromThis<FSetAudioInputVolumeVoiceTask>
{
private:
    FSetAudioInputVolumeVoiceTaskData Data;

public:
    FSetAudioInputVolumeVoiceTask(FSetAudioInputVolumeVoiceTaskData InData)
        : Data(MoveTemp(InData)){};
    UE_NONCOPYABLE(FSetAudioInputVolumeVoiceTask);
    virtual ~FSetAudioInputVolumeVoiceTask() override = default;

    virtual FString GetLogName() override
    {
        return TEXT("FSetAudioInputVolumeVoiceTask");
    };

    virtual void Run(FVoiceTaskComplete OnComplete) override;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat
{
REDPOINT_EOS_FILE_NS_EXPORT(
    4117700320,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat::Tasks,
    FSetAudioInputVolumeVoiceTaskData)
REDPOINT_EOS_FILE_NS_EXPORT(
    4117700320,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat::Tasks,
    FSetAudioInputVolumeVoiceTask)
}

#endif // #if !REDPOINT_EOS_IS_DEDICATED_SERVER

REDPOINT_EOS_CODE_GUARD_END()