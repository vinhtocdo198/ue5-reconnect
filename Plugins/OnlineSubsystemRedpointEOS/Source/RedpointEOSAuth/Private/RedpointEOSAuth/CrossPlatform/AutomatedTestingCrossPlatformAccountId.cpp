// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/CrossPlatform/AutomatedTestingCrossPlatformAccountId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2611543896, Redpoint::EOS::Auth::CrossPlatform)
{

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

#if !defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING

FAutomatedTestingCrossPlatformAccountId::FAutomatedTestingCrossPlatformAccountId(
    const FString &InAutomatedTestingAccountId)
    : DataBytes(nullptr)
    , DataBytesSize(0)
    , AutomatedTestingAccountId(InAutomatedTestingAccountId)
{
    auto Str = StringCast<ANSICHAR>(*this->ToString());
    this->DataBytesSize = Str.Length() + 1;
    this->DataBytes = (uint8 *)FMemory::MallocZeroed(static_cast<SIZE_T>(this->DataBytesSize));
    FMemory::Memcpy(this->DataBytes, Str.Get(), static_cast<SIZE_T>(Str.Length()));
}

bool FAutomatedTestingCrossPlatformAccountId::Compare(const FCrossPlatformAccountId &Other) const
{
    if (Other.GetType() != GetType())
    {
        return false;
    }

    if (Other.GetType() == AUTOMATED_TESTING_ACCOUNT_ID)
    {
        const FAutomatedTestingCrossPlatformAccountId &OtherEOS =
            (const FAutomatedTestingCrossPlatformAccountId &)Other;
        return OtherEOS.AutomatedTestingAccountId == this->AutomatedTestingAccountId;
    }

    return (GetType() == Other.GetType() && GetSize() == Other.GetSize()) &&
           (FMemory::Memcmp(GetBytes(), Other.GetBytes(), static_cast<SIZE_T>(GetSize())) == 0);
}

FAutomatedTestingCrossPlatformAccountId::~FAutomatedTestingCrossPlatformAccountId()
{
    FMemory::Free(this->DataBytes);
}

FName FAutomatedTestingCrossPlatformAccountId::GetType() const
{
    return AUTOMATED_TESTING_ACCOUNT_ID;
}

const uint8 *FAutomatedTestingCrossPlatformAccountId::GetBytes() const
{
    return this->DataBytes;
}

int32 FAutomatedTestingCrossPlatformAccountId::GetSize() const
{
    return this->DataBytesSize;
}

bool FAutomatedTestingCrossPlatformAccountId::IsValid() const
{
    return true;
}

FString FAutomatedTestingCrossPlatformAccountId::ToString() const
{
    return this->AutomatedTestingAccountId;
}

TSharedPtr<const FCrossPlatformAccountId> FAutomatedTestingCrossPlatformAccountId::ParseFromString(const FString &In)
{
    return MakeShared<FAutomatedTestingCrossPlatformAccountId>(In);
}

#endif

#endif

}

REDPOINT_EOS_CODE_GUARD_END()