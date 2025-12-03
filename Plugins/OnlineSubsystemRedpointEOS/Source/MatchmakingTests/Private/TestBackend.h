// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "OnlineKeyValuePair.h"
#include "RedpointEOSInterfaces/Interfaces/OnlineLobbyInterface.h"
#include "RedpointMatchmaking/Utility/UserIdContainers.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

typedef int64 FTestBackendLobbyId;

class FOnlineLobbyIdTest : public FOnlineLobbyId
{
public:
    FTestBackendLobbyId Id;

    FOnlineLobbyIdTest(FTestBackendLobbyId InId)
        : Id(InId) {};
    FOnlineLobbyIdTest(const FOnlineLobbyIdTest &) = delete;
    virtual ~FOnlineLobbyIdTest() override = default;

    virtual const uint8 *GetBytes() const override;
    virtual int32 GetSize() const override;
    virtual bool IsValid() const override;
    virtual FString ToString() const override;
    virtual FString ToDebugString() const override;
};

struct FTestBackendLobbyMember
{
    TSharedPtr<const FUniqueNetId> UserId;
    TMap<FString, FVariantData> Metadata;
    TSharedPtr<IOnlineLobby, ESPMode::ThreadSafe> OnlineLobby;
    TSharedPtr<IOnlinePartySystem, ESPMode::ThreadSafe> OnlineParty;
};

struct FTestBackendLobby
{
    FTestBackendLobbyId LobbyId;
    TSharedPtr<const FUniqueNetId> OwnerId;
    int32 Capacity;
    TUserIdMap<TSharedPtr<FTestBackendLobbyMember>> Members;
    TArray<TSharedPtr<const FUniqueNetId>> MembersSortedIds;
    TMap<FString, FVariantData> Metadata;

    TSharedRef<FTestBackendLobbyMember> AddMember(
        const FUniqueNetId &Id,
        TSharedPtr<IOnlineLobby, ESPMode::ThreadSafe> InOnlineLobby,
        TSharedPtr<IOnlinePartySystem, ESPMode::ThreadSafe> InOnlineParty)
    {
        auto Member = MakeShared<FTestBackendLobbyMember>();
        Member->UserId = Id.AsShared();
        Member->OnlineLobby = InOnlineLobby;
        Member->OnlineParty = InOnlineParty;
        Members.Add(Id, Member);
        MembersSortedIds.Add(Id.AsShared());
        return Member;
    }

    void RemoveMember(const FUniqueNetId &Id)
    {
        Members.Remove(Id);
        for (int i = MembersSortedIds.Num() - 1; i >= 0; i--)
        {
            if (*MembersSortedIds[i] == Id)
            {
                MembersSortedIds.RemoveAt(i);
            }
        }
    }

    template <typename T> T Val(const FVariantData &Data)
    {
        T Local;
        Data.GetValue(Local);
        return Local;
    }

