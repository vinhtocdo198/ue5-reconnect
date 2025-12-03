// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/MultiResolve/GetAddressInfoTask.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !UE_BUILD_SHIPPING

namespace REDPOINT_EOS_FILE_NS_ID(3117334082, Redpoint::EOS::Networking::MultiIpResolve)
{

FGetAddressInfoTask::FGetAddressInfoTask(
    ISocketSubsystem *InSocketSubsystem,
    const FString &InQueryHost,
    const FString &InQueryService,
    EAddressInfoFlags InQueryFlags,
    const FName &InQueryProtocol,
    ESocketType InQuerySocketType,
    FAsyncGetAddressInfoCallback InCallbackFunction)
    : SocketSubsystem(InSocketSubsystem)
    , QueryHost(InQueryHost)
    , QueryService(InQueryService)
    , QueryFlags(InQueryFlags)
    , QueryProtocol(InQueryProtocol)
    , QuerySocketType(InQuerySocketType)
    , CallbackFunction(MoveTemp(InCallbackFunction))
{
}

void FGetAddressInfoTask::DoWork()
{
    this->CallbackFunction(this->SocketSubsystem->GetAddressInfo(
        *this->QueryHost,
        *this->QueryService,
        this->QueryFlags,
        this->QueryProtocol,
        this->QuerySocketType));
}

TStatId FGetAddressInfoTask::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(FGetAddressInfoTask, STATGROUP_ThreadPoolAsyncTasks);
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()