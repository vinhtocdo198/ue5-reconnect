// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/Ticker.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3569793395, Redpoint::EOS::Core::Utils)
{

typedef TDelegate<void()> FPeriodicTimerDelegate;

class REDPOINTEOSCORE_API FPeriodicTimer : public TSharedFromThis<FPeriodicTimer>
{
private:
    float FrequencySeconds;
    FTSTicker::FDelegateHandle UpcomingHandle;
    FPeriodicTimerDelegate Invocation;

    bool InternalHandler(float DeltaSeconds);

public:
    FPeriodicTimer(float InFrequencySeconds);
    UE_NONCOPYABLE(FPeriodicTimer);
    ~FPeriodicTimer();
    void Start(const FPeriodicTimerDelegate &InInvocation);
    void Stop();
};

}

namespace Redpoint::EOS::Core::Utils
{
REDPOINT_EOS_FILE_NS_EXPORT(3569793395, Redpoint::EOS::Core::Utils, FPeriodicTimerDelegate)
REDPOINT_EOS_FILE_NS_EXPORT(3569793395, Redpoint::EOS::Core::Utils, FPeriodicTimer)
}

REDPOINT_EOS_CODE_GUARD_END()