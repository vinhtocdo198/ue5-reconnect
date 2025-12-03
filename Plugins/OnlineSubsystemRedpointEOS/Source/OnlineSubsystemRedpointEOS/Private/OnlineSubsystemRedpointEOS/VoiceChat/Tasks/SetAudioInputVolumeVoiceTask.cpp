// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/VoiceChat/Tasks/SetAudioInputVolumeVoiceTask.h"

#include "OnlineSubsystemRedpointEOS/Logging.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Logging.h"
#include "RedpointEOSAPI/RTCAudio/UpdateSendingVolume.h"
#include "RedpointEOSAPI/Stats.h"
#include "RedpointEOSCore/Legacy/WeakPtrHelpers.h"
#include "RedpointEOSCore/Utils/MultiOperation.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(4117700320, Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat::Tasks)
{
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS;

REDPOINT_EOS_DECLARE_CYCLE_STAT_INLINE_WITH_CALLBACK(
    VoiceChat,
    TEXT("Tasks/SetAudioInputVolume"),
    Tasks_SetAudioInputVolume);

void FSetAudioInputVolumeVoiceTask::Run(FVoiceTaskComplete OnComplete)
{
    using namespace ::Redpoint::EOS::API::RTCAudio;

    REDPOINT_EOS_SCOPE_CYCLE_COUNTER(VoiceChat, Tasks_SetAudioInputVolume);

    FMultiOperation<FSetAudioInputVolumeVoiceTaskData::FRoomNameWithTransmitStatus, bool>::RunSP(
        this,
        this->Data.RoomNamesWithTransmitStatus,
        [CapturedData = this->Data](
            const FSetAudioInputVolumeVoiceTaskData::FRoomNameWithTransmitStatus &RoomNameWithTransmitStatus,
            auto OnDone) {
            FUpdateSendingVolume::Execute(
                CapturedData.PlatformHandle.ToSharedRef(),
                FUpdateSendingVolume::Options{
                    CapturedData.LocalUserId,
                    RoomNameWithTransmitStatus.RoomName,
                    RoomNameWithTransmitStatus.bIsTransmitting
                        ? (CapturedData.bUserInputMuted ? 0.0f : (CapturedData.UserInputVolume / 2.0f))
                        : 0.0f},
                FUpdateSendingVolume::CompletionDelegate::CreateLambda(
                    [OnDone, RoomName = RoomNameWithTransmitStatus.RoomName](
                        const FUpdateSendingVolume::Result &SetInputVolumeResult) {
                        REDPOINT_EOS_LOG_RESULT_IF_UNSUCCESSFUL(
                            LogRedpointEOSVoiceChat,
                            Warning,
                            SetInputVolumeResult.ResultCode,
                            *FString::Printf(TEXT("Unable to set sending volume on room '%s'."), *RoomName));
                        OnDone(SetInputVolumeResult.ResultCode == EOS_EResult::EOS_Success);
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