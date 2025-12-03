// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4133619841, Redpoint::EOS::Platform)
{

class REDPOINTEOSPLATFORM_API ILifecycleManager
{
public:
    ILifecycleManager() = default;
    UE_NONCOPYABLE(ILifecycleManager);
    virtual ~ILifecycleManager() = default;

    virtual void UpdateApplicationStatus(const EOS_EApplicationStatus &InNewStatus) = 0;
    virtual void UpdateNetworkStatus(const EOS_ENetworkStatus &InNewStatus) = 0;
};

}

namespace Redpoint::EOS::Platform
{
REDPOINT_EOS_FILE_NS_EXPORT(4133619841, Redpoint::EOS::Platform, ILifecycleManager)
}

REDPOINT_EOS_CODE_GUARD_END()