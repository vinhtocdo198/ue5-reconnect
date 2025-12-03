// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/VoiceChat/Tasks/SetAudioOutputVolumeVoiceTask.h"

#include "OnlineSubsystemRedpointEOS/Error/ErrorConv.h"
#include "OnlineSubsystemRedpointEOS/Logging.h"
#include "RedpointEOSAPI/Logging.h"
#include "RedpointEOSAPI/RTCAudio/UpdateReceivingVolume.h"
#include "RedpointEOSAPI/Stats.h"
#include "RedpointEOSCore/Legacy/EOSString.h"
#include "RedpointEOSCore/Legacy/WeakPtrHelpers.h"
#include "RedpointEOSCore/Utils/MultiOperation.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3087946605, Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat::Tasks)
{
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS;

REDPOINT_EOS_DECLARE_CYCLE_STAT_INLINE_WITH_CALLBACK(
    VoiceChat,
    TEXT("Tasks/SetAudioOutputVolume"),
    Tasks_SetAudioOutputVolume);

void FSetAudioOutputVolumeVoiceTask::Run(FVoiceTaskComplete OnComplete)
{
    using namespace ::Redpoint::EOS::API::RTCAudio;

    REDPOINT_EOS_SCOPE_CYCLE_COUNTER(VoiceChat, Tasks_SetAudioOutputVolume);

    FMultiOperation<FString, bool>::RunSP(
        this,
        this->Data.RoomNames,
        [CapturedData = this->Data](const FString &RoomName, auto OnDone) {
            FUpdateReceivingVolume::Execute(
                CapturedData.PlatformHandle.ToSharedRef(),
                FUpdateReceivingVolume::Options{
                    CapturedData.LocalUserId,
                    RoomName,
                    CapturedData.bUserOutputMuted ? 0.0f : (CapturedData.UserOutputVolume / 2.0f)},
                FUpdateReceivingVolume::CompletionDelegate::CreateLambda(
                    [OnDone, RoomName](const FUpdateReceivingVolume::Result &SetOutputVolumeResult) {
                        REDPOINT_EOS_LOG_RESULT_IF_UNSUCCESSFUL(
                            LogRedpointEOSVoiceChat,
                            Warning,
                            SetOutputVolumeResult.ResultCode,
                            *FString::Printf(TEXT("Unable to set receiving volume on room '%s'."), *RoomName));
                        OnDone(SetOutputVolumeResult.ResultCode == EOS_EResult::EOS_Success);
                    }));
            return true;
        },
        [OnComplete](const TArray<bool> &Results) {
            OnComplete.ExecuteIfBound();
        });
}

}

#endif // #if !REDPOINT_EOS_IS_DEDICATED_SERVER

REDPOINT_EOS_CODE_GUARD_END()