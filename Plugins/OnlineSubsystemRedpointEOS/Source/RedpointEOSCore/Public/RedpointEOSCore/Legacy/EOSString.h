// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/StringConv.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Logging.h"

#ifndef EOS_UNLIMITED_MAX_LENGTH
#define EOS_UNLIMITED_MAX_LENGTH (-1)
#endif

// @note: Legacy support while ever this file is still around.
#if !defined(__CDECL_ATTR)
#if PLATFORM_ANDROID
#define __CDECL_ATTR
#else
#define __CDECL_ATTR __cdecl
#endif
#define __CDECL_ATTR_DECLARED_IN_THIS_HEADER 1
#endif

// @note: Legacy support while ever this file is still around.
#ifndef EOS_P2P_SOCKET_NAME_MAX_LENGTH
#define EOS_P2P_SOCKET_NAME_MAX_LENGTH 32
#endif

template <
    char const *TIdTypeName,
    typename TId,
    int32_t MaxLengthExcludingNullTerminator,
    EOS_Bool (*Func_IsValid)(TId InId),
    EOS_EResult (*Func_ToString)(TId InId, char *OutBuffer, int32_t *InOutBufferLength),
    TId (*Func_FromString)(const char *InString)>
class EOSString_OpaqueId
{
    static_assert(
        MaxLengthExcludingNullTerminator != EOS_UNLIMITED_MAX_LENGTH,
        "MaxLengthExcludingNullTerminator for EOSString_OpaqueId must not be unlimited");
    static_assert(
        MaxLengthExcludingNullTerminator > EOS_UNLIMITED_MAX_LENGTH,
        "MaxLengthExcludingNullTerminator must be non-negative");

public:
    struct AnsiString
    {
        FString StrTCHAR;
        EOS_EResult Result;
        TStringConversion<TStringConvert<TCHAR, ANSICHAR>> Ptr;

        AnsiString(const TId InId)
            : StrTCHAR()
            , Result(EOSString_OpaqueId<
                     TIdTypeName,
                     TId,
                     MaxLengthExcludingNullTerminator,
                     Func_IsValid,
                     Func_ToString,
                     Func_FromString>::ToString(InId, this->StrTCHAR))
            , Ptr(StringCast<ANSICHAR>(*this->StrTCHAR))
        {
        }
    };

    static constexpr const TId None = nullptr;

    static bool IsNone(const TId InId)
    {
        return InId == nullptr;
    }

    static bool IsValid(const TId InId)
    {
        if (InId == nullptr)
        {
            return false;
        }

        return Func_IsValid(InId) == EOS_TRUE;
    }

    static bool IsEqual(const TId InA, const TId InB)
    {
        FString StrA, StrB;
        EOS_EResult ResultA, ResultB;
        ResultA = ToString(InA, StrA);
        ResultB = ToString(InB, StrB);
        if (ResultA != EOS_EResult::EOS_Success || ResultB != EOS_EResult::EOS_Success)
        {
            return ResultA == ResultB;
        }

        return StrA == StrB;
    }

    static EOS_EResult ToString(const TId InId, FString &OutString)
    {
        char Buffer[MaxLengthExcludingNullTerminator + 1];
        FMemory::Memzero(Buffer, MaxLengthExcludingNullTerminator + 1);
        int32_t BufferLen = MaxLengthExcludingNullTerminator + 1;
        EOS_EResult Result = Func_ToString(InId, Buffer, &BufferLen);
        if (Result != EOS_EResult::EOS_Success)
        {
            OutString = TEXT("");
            return Result;
        }

        OutString = ANSI_TO_TCHAR(Buffer);
        return Result;
    }

    static AnsiString ToAnsiString(const TId InId)
    {
        return AnsiString(InId);
    }

    static EOS_EResult FromString(const FString &InString, TId &OutId)
    {
        OutId = Func_FromString(TCHAR_TO_ANSI(*InString));
        if (!Func_IsValid(OutId))
        {
            OutId = nullptr;
            return EOS_EResult::EOS_InvalidParameters;
        }

        return EOS_EResult::EOS_Success;
    }