    bool MatchesFilter(const FOnlineLobbySearchQueryFilter &Filter)
    {
        if (!Metadata.Contains(Filter.Key))
        {
            return false;
        }

        auto MetadataValue = Metadata[Filter.Key];

        // Check that we can actually do the comparison.
        switch (Filter.Comparison)
        {
        case EOnlineLobbySearchQueryFilterComparator::GreaterThan:
        case EOnlineLobbySearchQueryFilterComparator::LessThan:
        case EOnlineLobbySearchQueryFilterComparator::GreaterThanOrEqual:
        case EOnlineLobbySearchQueryFilterComparator::LessThanOrEqual:
            if (MetadataValue.GetType() != Filter.Value.GetType() || !MetadataValue.IsNumeric())
            {
                return false;
            }
            break;
        default:
            break;
        }

        // Now do the comparison.
        switch (Filter.Comparison)
        {
        case EOnlineLobbySearchQueryFilterComparator::Equal:
            return MetadataValue == Filter.Value;
        case EOnlineLobbySearchQueryFilterComparator::NotEqual:
            return MetadataValue != Filter.Value;
        case EOnlineLobbySearchQueryFilterComparator::GreaterThan:
            switch (MetadataValue.GetType())
            {
            case EOnlineKeyValuePairDataType::Int32:
                return Val<int32>(MetadataValue) > Val<int32>(Filter.Value);
            case EOnlineKeyValuePairDataType::Int64:
                return Val<int64>(MetadataValue) > Val<int64>(Filter.Value);
            case EOnlineKeyValuePairDataType::UInt32:
                return Val<uint32>(MetadataValue) > Val<uint32>(Filter.Value);
            case EOnlineKeyValuePairDataType::UInt64:
                return Val<uint64>(MetadataValue) > Val<uint64>(Filter.Value);
            case EOnlineKeyValuePairDataType::Float:
                return Val<float>(MetadataValue) > Val<float>(Filter.Value);
            case EOnlineKeyValuePairDataType::Double:
                return Val<double>(MetadataValue) > Val<double>(Filter.Value);
            default:
                check(false /* Non-numeric type in numeric comparison */);
                return false;
            }
        case EOnlineLobbySearchQueryFilterComparator::LessThan:
            switch (MetadataValue.GetType())
            {
            case EOnlineKeyValuePairDataType::Int32:
                return Val<int32>(MetadataValue) < Val<int32>(Filter.Value);
            case EOnlineKeyValuePairDataType::Int64:
                return Val<int64>(MetadataValue) < Val<int64>(Filter.Value);
            case EOnlineKeyValuePairDataType::UInt32:
                return Val<uint32>(MetadataValue) < Val<uint32>(Filter.Value);
            case EOnlineKeyValuePairDataType::UInt64:
                return Val<uint64>(MetadataValue) < Val<uint64>(Filter.Value);
            case EOnlineKeyValuePairDataType::Float:
                return Val<float>(MetadataValue) < Val<float>(Filter.Value);
            case EOnlineKeyValuePairDataType::Double:
                return Val<double>(MetadataValue) < Val<double>(Filter.Value);
            default:
                check(false /* Non-numeric type in numeric comparison */);
                return false;
            }
        case EOnlineLobbySearchQueryFilterComparator::GreaterThanOrEqual:
            switch (MetadataValue.GetType())
            {
            case EOnlineKeyValuePairDataType::Int32:
                return Val<int32>(MetadataValue) >= Val<int32>(Filter.Value);
            case EOnlineKeyValuePairDataType::Int64:
                return Val<int64>(MetadataValue) >= Val<int64>(Filter.Value);
            case EOnlineKeyValuePairDataType::UInt32:
                return Val<uint32>(MetadataValue) >= Val<uint32>(Filter.Value);
            case EOnlineKeyValuePairDataType::UInt64:
                return Val<uint64>(MetadataValue) >= Val<uint64>(Filter.Value);
            case EOnlineKeyValuePairDataType::Float:
                return Val<float>(MetadataValue) >= Val<float>(Filter.Value);
            case EOnlineKeyValuePairDataType::Double:
                return Val<double>(MetadataValue) >= Val<double>(Filter.Value);
            default:
                check(false /* Non-numeric type in numeric comparison */);
                return false;
            }
        case EOnlineLobbySearchQueryFilterComparator::LessThanOrEqual:
            switch (MetadataValue.GetType())
            {
            case EOnlineKeyValuePairDataType::Int32:
                return Val<int32>(MetadataValue) <= Val<int32>(Filter.Value);
            case EOnlineKeyValuePairDataType::Int64:
                return Val<int64>(MetadataValue) <= Val<int64>(Filter.Value);
            case EOnlineKeyValuePairDataType::UInt32:
                return Val<uint32>(MetadataValue) <= Val<uint32>(Filter.Value);
            case EOnlineKeyValuePairDataType::UInt64:
                return Val<uint64>(MetadataValue) <= Val<uint64>(Filter.Value);
            case EOnlineKeyValuePairDataType::Float:
                return Val<float>(MetadataValue) <= Val<float>(Filter.Value);
            case EOnlineKeyValuePairDataType::Double:
                return Val<double>(MetadataValue) <= Val<double>(Filter.Value);
            default:
                check(false /* Non-numeric type in numeric comparison */);
                return false;
            }
        case EOnlineLobbySearchQueryFilterComparator::Distance:
            // We don't need ordering in the tests.
            return true;
        default:
            check(false /* Invalid comparator in query */);
            return false;
        }
    }
};

class FTestBackend
{
public:
    FTestBackendLobbyId NextLobbyId;
    TMap<FTestBackendLobbyId, TSharedPtr<FTestBackendLobby>> Lobbies;

    TSharedPtr<FTestBackendLobby> CreateLobby();
};

#endif

REDPOINT_EOS_CODE_GUARD_END()