// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/CrossPlatform/CrossPlatformAccountId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && (!defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING)

namespace REDPOINT_EOS_FILE_NS_ID(2611543896, Redpoint::EOS::Auth::CrossPlatform)
{
using namespace ::Redpoint::EOS::Auth::CrossPlatform;

class REDPOINTEOSAUTH_API FAutomatedTestingCrossPlatformAccountId : public FCrossPlatformAccountId
{
private:
    uint8 *DataBytes;
    int32 DataBytesSize;
    FString AutomatedTestingAccountId;

public:
    FAutomatedTestingCrossPlatformAccountId(const FString &InAutomatedTestingAccountId);
    virtual ~FAutomatedTestingCrossPlatformAccountId() override;
    UE_NONCOPYABLE(FAutomatedTestingCrossPlatformAccountId);

    virtual bool Compare(const FCrossPlatformAccountId &Other) const override;
    virtual FName GetType() const override;
    virtual const uint8 *GetBytes() const override;
    virtual int32 GetSize() const override;
    virtual bool IsValid() const override;
    virtual FString ToString() const override;

    static TSharedPtr<const FCrossPlatformAccountId> ParseFromString(const FString &In);
};

}

namespace Redpoint::EOS::Auth::CrossPlatform
{
REDPOINT_EOS_FILE_NS_EXPORT(2611543896, Redpoint::EOS::Auth::CrossPlatform, FAutomatedTestingCrossPlatformAccountId)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()