    static EOS_EResult AllocateToCharBuffer(const TId InId, const char *&OutCharPtr)
    {
        check(OutCharPtr == nullptr);
        OutCharPtr = (char *)FMemory::MallocZeroed(MaxLengthExcludingNullTerminator + 1);
        int32_t BufferLen = MaxLengthExcludingNullTerminator + 1;
        EOS_EResult Result = Func_ToString(InId, (char *&)OutCharPtr, &BufferLen);
        if (Result != EOS_EResult::EOS_Success)
        {
            UE_LOG(
                LogRedpointEOS,
                Error,
                TEXT("EOSString_OpaqueId::AllocateToCharBuffer failed when converting '%s', got result: %s"),
                ANSI_TO_TCHAR(TIdTypeName),
                ANSI_TO_TCHAR(EOS_EResult_ToString(Result)));
            FMemory::Free((void *)OutCharPtr);
            OutCharPtr = nullptr;
            return Result;
        }

        return Result;
    }

    static void FreeFromCharBuffer(const char *&CharPtr)
    {
        if (CharPtr != nullptr)
        {
            FMemory::Free((void *)CharPtr);
            CharPtr = nullptr;
        }
    }

    static void AllocateToCharList(const TArray<TId> &InIds, uint32_t &OutIdCount, const char **&OutIds)
    {
        if (InIds.Num() == 0)
        {
            OutIdCount = 0;
            OutIds = nullptr;
            return;
        }

        OutIdCount = InIds.Num();
        OutIds = (const char **)FMemory::MallocZeroed(sizeof(const char *) * OutIdCount, 0);
        for (uint32_t i = 0; i < OutIdCount; i++)
        {
            if (AllocateToCharBuffer(InIds[i], OutIds[i]) != EOS_EResult::EOS_Success)
            {
                continue;
            }
        }
    }

    static void FreeFromCharListConst(uint32_t IdCount, const char **const Ids)
    {
        if (IdCount == 0 && Ids == nullptr)
        {
            return;
        }

        for (uint32_t i = 0; i < IdCount; i++)
        {
            FreeFromCharBuffer(Ids[i]);
        }
        FMemory::Free((void *)Ids);
    }

    static void FreeFromCharList(uint32_t IdCount, const char **&Ids)
    {
        if (IdCount == 0 && Ids == nullptr)
        {
            return;
        }

        for (uint32_t i = 0; i < IdCount; i++)
        {
            FreeFromCharBuffer(Ids[i]);
        }
        FMemory::Free((void *)Ids);
        Ids = nullptr;
    }

    static void AllocateToIdList(const TArray<TId> &InIds, uint32_t &OutIdCount, TId *&OutIds)
    {
        OutIdCount = InIds.Num();
        // NOLINTNEXTLINE(bugprone-sizeof-expression)
        OutIds = (TId *)FMemory::MallocZeroed(sizeof(TId) * OutIdCount, 0);
        for (uint32_t i = 0; i < OutIdCount; i++)
        {
            OutIds[i] = InIds[i];
        }
    }

    template <typename TObject>
    static void AllocateToIdListViaAccessor(
        const TArray<TObject> &InObjects,
        TFunction<TId(const TObject &)> Accessor,
        uint32_t &OutIdCount,
        TId *&OutIds)
    {
        TArray<TId> Ids;
        for (const auto &Object : InObjects)
        {
            Ids.Add(Accessor(Object));
        }
        return AllocateToIdList(Ids, OutIdCount, OutIds);
    }

    static void FreeFromIdListConst(uint32_t IdCount, TId *const Ids)
    {
        FMemory::Free((void *)Ids);
    }

    static void FreeFromIdList(uint32_t IdCount, TId *&Ids)
    {
        FMemory::Free((void *)Ids);
        Ids = nullptr;
    }
};

