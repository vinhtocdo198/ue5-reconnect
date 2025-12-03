// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "ProfilingDebugging/CountersTrace.h"
#include "RedpointEOSAPI/Stats.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "Stats/Stats.h"

// @todo: The rest of this file should be moved out. We should no longer have a EOS_ENABLE_TRACE
// macro, since it's redundant as stats get compiled out in shipping anyway.

DECLARE_CYCLE_STAT_EXTERN(
    TEXT("RedpointEOS/IdentifiableCallbackDispatcher"),
    STAT_EOSIdentifiableCallbackDispatcher,
    STATGROUP_RedpointEOS,
    ONLINESUBSYSTEMREDPOINTEOS_API);

#if defined(EOS_ENABLE_TRACE)
DECLARE_CYCLE_STAT_EXTERN(
    TEXT("RedpointEOS/RunOperation/Invoke"),
    STAT_EOSOpInvoke,
    STATGROUP_RedpointEOS,
    ONLINESUBSYSTEMREDPOINTEOS_API);
DECLARE_CYCLE_STAT_EXTERN(
    TEXT("RedpointEOS/RunOperation/Callback"),
    STAT_EOSOpCallback,
    STATGROUP_RedpointEOS,
    ONLINESUBSYSTEMREDPOINTEOS_API);
DECLARE_CYCLE_STAT_EXTERN(
    TEXT("RedpointEOS/RunOperationKeepAlive/Invoke"),
    STAT_EOSOpKeepAliveInvoke,
    STATGROUP_RedpointEOS,
    ONLINESUBSYSTEMREDPOINTEOS_API);
DECLARE_CYCLE_STAT_EXTERN(
    TEXT("RedpointEOS/RunOperationKeepAlive/Callback"),
    STAT_EOSOpKeepAliveCallback,
    STATGROUP_RedpointEOS,
    ONLINESUBSYSTEMREDPOINTEOS_API);
DECLARE_CYCLE_STAT_EXTERN(
    TEXT("RedpointEOS/RegisterEvent/Register"),
    STAT_EOSEvRegister,
    STATGROUP_RedpointEOS,
    ONLINESUBSYSTEMREDPOINTEOS_API);
DECLARE_CYCLE_STAT_EXTERN(
    TEXT("RedpointEOS/RegisterEvent/Callback"),
    STAT_EOSEvCallback,
    STATGROUP_RedpointEOS,
    ONLINESUBSYSTEMREDPOINTEOS_API);
DECLARE_CYCLE_STAT_EXTERN(
    TEXT("RedpointEOS/RegisterEvent/Deregister"),
    STAT_EOSEvDeregister,
    STATGROUP_RedpointEOS,
    ONLINESUBSYSTEMREDPOINTEOS_API);

DECLARE_CYCLE_STAT_EXTERN(
    TEXT("RedpointEOS/OnlineSubsystem/Init"),
    STAT_EOSOnlineSubsystemInit,
    STATGROUP_RedpointEOS,
    ONLINESUBSYSTEMREDPOINTEOS_API);
DECLARE_CYCLE_STAT_EXTERN(
    TEXT("RedpointEOS/OnlineSubsystem/Tick"),
    STAT_EOSOnlineSubsystemTick,
    STATGROUP_RedpointEOS,
    ONLINESUBSYSTEMREDPOINTEOS_API);
DECLARE_CYCLE_STAT_EXTERN(
    TEXT("RedpointEOS/OnlineSubsystem/Shutdown"),
    STAT_EOSOnlineSubsystemShutdown,
    STATGROUP_RedpointEOS,
    ONLINESUBSYSTEMREDPOINTEOS_API);

DECLARE_DWORD_COUNTER_STAT_EXTERN(
    TEXT("RedpointEOS/RegulatedTicks/InvocationsLastSecond"),
    STAT_EOSRegulatedTicksInvocationsLastSecond,
    STATGROUP_RedpointEOS,
    ONLINESUBSYSTEMREDPOINTEOS_API);

// Deprecated trace usage macros.
#define EOS_SCOPE_CYCLE_COUNTER(StatName) SCOPE_CYCLE_COUNTER(StatName)
#define EOS_TRACE_COUNTER_SET(Ctr, CtrValue) TRACE_COUNTER_SET(Ctr, CtrValue)
#define EOS_TRACE_COUNTER_INCREMENT(Ctr) TRACE_COUNTER_INCREMENT(Ctr)
#define EOS_TRACE_COUNTER_ADD(Ctr, CtrValue) TRACE_COUNTER_ADD(Ctr, CtrValue)
#define EOS_INC_DWORD_STAT(StatName) INC_DWORD_STAT(StatName)
#define EOS_INC_DWORD_STAT_BY(StatName, StatValue) INC_DWORD_STAT_BY(StatName, StatValue)
#else
// Deprecated trace usage macros.
#define EOS_SCOPE_CYCLE_COUNTER(StatName)
#define EOS_TRACE_COUNTER_SET(Ctr, CtrValue)
#define EOS_TRACE_COUNTER_INCREMENT(Ctr)
#define EOS_TRACE_COUNTER_ADD(Ctr, CtrValue)
#define EOS_INC_DWORD_STAT(StatName)
#define EOS_INC_DWORD_STAT_BY(StatName, StatValue)
#endif