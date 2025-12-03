// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "OnlineSubsystemTypes.h"
#include "RedpointEOSAPI/SDK.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1487389753, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Social)
{

class FNonUnifiedUniqueNetId : public FUniqueNetId
{
private:
    TArray<uint8> DataBytes;
    EOS_EExternalAccountType AccountType;
    FString AccountId;

public:
    FNonUnifiedUniqueNetId(EOS_EExternalAccountType InAccountType, FString InAccountId);
    FNonUnifiedUniqueNetId(const FNonUnifiedUniqueNetId &);
    virtual ~FNonUnifiedUniqueNetId() override = default;

    virtual FName GetType() const override;
    virtual const uint8 *GetBytes() const override;
    virtual int32 GetSize() const override;
    virtual bool IsValid() const override;
    virtual FString ToString() const override;
    virtual FString ToDebugString() const override;

    EOS_EExternalAccountType GetAccountType() const;
    FString GetAccountId() const;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Social
{
REDPOINT_EOS_FILE_NS_EXPORT(
    1487389753,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Social,
    FNonUnifiedUniqueNetId);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()