template <typename TId, int32_t MaxLengthExcludingNullTerminator> class EOSString_CharBasedId
{
    static_assert(
        MaxLengthExcludingNullTerminator >= EOS_UNLIMITED_MAX_LENGTH,
        "MaxLengthExcludingNullTerminator must be non-negative");

public:
    static constexpr const TId None = nullptr;

    static bool IsNone(const TId InId)
    {
        return InId == nullptr;
    }

    static bool IsValid(const TId InId)
    {
        if (InId == nullptr)
        {
            return false;
        }

        FString IdAsString = ANSI_TO_TCHAR(InId);
        if (IdAsString.Len() == 0)
        {
            return false;
        }
        if (MaxLengthExcludingNullTerminator != EOS_UNLIMITED_MAX_LENGTH &&
            IdAsString.Len() > MaxLengthExcludingNullTerminator)
        {
            return false;
        }

        return true;
    }

    static FString ToString(const TId InId)
    {
        return ANSI_TO_TCHAR(InId);
    }

    static EOS_EResult AllocateToCharBuffer(const TId InId, const char *&OutCharPtr, int32 &OutLen)
    {
        if (OutCharPtr != nullptr)
        {
            return EOS_EResult::EOS_InvalidParameters;
        }
        if (!IsValid(InId))
        {
            return EOS_EResult::EOS_InvalidParameters;
        }

        const int ScanLen =
            MaxLengthExcludingNullTerminator == EOS_UNLIMITED_MAX_LENGTH ? 100000 : MaxLengthExcludingNullTerminator;
        int IdLen = strnlen(InId, ScanLen);
        OutCharPtr = (char *)FMemory::MallocZeroed(IdLen + 1);
        OutLen = IdLen + 1;
        FMemory::Memcpy((void *)OutCharPtr, (void *)InId, IdLen);
        return EOS_EResult::EOS_Success;
    }

    static void FreeFromCharBuffer(const char *&CharPtr)
    {
        if (CharPtr != nullptr)
        {
            FMemory::Free((void *)CharPtr);
            CharPtr = nullptr;
        }
    }
};

template <typename TToken> class EOSString_OpaqueToken
{
public:
    static constexpr const TToken None = nullptr;

    static bool IsNone(const TToken InToken)
    {
        return InToken == nullptr;
    }
};

