// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSCore/Utils/PeriodicTimer.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3569793395, Redpoint::EOS::Core::Utils)
{

bool FPeriodicTimer::InternalHandler(float DeltaSeconds)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    if (this->Invocation.IsBound())
    {
        this->Invocation.Execute();
        this->UpcomingHandle = FTSTicker::GetCoreTicker().AddTicker(
            FTickerDelegate::CreateSP(this, &FPeriodicTimer::InternalHandler),
            FrequencySeconds);
    }
    return false;
}

FPeriodicTimer::FPeriodicTimer(float InFrequencySeconds)
    : FrequencySeconds(InFrequencySeconds)
    , UpcomingHandle()
    , Invocation()
{
}

FPeriodicTimer::~FPeriodicTimer()
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    this->Stop();
}

void FPeriodicTimer::Start(const FPeriodicTimerDelegate &InInvocation)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    checkf(
        !this->Invocation.IsBound() && !this->UpcomingHandle.IsValid(),
        TEXT("FPeriodicTimer::Start must not be called a second time before FPeriodicTimer::Stop."));
    this->Invocation = InInvocation;
    this->UpcomingHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateSP(this, &FPeriodicTimer::InternalHandler),
        FrequencySeconds);
}

void FPeriodicTimer::Stop()
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    if (this->UpcomingHandle.IsValid())
    {
        FTSTicker::GetCoreTicker().RemoveTicker(this->UpcomingHandle);
        this->UpcomingHandle.Reset();
    }
    this->Invocation.Unbind();
}

}

REDPOINT_EOS_CODE_GUARD_END()