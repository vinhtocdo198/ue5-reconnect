// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/CrossPlatform/EpicGamesCrossPlatformAccountId.h"

#include "RedpointEOSAuth/Logging.h"
#include "RedpointEOSCore/Legacy/EOSString.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(177642964, Redpoint::EOS::Auth::CrossPlatform)
{
using namespace ::Redpoint::EOS::Auth;

FEpicGamesCrossPlatformAccountId::FEpicGamesCrossPlatformAccountId(EOS_EpicAccountId InEpicAccountId)
    : DataBytes(nullptr)
    , DataBytesSize(0)
    , EpicAccountId(InEpicAccountId)
{
    check(!EOSString_EpicAccountId::IsNone(this->EpicAccountId));

    auto Str = StringCast<ANSICHAR>(*this->ToString());
    this->DataBytesSize = Str.Length() + 1;
    this->DataBytes = (uint8 *)FMemory::MallocZeroed(static_cast<SIZE_T>(this->DataBytesSize));
    FMemory::Memcpy(this->DataBytes, Str.Get(), static_cast<SIZE_T>(Str.Length()));
}

bool FEpicGamesCrossPlatformAccountId::Compare(const FCrossPlatformAccountId &Other) const
{
    if (Other.GetType() != GetType())
    {
        return false;
    }

    if (Other.GetType() == EPIC_GAMES_ACCOUNT_ID)
    {
        const FEpicGamesCrossPlatformAccountId &OtherEOS = (const FEpicGamesCrossPlatformAccountId &)Other;
        if (EOSString_EpicAccountId::IsValid(OtherEOS.GetEpicAccountId()) &&
            EOSString_EpicAccountId::IsValid(this->GetEpicAccountId()))
        {
            return OtherEOS.GetEpicAccountIdString() == this->GetEpicAccountIdString();
        }
    }

    return (GetType() == Other.GetType() && GetSize() == Other.GetSize()) &&
           (FMemory::Memcmp(GetBytes(), Other.GetBytes(), static_cast<SIZE_T>(GetSize())) == 0);
}

FEpicGamesCrossPlatformAccountId::~FEpicGamesCrossPlatformAccountId()
{
    FMemory::Free(this->DataBytes);
}

FName FEpicGamesCrossPlatformAccountId::GetType() const
{
    return EPIC_GAMES_ACCOUNT_ID;
}

const uint8 *FEpicGamesCrossPlatformAccountId::GetBytes() const
{
    return this->DataBytes;
}

int32 FEpicGamesCrossPlatformAccountId::GetSize() const
{
    return this->DataBytesSize;
}

bool FEpicGamesCrossPlatformAccountId::IsValid() const
{
    return EOSString_EpicAccountId::IsValid(this->EpicAccountId);
}

FString FEpicGamesCrossPlatformAccountId::ToString() const
{
    return this->GetEpicAccountIdString();
}

TSharedPtr<const FCrossPlatformAccountId> FEpicGamesCrossPlatformAccountId::ParseFromString(const FString &In)
{
    EOS_EpicAccountId EpicAccountId = nullptr;
    if (EOSString_EpicAccountId::FromString(In, EpicAccountId) != EOS_EResult::EOS_Success)
    {
        UE_LOG(LogRedpointEOSAuth, Error, TEXT("Malformed Epic account ID component of unique net ID: %s"), *In);
        return nullptr;
    }
    return MakeShared<FEpicGamesCrossPlatformAccountId>(EpicAccountId);
}

bool FEpicGamesCrossPlatformAccountId::HasValidEpicAccountId() const
{
    return EOSString_EpicAccountId::IsValid(this->EpicAccountId);
}

EOS_EpicAccountId FEpicGamesCrossPlatformAccountId::GetEpicAccountId() const
{
    return this->EpicAccountId;
}

FString FEpicGamesCrossPlatformAccountId::GetEpicAccountIdString() const
{
    if (EOSString_EpicAccountId::IsNone(this->EpicAccountId))
    {
        return TEXT("");
    }

    FString Str;
    if (EOSString_EpicAccountId::ToString(this->EpicAccountId, Str) == EOS_EResult::EOS_Success)
    {
        return Str;
    }

    return TEXT("");
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()