template <int32_t MaxLengthExcludingNullTerminator> class EOSString_AnsiString
{
    static_assert(
        MaxLengthExcludingNullTerminator >= EOS_UNLIMITED_MAX_LENGTH,
        "MaxLengthExcludingNullTerminator must be non-negative");

private:
    static EOS_EResult CopyToExistingBufferPtr(const FString &InString, char *ExistingPtr)
    {
        if (MaxLengthExcludingNullTerminator == EOS_UNLIMITED_MAX_LENGTH)
        {
            return EOS_EResult::EOS_LimitExceeded;
        }

        auto StrBuf = StringCast<ANSICHAR>(*InString);
        if (InString.Len() > MaxLengthExcludingNullTerminator)
        {
            return EOS_EResult::EOS_LimitExceeded;
        }
        FMemory::Memzero(ExistingPtr, MaxLengthExcludingNullTerminator + 1);
        FMemory::Memcpy(ExistingPtr, StrBuf.Get(), StrBuf.Length());
        return EOS_EResult::EOS_Success;
    }

public:
    struct AnsiString
    {
        FString StrTCHAR;
        EOS_EResult Result;
        TStringConversion<TStringConvert<TCHAR, ANSICHAR>> Ptr;

        AnsiString(const FString &InString)
            : StrTCHAR(InString)
            , Result(
                  (MaxLengthExcludingNullTerminator == EOS_UNLIMITED_MAX_LENGTH ||
                   this->StrTCHAR.Len() <= MaxLengthExcludingNullTerminator)
                      ? EOS_EResult::EOS_Success
                      : EOS_EResult::EOS_LimitExceeded)
            , Ptr(StringCast<ANSICHAR>(*this->StrTCHAR))
        {
        }
    };

    static bool IsValid(const FString &InString)
    {
        return InString.Len() > 0 && (MaxLengthExcludingNullTerminator == EOS_UNLIMITED_MAX_LENGTH ||
                                      InString.Len() <= MaxLengthExcludingNullTerminator);
    }

    static AnsiString ToAnsiString(const FString &InString)
    {
        return AnsiString(InString);
    }

    static FString FromAnsiString(const char *InAnsiString)
    {
        return ANSI_TO_TCHAR(InAnsiString);
    }

    static void AllocateEmptyCharBuffer(char *&OutCharPtr, int32 &OutLen)
    {
        check(OutCharPtr == nullptr);
        check(MaxLengthExcludingNullTerminator > 0);

        OutCharPtr = (char *)FMemory::MallocZeroed(MaxLengthExcludingNullTerminator + 1);
        OutLen = MaxLengthExcludingNullTerminator + 1;
    }

    static EOS_EResult AllocateToCharBuffer(const FString &InString, const char *&OutCharPtr)
    {
        int32 Len;
        return AllocateToCharBuffer(InString, OutCharPtr, Len);
    }

    static EOS_EResult AllocateToCharBuffer(const FString &InString, const char *&OutCharPtr, int32 &OutLen)
    {
        if (OutCharPtr != nullptr)
        {
            return EOS_EResult::EOS_InvalidParameters;
        }

        auto StrBuf = StringCast<ANSICHAR>(*InString);
        if (MaxLengthExcludingNullTerminator != EOS_UNLIMITED_MAX_LENGTH &&
            StrBuf.Length() > MaxLengthExcludingNullTerminator)
        {
            OutLen = 0;
            return EOS_EResult::EOS_LimitExceeded;
        }
        OutCharPtr = (char *)FMemory::MallocZeroed(StrBuf.Length() + 1);
        FMemory::Memcpy((void *)OutCharPtr, (const void *)StrBuf.Get(), StrBuf.Length());
        OutLen = StrBuf.Length() + 1;
        return EOS_EResult::EOS_Success;
    }

    template <int32_t N = MaxLengthExcludingNullTerminator + 1>
    static EOS_EResult CopyToExistingBuffer(const FString &InString, char ExistingPtr[N])
    {
        static_assert(
            N == MaxLengthExcludingNullTerminator + 1,
            "N must be set to MaxLengthExcludingNullTerminator + 1");
        static_assert(
            MaxLengthExcludingNullTerminator >= 0,
            "CopyToExistingBuffer can only be used when the string has a fixed size");
        return CopyToExistingBufferPtr(InString, &ExistingPtr[0]);
    }

    static void FreeFromCharBuffer(char *&CharPtr)
    {
        if (CharPtr != nullptr)
        {
            FMemory::Free((void *)CharPtr);
            CharPtr = nullptr;
        }
    }

    static void FreeFromCharBuffer(const char *&CharPtr)
    {
        if (CharPtr != nullptr)
        {
            FMemory::Free((void *)CharPtr);
            CharPtr = nullptr;
        }
    }

    static void FreeFromCharBufferConst(const char *const CharPtr)
    {
        if (CharPtr != nullptr)
        {
            FMemory::Free((void *)CharPtr);
        }
    }

    static void AllocateToCharList(const TArray<FString> &InIds, uint32_t &OutIdCount, const char **&OutIds)
    {
        if (InIds.Num() == 0)
        {
            OutIdCount = 0;
            OutIds = nullptr;
            return;
        }

        OutIdCount = InIds.Num();
        OutIds = (const char **)FMemory::MallocZeroed(OutIdCount * sizeof(const char *), 0);
        for (uint32_t i = 0; i < OutIdCount; i++)
        {
            if (AllocateToCharBuffer(InIds[i], OutIds[i]) != EOS_EResult::EOS_Success)
            {
                continue;
            }
        }
    }

    template <typename TObject>
    static void AllocateToCharListViaAccessor(
        const TArray<TObject> &InObjects,
        TFunction<FString(const TObject &)> Accessor,
        uint32_t &OutIdCount,
        const char **&OutIds)
    {
        TArray<FString> Ids;
        for (const auto &Object : InObjects)
        {
            Ids.Add(Accessor(Object));
        }
        return AllocateToCharList(Ids, OutIdCount, OutIds);
    }

    static void FreeFromCharList(uint32_t IdCount, const char **&Ids)
    {
        if (IdCount == 0 && Ids == nullptr)
        {
            return;
        }

        for (uint32_t i = 0; i < IdCount; i++)
        {
            FreeFromCharBuffer(Ids[i]);
        }
        FMemory::Free((void *)Ids);
        Ids = nullptr;
    }

    static void FreeFromCharListConst(uint32_t IdCount, const char **const Ids)
    {
        if (IdCount == 0 && Ids == nullptr)
        {
            return;
        }

        for (uint32_t i = 0; i < IdCount; i++)
        {
            FreeFromCharBuffer(Ids[i]);
        }
        FMemory::Free((void *)Ids);
    }

    /**
     * Calls a synchronous EOS operation that can return a string of unlimited length. This is only valid if
     * MaxLengthExcludingNullTerminator == EOS_UNLIMITED_MAX_LENGTH.
     */
    template <typename TInter, typename TOpts>
    static EOS_EResult FromDynamicLengthApiCall(
        TInter Handle,
        const TOpts *Opts,
        EOS_EResult(__CDECL_ATTR *Operation)(TInter, const TOpts *, char *, uint32_t *),
        FString &OutResult)
    {
        static_assert(
            MaxLengthExcludingNullTerminator == EOS_UNLIMITED_MAX_LENGTH,
            "FromDynamicLengthApiCall can only be used on unlimited length strings");

        uint32_t BufferSize = 2048;
        char *Buffer = (char *)FMemory::MallocZeroed(BufferSize + 1);
        EOS_EResult Result = Operation(Handle, Opts, Buffer, &BufferSize);
        if (Result == EOS_EResult::EOS_LimitExceeded)
        {
            // The buffer isn't big enough to hold the result. Resize, then run the operation again.
            FMemory::Free((void *)Buffer);
            Buffer = (char *)FMemory::MallocZeroed(BufferSize + 1);
            Result = Operation(Handle, Opts, Buffer, &BufferSize);
        }
        OutResult = FromAnsiString(Buffer);
        FMemory::Free(Buffer);
        return Result;
    }
};

