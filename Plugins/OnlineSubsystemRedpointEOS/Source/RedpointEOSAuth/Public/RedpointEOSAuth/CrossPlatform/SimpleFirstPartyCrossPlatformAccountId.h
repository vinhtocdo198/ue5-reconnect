// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/CrossPlatform/CrossPlatformAccountId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3109145890, Redpoint::EOS::Auth::CrossPlatform)
{
using namespace ::Redpoint::EOS::Auth::CrossPlatform;

class REDPOINTEOSAUTH_API FSimpleFirstPartyCrossPlatformAccountId : public FCrossPlatformAccountId
{
private:
    uint8 *DataBytes;
    int32 DataBytesSize;
    int64 FirstPartyAccountId;

public:
    FSimpleFirstPartyCrossPlatformAccountId(int64 InFirstPartyAccountId);
    virtual ~FSimpleFirstPartyCrossPlatformAccountId() override;
    UE_NONCOPYABLE(FSimpleFirstPartyCrossPlatformAccountId);

    virtual bool Compare(const FCrossPlatformAccountId &Other) const override;
    virtual FName GetType() const override;
    virtual const uint8 *GetBytes() const override;
    virtual int32 GetSize() const override;
    virtual bool IsValid() const override;
    virtual FString ToString() const override;

    static TSharedPtr<const FCrossPlatformAccountId> ParseFromString(const FString &In);

    int64 GetFirstPartyAccountId() const;
};

}

namespace Redpoint::EOS::Auth::CrossPlatform
{
REDPOINT_EOS_FILE_NS_EXPORT(3109145890, Redpoint::EOS::Auth::CrossPlatform, FSimpleFirstPartyCrossPlatformAccountId)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()