// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "OnlineSubsystemTypes.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSIdentity/IdentityUser.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1969834351, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Identity)
{
using namespace Redpoint::EOS::Identity;

class FUserOnlineAccountEOSImpl : public FUserOnlineAccount
{
private:
    FIdentityUserRef IdentityUser;

public:
    FUserOnlineAccountEOSImpl(const FIdentityUserRef &InIdentityUser);
    UE_NONCOPYABLE(FUserOnlineAccountEOSImpl);
    virtual ~FUserOnlineAccountEOSImpl() override = default;

    virtual FUniqueNetIdRef GetUserId() const override;
    virtual FString GetRealName() const override;
    virtual FString GetDisplayName(const FString &Platform = FString()) const override;
    virtual bool GetUserAttribute(const FString &AttrName, FString &OutAttrValue) const override;

    virtual FString GetAccessToken() const override;
    virtual bool GetAuthAttribute(const FString &AttrName, FString &OutAttrValue) const override;
    virtual bool SetUserAttribute(const FString &AttrName, const FString &AttrValue) override;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Identity
{
REDPOINT_EOS_FILE_NS_EXPORT(
    1969834351,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Identity,
    FUserOnlineAccountEOSImpl)
}

REDPOINT_EOS_CODE_GUARD_END()