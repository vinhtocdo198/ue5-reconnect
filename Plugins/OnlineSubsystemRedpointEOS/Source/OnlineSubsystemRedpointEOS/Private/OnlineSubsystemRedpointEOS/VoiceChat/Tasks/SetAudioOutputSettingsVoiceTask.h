// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "OnlineSubsystemRedpointEOS/VoiceChat/Tasks/VoiceTask.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3114724258, Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat::Tasks)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat::Tasks;

class FSetAudioOutputSettingsVoiceTaskData
{
public:
    TSharedPtr<FPlatformInstance> PlatformHandle;
    EOS_ProductUserId LocalUserId;
    bool bHasSetCurrentOutputDevice;
    FString CurrentOutputDeviceId;
    FString DefaultOutputDeviceId;
    bool bUserOutputMuted;
    float UserOutputVolume;
};

class FSetAudioOutputSettingsVoiceTask : public FVoiceTask
{
private:
    FSetAudioOutputSettingsVoiceTaskData Data;

public:
    FSetAudioOutputSettingsVoiceTask(FSetAudioOutputSettingsVoiceTaskData InData)
        : Data(MoveTemp(InData)){};
    UE_NONCOPYABLE(FSetAudioOutputSettingsVoiceTask);
    virtual ~FSetAudioOutputSettingsVoiceTask() override = default;

    virtual FString GetLogName() override
    {
        return TEXT("FSetAudioOutputSettingsVoiceTask");
    };

    virtual void Run(FVoiceTaskComplete OnComplete) override;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat
{
REDPOINT_EOS_FILE_NS_EXPORT(
    3114724258,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat::Tasks,
    FSetAudioOutputSettingsVoiceTaskData)
REDPOINT_EOS_FILE_NS_EXPORT(
    3114724258,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat::Tasks,
    FSetAudioOutputSettingsVoiceTask)
}

#endif // #if !REDPOINT_EOS_IS_DEDICATED_SERVER

REDPOINT_EOS_CODE_GUARD_END()