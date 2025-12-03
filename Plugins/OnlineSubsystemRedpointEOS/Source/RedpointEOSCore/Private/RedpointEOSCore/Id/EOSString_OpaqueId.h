// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Logging.h"
#include <functional>

// @note: This is considered legacy code, ported from OnlineSubsystemRedpointEOS so that FUniqueNetIdEOS can live in
// this module. All API calls outside FUniqueNetIdEOS use RedpointEOSAPI which handles the string marshalling in a more
// modern way.

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2619528734, Redpoint::EOS::Core::Id)
{
using namespace ::Redpoint::EOS::Core;

template <
    char const *TIdTypeName,
    typename TId,
    int32_t MaxLengthExcludingNullTerminator,
    EOS_Bool (*Func_IsValid)(TId InId),
    EOS_EResult (*Func_ToString)(TId InId, char *OutBuffer, int32_t *InOutBufferLength),
    TId (*Func_FromString)(const char *InString)>
class EOSString_OpaqueId
{
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

    static const TId None;

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
            UE_LOG(
                LogRedpointEOSCore,
                Error,
                TEXT("EOSString_OpaqueId::ToString failed when converting '%s', got result: %s"),
                ANSI_TO_TCHAR(TIdTypeName),
                ANSI_TO_TCHAR(EOS_EResult_ToString(Result)));
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
                LogRedpointEOSCore,
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

extern char const EOS_ProductUserId_TypeName[];
extern char const EOS_EpicAccountId_TypeName[];

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

}

namespace Redpoint::EOS::Core::Id
{
REDPOINT_EOS_FILE_NS_EXPORT(2619528734, Redpoint::EOS::Core::Id, EOSString_ProductUserId)
REDPOINT_EOS_FILE_NS_EXPORT(2619528734, Redpoint::EOS::Core::Id, EOSString_EpicAccountId)
}

REDPOINT_EOS_CODE_GUARD_END()