template <int32_t MaxLengthExcludingNullTerminator> class EOSString_Utf8String
{
    static_assert(
        MaxLengthExcludingNullTerminator >= EOS_UNLIMITED_MAX_LENGTH,
        "MaxLengthExcludingNullTerminator must be non-negative");

public:
    struct Utf8String
    {
        FString StrTCHAR;
        EOS_EResult Result;
        FTCHARToUTF8 Ptr;

        const char *GetAsChar()
        {
            return (const char *)Ptr.Get();
        }

        Utf8String(const FString &InString)
            : StrTCHAR(InString)
            , Result(
                  (MaxLengthExcludingNullTerminator == EOS_UNLIMITED_MAX_LENGTH ||
                   this->StrTCHAR.Len() <= MaxLengthExcludingNullTerminator)
                      ? EOS_EResult::EOS_Success
                      : EOS_EResult::EOS_LimitExceeded)
            , Ptr(FTCHARToUTF8(*this->StrTCHAR))
        {
        }
    };

    static bool IsValid(const FString &InString)
    {
        return InString.Len() > 0 && (MaxLengthExcludingNullTerminator == EOS_UNLIMITED_MAX_LENGTH ||
                                      InString.Len() <= MaxLengthExcludingNullTerminator);
    }

    static FString FromUtf8String(const char *InPtr)
    {
        return FromUtf8String((const UTF8CHAR *)InPtr);
    }

    static FString FromUtf8String(const UTF8CHAR *InPtr)
    {
        return UTF8_TO_TCHAR(InPtr);
    }

    static Utf8String ToUtf8String(const FString &InString)
    {
        return Utf8String(InString);
    }

    static EOS_EResult AllocateToCharBuffer(const FString &InString, const char *&OutCharPtr)
    {
        if (OutCharPtr != nullptr)
        {
            return EOS_EResult::EOS_InvalidParameters;
        }

        auto StrBuf = FTCHARToUTF8(*InString);
        if (MaxLengthExcludingNullTerminator != EOS_UNLIMITED_MAX_LENGTH &&
            StrBuf.Length() > MaxLengthExcludingNullTerminator)
        {
            return EOS_EResult::EOS_LimitExceeded;
        }
        OutCharPtr = (char *)FMemory::MallocZeroed(StrBuf.Length() + 1);
        FMemory::Memcpy((void *)OutCharPtr, (const void *)StrBuf.Get(), StrBuf.Length());
        return EOS_EResult::EOS_Success;
    }

    static void FreeFromCharBuffer(const char *&CharPtr)
    {
        if (CharPtr != nullptr)
        {
            FMemory::Free((void *)CharPtr);
            CharPtr = nullptr;
        }
    }
};

