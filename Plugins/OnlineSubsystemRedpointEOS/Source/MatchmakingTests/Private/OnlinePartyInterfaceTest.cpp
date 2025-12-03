// Copyright June Rhodes. All Rights Reserved.

#include "OnlinePartyInterfaceTest.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "TestBackend.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

class FOnlinePartyMemberTest : public FOnlinePartyMember
{
private:
    TSharedRef<const FUniqueNetId> UserId;

public:
    FOnlinePartyMemberTest(TSharedRef<const FUniqueNetId> InUserId)
        : UserId(InUserId) {};

    virtual TSharedRef<const FUniqueNetId> GetUserId() const override
    {
        return this->UserId;
    }
    virtual FString GetRealName() const override
    {
        return TEXT("");
    }
    virtual FString GetDisplayName(const FString &Platform = FString()) const override
    {
        return TEXT("");
    }
    virtual bool GetUserAttribute(const FString &AttrName, FString &OutAttrValue) const override
    {
        return false;
    }
};

FOnlinePartyInterfaceTest::FOnlinePartyInterfaceTest(TSharedPtr<class FTestBackend> InBackend)
    : Backend(InBackend)
{
}

void FOnlinePartyInterfaceTest::JoinPartyAtStartup(const FUniqueNetId &UserId, const FOnlinePartyId &PartyId)
{
    TSharedRef<const FOnlineLobbyIdTest> PartyIdRef = StaticCastSharedRef<const FOnlineLobbyIdTest>(PartyId.AsShared());

    this->Backend->Lobbies[PartyIdRef->Id]->AddMember(UserId, nullptr, this->AsShared());

    FOnlinePartyTypeId PartyTypeId = FOnlinePartyTypeId(0);
    TSharedPtr<FOnlinePartyTest> Party = MakeShared<FOnlinePartyTest>(
        PartyIdRef,
        PartyTypeId,
        this->Backend->Lobbies[PartyIdRef->Id]->OwnerId.ToSharedRef());
    this->JoinedParty.Add(UserId, Party);
}

TSharedPtr<const FOnlinePartyId> FOnlinePartyInterfaceTest::CreatePartyAtStartup(const FUniqueNetId &UserId)
{
    auto Lobby = this->Backend->CreateLobby();
    Lobby->OwnerId = UserId.AsShared();
    Lobby->Capacity = 100;
    Lobby->AddMember(UserId, nullptr, this->AsShared());
    Lobby->Metadata.Add("is_party", true);
    this->Backend->Lobbies.Add(Lobby->LobbyId, Lobby);

    TSharedRef<const FOnlinePartyId> PartyId = MakeShared<FOnlineLobbyIdTest>(Lobby->LobbyId);
    FOnlinePartyTypeId PartyTypeId = FOnlinePartyTypeId(0);
    TSharedPtr<FOnlinePartyTest> Party = MakeShared<FOnlinePartyTest>(PartyId, PartyTypeId, UserId.AsShared());
    this->JoinedParty.Add(UserId, Party);
    return PartyId;
}

bool FOnlinePartyInterfaceTest::UpdateParty(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FPartyConfiguration &PartyConfig,
    bool bShouldRegenerateReservationKey,
    const FOnUpdatePartyComplete &Delegate)
{
    // This is just used to the lock the party, which doesn't affect anything during testing.
    Delegate.ExecuteIfBound(LocalUserId, PartyId, EUpdateConfigCompletionResult::Succeeded);
    return true;
}

bool FOnlinePartyInterfaceTest::UpdatePartyData(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FName &,
    const FOnlinePartyData &NewData)
{
    if (this->JoinedParty.Contains(LocalUserId))
    {
        if (*this->JoinedParty[LocalUserId]->PartyId == PartyId)
        {
            auto BackendLobby = this->Backend->Lobbies[((const FOnlineLobbyIdTest &)PartyId).Id];
            FOnlineKeyValuePairs<FString, FVariantData> DirtyAttrs;
            TArray<FString> RemovedAttrs;
            NewData.GetDirtyKeyValAttrs(DirtyAttrs, RemovedAttrs);
            for (const auto &KV : DirtyAttrs)
            {
                BackendLobby->Metadata.Add(KV.Key, KV.Value);
            }
            for (const auto &Key : RemovedAttrs)
            {
                BackendLobby->Metadata.Remove(Key);
            }
            for (const auto &KV : BackendLobby->Members)
            {
                KV.Value->OnlineParty
                    ->TriggerOnPartyDataReceivedDelegates(*KV.Value->UserId, PartyId, NAME_Default, NewData);
            }
            return true;
        }
    }

    return false;
}

