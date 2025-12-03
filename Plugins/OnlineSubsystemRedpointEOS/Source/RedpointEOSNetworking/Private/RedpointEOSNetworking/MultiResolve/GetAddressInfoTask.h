// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !UE_BUILD_SHIPPING
#include "Async/AsyncWork.h"
#include "SocketSubsystem.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !UE_BUILD_SHIPPING

namespace REDPOINT_EOS_FILE_NS_ID(3117334082, Redpoint::EOS::Networking::MultiIpResolve)
{

class FGetAddressInfoTask : public FNonAbandonableTask
{
private:
    ISocketSubsystem *SocketSubsystem;
    const FString QueryHost;
    const FString QueryService;
    EAddressInfoFlags QueryFlags;
    const FName QueryProtocol;
    ESocketType QuerySocketType;
    FAsyncGetAddressInfoCallback CallbackFunction;

public:
    friend FAutoDeleteAsyncTask<FGetAddressInfoTask>;

    FGetAddressInfoTask(
        ISocketSubsystem *InSocketSubsystem,
        const FString &InQueryHost,
        const FString &InQueryService,
        EAddressInfoFlags InQueryFlags,
        const FName &InQueryProtocol,
        ESocketType InQuerySocketType,
        FAsyncGetAddressInfoCallback InCallbackFunction);

    void DoWork();

    TStatId GetStatId() const;
};

}

namespace Redpoint::EOS::Networking::MultiIpResolve
{
REDPOINT_EOS_FILE_NS_EXPORT(3117334082, Redpoint::EOS::Networking::MultiIpResolve, FGetAddressInfoTask)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()
