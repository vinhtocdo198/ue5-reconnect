// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/CrossPlatform/SimpleFirstPartyCrossPlatformAccountId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3109145890, Redpoint::EOS::Auth::CrossPlatform)
{

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

FSimpleFirstPartyCrossPlatformAccountId::FSimpleFirstPartyCrossPlatformAccountId(int64 InFirstPartyAccountId)
    : DataBytes(nullptr)
    , DataBytesSize(0)
    , FirstPartyAccountId(InFirstPartyAccountId)
{
    auto Str = StringCast<ANSICHAR>(*this->ToString());
    this->DataBytesSize = Str.Length() + 1;
    this->DataBytes = (uint8 *)FMemory::MallocZeroed(static_cast<SIZE_T>(this->DataBytesSize));
    FMemory::Memcpy(this->DataBytes, Str.Get(), static_cast<SIZE_T>(Str.Length()));
}

bool FSimpleFirstPartyCrossPlatformAccountId::Compare(const FCrossPlatformAccountId &Other) const
{
    if (Other.GetType() != GetType())
    {
        return false;
    }

    if (Other.GetType() == SIMPLE_FIRST_PARTY_ACCOUNT_ID)
    {
        const FSimpleFirstPartyCrossPlatformAccountId &OtherId = (const FSimpleFirstPartyCrossPlatformAccountId &)Other;
        return OtherId.GetFirstPartyAccountId() == this->GetFirstPartyAccountId();
    }

    return (GetType() == Other.GetType() && GetSize() == Other.GetSize()) &&
           (FMemory::Memcmp(GetBytes(), Other.GetBytes(), static_cast<SIZE_T>(GetSize())) == 0);
}

FSimpleFirstPartyCrossPlatformAccountId::~FSimpleFirstPartyCrossPlatformAccountId()
{
    FMemory::Free(this->DataBytes);
}

FName FSimpleFirstPartyCrossPlatformAccountId::GetType() const
{
    return SIMPLE_FIRST_PARTY_ACCOUNT_ID;
}

const uint8 *FSimpleFirstPartyCrossPlatformAccountId::GetBytes() const
{
    return this->DataBytes;
}

int32 FSimpleFirstPartyCrossPlatformAccountId::GetSize() const
{
    return this->DataBytesSize;
}

bool FSimpleFirstPartyCrossPlatformAccountId::IsValid() const
{
    return this->FirstPartyAccountId != 0;
}

FString FSimpleFirstPartyCrossPlatformAccountId::ToString() const
{
    return FString::Printf(TEXT("%lld"), this->FirstPartyAccountId);
}

int64 FSimpleFirstPartyCrossPlatformAccountId::GetFirstPartyAccountId() const
{
    return this->FirstPartyAccountId;
}

TSharedPtr<const FCrossPlatformAccountId> FSimpleFirstPartyCrossPlatformAccountId::ParseFromString(const FString &In)
{
    return MakeShared<FSimpleFirstPartyCrossPlatformAccountId>(FCString::Atoi64(*In));
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()