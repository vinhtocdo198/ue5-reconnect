// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "OnlineSubsystemImpl.h"
#include "RedpointEOSInterfaces/Interfaces/OnlineLobbyInterface.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

class FOnlineSubsystemTest : public FOnlineSubsystemImpl,
                             public TSharedFromThis<FOnlineSubsystemTest, ESPMode::ThreadSafe>
{
private:
    TSharedPtr<IOnlineLobby, ESPMode::ThreadSafe> LobbyImpl;
    TSharedPtr<class FOnlinePartyInterfaceTest, ESPMode::ThreadSafe> PartyImpl;
    TSharedPtr<class FOnlineIdentityInterfaceTest, ESPMode::ThreadSafe> IdentityImpl;
    TSharedPtr<class FOnlineStatsInterfaceTest, ESPMode::ThreadSafe> StatsImpl;

public:
    FOnlineSubsystemTest(TSharedPtr<class FTestBackend> InBackend);

    virtual bool Init(void) override
    {
        return true;
    };
    virtual FString GetAppId(void) const override
    {
        return TEXT("");
    };
    virtual FText GetOnlineServiceName(void) const override
    {
        return FText::GetEmpty();
    };

    virtual IOnlineIdentityPtr GetIdentityInterface() const override;
    virtual IOnlinePartyPtr GetPartyInterface() const override;
    virtual IOnlineStatsPtr GetStatsInterface() const override;
    virtual class UObject *GetNamedInterface(FName InterfaceName) override
    {
        if (InterfaceName.IsEqual(ONLINE_LOBBY_INTERFACE_NAME))
        {
            return (class UObject *)(void *)&this->LobbyImpl;
        }

        return nullptr;
    }

    virtual IOnlineSessionPtr GetSessionInterface(void) const override
    {
        return nullptr;
    };
    virtual IOnlineFriendsPtr GetFriendsInterface(void) const override
    {
        return nullptr;
    };
    virtual IOnlineTurnBasedPtr GetTurnBasedInterface(void) const override
    {
        return nullptr;
    };
    virtual IOnlineTournamentPtr GetTournamentInterface(void) const override
    {
        return nullptr;
    };

    TSharedPtr<const FOnlinePartyId> CreatePartyAtStartup(const FUniqueNetId &UserId);
    void JoinPartyAtStartup(const FUniqueNetId &UserId, const FOnlinePartyId &PartyId);
};

#endif

REDPOINT_EOS_CODE_GUARD_END()