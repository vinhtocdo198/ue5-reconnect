// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Social/NonUnifiedUniqueNetId.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSAPI/Common/ExternalAccountType.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1487389753, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Social)
{
using namespace ::Redpoint::EOS::API::Common;

FNonUnifiedUniqueNetId::FNonUnifiedUniqueNetId(EOS_EExternalAccountType InAccountType, FString InAccountId)
    : AccountType(InAccountType)
    , AccountId(InAccountId)
{
    auto Str = StringCast<ANSICHAR>(*this->ToString());
    this->DataBytes = TArray<uint8>(reinterpret_cast<const uint8 *>(Str.Get()), Str.Length());
}

FName FNonUnifiedUniqueNetId::GetType() const
{
    return FName(TEXT("NonUnified"));
}

const uint8 *FNonUnifiedUniqueNetId::GetBytes() const
{
    return this->DataBytes.GetData();
}

int32 FNonUnifiedUniqueNetId::GetSize() const
{
    return this->DataBytes.Num();
}

bool FNonUnifiedUniqueNetId::IsValid() const
{
    return true;
}

FString FNonUnifiedUniqueNetId::ToString() const
{
    return FString::Printf(TEXT("%s:%s"), *ExternalAccountTypeMap.Get(this->GetAccountType()), *this->AccountId);
}

FString FNonUnifiedUniqueNetId::ToDebugString() const
{
    return this->ToString();
}

EOS_EExternalAccountType FNonUnifiedUniqueNetId::GetAccountType() const
{
    return this->AccountType;
}

FString FNonUnifiedUniqueNetId::GetAccountId() const
{
    return this->AccountId;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()