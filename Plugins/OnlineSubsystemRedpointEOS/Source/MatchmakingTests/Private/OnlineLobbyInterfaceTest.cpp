// Copyright June Rhodes. All Rights Reserved.

#include "OnlineLobbyInterfaceTest.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "Containers/Ticker.h"
#include "OnlineError.h"
#include "TestBackend.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define ONLINE_ERROR_NAMESPACE "errors.games.redpoint.matchmaking"

class FOnlineLobbyTransactionTest : public FOnlineLobbyTransaction
{
};

class FOnlineLobbyMemberTransactionTest : public FOnlineLobbyMemberTransaction
{
};

class FOnlineLobbyTest : public FOnlineLobby
{
};

TSharedPtr<FOnlineLobbyId> FOnlineLobbyInterfaceTest::ParseSerializedLobbyId(const FString &InLobbyId)
{
    return MakeShared<FOnlineLobbyIdTest>(FPlatformString::Atoi64(*InLobbyId));
}

TSharedPtr<FOnlineLobbyTransaction> FOnlineLobbyInterfaceTest::MakeCreateLobbyTransaction(const FUniqueNetId &UserId)
{
    return MakeShared<FOnlineLobbyTransactionTest>();
}

TSharedPtr<FOnlineLobbyTransaction> FOnlineLobbyInterfaceTest::MakeUpdateLobbyTransaction(
    const FUniqueNetId &UserId,
    const FOnlineLobbyId &LobbyId)
{
    return MakeShared<FOnlineLobbyTransactionTest>();
}

TSharedPtr<FOnlineLobbyMemberTransaction> FOnlineLobbyInterfaceTest::MakeUpdateLobbyMemberTransaction(
    const FUniqueNetId &UserId,
    const FOnlineLobbyId &LobbyId,
    const FUniqueNetId &MemberId)
{
    return MakeShared<FOnlineLobbyMemberTransactionTest>();
}

bool FOnlineLobbyInterfaceTest::CreateLobby(
    const FUniqueNetId &UserId,
    const FOnlineLobbyTransaction &Transaction,
    FOnLobbyCreateOrConnectComplete OnComplete)
{
    auto Lobby = this->Backend->CreateLobby();
    Lobby->OwnerId = UserId.AsShared();
    Lobby->Capacity = static_cast<int32>(Transaction.Capacity.Get(4));
    Lobby->AddMember(UserId, this->AsShared(), nullptr);
    for (auto Metadata : Transaction.SetMetadata)
    {
        Lobby->Metadata.Add(Metadata);
    }
    for (auto MetadataKey : Transaction.DeleteMetadata)
    {
        Lobby->Metadata.Remove(MetadataKey);
    }

    this->Backend->Lobbies.Add(Lobby->LobbyId, Lobby);

    auto ResultLobby = MakeShared<FOnlineLobbyTest>();
    ResultLobby->Id = MakeShared<FOnlineLobbyIdTest>(Lobby->LobbyId);
    ResultLobby->OwnerId = UserId.AsShared();

    OnComplete.ExecuteIfBound(FOnlineError::Success(), UserId, ResultLobby);

    return true;
}

bool FOnlineLobbyInterfaceTest::UpdateLobby(
    const FUniqueNetId &UserId,
    const FOnlineLobbyId &LobbyId,
    const FOnlineLobbyTransaction &Transaction,
    FOnLobbyOperationComplete OnComplete)
{
    auto Lobby = this->Backend->Lobbies[((const FOnlineLobbyIdTest &)LobbyId).Id];
    if (Transaction.Capacity.IsSet())
    {
        Lobby->Capacity = static_cast<int32>(Transaction.Capacity.GetValue());
    }
    for (auto Metadata : Transaction.SetMetadata)
    {
        Lobby->Metadata.Add(Metadata);
    }
    for (auto MetadataKey : Transaction.DeleteMetadata)
    {
        Lobby->Metadata.Remove(MetadataKey);
    }

    // Make a copy so if the delegate modifies the list, we don't crash.
    auto MemberListCopy = TUserIdMap<TSharedPtr<FTestBackendLobbyMember>>(Lobby->Members);
    for (auto Member : MemberListCopy)
    {
        if (Lobby->Members.Contains(*Member.Key))
        {
            Member.Value->OnlineLobby->TriggerOnLobbyUpdateDelegates(
                *Member.Key,
                *MakeShared<FOnlineLobbyIdTest>(Lobby->LobbyId));
        }
    }

    OnComplete.ExecuteIfBound(FOnlineError::Success(), UserId);

    return true;
}

