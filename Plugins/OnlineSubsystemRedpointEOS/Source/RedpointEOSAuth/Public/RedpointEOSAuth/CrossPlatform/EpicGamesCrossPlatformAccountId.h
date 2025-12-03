// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSAuth/CrossPlatform/CrossPlatformAccountId.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(177642964, Redpoint::EOS::Auth::CrossPlatform)
{
using namespace ::Redpoint::EOS::Auth::CrossPlatform;

class REDPOINTEOSAUTH_API FEpicGamesCrossPlatformAccountId : public FCrossPlatformAccountId
{
private:
    uint8 *DataBytes;
    int32 DataBytesSize;
    EOS_EpicAccountId EpicAccountId;

public:
    FEpicGamesCrossPlatformAccountId(EOS_EpicAccountId InEpicAccountId);
    virtual ~FEpicGamesCrossPlatformAccountId() override;
    UE_NONCOPYABLE(FEpicGamesCrossPlatformAccountId);

    virtual bool Compare(const FCrossPlatformAccountId &Other) const override;
    virtual FName GetType() const override;
    virtual const uint8 *GetBytes() const override;
    virtual int32 GetSize() const override;
    virtual bool IsValid() const override;
    virtual FString ToString() const override;

    static TSharedPtr<const FCrossPlatformAccountId> ParseFromString(const FString &In);

    bool HasValidEpicAccountId() const;
    EOS_EpicAccountId GetEpicAccountId() const;
    FString GetEpicAccountIdString() const;
};

}

namespace Redpoint::EOS::Auth::CrossPlatform
{
REDPOINT_EOS_FILE_NS_EXPORT(177642964, Redpoint::EOS::Auth::CrossPlatform, FEpicGamesCrossPlatformAccountId)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()