bool FOnlinePartyInterfaceTest::UpdatePartyMemberData(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FName &,
    const FOnlinePartyData &NewData)
{
    if (this->JoinedParty.Contains(LocalUserId))
    {
        if (*this->JoinedParty[LocalUserId]->PartyId == PartyId)
        {
            auto BackendLobby = this->Backend->Lobbies[((const FOnlineLobbyIdTest &)PartyId).Id];
            FOnlineKeyValuePairs<FString, FVariantData> DirtyAttrs;
            TArray<FString> RemovedAttrs;
            NewData.GetDirtyKeyValAttrs(DirtyAttrs, RemovedAttrs);
            for (const auto &KV : DirtyAttrs)
            {
                BackendLobby->Members[LocalUserId]->Metadata.Add(KV.Key, KV.Value);
            }
            for (const auto &Key : RemovedAttrs)
            {
                BackendLobby->Members[LocalUserId]->Metadata.Remove(Key);
            }
            for (const auto &KV : BackendLobby->Members)
            {
                KV.Value->OnlineParty->TriggerOnPartyMemberDataReceivedDelegates(
                    *KV.Value->UserId,
                    PartyId,
                    LocalUserId,
                    NAME_Default,
                    NewData);
            }
            return true;
        }
    }

    return false;
}

bool FOnlinePartyInterfaceTest::IsMemberLeader(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FUniqueNetId &MemberId) const
{
    if (this->JoinedParty.Contains(LocalUserId))
    {
        if (*this->JoinedParty[LocalUserId]->PartyId == PartyId)
        {
            if (*this->JoinedParty[LocalUserId]->LeaderId == MemberId)
            {
                return true;
            }

            return false;
        }
    }

    return false;
}

FOnlinePartyConstPtr FOnlinePartyInterfaceTest::GetParty(const FUniqueNetId &LocalUserId, const FOnlinePartyId &PartyId)
    const
{
    if (this->JoinedParty.Contains(LocalUserId))
    {
        if (*this->JoinedParty[LocalUserId]->PartyId == PartyId)
        {
            return this->JoinedParty[LocalUserId];
        }
    }
    return nullptr;
}

FOnlinePartyMemberConstPtr FOnlinePartyInterfaceTest::GetPartyMember(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FUniqueNetId &MemberId) const
{
    if (this->JoinedParty.Contains(LocalUserId))
    {
        if (*this->JoinedParty[LocalUserId]->PartyId == PartyId)
        {
            for (const auto &TargetMemberId :
                 this->Backend->Lobbies[((const FOnlineLobbyIdTest &)PartyId).Id]->MembersSortedIds)
            {
                if (*TargetMemberId == MemberId)
                {
                    return MakeShared<FOnlinePartyMemberTest>(TargetMemberId.ToSharedRef());
                }
            }
        }
    }
    return nullptr;
}

FOnlinePartyDataConstPtr FOnlinePartyInterfaceTest::GetPartyData(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FName &) const
{
    if (this->JoinedParty.Contains(LocalUserId))
    {
        if (*this->JoinedParty[LocalUserId]->PartyId == PartyId)
        {
            auto PartyData = MakeShared<FOnlinePartyData>();

            auto BackendLobby = this->Backend->Lobbies[((const FOnlineLobbyIdTest &)PartyId).Id];
            for (const auto &KV : BackendLobby->Metadata)
            {
                PartyData->SetAttribute(KV.Key, KV.Value);
            }
            PartyData->ClearDirty();

            return PartyData;
        }
    }
    return nullptr;
}

FOnlinePartyDataConstPtr FOnlinePartyInterfaceTest::GetPartyMemberData(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FUniqueNetId &MemberId,
    const FName &) const
{
    if (this->JoinedParty.Contains(LocalUserId))
    {
        if (*this->JoinedParty[LocalUserId]->PartyId == PartyId)
        {
            auto PartyMemberData = MakeShared<FOnlinePartyData>();

            auto BackendLobby = this->Backend->Lobbies[((const FOnlineLobbyIdTest &)PartyId).Id];
            for (const auto &KV : BackendLobby->Members[MemberId]->Metadata)
            {
                PartyMemberData->SetAttribute(KV.Key, KV.Value);
            }
            PartyMemberData->ClearDirty();

            return PartyMemberData;
        }
    }
    return nullptr;
}

bool FOnlinePartyInterfaceTest::GetJoinedParties(
    const FUniqueNetId &LocalUserId,
    TArray<TSharedRef<const FOnlinePartyId>> &OutPartyIdArray) const
{
    if (this->JoinedParty.Contains(LocalUserId))
    {
        OutPartyIdArray.Add(this->JoinedParty[LocalUserId]->PartyId);
    }
    return true;
}

bool FOnlinePartyInterfaceTest::GetPartyMembers(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    TArray<FOnlinePartyMemberConstRef> &OutPartyMembersArray) const
{
    OutPartyMembersArray.Empty();
    for (const auto &MemberId : this->Backend->Lobbies[((const FOnlineLobbyIdTest &)PartyId).Id]->MembersSortedIds)
    {
        OutPartyMembersArray.Add(MakeShared<FOnlinePartyMemberTest>(MemberId.ToSharedRef()));
    }
    return true;
}

#endif

REDPOINT_EOS_CODE_GUARD_END()