bool FOnlineLobbyInterfaceTest::DeleteLobby(
    const FUniqueNetId &UserId,
    const FOnlineLobbyId &LobbyId,
    FOnLobbyOperationComplete OnComplete)
{
    auto Lobby = this->Backend->Lobbies[((const FOnlineLobbyIdTest &)LobbyId).Id];

    // Make a copy so if the delegate modifies the list, we don't crash.
    auto MemberListCopy = TUserIdMap<TSharedPtr<FTestBackendLobbyMember>>(Lobby->Members);
    for (auto Member : MemberListCopy)
    {
        if (Lobby->Members.Contains(*Member.Key))
        {
            Member.Value->OnlineLobby->TriggerOnLobbyDeleteDelegates(
                *Member.Key,
                *MakeShared<FOnlineLobbyIdTest>(Lobby->LobbyId));
        }
    }

    this->Backend->Lobbies.Remove(((const FOnlineLobbyIdTest &)LobbyId).Id);

    OnComplete.ExecuteIfBound(FOnlineError::Success(), UserId);

    return true;
}

bool FOnlineLobbyInterfaceTest::ConnectLobby(
    const FUniqueNetId &UserId,
    const FOnlineLobbyId &LobbyId,
    FOnLobbyCreateOrConnectComplete OnComplete)
{
    auto Lobby = this->Backend->Lobbies[((const FOnlineLobbyIdTest &)LobbyId).Id];

    if (Lobby->Members.Contains(UserId))
    {
        // The user has already joined, emulate the same behaviour an online API would have with an error.
        OnComplete.ExecuteIfBound(
            ONLINE_ERROR(EOnlineErrorResult::RequestFailure, TEXT("user_already_member")),
            UserId,
            nullptr);
        return true;
    }

    Lobby->AddMember(UserId, this->AsShared(), nullptr);

    // Make a copy so if the delegate modifies the list, we don't crash.
    auto MemberListCopy = TUserIdMap<TSharedPtr<FTestBackendLobbyMember>>(Lobby->Members);
    for (auto Member : MemberListCopy)
    {
        if (*Member.Key == UserId)
        {
            continue;
        }

        if (Lobby->Members.Contains(*Member.Key))
        {
            Member.Value->OnlineLobby->TriggerOnMemberConnectDelegates(
                *Member.Key,
                *MakeShared<FOnlineLobbyIdTest>(Lobby->LobbyId),
                UserId);
        }
    }

    auto ResultLobby = MakeShared<FOnlineLobbyTest>();
    ResultLobby->Id = MakeShared<FOnlineLobbyIdTest>(Lobby->LobbyId);
    ResultLobby->OwnerId = Lobby->OwnerId;

    OnComplete.ExecuteIfBound(FOnlineError::Success(), UserId, ResultLobby);

    return true;
}

bool FOnlineLobbyInterfaceTest::DisconnectLobby(
    const FUniqueNetId &UserIdPtr,
    const FOnlineLobbyId &LobbyIdPtr,
    FOnLobbyOperationComplete OnComplete)
{
    // We must delay DisconnectLobby in tests, because if it's called in response to a data update, subsequent data
    // update callbacks registered for the event won't be able to find the metadata because the member already
    // disconnected.
    FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateLambda(
            [this, OnComplete, LobbyId = LobbyIdPtr.AsShared(), UserId = UserIdPtr.AsShared()](float DeltaSeconds) {
                auto LobbyNumId = StaticCastSharedRef<const FOnlineLobbyIdTest>(LobbyId)->Id;
                if (!this->Backend->Lobbies.Contains(LobbyNumId))
                {
                    OnComplete.ExecuteIfBound(FOnlineError::Success(), *UserId);
                }
                else
                {
                    auto Lobby = this->Backend->Lobbies[LobbyNumId];

                    check(Lobby->Members.Contains(*UserId));
                    Lobby->RemoveMember(*UserId);

                    OnComplete.ExecuteIfBound(FOnlineError::Success(), *UserId);

                    // Make a copy so if the delegate modifies the list, we don't crash.
                    auto MemberListCopy = TUserIdMap<TSharedPtr<FTestBackendLobbyMember>>(Lobby->Members);
                    for (auto Member : MemberListCopy)
                    {
                        if (Lobby->Members.Contains(*Member.Key))
                        {
                            Member.Value->OnlineLobby->TriggerOnMemberDisconnectDelegates(
                                *Member.Key,
                                *MakeShared<FOnlineLobbyIdTest>(Lobby->LobbyId),
                                *UserId,
                                false);
                        }
                    }
                }

                return false;
            }),
        0.0f);
    return true;
}

