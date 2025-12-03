// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "OnlineSubsystemRedpointEOS/VoiceChat/Tasks/VoiceTask.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3087946605, Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat::Tasks)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat::Tasks;

class FSetAudioOutputVolumeVoiceTaskData
{
public:
    TSharedPtr<FPlatformInstance> PlatformHandle;
    EOS_ProductUserId LocalUserId;
    TArray<FString> RoomNames;
    bool bUserOutputMuted;
    float UserOutputVolume;
};

class FSetAudioOutputVolumeVoiceTask : public FVoiceTask, public TSharedFromThis<FSetAudioOutputVolumeVoiceTask>
{
private:
    FSetAudioOutputVolumeVoiceTaskData Data;

public:
    FSetAudioOutputVolumeVoiceTask(FSetAudioOutputVolumeVoiceTaskData InData)
        : Data(MoveTemp(InData)){};
    UE_NONCOPYABLE(FSetAudioOutputVolumeVoiceTask);
    virtual ~FSetAudioOutputVolumeVoiceTask() override = default;

    virtual FString GetLogName() override
    {
        return TEXT("FSetAudioOutputVolumeVoiceTask");
    };

    virtual void Run(FVoiceTaskComplete OnComplete) override;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat
{
REDPOINT_EOS_FILE_NS_EXPORT(
    3087946605,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat::Tasks,
    FSetAudioOutputVolumeVoiceTaskData)
REDPOINT_EOS_FILE_NS_EXPORT(
    3087946605,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat::Tasks,
    FSetAudioOutputVolumeVoiceTask)
}

#endif // #if !REDPOINT_EOS_IS_DEDICATED_SERVER

REDPOINT_EOS_CODE_GUARD_END()