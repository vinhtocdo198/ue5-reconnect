// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/VoiceChat/Tasks/SetAudioOutputSettingsVoiceTask.h"

#include "OnlineSubsystemRedpointEOS/Error/ErrorConv.h"
#include "OnlineSubsystemRedpointEOS/Logging.h"
#include "RedpointEOSAPI/Logging.h"
#include "RedpointEOSAPI/RTC/SetSetting.h"
#include "RedpointEOSAPI/RTCAudio/SetOutputDeviceSettings.h"
#include "RedpointEOSAPI/Stats.h"
#include "RedpointEOSCore/Legacy/EOSString.h"
#include "RedpointEOSCore/Legacy/WeakPtrHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3114724258, Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat::Tasks)
{
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS;

REDPOINT_EOS_DECLARE_CYCLE_STAT_INLINE_WITH_CALLBACK(
    VoiceChat,
    TEXT("Tasks/SetAudioOutputSettings"),
    Tasks_SetAudioOutputSettings);

void FSetAudioOutputSettingsVoiceTask::Run(FVoiceTaskComplete OnComplete)
{
    using namespace ::Redpoint::EOS::API::RTC;
    using namespace ::Redpoint::EOS::API::RTCAudio;

    REDPOINT_EOS_SCOPE_CYCLE_COUNTER(VoiceChat, Tasks_SetAudioOutputSettings);

    FString DeviceId;
    if (this->Data.bHasSetCurrentOutputDevice)
    {
        DeviceId = this->Data.CurrentOutputDeviceId;
    }
    else
    {
        DeviceId = this->Data.DefaultOutputDeviceId;
    }

    // Now we set the output settings.
    FSetOutputDeviceSettings::Execute(
        this->Data.PlatformHandle.ToSharedRef(),
        FSetOutputDeviceSettings::Options{this->Data.LocalUserId, DeviceId},
        FSetOutputDeviceSettings::CompletionDelegate::CreateLambda(
            [OnComplete](const FSetOutputDeviceSettings::Result &SetOutputDeviceResult) {
                REDPOINT_EOS_SCOPE_CYCLE_COUNTER(VoiceChat, Tasks_SetAudioOutputSettings_Callback);
                REDPOINT_EOS_LOG_RESULT_IF_UNSUCCESSFUL(
                    LogRedpointEOSVoiceChat,
                    Warning,
                    SetOutputDeviceResult.ResultCode,
                    TEXT("Failed to set audio output options."));
                OnComplete.ExecuteIfBound();
            }));
}

}

#endif // #if !REDPOINT_EOS_IS_DEDICATED_SERVER

REDPOINT_EOS_CODE_GUARD_END()