// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSFriends/Database/SerializedFriendDatabase.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSFriends/Database/LegacySerializedCachedFriend.h"
#include "RedpointEOSFriends/Logging.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3612751533, Redpoint::EOS::Friends::Database)
{
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Friends;
using namespace ::Redpoint::EOS::Friends::Database;

template <typename T>
void MapArchive(FArchive &Ar, TAccountIdMap<T> &Map, TFunction<TOptional<UE::Online::FAccountId>(const T &)> IdMapper)
{
    if (Ar.IsLoading())
    {
        int32 Count;
        Ar << Count;
        for (int32 i = 0; i < Count; i++)
        {
            T Val;
            Ar << Val;
            auto KeyId = IdMapper(Val);
            if (KeyId.IsSet() && KeyId->IsValid())
            {
                Map.Add(*KeyId, Val);
            }
            else
            {
                UE_LOG(
                    LogRedpointEOSFriends,
                    Warning,
                    TEXT("Invalid user ID on archive entry %d; it will be ignored during deserialization."),
                    i);
            }
        }
    }
    else
    {
        int32 Count = Map.Num();
        Ar << Count;
        for (const auto &KV : Map)
        {
            T Val = KV.Value;
            Ar << Val;
        }
    }
}

void FSerializedFriendDatabase::Archive(FArchive &Ar)
{
    int8 Version;
    if (Ar.IsSaving())
    {
        Version = 3;
    }
    Ar << Version;
    MapArchive<FSerializedFriend>(Ar, this->Friends, [](const FSerializedFriend &F) {
        return TryParseAccountId(F.ProductUserId);
    });
    if (Ar.IsSaving())
    {
        // For any entries above the 99th index, remove them from map. This prevents
        // recent players from expanding beyond 100 entries.
        TArray<TPair<UE::Online::FAccountId, FSerializedRecentPlayer>> RecentPlayersSorted;
        for (const auto &KV : this->RecentPlayers)
        {
            RecentPlayersSorted.Add(KV);
        }
        RecentPlayersSorted.Sort([](const auto &A, const auto &B) {
            return A.Value.LastSeen > B.Value.LastSeen;
        });
        for (int32 i = 100; i < RecentPlayersSorted.Num(); i++)
        {
            this->RecentPlayers.Remove(RecentPlayersSorted[i].Key);
        }
        RecentPlayersSorted.Empty();
    }
    MapArchive<FSerializedRecentPlayer>(Ar, this->RecentPlayers, [](const FSerializedRecentPlayer &F) {
        return TryParseAccountId(F.ProductUserId);
    });
    MapArchive<FSerializedBlockedUser>(Ar, this->BlockedUsers, [](const FSerializedBlockedUser &F) {
        return TryParseAccountId(F.ProductUserId);
    });
    MapArchive<FSerializedPendingFriendRequest>(
        Ar,
        this->PendingFriendRequests,
        [](const FSerializedPendingFriendRequest &F) {
            return TryParseAccountId(F.ProductUserId);
        });
    if (Version < 3 && !Ar.IsSaving())
    {
        this->RuntimeIntegrationCachedFriends.Empty();
        TMap<FName, TMap<FString, FLegacySerializedCachedFriend>> DelegatedSubsystemCachedFriends;
        Ar << DelegatedSubsystemCachedFriends;
        for (const auto &SubsystemToKV : DelegatedSubsystemCachedFriends)
        {
            for (const auto &AccountIdToInfo : SubsystemToKV.Value)
            {
                FString CacheKey;
                TOptional<EOS_EExternalAccountType> ExternalAccountType;
                // @note: Best effort, since the cache will be re-populated the next time the user is on the platform
                // anyway.
                if (SubsystemToKV.Key.IsEqual(FName("Steam")))
                {
                    ExternalAccountType = EOS_EExternalAccountType::EOS_EAT_STEAM;
                    CacheKey = TEXT("Steam");
                }
                else if (
                    SubsystemToKV.Key.IsEqual(FName("Discord")) || SubsystemToKV.Key.IsEqual(FName("RedpointDiscord")))
                {
                    ExternalAccountType = EOS_EExternalAccountType::EOS_EAT_DISCORD;
                    CacheKey = TEXT("Discord");
                }
                else if (
                    SubsystemToKV.Key.IsEqual(FName("ItchIo")) || SubsystemToKV.Key.IsEqual(FName("RedpointItchIo")))
                {
                    ExternalAccountType = EOS_EExternalAccountType::EOS_EAT_ITCHIO;
                    CacheKey = TEXT("ItchIo");
                }
                else if (
                    SubsystemToKV.Key.IsEqual(FName("Google")) || SubsystemToKV.Key.IsEqual(FName("RedpointGoogle")))
                {
                    ExternalAccountType = EOS_EExternalAccountType::EOS_EAT_GOOGLE;
                    CacheKey = TEXT("Google");
                }
                else if (SubsystemToKV.Key.IsEqual(FName("Apple")))
                {
                    ExternalAccountType = EOS_EExternalAccountType::EOS_EAT_APPLE;
                    CacheKey = TEXT("Apple");
                }
                else if (SubsystemToKV.Key.IsEqual(FName("RedpointEAS")))
                {
                    ExternalAccountType = EOS_EExternalAccountType::EOS_EAT_EPIC;
                    CacheKey = TEXT("Epic");
                }
                if (ExternalAccountType.IsSet())
                {
                    FSerializedCachedFriend::FSerializedCachedFriendLinkedAccount LinkedAccount = {};
                    LinkedAccount.ExternalAccountType = ExternalAccountType.GetValue();
                    LinkedAccount.DisplayName = AccountIdToInfo.Value.AccountDisplayName;
                    LinkedAccount.AccountId = AccountIdToInfo.Key;
                    auto LinkedAccounts =
                        TMap<EOS_EExternalAccountType, FSerializedCachedFriend::FSerializedCachedFriendLinkedAccount>{
                            {ExternalAccountType.GetValue(), LinkedAccount}};
                    this->RuntimeIntegrationCachedFriends.FindOrAdd(CacheKey).Add(FSerializedCachedFriend{
                        LinkedAccounts,
                        TMap<FString, FString>(),
                        AccountIdToInfo.Value.AccountAvatarUrl,
                        AccountIdToInfo.Value.AccountAvatarUrlLastFetched});
                }
            }
        }
    }
    else
    {
        Ar << this->RuntimeIntegrationCachedFriends;
    }
    if (Version >= 2)
    {
        if (Ar.IsSaving())
        {
            int32 AliasCount = this->FriendAliases.Num();
            Ar << AliasCount;
            for (const auto &KV : this->FriendAliases)
            {
                FString Key = FString::Printf(TEXT("RedpointEAS:%s"), *GetUserIdString(KV.Key));
                FString Value = KV.Value;
                Ar << Key;
                Ar << Value;
            }
        }
        else
        {
            this->FriendAliases.Empty();
            int32 AliasCount;
            Ar << AliasCount;
            for (int32 i = 0; i < AliasCount; i++)
            {
                FString Key, Value;
                Ar << Key;
                Ar << Value;

                TArray<FString> Elements;
                Key.ParseIntoArray(Elements, TEXT(":"));
                if (Elements.Num() == 2 && Elements[0] == TEXT("RedpointEAS"))
                {
                    auto UserId = TryParseAccountId(Elements[1]);
                    if (UserId.IsSet())
                    {
                        this->FriendAliases.Add(UserId.GetValue(), Value);
                    }
                }
            }
        }
    }
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()