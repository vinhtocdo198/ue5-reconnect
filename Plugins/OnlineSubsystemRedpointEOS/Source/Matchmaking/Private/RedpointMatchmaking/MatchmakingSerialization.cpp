// Copyright June Rhodes. All Rights Reserved.

#include "MatchmakingSerialization.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "Interfaces/OnlineIdentityInterface.h"
#include "Misc/AutomationTest.h"
#include "Misc/Base64.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

FString FMatchmakingSerialization::SerializeResult(const TArray<FMatchmakingEngineResponseTeam> &InTeams)
{
    TArray<uint8> Buffer;

    checkf(InTeams.Num() < MAX_int8, TEXT("Too many teams for serialization!"));

    int8 TeamSize = (int8)InTeams.Num();

    FMemoryWriter Ar(Buffer, true);
    Ar << TeamSize;

    UE_LOG(LogMatchmakingSerialization, Verbose, TEXT("SerializeResult: Serializing result with %d teams"), TeamSize);

    for (const auto &Team : InTeams)
    {
        checkf(Team.Slots.Num() < MAX_int8, TEXT("Too many slots for serialization!"));

        int8 SlotCount = (int8)Team.Slots.Num();
        Ar << SlotCount;

        UE_LOG(
            LogMatchmakingSerialization,
            Verbose,
            TEXT("SerializeResult:  -- Serializing team with %d slots"),
            SlotCount);

        for (const auto &Slot : Team.Slots)
        {
            uint8 TypeSmall = (uint8)Slot.Type;
            Ar << TypeSmall;

            if (Slot.Type == EMatchmakingEngineResponseTeamSlotType::User)
            {
                UE_LOG(
                    LogMatchmakingSerialization,
                    Verbose,
                    TEXT("SerializeResult:  ---- Slot type %u, user ID %s"),
                    TypeSmall,
                    *Slot.UserId->ToString());

                TArray<uint8> UserIdBytes(Slot.UserId->GetBytes(), Slot.UserId->GetSize());
                Ar << UserIdBytes;
            }
            else
            {
                UE_LOG(LogMatchmakingSerialization, Verbose, TEXT("SerializeResult:  ---- Slot type %u"), TypeSmall);
            }
        }
    }

    Ar.Flush();

    return FBase64::Encode(Buffer);
}

TArray<FMatchmakingEngineResponseTeam> FMatchmakingSerialization::DeserializeResult(
    const TSharedRef<IOnlineIdentity, ESPMode::ThreadSafe> &InOnlineIdentity,
    const FString &InValue)
{
    TArray<uint8> Buffer;
    if (!FBase64::Decode(InValue, Buffer))
    {
        UE_LOG(LogMatchmakingSerialization, Error, TEXT("Invalid buffer for result deserialization!"));
        return TArray<FMatchmakingEngineResponseTeam>();
    }

    FMemoryReader Ar(Buffer, true);

    int8 TeamSize;
    Ar << TeamSize;

    UE_LOG(
        LogMatchmakingSerialization,
        Verbose,
        TEXT("DeserializeResult: Deserializing result with %d teams"),
        TeamSize);

    TArray<FMatchmakingEngineResponseTeam> Teams;

    for (int8 t = 0; t < TeamSize; t++)
    {
        int8 SlotSize;
        Ar << SlotSize;

        UE_LOG(
            LogMatchmakingSerialization,
            Verbose,
            TEXT("DeserializeResult:  -- Serializing team with %d slots"),
            SlotSize);

        TArray<FMatchmakingEngineResponseTeamSlot> Slots;

        for (int8 s = 0; s < SlotSize; s++)
        {
            uint8 TypeSmall;
            TArray<uint8> UserIdBytes;

            Ar << TypeSmall;

            FMatchmakingEngineResponseTeamSlot Slot;
            Slot.Type = (EMatchmakingEngineResponseTeamSlotType)TypeSmall;
            if (Slot.Type == EMatchmakingEngineResponseTeamSlotType::User)
            {
                Ar << UserIdBytes;

                Slot.UserId = InOnlineIdentity->CreateUniquePlayerId(UserIdBytes.GetData(), UserIdBytes.Num());
                UE_LOG(
                    LogMatchmakingSerialization,
                    Verbose,
                    TEXT("DeserializeResult:  ---- Slot type %u, user ID %s"),
                    TypeSmall,
                    *Slot.UserId->ToString());
            }
            else
            {
                UE_LOG(LogMatchmakingSerialization, Verbose, TEXT("DeserializeResult:  ---- Slot type %u"), TypeSmall);
            }
            Slots.Add(Slot);
        }

        FMatchmakingEngineResponseTeam Team;
        Team.Slots = Slots;

        Teams.Add(Team);
    }

    return Teams;
}

