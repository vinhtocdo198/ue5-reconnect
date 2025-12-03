// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemTest.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "Online/CoreOnline.h"
#include "OnlineIdentityInterfaceTest.h"
#include "OnlineLobbyInterfaceTest.h"
#include "OnlinePartyInterfaceTest.h"
#include "OnlineStatsInterfaceTest.h"
#include "TestBackend.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

FOnlineSubsystemTest::FOnlineSubsystemTest(TSharedPtr<FTestBackend> InBackend)
    : FOnlineSubsystemImpl(FName(TEXT("TEST")), FName(TEXT("TEST")))
    , LobbyImpl(MakeShared<FOnlineLobbyInterfaceTest, ESPMode::ThreadSafe>(InBackend))
    , PartyImpl(MakeShared<FOnlinePartyInterfaceTest, ESPMode::ThreadSafe>(InBackend))
    , IdentityImpl(MakeShared<FOnlineIdentityInterfaceTest, ESPMode::ThreadSafe>())
    , StatsImpl(MakeShared<FOnlineStatsInterfaceTest, ESPMode::ThreadSafe>()) {};

IOnlineIdentityPtr FOnlineSubsystemTest::GetIdentityInterface() const
{
    return this->IdentityImpl;
}

IOnlinePartyPtr FOnlineSubsystemTest::GetPartyInterface() const
{
    return this->PartyImpl;
}

IOnlineStatsPtr FOnlineSubsystemTest::GetStatsInterface() const
{
    return this->StatsImpl;
}

TSharedPtr<const FOnlinePartyId> FOnlineSubsystemTest::CreatePartyAtStartup(const FUniqueNetId &UserId)
{
    return this->PartyImpl->CreatePartyAtStartup(UserId);
};

void FOnlineSubsystemTest::JoinPartyAtStartup(const FUniqueNetId &UserId, const FOnlinePartyId &PartyId)
{
    this->PartyImpl->JoinPartyAtStartup(UserId, PartyId);
};

#endif

REDPOINT_EOS_CODE_GUARD_END()