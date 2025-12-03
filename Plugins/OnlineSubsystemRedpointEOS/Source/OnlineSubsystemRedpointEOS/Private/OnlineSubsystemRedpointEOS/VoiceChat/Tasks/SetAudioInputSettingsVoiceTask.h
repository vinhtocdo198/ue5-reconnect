// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "OnlineSubsystemRedpointEOS/VoiceChat/Tasks/VoiceTask.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1010709396, Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat::Tasks)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat::Tasks;

class FSetAudioInputSettingsVoiceTaskData
{
public:
    TSharedPtr<FPlatformInstance> PlatformHandle;
    EOS_ProductUserId LocalUserId;
    bool bHasSetCurrentInputDevice;
    FString CurrentInputDeviceId;
    FString DefaultInputDeviceId;
    bool bUserInputMuted;
    float UserInputVolume;

    bool bEnableEchoCancellation;
    bool bEnableNoiseSuppression;
    bool bEnableAutoGainControl;
    bool bEnableDtx;
    bool bEnablePlatformAEC;
};

class FSetAudioInputSettingsVoiceTask : public FVoiceTask
{
private:
    FSetAudioInputSettingsVoiceTaskData Data;

public:
    FSetAudioInputSettingsVoiceTask(FSetAudioInputSettingsVoiceTaskData InData)
        : Data(MoveTemp(InData)){};
    UE_NONCOPYABLE(FSetAudioInputSettingsVoiceTask);
    virtual ~FSetAudioInputSettingsVoiceTask() override = default;

    virtual FString GetLogName() override
    {
        return TEXT("FSetAudioInputSettingsVoiceTask");
    };

    virtual void Run(FVoiceTaskComplete OnComplete) override;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat
{
REDPOINT_EOS_FILE_NS_EXPORT(
    1010709396,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat::Tasks,
    FSetAudioInputSettingsVoiceTaskData)
REDPOINT_EOS_FILE_NS_EXPORT(
    1010709396,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat::Tasks,
    FSetAudioInputSettingsVoiceTask)
}

#endif // #if !REDPOINT_EOS_IS_DEDICATED_SERVER

REDPOINT_EOS_CODE_GUARD_END()