#ifndef EOS_ATTRIBUTE_MAX_LENGTH
#define EOS_ATTRIBUTE_MAX_LENGTH 1000
#endif
#ifdef EOS_STRING_TESTS_ANSI_STRING_LENGTH_FOR_TESTS
#undef EOS_STRING_TESTS_ANSI_STRING_LENGTH_FOR_TESTS
#endif
#define EOS_STRING_TESTS_ANSI_STRING_LENGTH_FOR_TESTS 32
#ifdef EOS_STRING_TESTS_UTF8_STRING_LENGTH_FOR_TESTS
#undef EOS_STRING_TESTS_UTF8_STRING_LENGTH_FOR_TESTS
#endif
#define EOS_STRING_TESTS_UTF8_STRING_LENGTH_FOR_TESTS 32
#ifndef EOS_PLAYERDATASTORAGE_MD5_MAX_LENGTH
#define EOS_PLAYERDATASTORAGE_MD5_MAX_LENGTH 32
#endif
#ifndef EOS_TITLESTORAGE_TAG_MAX_LENGTH
#define EOS_TITLESTORAGE_TAG_MAX_LENGTH 15
#endif
#ifndef EOS_ANDROID_DIRECTORY_MAX_LENGTH
#define EOS_ANDROID_DIRECTORY_MAX_LENGTH 2048
#endif

extern REDPOINTEOSCORE_API char const EOS_ProductUserId_TypeName[];
extern REDPOINTEOSCORE_API char const EOS_EpicAccountId_TypeName[];

typedef EOSString_OpaqueId<
    EOS_ProductUserId_TypeName,
    EOS_ProductUserId,
    EOS_PRODUCTUSERID_MAX_LENGTH,
    EOS_ProductUserId_IsValid,
    EOS_ProductUserId_ToString,
    EOS_ProductUserId_FromString>
    EOSString_ProductUserId;
typedef EOSString_OpaqueId<
    EOS_EpicAccountId_TypeName,
    EOS_EpicAccountId,
    EOS_EPICACCOUNTID_MAX_LENGTH,
    EOS_EpicAccountId_IsValid,
    EOS_EpicAccountId_ToString,
    EOS_EpicAccountId_FromString>
    EOSString_EpicAccountId;

typedef EOSString_OpaqueToken<EOS_ContinuanceToken> EOSString_ContinuanceToken;

typedef EOSString_CharBasedId<EOS_LobbyId, EOS_UNLIMITED_MAX_LENGTH> EOSString_LobbyId;

typedef EOSString_AnsiString<EOS_UNLIMITED_MAX_LENGTH> EOSString_Achievements_AchievementId;
typedef EOSString_AnsiString<EOS_UNLIMITED_MAX_LENGTH> EOSString_AnsiUnlimited;
typedef EOSString_AnsiString<EOS_STRING_TESTS_ANSI_STRING_LENGTH_FOR_TESTS> EOSString_AnsiStringForTests;
typedef EOSString_AnsiString<EOS_CONNECT_EXTERNAL_ACCOUNT_ID_MAX_LENGTH> EOSString_Connect_ExternalAccountId;
typedef EOSString_AnsiString<EOS_LOBBY_INVITEID_MAX_LENGTH> EOSString_Lobby_InviteId;
typedef EOSString_AnsiString<EOS_LOBBYMODIFICATION_MAX_ATTRIBUTE_LENGTH> EOSString_LobbyModification_AttributeKey;
typedef EOSString_AnsiString<EOS_PLAYERDATASTORAGE_MD5_MAX_LENGTH> EOSString_PlayerDataStorage_MD5Hash;
typedef EOSString_AnsiString<EOS_PRESENCEMODIFICATION_JOININFO_MAX_LENGTH> EOSString_PresenceModification_JoinInfo;
typedef EOSString_AnsiString<EOS_SESSIONS_INVITEID_MAX_LENGTH> EOSString_Session_InviteId;
typedef EOSString_AnsiString<EOS_TITLESTORAGE_TAG_MAX_LENGTH> EOSString_TitleStorage_Tag;
typedef EOSString_AnsiString<EOS_ECOM_TRANSACTIONID_MAXIMUM_LENGTH> EOSString_Ecom_TransactionId;
typedef EOSString_AnsiString<EOS_UNLIMITED_MAX_LENGTH> EOSString_Ecom_CatalogOfferId;
typedef EOSString_AnsiString<EOS_UNLIMITED_MAX_LENGTH> EOSString_Ecom_CatalogItemId;
typedef EOSString_AnsiString<EOS_UNLIMITED_MAX_LENGTH> EOSString_Ecom_CatalogNamespace;
typedef EOSString_AnsiString<EOS_UNLIMITED_MAX_LENGTH> EOSString_Ecom_EntitlementName;
typedef EOSString_AnsiString<EOS_P2P_SOCKET_NAME_MAX_LENGTH> EOSString_P2P_SocketName;
typedef EOSString_AnsiString<EOS_UNLIMITED_MAX_LENGTH> EOSString_Stats_StatName;
typedef EOSString_AnsiString<EOS_UNLIMITED_MAX_LENGTH> EOSString_Leaderboards_LeaderboardId;
typedef EOSString_AnsiString<EOS_PRESENCE_DATA_MAX_KEY_LENGTH> EOSString_Presence_DataRecord_Key;
typedef EOSString_AnsiString<EOS_PRESENCE_DATA_MAX_VALUE_LENGTH> EOSString_Presence_DataRecord_Value;
typedef EOSString_AnsiString<EOS_LOBBYMODIFICATION_MAX_ATTRIBUTE_LENGTH> EOSString_SessionModification_AttributeKey;
typedef EOSString_AnsiString<EOS_ATTRIBUTE_MAX_LENGTH> EOSString_SessionModification_SessionName;
typedef EOSString_AnsiString<EOS_ATTRIBUTE_MAX_LENGTH> EOSString_SessionModification_HostAddress;
typedef EOSString_AnsiString<EOS_ATTRIBUTE_MAX_LENGTH> EOSString_SessionModification_BucketId;
typedef EOSString_AnsiString<EOS_SESSIONMODIFICATION_MAX_SESSIONIDOVERRIDE_LENGTH>
    EOSString_SessionModification_SessionId;
