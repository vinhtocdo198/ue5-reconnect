// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemRedpointEOS/VoiceChat/EOSVoiceManager.h"
#include "RedpointEOSAuth/CrossPlatform/CrossPlatformAccountId.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

class FOnlineSubsystemRedpointEAS;

namespace REDPOINT_EOS_FILE_NS_ID(1057129160, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Identity)
{
using namespace ::Redpoint::EOS::Auth::CrossPlatform;
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat;
#endif

class ONLINESUBSYSTEMREDPOINTEOS_API IOnlineIdentityInterfaceEOS : public IOnlineIdentity, public FExec
{
public:
    IOnlineIdentityInterfaceEOS() = default;
    UE_NONCOPYABLE(IOnlineIdentityInterfaceEOS);
    virtual ~IOnlineIdentityInterfaceEOS() override = default;

    /** Returns the local user number of the given player. */
    virtual bool GetLocalUserNum(const FUniqueNetId &UniqueNetId, int32 &OutLocalUserNum) const = 0;

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    /** Returns the cross-platform account ID of the given local player. */
    virtual TSharedPtr<const FCrossPlatformAccountId> GetCrossPlatformAccountId(
        const FUniqueNetId &UniqueNetId) const = 0;

    /** Returns true if a cross-platform account provider is available. */
    virtual bool IsCrossPlatformAccountProviderAvailable() const = 0;
#endif

protected:
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    virtual TSharedPtr<FOnlineSubsystemRedpointEAS> GetEpicGamesOnlineSubsystem() const = 0;
#endif
    virtual FName GetInstanceName() const = 0;
    virtual bool GetNativeSubsystemName(int32 LocalUserNum, FName &OutNativeSubsystemName) const = 0;

public:
    IOnlineSubsystem *GetNativeSubsystem(int32 LocalUserNum, bool bObtainRedpointImplementation = false) const;

    IOnlineSubsystem *GetNativeSubsystem(const FUniqueNetId &UserIdEOS, bool bObtainRedpointImplementation = false)
        const;

    template <typename TInterface>
    TSharedPtr<TInterface, ESPMode::ThreadSafe> GetNativeInterface(
        int32 LocalUserNum,
        TSharedPtr<const FUniqueNetId> &OutNativeUserId,
        TFunction<TSharedPtr<TInterface, ESPMode::ThreadSafe>(IOnlineSubsystem *OSS)> Accessor)
    {
        // Get the native user IDs, which callers will need to use for passthrough.
        OutNativeUserId = nullptr;
        IOnlineSubsystem *NativeOSS = this->GetNativeSubsystem(LocalUserNum, false);
        if (NativeOSS == nullptr)
        {
            return nullptr;
        }
        TSharedPtr<IOnlineIdentity, ESPMode::ThreadSafe> NativeIdentity = NativeOSS->GetIdentityInterface();
        if (!NativeIdentity.IsValid())
        {
            return nullptr;
        }
        OutNativeUserId = NativeIdentity->GetUniquePlayerId(LocalUserNum);
        if (!OutNativeUserId.IsValid())
        {
            return nullptr;
        }

        // Prefer Redpoint implementation first.
        IOnlineSubsystem *RedpointOSS = this->GetNativeSubsystem(LocalUserNum, true);
        if (RedpointOSS != nullptr)
        {
            TSharedPtr<TInterface, ESPMode::ThreadSafe> RedpointInterface = Accessor(RedpointOSS);
            if (RedpointInterface.IsValid())
            {
                return RedpointInterface;
            }
        }

        // Then use native implementation if there's no Redpoint override.
        TSharedPtr<TInterface, ESPMode::ThreadSafe> NativeInterface = Accessor(NativeOSS);
        if (NativeInterface.IsValid())
        {
            return NativeInterface;
        }

        // Otherwise it's not available.
        return nullptr;
    }

    template <typename TInterface>
    TSharedPtr<TInterface, ESPMode::ThreadSafe> GetNativeInterface(
        const FUniqueNetId &UserIdEOS,
        TSharedPtr<const FUniqueNetId> &OutNativeUserId,
        TFunction<TSharedPtr<TInterface, ESPMode::ThreadSafe>(IOnlineSubsystem *OSS)> Accessor)
    {
        int32 LocalUserNum;
        OutNativeUserId = nullptr;
        if (this->GetLocalUserNum(UserIdEOS, LocalUserNum))
        {
            return this->GetNativeInterface<TInterface>(LocalUserNum, OutNativeUserId, Accessor);
        }
        return nullptr;
    }
};

typedef TSharedRef<IOnlineIdentityInterfaceEOS> FOnlineIdentityInterfaceEOSRef;
typedef TSharedPtr<IOnlineIdentityInterfaceEOS> FOnlineIdentityInterfaceEOSPtr;

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Identity
{
REDPOINT_EOS_FILE_NS_EXPORT(
    1057129160,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Identity,
    IOnlineIdentityInterfaceEOS)
REDPOINT_EOS_FILE_NS_EXPORT(
    1057129160,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Identity,
    FOnlineIdentityInterfaceEOSRef)
REDPOINT_EOS_FILE_NS_EXPORT(
    1057129160,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Identity,
    FOnlineIdentityInterfaceEOSPtr)
}

REDPOINT_EOS_CODE_GUARD_END()