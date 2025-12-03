// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "OnlineSubsystemRedpointEOS/Shared/EOSCommon.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/UniqueNetIdEOS.h"
#include "RedpointEOSInterfaces/Interfaces/OnlineVoiceAdminInterface.h"

EOS_ENABLE_STRICT_WARNINGS

class ONLINESUBSYSTEMREDPOINTEOS_API FOnlineVoiceAdminInterfaceEOS
    : public IOnlineVoiceAdmin,
      public TSharedFromThis<FOnlineVoiceAdminInterfaceEOS, ESPMode::ThreadSafe>
{
private:
    EOS_HRTCAdmin EOSRTCAdmin;

public:
    FOnlineVoiceAdminInterfaceEOS(EOS_HRTCAdmin InRTCAdmin);
    UE_NONCOPYABLE(FOnlineVoiceAdminInterfaceEOS);
    virtual ~FOnlineVoiceAdminInterfaceEOS(){};

    virtual bool CreateChannelCredentials(
        const FUniqueNetId &LocalUserId,
        const FString &ChannelName,
        const TArray<TSharedRef<const FUniqueNetId>> &TargetUserIds,
        FOnVoiceAdminCreateChannelCredentialsComplete OnComplete) override;

    virtual bool KickParticipant(
        const FUniqueNetId &LocalUserId,
        const FString &ChannelName,
        const FUniqueNetId &TargetUserId,
        FOnVoiceAdminKickParticipantComplete OnComplete) override;

    virtual bool SetParticipantHardMute(
        const FUniqueNetId &LocalUserId,
        const FString &ChannelName,
        const FUniqueNetId &TargetUserId,
        bool bMuted,
        FOnVoiceAdminSetParticipantHardMuteComplete OnComplete) override;
};

EOS_DISABLE_STRICT_WARNINGS