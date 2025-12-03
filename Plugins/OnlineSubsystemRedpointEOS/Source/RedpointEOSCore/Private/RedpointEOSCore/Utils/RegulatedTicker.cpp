// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSCore/Utils/RegulatedTicker.h"

#include "HAL/IConsoleManager.h"
#include "ProfilingDebugging/CountersTrace.h"
#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(269165240, Redpoint::EOS::Core::Utils)
{

TRACE_DECLARE_INT_COUNTER_EXTERN(CTR_EOSRegulatedTicksInvocationsLastSecond);
TRACE_DECLARE_INT_COUNTER(
    CTR_EOSRegulatedTicksInvocationsLastSecond,
    TEXT("RedpointEOS/TickRegulation/InvocationsLastSecond"));

#if !(defined(UE_BUILD_SHIPPING) && UE_BUILD_SHIPPING)
static TAutoConsoleVariable<bool> CVarEOSTickRegulation(
    TEXT("t.EOSTickRegulation"),
    true,
    TEXT("When on (default), EOS only ticks a maximum number of times a second, regardless of FPS. When off, EOS ticks "
         "as fast as the game renders."));
#endif

bool FRegulatedTicker::Tick(float DeltaSeconds)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    this->CounterAccumulatedSeconds += DeltaSeconds;
    if (this->CounterAccumulatedSeconds > 1.0f)
    {
        this->InvocationCount = this->InvocationCountThisSecond;
        this->InvocationCountThisSecond = 0;
        this->CounterAccumulatedSeconds = 0.0f;
    }

    TRACE_COUNTER_SET(CTR_EOSRegulatedTicksInvocationsLastSecond, InvocationCount);

#if !(defined(UE_BUILD_SHIPPING) && UE_BUILD_SHIPPING)
    if (CVarEOSTickRegulation.GetValueOnGameThread())
    {
#endif
        this->AccumulatedDeltaSeconds += DeltaSeconds;
        if (this->AccumulatedDeltaSeconds > 1.0f / (float)this->TicksPerSecond)
        {
            // Enough time has elapsed to fire our delegates.
            float DeltaSecondsSinceLastRegulatedTick = this->AccumulatedDeltaSeconds;
            this->AccumulatedDeltaSeconds -= DeltaSeconds;
            if (this->AccumulatedDeltaSeconds > DeltaSeconds)
            {
                // Game stalled and enough time elapsed to fire multiple times.
                // However, we don't want to run delegates multiple times because
                // that might exacerbate lag issues, and we don't need to ensure
                // a minimum tick rate for EOS operations, only a maximum.
                this->AccumulatedDeltaSeconds = 0.0f;
            }
            TArray<FDelegateHandle> TickerHandles;
            this->RegisteredTickers.GetKeys(TickerHandles);
            for (const auto &Key : TickerHandles)
            {
                if (this->RegisteredTickers.Contains(Key))
                {
                    auto &Ticker = this->RegisteredTickers[Key];
                    if (Ticker.IsBound())
                    {
                        if (!Ticker.Execute(DeltaSecondsSinceLastRegulatedTick))
                        {
                            this->RegisteredTickers.Remove(Key);
                        }
                    }
                }
            }
            InvocationCountThisSecond += 1;
        }
#if !(defined(UE_BUILD_SHIPPING) && UE_BUILD_SHIPPING)
    }
    else
    {
        TArray<FDelegateHandle> TickerHandles;
        this->RegisteredTickers.GetKeys(TickerHandles);
        for (const auto &Key : TickerHandles)
        {
            if (this->RegisteredTickers.Contains(Key))
            {
                auto &Ticker = this->RegisteredTickers[Key];
                if (Ticker.IsBound())
                {
                    if (!Ticker.Execute(DeltaSeconds))
                    {
                        this->RegisteredTickers.Remove(Key);
                    }
                }
            }
        }
        InvocationCountThisSecond += 1;
    }
#endif
    return true;
}

FRegulatedTicker::FRegulatedTicker(int32 InTicksPerSecond)
    : RegisteredTickers()
    , GlobalHandle()
    , TicksPerSecond(InTicksPerSecond)
    , AccumulatedDeltaSeconds(0.0f)
    , CounterAccumulatedSeconds(0.0f)
    , InvocationCount(0)
    , InvocationCountThisSecond(0)
{
}

FRegulatedTicker::~FRegulatedTicker()
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    if (this->GlobalHandle.IsValid())
    {
        FTSTicker::GetCoreTicker().RemoveTicker(this->GlobalHandle);
        this->GlobalHandle.Reset();
    }
}

static TSharedPtr<FRegulatedTicker, ESPMode::ThreadSafe> GRedpointEOSCoreRegulatedTicker;
static FCriticalSection GRedpointEOSCoreRegulatedTickerLock;
FRegulatedTicker &FRegulatedTicker::GetCoreTicker()
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    // Optimistic check outside the lock.
    if (GRedpointEOSCoreRegulatedTicker.IsValid())
    {
        return *GRedpointEOSCoreRegulatedTicker.Get();
    }

    {
        // @note: Ensures we don't race on the initialisation of the platform pool.
        FScopeLock Lock(&GRedpointEOSCoreRegulatedTickerLock);
        if (GRedpointEOSCoreRegulatedTicker.IsValid())
        {
            return *GRedpointEOSCoreRegulatedTicker.Get();
        }
        GRedpointEOSCoreRegulatedTicker = MakeShared<FRegulatedTicker>();
        return *GRedpointEOSCoreRegulatedTicker.Get();
    }
}

FDelegateHandle FRegulatedTicker::AddTicker(const FTickerDelegate &InDelegate)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    if (!this->GlobalHandle.IsValid())
    {
        this->GlobalHandle =
            FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateSP(this, &FRegulatedTicker::Tick));
    }

    FDelegateHandle Handle(FDelegateHandle::EGenerateNewHandleType::GenerateNewHandle);
    this->RegisteredTickers.Add(Handle, InDelegate);
    return Handle;
}

void FRegulatedTicker::RemoveTicker(FDelegateHandle Handle)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    if (this->RegisteredTickers.Contains(Handle))
    {
        this->RegisteredTickers.Remove(Handle);

        if (this->RegisteredTickers.Num() == 0 && this->GlobalHandle.IsValid())
        {
            FTSTicker::GetCoreTicker().RemoveTicker(this->GlobalHandle);
            this->GlobalHandle.Reset();
        }
    }
}

}

REDPOINT_EOS_CODE_GUARD_END()