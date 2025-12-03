// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/VoiceChat/EOSVoiceManagerDeviceList.h"

#include "OnlineSubsystemRedpointEOS/Error/ErrorConv.h"
#include "OnlineSubsystemRedpointEOS/Logging.h"
#include "RedpointEOSAPI/Logging.h"
#include "RedpointEOSCore/Legacy/EOSString.h"
#include "RedpointEOSCore/Legacy/WeakPtrHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2949004011, Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat)
{
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS;

void FEOSVoiceManagerDeviceList::SynchroniseDeviceListFromEOS(
    const FEOSVoiceManagerDeviceListSynchronisationComplete &OnSynchronisationComplete)
{
    // Query input devices.
    FQueryInputDevicesInformation::Execute(
        this->PlatformHandle,
        FQueryInputDevicesInformation::Options{},
        FQueryInputDevicesInformation::CompletionDelegate::CreateSP(
            this,
            &FEOSVoiceManagerDeviceList::OnQueryInputDevicesForSynchroniseComplete,
            OnSynchronisationComplete));
}

void FEOSVoiceManagerDeviceList::OnQueryInputDevicesForSynchroniseComplete(
    EOS_EResult InputResultCode,
    const TArray<FDeviceInformation> &InputRawDevices,
    FEOSVoiceManagerDeviceListSynchronisationComplete OnSynchronisationComplete)
{
    if (InputResultCode != EOS_EResult::EOS_Success)
    {
        REDPOINT_EOS_LOG_RESULT(
            LogRedpointEOSVoiceChat,
            Error,
            InputResultCode,
            TEXT("Failed to query input devices."));
        OnSynchronisationComplete.ExecuteIfBound(false, false);
        return;
    }

    // Copy and set new input devices array.
    TArray<FEOSVoiceManagerDevice> NewInputDevices;
    for (const auto &InputDevice : InputRawDevices)
    {
        FEOSVoiceManagerDevice NewDevice;
        NewDevice.Id = InputDevice.Id;
        NewDevice.DisplayName = InputDevice.DisplayName;
        NewDevice.bIsDefaultDevice = InputDevice.bIsDefaultDevice;
        NewInputDevices.Add(NewDevice);
    }
    this->InputDevices = NewInputDevices;

    // Query output devices.
    FQueryOutputDevicesInformation::Execute(
        this->PlatformHandle,
        FQueryOutputDevicesInformation::Options{},
        FQueryOutputDevicesInformation::CompletionDelegate::CreateSP(
            this,
            &FEOSVoiceManagerDeviceList::OnQueryOutputDevicesForSynchroniseComplete,
            OnSynchronisationComplete));
}

void FEOSVoiceManagerDeviceList::OnQueryOutputDevicesForSynchroniseComplete(
    EOS_EResult OutputResultCode,
    const TArray<FDeviceInformation> &OutputRawDevices,
    FEOSVoiceManagerDeviceListSynchronisationComplete OnSynchronisationComplete)
{
    if (OutputResultCode != EOS_EResult::EOS_Success)
    {
        REDPOINT_EOS_LOG_RESULT(
            LogRedpointEOSVoiceChat,
            Error,
            OutputResultCode,
            TEXT("Failed to query output devices."));

        this->bHasSyncedAtLeastOnce = true;
        OnSynchronisationComplete.ExecuteIfBound(false, true);
        return;
    }

    // Copy and set new output devices array.
    TArray<FEOSVoiceManagerDevice> NewOutputDevices;
    for (const auto &OutputDevice : OutputRawDevices)
    {
        FEOSVoiceManagerDevice NewDevice;
        NewDevice.Id = OutputDevice.Id;
        NewDevice.DisplayName = OutputDevice.DisplayName;
        NewDevice.bIsDefaultDevice = OutputDevice.bIsDefaultDevice;
        NewOutputDevices.Add(NewDevice);
    }
    this->OutputDevices = NewOutputDevices;

    // Notify that input and output devices have changed.
    this->bHasSyncedAtLeastOnce = true;
    OnSynchronisationComplete.ExecuteIfBound(true, true);
}

void FEOSVoiceManagerDeviceList::OnSynchronisedAfterAudioDevicesChanged(bool bWasSuccessful, bool bWasDeviceListUpdated)
{
    if (bWasDeviceListUpdated)
    {
        this->OnAudioDevicesChangedDelegate.Broadcast();
    }
}

void FEOSVoiceManagerDeviceList::OnSynchronisedAfterInitialDevicesReceived(
    bool bWasSuccessful,
    bool bWasDeviceListUpdated,
    FEOSVoiceManagerDeviceListWaitForInitialDevices OnInitialDevicesReceived)
{
    OnInitialDevicesReceived.ExecuteIfBound();
}

void FEOSVoiceManagerDeviceList::OnAudioDevicesChanged(const FNotifyAudioDevicesChanged::Result &Result)
{
    this->SynchroniseDeviceListFromEOS(FEOSVoiceManagerDeviceListSynchronisationComplete::CreateSP(
        this,
        &FEOSVoiceManagerDeviceList::OnSynchronisedAfterAudioDevicesChanged));
}

FEOSVoiceManagerDeviceList::FEOSVoiceManagerDeviceList(const FPlatformHandle &InPlatformHandle)
    : PlatformHandle(InPlatformHandle)
    , OnNativeAudioDevicesChanged()
    , InputDevices()
    , OutputDevices()
    , bHasSyncedAtLeastOnce(false)
    , OnAudioDevicesChangedDelegate()
{
}

void FEOSVoiceManagerDeviceList::RegisterEvents()
{
    this->OnNativeAudioDevicesChanged = FNotifyAudioDevicesChanged::Register(
        this->PlatformHandle,
        FNotifyAudioDevicesChanged::Options{},
        FNotifyAudioDevicesChanged::EventDelegate::CreateSP(this, &FEOSVoiceManagerDeviceList::OnAudioDevicesChanged));
}

void FEOSVoiceManagerDeviceList::WaitForInitialDevices(
    const FEOSVoiceManagerDeviceListWaitForInitialDevices &OnInitialDevicesReceived)
{
    if (this->bHasSyncedAtLeastOnce)
    {
        // We've already synchronised the device list previously.
        OnInitialDevicesReceived.ExecuteIfBound();
    }
    else
    {
        // Perform an initial synchronisation.
        this->SynchroniseDeviceListFromEOS(FEOSVoiceManagerDeviceListSynchronisationComplete::CreateSP(
            this,
            &FEOSVoiceManagerDeviceList::OnSynchronisedAfterInitialDevicesReceived,
            OnInitialDevicesReceived));
    }
}

const TArray<FEOSVoiceManagerDevice> &FEOSVoiceManagerDeviceList::GetInputDevices() const
{
    return this->InputDevices;
}

const TArray<FEOSVoiceManagerDevice> &FEOSVoiceManagerDeviceList::GetOutputDevices() const
{
    return this->OutputDevices;
}

FOnVoiceChatAvailableAudioDevicesChangedDelegate &FEOSVoiceManagerDeviceList::OnDevicesChanged()
{
    return this->OnAudioDevicesChangedDelegate;
}

}

#endif // #if !REDPOINT_EOS_IS_DEDICATED_SERVER

REDPOINT_EOS_CODE_GUARD_END()