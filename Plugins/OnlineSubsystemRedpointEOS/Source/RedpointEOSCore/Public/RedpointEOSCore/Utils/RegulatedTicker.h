// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/Ticker.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(269165240, Redpoint::EOS::Core::Utils)
{

class REDPOINTEOSCORE_API FRegulatedTicker : public TSharedFromThis<FRegulatedTicker>
{
private:
    TMap<FDelegateHandle, FTickerDelegate> RegisteredTickers;
    FTSTicker::FDelegateHandle GlobalHandle;
    int32 TicksPerSecond;
    float AccumulatedDeltaSeconds;
    float CounterAccumulatedSeconds;
    int32 InvocationCount;
    int32 InvocationCountThisSecond;

    bool Tick(float DeltaSeconds);

public:
    FRegulatedTicker(int32 InTicksPerSecond = 60);
    UE_NONCOPYABLE(FRegulatedTicker);
    ~FRegulatedTicker();

    static FRegulatedTicker &GetCoreTicker();

    FDelegateHandle AddTicker(const FTickerDelegate &InDelegate);
    void RemoveTicker(FDelegateHandle Handle);
};

}

namespace Redpoint::EOS::Core::Utils
{
REDPOINT_EOS_FILE_NS_EXPORT(269165240, Redpoint::EOS::Core::Utils, FRegulatedTicker)
}

REDPOINT_EOS_CODE_GUARD_END()