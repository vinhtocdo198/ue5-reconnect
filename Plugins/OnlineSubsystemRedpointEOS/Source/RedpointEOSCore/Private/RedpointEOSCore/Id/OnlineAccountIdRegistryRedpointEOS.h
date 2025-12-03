// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Online/CoreOnline.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/UniqueNetIdEOS.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2175684931, Redpoint::EOS::Core::Id)
{
using namespace UE::Online;

class IOnlineAccountIdRegistryRedpointEOS : public IOnlineAccountIdRegistry
{
public:
    static IOnlineAccountIdRegistryRedpointEOS &GetRegistered();

    virtual FAccountId Get(const FUniqueNetId &InUserId) = 0;
    virtual FAccountId Get(const TSharedRef<const FUniqueNetId> &InUserId) = 0;
    virtual FAccountId Get(const EOS_ProductUserId &InProductUserId) = 0;
    virtual TSharedPtr<const FUniqueNetIdEOS> Get(const FAccountId &InAccountId) const = 0;

    virtual void DumpRegistry() const = 0;
};

}

namespace Redpoint::EOS::Core::Id
{
REDPOINT_EOS_FILE_NS_EXPORT(2175684931, Redpoint::EOS::Core::Id, IOnlineAccountIdRegistryRedpointEOS)
}

REDPOINT_EOS_CODE_GUARD_END()