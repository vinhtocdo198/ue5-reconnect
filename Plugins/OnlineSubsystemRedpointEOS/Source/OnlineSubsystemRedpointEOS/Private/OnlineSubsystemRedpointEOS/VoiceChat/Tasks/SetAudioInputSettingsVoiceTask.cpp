// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/VoiceChat/Tasks/SetAudioInputSettingsVoiceTask.h"

#include "OnlineSubsystemRedpointEOS/Error/ErrorConv.h"
#include "OnlineSubsystemRedpointEOS/Logging.h"
#include "RedpointEOSAPI/Logging.h"
#include "RedpointEOSAPI/RTC/SetSetting.h"
#include "RedpointEOSAPI/RTCAudio/SetInputDeviceSettings.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSAPI/Stats.h"
#include "RedpointEOSCore/Legacy/EOSString.h"
#include "RedpointEOSCore/Legacy/WeakPtrHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1010709396, Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat::Tasks)
{
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS;

REDPOINT_EOS_DECLARE_CYCLE_STAT_INLINE_WITH_CALLBACK(
    VoiceChat,
    TEXT("Tasks/SetAudioInputSettings"),
    Tasks_SetAudioInputSettings);

void FSetAudioInputSettingsVoiceTask::Run(FVoiceTaskComplete OnComplete)
{
    using namespace ::Redpoint::EOS::API::RTC;
    using namespace ::Redpoint::EOS::API::RTCAudio;

    REDPOINT_EOS_SCOPE_CYCLE_COUNTER(VoiceChat, Tasks_SetAudioInputSettings);

    FString DeviceId;
    if (this->Data.bHasSetCurrentInputDevice)
    {
        DeviceId = this->Data.CurrentInputDeviceId;
    }
    else
    {
        DeviceId = this->Data.DefaultInputDeviceId;
    }

    // First we set all of the settings, since if any of these
    // are enabled, we can only set Volume to 100.0f.
    {
        EOS_EResult SetResult = FSetSetting::Execute(
            this->Data.PlatformHandle.ToSharedRef(),
            FSetSetting::Options{"DisableEchoCancelation", !this->Data.bEnableEchoCancellation ? "True" : "False"});
        REDPOINT_EOS_LOG_RESULT_IF_UNSUCCESSFUL(
            LogRedpointEOSVoiceChat,
            Warning,
            SetResult,
            TEXT("Unable to set echo cancellation on EOS RTC. Further calls may fail."));
    }
    {
        EOS_EResult SetResult = FSetSetting::Execute(
            this->Data.PlatformHandle.ToSharedRef(),
            FSetSetting::Options{"DisableNoiseSupression", !this->Data.bEnableNoiseSuppression ? "True" : "False"});
        REDPOINT_EOS_LOG_RESULT_IF_UNSUCCESSFUL(
            LogRedpointEOSVoiceChat,
            Warning,
            SetResult,
            TEXT("Unable to set noise suppression on EOS RTC. Further calls may fail."));
    }
    {
        EOS_EResult SetResult = FSetSetting::Execute(
            this->Data.PlatformHandle.ToSharedRef(),
            FSetSetting::Options{"DisableAutoGainControl", !this->Data.bEnableAutoGainControl ? "True" : "False"});
        REDPOINT_EOS_LOG_RESULT_IF_UNSUCCESSFUL(
            LogRedpointEOSVoiceChat,
            Warning,
            SetResult,
            TEXT("Unable to set auto gain control on EOS RTC. Further calls may fail."));
    }
    {
        EOS_EResult SetResult = FSetSetting::Execute(
            this->Data.PlatformHandle.ToSharedRef(),
            FSetSetting::Options{"DisableDtx", !this->Data.bEnableDtx ? "True" : "False"});
        REDPOINT_EOS_LOG_RESULT_IF_UNSUCCESSFUL(
            LogRedpointEOSVoiceChat,
            Warning,
            SetResult,
            TEXT("Unable to set DTX on EOS RTC. Further calls may fail."));
    }

    // Now we set the input settings.
    FSetInputDeviceSettings::Execute(
        this->Data.PlatformHandle.ToSharedRef(),
        FSetInputDeviceSettings::Options{this->Data.LocalUserId, DeviceId, this->Data.bEnablePlatformAEC},
        FSetInputDeviceSettings::CompletionDelegate::CreateLambda(
            [OnComplete](const FSetInputDeviceSettings::Result &SetInputDeviceResult) {
                REDPOINT_EOS_SCOPE_CYCLE_COUNTER(VoiceChat, Tasks_SetAudioInputSettings_Callback);
                REDPOINT_EOS_LOG_RESULT_IF_UNSUCCESSFUL(
                    LogRedpointEOSVoiceChat,
                    Warning,
                    SetInputDeviceResult.ResultCode,
                    TEXT("Failed to set audio input options."));
                OnComplete.ExecuteIfBound();
            }));
}

}

#endif // #if !REDPOINT_EOS_IS_DEDICATED_SERVER

REDPOINT_EOS_CODE_GUARD_END()