// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_IWYU_ALLOW_UNUSED_HEADERS_BEGIN()
// @note: This gets flagged if something else includes "RedpointEOSAPI/Stats.h" after "Stats/Stats.h", and
// that something else doesn't have IWYU tracking enabled.
#include "Stats/Stats.h"

#include "ProfilingDebugging/CountersTrace.h"
REDPOINT_EOS_IWYU_ALLOW_UNUSED_HEADERS_END()

// Stat group for all Redpoint EOS stats.
DECLARE_STATS_GROUP(TEXT("RedpointEOS"), STATGROUP_RedpointEOS, STATCAT_Advanced);

REDPOINT_EOS_CODE_GUARD_BEGIN()

// Trace declaration macros.
#define REDPOINT_EOS_DECLARE_CYCLE_STAT(CounterScope, CounterName, StatIdSuffix)                                       \
    DECLARE_CYCLE_STAT_EXTERN(                                                                                         \
        TEXT("RedpointEOS/") TEXT(PREPROCESSOR_TO_STRING(CounterScope)) TEXT("/") CounterName,                         \
        STAT_RedpointEOS_##CounterScope##_##StatIdSuffix,                                                              \
        STATGROUP_RedpointEOS, )
#define REDPOINT_EOS_DEFINE_STAT(CounterScope, StatIdSuffix)                                                           \
    DEFINE_STAT(STAT_RedpointEOS_##CounterScope##_##StatIdSuffix)
#define REDPOINT_EOS_DECLARE_CYCLE_STAT_INLINE(CounterScope, CounterName, StatIdSuffix)                                \
    REDPOINT_EOS_DECLARE_CYCLE_STAT(CounterScope, CounterName, StatIdSuffix);                                          \
    REDPOINT_EOS_DEFINE_STAT(CounterScope, StatIdSuffix)
#define REDPOINT_EOS_DECLARE_CYCLE_STAT_INLINE_WITH_CALLBACK(CounterScope, CounterName, StatIdSuffix)                  \
    REDPOINT_EOS_DECLARE_CYCLE_STAT_INLINE(CounterScope, CounterName, StatIdSuffix);                                   \
    REDPOINT_EOS_DECLARE_CYCLE_STAT_INLINE(CounterScope, CounterName TEXT("/Callback"), StatIdSuffix##_Callback)

#define REDPOINT_EOS_DECLARE_DWORD_COUNTER_STAT(CounterScope, CounterName, StatIdSuffix)                               \
    DECLARE_DWORD_COUNTER_STAT_EXTERN(                                                                                 \
        TEXT("RedpointEOS/") TEXT(PREPROCESSOR_TO_STRING(CounterScope)) TEXT("/") CounterName,                         \
        STAT_RedpointEOS_##CounterScope##_##StatIdSuffix,                                                              \
        STATGROUP_RedpointEOS, )

#define REDPOINT_EOS_DECLARE_INT_COUNTER(CounterScope, StatIdSuffix)                                                   \
    TRACE_DECLARE_INT_COUNTER_EXTERN(CTR_RedpointEOS_##CounterScope##_##StatIdSuffix)
#define REDPOINT_EOS_DEFINE_INT_COUNTER(CounterScope, CounterName, StatIdSuffix)                                       \
    TRACE_DECLARE_INT_COUNTER(                                                                                         \
        CTR_RedpointEOS_##CounterScope##_##StatIdSuffix,                                                               \
        TEXT("RedpointEOS/") TEXT(PREPROCESSOR_TO_STRING(CounterScope)) TEXT("/") CounterName)

// Trace usage macros.
#define REDPOINT_EOS_SCOPE_CYCLE_COUNTER(CounterScope, StatIdSuffix)                                                   \
    SCOPE_CYCLE_COUNTER(STAT_RedpointEOS_##CounterScope##_##StatIdSuffix)

#define REDPOINT_EOS_INT_COUNTER_SET(CounterScope, StatIdSuffix, CtrValue)                                             \
    TRACE_COUNTER_SET(CTR_RedpointEOS_##CounterScope##_##StatIdSuffix, CtrValue)
#define REDPOINT_EOS_INT_COUNTER_INCREMENT(CounterScope, StatIdSuffix)                                                 \
    TRACE_COUNTER_INCREMENT(CTR_RedpointEOS_##CounterScope##_##StatIdSuffix)
#define REDPOINT_EOS_INT_COUNTER_ADD(CounterScope, StatIdSuffix, CtrValue)                                             \
    TRACE_COUNTER_ADD(CTR_RedpointEOS_##CounterScope##_##StatIdSuffix, CtrValue)

#define REDPOINT_EOS_DWORD_COUNTER_STAT_INCREMENT(CounterScope, StatIdSuffix)                                          \
    INC_DWORD_STAT(STAT_RedpointEOS_##CounterScope##_##StatIdSuffix)
#define REDPOINT_EOS_DWORD_COUNTER_STAT_INCREMENT_BY(CounterScope, StatIdSuffix, StatValue)                            \
    INC_DWORD_STAT_BY(STAT_RedpointEOS_##CounterScope##_##StatIdSuffix, StatValue)

REDPOINT_EOS_CODE_GUARD_END()