typedef EOSString_AnsiString<EOS_TITLESTORAGE_TAG_MAX_LENGTH> EOSString_TitleStorage_Tag;
typedef EOSString_AnsiString<EOS_ECOM_ENTITLEMENTID_MAX_LENGTH> EOSString_Ecom_EntitlementId;

typedef EOSString_Utf8String<EOS_STRING_TESTS_UTF8_STRING_LENGTH_FOR_TESTS> EOSString_Utf8StringForTests;
typedef EOSString_Utf8String<EOS_CONNECT_USERLOGININFO_DISPLAYNAME_MAX_LENGTH>
    EOSString_Connect_UserLoginInfo_DisplayName;
typedef EOSString_Utf8String<EOS_CONNECT_CREATEDEVICEID_DEVICEMODEL_MAX_LENGTH>
    EOSString_Connect_CreateDeviceId_DeviceModel;
typedef EOSString_Utf8String<EOS_ATTRIBUTE_MAX_LENGTH> EOSString_LobbyModification_AttributeStringValue;
typedef EOSString_Utf8String<EOS_PLAYERDATASTORAGE_FILENAME_MAX_LENGTH_BYTES> EOSString_PlayerDataStorage_Filename;
typedef EOSString_Utf8String<EOS_USERINFO_MAX_DISPLAYNAME_UTF8_LENGTH> EOSString_UserInfo_DisplayName;
typedef EOSString_Utf8String<EOS_ATTRIBUTE_MAX_LENGTH> EOSString_SessionModification_AttributeStringValue;
typedef EOSString_Utf8String<EOS_ATTRIBUTE_MAX_LENGTH> EOSString_QueryUserInfoByDisplayName_DisplayName;
typedef EOSString_Utf8String<EOS_PRESENCE_RICH_TEXT_MAX_VALUE_LENGTH> EOSString_Presence_RichTextValue;
typedef EOSString_Utf8String<EOS_ANDROID_DIRECTORY_MAX_LENGTH> EOSString_Android_InitializeOptions_Directory;
typedef EOSString_Utf8String<EOS_UNLIMITED_MAX_LENGTH> EOSString_AntiCheat_ActionReasonDetailsString;
typedef EOSString_Utf8String<EOS_UNLIMITED_MAX_LENGTH> EOSString_Utf8Unlimited;
typedef EOSString_Utf8String<EOS_TITLESTORAGE_FILENAME_MAX_LENGTH_BYTES> EOSString_TitleStorage_Filename;

#if defined(__CDECL_ATTR_DECLARED_IN_THIS_HEADER)
#undef __CDECL_ATTR
#undef __CDECL_ATTR_DECLARED_IN_THIS_HEADER
#endif