bool FOnlineLobbyInterfaceTest::UpdateMemberSelf(
    const FUniqueNetId &UserId,
    const FOnlineLobbyId &LobbyId,
    const FOnlineLobbyMemberTransaction &Transaction,
    FOnLobbyOperationComplete OnComplete)
{
    auto Lobby = this->Backend->Lobbies[((const FOnlineLobbyIdTest &)LobbyId).Id];
    auto Member = Lobby->Members[UserId];
    for (auto Metadata : Transaction.SetMetadata)
    {
        Member->Metadata.Add(Metadata);
    }
    for (auto MetadataKey : Transaction.DeleteMetadata)
    {
        Member->Metadata.Remove(MetadataKey);
    }

    OnComplete.ExecuteIfBound(FOnlineError::Success(), UserId);

    // Make a copy so if the delegate modifies the list, we don't crash.
    auto MemberListCopy = TUserIdMap<TSharedPtr<FTestBackendLobbyMember>>(Lobby->Members);
    for (auto OtherMember : MemberListCopy)
    {
        if (Lobby->Members.Contains(*OtherMember.Key))
        {
            OtherMember.Value->OnlineLobby->TriggerOnMemberUpdateDelegates(
                *OtherMember.Key,
                *MakeShared<FOnlineLobbyIdTest>(Lobby->LobbyId),
                UserId);
        }
    }

    return true;
}

bool FOnlineLobbyInterfaceTest::GetMemberCount(
    const FUniqueNetId &UserId,
    const FOnlineLobbyId &LobbyId,
    int32 &OutMemberCount)
{
    auto Lobby = this->Backend->Lobbies[((const FOnlineLobbyIdTest &)LobbyId).Id];
    OutMemberCount = Lobby->MembersSortedIds.Num();
    return true;
}

bool FOnlineLobbyInterfaceTest::GetMemberUserId(
    const FUniqueNetId &UserId,
    const FOnlineLobbyId &LobbyId,
    int32 MemberIndex,
    TSharedPtr<const FUniqueNetId> &OutMemberId)
{
    auto Lobby = this->Backend->Lobbies[((const FOnlineLobbyIdTest &)LobbyId).Id];
    OutMemberId = Lobby->MembersSortedIds[MemberIndex];
    return true;
}

bool FOnlineLobbyInterfaceTest::GetMemberMetadataValue(
    const FUniqueNetId &UserId,
    const FOnlineLobbyId &LobbyId,
    const FUniqueNetId &MemberId,
    const FString &MetadataKey,
    FVariantData &OutMetadataValue)
{
    auto LobbyIdValue = ((const FOnlineLobbyIdTest &)LobbyId).Id;
    if (!this->Backend->Lobbies.Contains(LobbyIdValue))
    {
        return false;
    }
    auto Lobby = this->Backend->Lobbies[LobbyIdValue];
    if (!Lobby->Members.Contains(MemberId))
    {
        return false;
    }
    auto Member = Lobby->Members[MemberId];
    if (Member->Metadata.Contains(MetadataKey))
    {
        OutMetadataValue = Member->Metadata[MetadataKey];
        return true;
    }
    return false;
}

bool FOnlineLobbyInterfaceTest::Search(
    const FUniqueNetId &UserId,
    const FOnlineLobbySearchQuery &Query,
    FOnLobbySearchComplete OnComplete)
{
    TArray<TSharedRef<const FOnlineLobbyId>> MatchingLobbies;
    if (Query.Filters.Num() == 0)
    {
        for (auto CandidateLobby : this->Backend->Lobbies)
        {
            if (!Query.Limit.IsSet() || MatchingLobbies.Num() < (int32)Query.Limit.GetValue())
            {
                MatchingLobbies.Add(MakeShared<FOnlineLobbyIdTest>(CandidateLobby.Value->LobbyId));
            }
        }
    }
    else
    {
        for (auto CandidateLobby : this->Backend->Lobbies)
        {
            bool bMatching = true;
            for (auto Filter : Query.Filters)
            {
                if (!CandidateLobby.Value->MatchesFilter(Filter))
                {
                    bMatching = false;
                    break;
                }
            }

            if (bMatching)
            {
                if (!Query.Limit.IsSet() || MatchingLobbies.Num() < (int32)Query.Limit.GetValue())
                {
                    MatchingLobbies.Add(MakeShared<FOnlineLobbyIdTest>(CandidateLobby.Value->LobbyId));
                }
            }
        }
    }
    OnComplete.ExecuteIfBound(FOnlineError::Success(), UserId, MatchingLobbies);
    return true;
}

bool FOnlineLobbyInterfaceTest::GetLobbyMetadataValue(
    const FUniqueNetId &UserId,
    const FOnlineLobbyId &LobbyId,
    const FString &MetadataKey,
    FVariantData &OutMetadataValue)
{
    auto Lobby = this->Backend->Lobbies[((const FOnlineLobbyIdTest &)LobbyId).Id];
    if (Lobby->Metadata.Contains(MetadataKey))
    {
        OutMetadataValue = Lobby->Metadata[MetadataKey];
        return true;
    }
    return false;
}

#endif

REDPOINT_EOS_CODE_GUARD_END()