static const int MaxSegmentLength = 1000;

void FMatchmakingSerialization::SerializeResultToTransaction(
    const TArray<FMatchmakingEngineResponseTeam> &InTeams,
    const FString &Prefix,
    TSharedPtr<FOnlineLobbyTransaction> &Txn)
{
    FString EncodedData = SerializeResult(InTeams);
    TArray<FString> Segments;
    for (int i = 0; i < EncodedData.Len(); i += MaxSegmentLength)
    {
        Segments.Add(EncodedData.Mid(i, FMath::Min(EncodedData.Len() - i + MaxSegmentLength, MaxSegmentLength)));
    }

    Txn->SetMetadata.Add(Prefix, (int64)Segments.Num());
    for (int i = 0; i < Segments.Num(); i++)
    {
        Txn->SetMetadata.Add(FString::Printf(TEXT("%s.%d"), *Prefix, i), Segments[i]);
    }
}

TArray<FMatchmakingEngineResponseTeam> FMatchmakingSerialization::DeserializeResultFromLobbyData(
    const TSharedRef<IOnlineLobby, ESPMode::ThreadSafe> &InOnlineLobby,
    const TSharedRef<IOnlineIdentity, ESPMode::ThreadSafe> &InOnlineIdentity,
    const FUniqueNetId &UserId,
    const FOnlineLobbyId &LobbyId,
    const FString &Prefix)
{
    FString EncodedBuffer;

    FVariantData CountVar;
    if (!InOnlineLobby->GetLobbyMetadataValue(UserId, LobbyId, Prefix, CountVar) ||
        CountVar.GetType() != EOnlineKeyValuePairDataType::Int64)
    {
        UE_LOG(LogMatchmakingSerialization, Error, TEXT("Invalid lobby data for result deserialization!"));
        return TArray<FMatchmakingEngineResponseTeam>();
    }

    int64 SegmentCount;
    ;
    CountVar.GetValue(SegmentCount);

    for (int i = 0; i < SegmentCount; i++)
    {
        FVariantData SegmentVar;
        if (!InOnlineLobby
                 ->GetLobbyMetadataValue(UserId, LobbyId, FString::Printf(TEXT("%s.%d"), *Prefix, i), SegmentVar) ||
            SegmentVar.GetType() != EOnlineKeyValuePairDataType::String)
        {
            UE_LOG(LogMatchmakingSerialization, Error, TEXT("Invalid lobby data for result deserialization!"));
            return TArray<FMatchmakingEngineResponseTeam>();
        }

        FString Segment;
        SegmentVar.GetValue(Segment);

        EncodedBuffer += Segment;
    }

    return DeserializeResult(InOnlineIdentity, EncodedBuffer);
}

void FMatchmakingSerialization::DumpTeamArrayState(
    const TArray<FMatchmakingEngineResponseTeam> &Data,
    const FString &Context)
{
#if !defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING
    UE_LOG(LogMatchmakingSerialization, Verbose, TEXT("%s"), *Context);
    UE_LOG(LogMatchmakingSerialization, Verbose, TEXT("%s:  Team | Slot | Type | Assigned User Id"), *Context);
    UE_LOG(LogMatchmakingSerialization, Verbose, TEXT("%s: ------|------|------|------------------"), *Context);
    for (int32 t = 0; t < Data.Num(); t++)
    {
        const auto &Team = Data[t];
        for (int32 s = 0; s < Team.Slots.Num(); s++)
        {
            const auto &Slot = Team.Slots[s];

            FString UserId = TEXT("");
            if (Slot.UserId.IsValid())
            {
                UserId = Slot.UserId->ToString();
            }
            UE_LOG(
                LogMatchmakingSerialization,
                Verbose,
                TEXT("%s:  %s | %s | %s | %s"),
                *Context,
                *FString::Printf(TEXT("%d"), t).LeftPad(4),
                *FString::Printf(TEXT("%d"), s).LeftPad(4),
                *FString::Printf(TEXT("%d"), (int32)Slot.Type).LeftPad(4),
                *UserId);
        }
    }
#endif
}

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMatchmakingSerializationTest,
    "Redpoint.Matchmaking.Serialization",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter);

