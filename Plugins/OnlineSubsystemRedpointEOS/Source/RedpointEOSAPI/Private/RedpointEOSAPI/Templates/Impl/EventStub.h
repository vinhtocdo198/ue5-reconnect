// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/EventStub.h"

#include "RedpointEOSAPI/Logging.h"
#include "RedpointEOSAPI/Stats.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#define REDPOINT_EOSSDK_API_CALL_EVENT_STUB_IMPL(CallName)                                                             \
    REDPOINT_EOS_DEFINE_API_CALL_LLM_TAG(CallName);                                                     \
                                                                                                                       \
    FEventHandleRef F##CallName::Register(                                                                             \
        FPlatformHandle InHandle,                                                                                      \
        const Options &InOptions,                                                                                      \
        const EventDelegate &InEventDelegate)                                                                          \
    {                                                                                                                  \
        checkf(IsInGameThread(), TEXT("Calls to the EOS SDK can only be made on the game thread!"));                   \
                                                                                                                       \
        DECLARE_CYCLE_STAT(StatCallRegisterName(), STAT_Call, STATGROUP_RedpointEOS);                                  \
        SCOPE_CYCLE_COUNTER(STAT_Call);                                                                                \
                                                                                                                       \
        UE_LOG(LogRedpointEOSAPI, VeryVerbose, TEXT("[not supported] %s"), LogCallName());                             \
                                                                                                                       \
        return MakeShared<FEventHandle>(InHandle);                                                                     \
    }

REDPOINT_EOS_CODE_GUARD_END()