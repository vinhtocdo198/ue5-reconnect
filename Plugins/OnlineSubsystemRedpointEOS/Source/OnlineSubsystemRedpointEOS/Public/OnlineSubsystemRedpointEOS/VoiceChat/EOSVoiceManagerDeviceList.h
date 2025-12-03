// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "OnlineSubsystemRedpointEOS/VoiceChat/EOSVoiceManagerDevice.h"
#include "RedpointEOSAPI/RTCAudio/NotifyAudioDevicesChanged.h"
#include "RedpointEOSAPI/RTCAudio/QueryInputDevicesInformation.h"
#include "RedpointEOSAPI/RTCAudio/QueryOutputDevicesInformation.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Utils/HasEventRegistration.h"
#include "VoiceChat.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2949004011, Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::API::RTCAudio;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat;

typedef TDelegate<void()> FEOSVoiceManagerDeviceListWaitForInitialDevices;
typedef TDelegate<void(bool bWasSuccessful, bool bWasDeviceListUpdated)>
    FEOSVoiceManagerDeviceListSynchronisationComplete;

class ONLINESUBSYSTEMREDPOINTEOS_API FEOSVoiceManagerDeviceList : public IHasEventRegistration,
                                                                  public TSharedFromThis<FEOSVoiceManagerDeviceList>
{
private:
    FPlatformHandle PlatformHandle;
    FEventHandlePtr OnNativeAudioDevicesChanged;
    TArray<FEOSVoiceManagerDevice> InputDevices;
    TArray<FEOSVoiceManagerDevice> OutputDevices;
    bool bHasSyncedAtLeastOnce;
    FOnVoiceChatAvailableAudioDevicesChangedDelegate OnAudioDevicesChangedDelegate;

    /** Queries devices in the EOS SDK asynchronously to update InputDevices and OutputDevices. */
    void SynchroniseDeviceListFromEOS(
        const FEOSVoiceManagerDeviceListSynchronisationComplete &OnSynchronisationComplete);
    void OnQueryInputDevicesForSynchroniseComplete(
        EOS_EResult InputResultCode,
        const TArray<FDeviceInformation> &InputRawDevices,
        FEOSVoiceManagerDeviceListSynchronisationComplete OnSynchronisationComplete);
    void OnQueryOutputDevicesForSynchroniseComplete(
        EOS_EResult OutputResultCode,
        const TArray<FDeviceInformation> &OutputRawDevices,
        FEOSVoiceManagerDeviceListSynchronisationComplete OnSynchronisationComplete);

    void OnSynchronisedAfterAudioDevicesChanged(bool bWasSuccessful, bool bWasDeviceListUpdated);
    void OnSynchronisedAfterInitialDevicesReceived(
        bool bWasSuccessful,
        bool bWasDeviceListUpdated,
        FEOSVoiceManagerDeviceListWaitForInitialDevices OnInitialDevicesReceived);

    void OnAudioDevicesChanged(const FNotifyAudioDevicesChanged::Result &Result);

public:
    FEOSVoiceManagerDeviceList(const FPlatformHandle &InPlatform);
    virtual void RegisterEvents() override;
    UE_NONCOPYABLE(FEOSVoiceManagerDeviceList);
    virtual ~FEOSVoiceManagerDeviceList() override = default;

    /** Returns asynchronously when the initial audio devices have been received. */
    void WaitForInitialDevices(const FEOSVoiceManagerDeviceListWaitForInitialDevices &OnInitialDevicesReceived);

    /** Retrieves the current input devices. If initial audio devices haven't been received yet, this returns an empty
     * array. */
    const TArray<FEOSVoiceManagerDevice> &GetInputDevices() const;

    /** Retrieves the current output devices. If initial audio devices haven't been received yet, this returns an empty
     * array. */
    const TArray<FEOSVoiceManagerDevice> &GetOutputDevices() const;

    /** Invoked when the available audio devices change. */
    FOnVoiceChatAvailableAudioDevicesChangedDelegate &OnDevicesChanged();
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat
{
REDPOINT_EOS_FILE_NS_EXPORT(
    2949004011,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat,
    FEOSVoiceManagerDeviceListWaitForInitialDevices)
REDPOINT_EOS_FILE_NS_EXPORT(
    2949004011,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat,
    FEOSVoiceManagerDeviceListSynchronisationComplete)
REDPOINT_EOS_FILE_NS_EXPORT(
    2949004011,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat,
    FEOSVoiceManagerDeviceList)
}

#endif // #if !REDPOINT_EOS_IS_DEDICATED_SERVER

REDPOINT_EOS_CODE_GUARD_END()