class FOnlineIdentityInterfaceString : public IOnlineIdentity,
                                       public TSharedFromThis<FOnlineIdentityInterfaceString, ESPMode::ThreadSafe>
{
public:
    FOnlineIdentityInterfaceString() = default;
    UE_NONCOPYABLE(FOnlineIdentityInterfaceString);
    virtual ~FOnlineIdentityInterfaceString() override = default;

    virtual bool Login(int32 LocalUserNum, const FOnlineAccountCredentials &AccountCredentials) override
        PURE_VIRTUAL(FOnlineIdentityInterfaceString::Login, return false;);
    virtual bool Logout(int32 LocalUserNum) override PURE_VIRTUAL(FOnlineIdentityInterfaceTest::Logout, return false;);

    virtual bool AutoLogin(int32 LocalUserNum) override
        PURE_VIRTUAL(FOnlineIdentityInterfaceString::AutoLogin, return false;);
    virtual TSharedPtr<FUserOnlineAccount> GetUserAccount(const FUniqueNetId &UserId) const override
        PURE_VIRTUAL(FOnlineIdentityInterfaceString::Login, return nullptr;);
    virtual TArray<TSharedPtr<FUserOnlineAccount>> GetAllUserAccounts() const override
        PURE_VIRTUAL(FOnlineIdentityInterfaceString::Login, return TArray<TSharedPtr<FUserOnlineAccount>>(););
    virtual TSharedPtr<const FUniqueNetId> GetUniquePlayerId(int32 LocalUserNum) const override
        PURE_VIRTUAL(FOnlineIdentityInterfaceString::Login, return nullptr;);
    virtual TSharedPtr<const FUniqueNetId> CreateUniquePlayerId(uint8 *Bytes, int32 Size) override
    {
        return FUniqueNetIdString::Create(
            FString(Size / static_cast<int32>(sizeof(TCHAR)), reinterpret_cast<TCHAR *>(Bytes)).TrimChar(TEXT('\0')),
            "Test");
    }
    virtual TSharedPtr<const FUniqueNetId> CreateUniquePlayerId(const FString &Str) override
        PURE_VIRTUAL(FOnlineIdentityInterfaceString::CreateUniquePlayerId, return nullptr;);
    virtual ELoginStatus::Type GetLoginStatus(int32 LocalUserNum) const override
        PURE_VIRTUAL(FOnlineIdentityInterfaceString::GetLoginStatus, return ELoginStatus::NotLoggedIn;);
    virtual ELoginStatus::Type GetLoginStatus(const FUniqueNetId &UserId) const override
        PURE_VIRTUAL(FOnlineIdentityInterfaceString::GetLoginStatus, return ELoginStatus::NotLoggedIn;);
    virtual FString GetPlayerNickname(int32 LocalUserNum) const override
        PURE_VIRTUAL(FOnlineIdentityInterfaceString::GetPlayerNickname, return TEXT(""););
    virtual FString GetPlayerNickname(const FUniqueNetId &) const override
        PURE_VIRTUAL(FOnlineIdentityInterfaceString::GetPlayerNickname, return TEXT(""););
    virtual FString GetAuthToken(int32 LocalUserNum) const override
        PURE_VIRTUAL(FOnlineIdentityInterfaceString::GetAuthToken, return TEXT(""););
    virtual void RevokeAuthToken(const FUniqueNetId &LocalUserId, const FOnRevokeAuthTokenCompleteDelegate &Delegate)
        override PURE_VIRTUAL(FOnlineIdentityInterfaceString::RevokeAuthToken, return;);
#if REDPOINT_EOS_UE_5_4_OR_LATER
    virtual void GetUserPrivilege(
        const FUniqueNetId &LocalUserId,
        EUserPrivileges::Type Privilege,
        const FOnGetUserPrivilegeCompleteDelegate &Delegate,
        EShowPrivilegeResolveUI ShowResolveUI = EShowPrivilegeResolveUI::Default) override
        PURE_VIRTUAL(FOnlineIdentityInterfaceString::GetUserPrivilege, return;);
#else
    virtual void GetUserPrivilege(
        const FUniqueNetId &LocalUserId,
        EUserPrivileges::Type Privilege,
        const FOnGetUserPrivilegeCompleteDelegate &Delegate) override
        PURE_VIRTUAL(FOnlineIdentityInterfaceString::GetUserPrivilege, return;);
#endif
    PRAGMA_DISABLE_DEPRECATION_WARNINGS
    virtual FPlatformUserId GetPlatformUserIdFromUniqueNetId(const FUniqueNetId &UniqueNetId) const override
        PURE_VIRTUAL(FOnlineIdentityInterfaceString::GetPlatformUserIdFromUniqueNetId,
                     return FPlatformUserId::CreateFromInternalId(0););
    PRAGMA_ENABLE_DEPRECATION_WARNINGS
    virtual FString GetAuthType() const override
        PURE_VIRTUAL(FOnlineIdentityInterfaceString::GetAuthType, return TEXT(""););
};

bool FMatchmakingSerializationTest::RunTest(const FString &Parameters)
{
    FMatchmakingEngineResponseTeam TeamA;
    TeamA.Slots.Add(FMatchmakingEngineResponseTeamSlot{
        EMatchmakingEngineResponseTeamSlotType::User,
        FUniqueNetIdString::Create("User1", "Test")});
    TeamA.Slots.Add(FMatchmakingEngineResponseTeamSlot{
        EMatchmakingEngineResponseTeamSlotType::User,
        FUniqueNetIdString::Create("User2", "Test")});
    TeamA.Slots.Add(FMatchmakingEngineResponseTeamSlot{EMatchmakingEngineResponseTeamSlotType::Empty, nullptr});
    TeamA.Slots.Add(FMatchmakingEngineResponseTeamSlot{EMatchmakingEngineResponseTeamSlotType::Empty, nullptr});

    FMatchmakingEngineResponseTeam TeamB;
    TeamB.Slots.Add(FMatchmakingEngineResponseTeamSlot{
        EMatchmakingEngineResponseTeamSlotType::User,
        FUniqueNetIdString::Create("User3", "Test")});
    TeamB.Slots.Add(FMatchmakingEngineResponseTeamSlot{
        EMatchmakingEngineResponseTeamSlotType::User,
        FUniqueNetIdString::Create("User4", "Test")});
    TeamB.Slots.Add(FMatchmakingEngineResponseTeamSlot{
        EMatchmakingEngineResponseTeamSlotType::User,
        FUniqueNetIdString::Create("User5", "Test")});
    TeamB.Slots.Add(FMatchmakingEngineResponseTeamSlot{
        EMatchmakingEngineResponseTeamSlotType::User,
        FUniqueNetIdString::Create("User6", "Test")});

    TArray<FMatchmakingEngineResponseTeam> Teams;
    Teams.Add(TeamA);
    Teams.Add(TeamB);

    FString SerializedTeams = FMatchmakingSerialization::SerializeResult(Teams);

    TArray<FMatchmakingEngineResponseTeam> DeserializedTeams = FMatchmakingSerialization::DeserializeResult(
        MakeShared<FOnlineIdentityInterfaceString, ESPMode::ThreadSafe>(),
        SerializedTeams);

    if (this->TestEqual("Team count", DeserializedTeams.Num(), Teams.Num()))
    {
        for (int32 i = 0; i < Teams.Num(); i++)
        {
            if (this->TestEqual(
                    FString::Printf(TEXT("Team %d slot count"), i),
                    DeserializedTeams[i].Slots.Num(),
                    Teams[i].Slots.Num()))
            {
                for (int32 s = 0; s < Teams[i].Slots.Num(); s++)
                {
                    this->TestEqual(
                        FString::Printf(TEXT("Team %d slot %d type"), i, s),
                        DeserializedTeams[i].Slots[s].Type,
                        Teams[i].Slots[s].Type);
                    if (!Teams[i].Slots[s].UserId.IsValid() || !DeserializedTeams[i].Slots[s].UserId.IsValid())
                    {
                        this->TestEqual(
                            FString::Printf(TEXT("Team %d slot %d user ID null comparison"), i, s),
                            DeserializedTeams[i].Slots[s].UserId.IsValid(),
                            Teams[i].Slots[s].UserId.IsValid());
                    }
                    else
                    {
                        this->TestEqual(
                            FString::Printf(TEXT("Team %d slot %d type user ID compare"), i, s),
                            DeserializedTeams[i].Slots[s].UserId->ToString(),
                            Teams[i].Slots[s].UserId->ToString());
                    }
                }
            }
        }
    }

    return true;
}

#endif

#endif

REDPOINT_EOS_CODE_GUARD_END()