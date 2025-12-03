// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "HAL/LowLevelMemTracker.h"

#if REDPOINT_EOS_UE_5_5_OR_LATER && ENABLE_LOW_LEVEL_MEM_TRACKER

#define REDPOINT_EOS_DECLARE_LLM_TAG(UniqueNameWithUnderscores)                                                        \
    extern REDPOINTEOSAPI_API FLLMTagDeclaration PREPROCESSOR_JOIN(                                                    \
        LLMTagDeclaration_RedpointEOS_,                                                                                \
        UniqueNameWithUnderscores);
#define REDPOINT_EOS_DEFINE_LLM_TAG(UniqueNameWithUnderscores, ...)                                                    \
    FLLMTagDeclaration PREPROCESSOR_JOIN(LLMTagDeclaration_RedpointEOS_, UniqueNameWithUnderscores)(                   \
        TEXT("RedpointEOS/") TEXT(#UniqueNameWithUnderscores),                                                         \
        ##__VA_ARGS__);
#define REDPOINT_EOS_USE_LLM_TAG(TagDeclName)                                                                          \
    FLLMScope SCOPE_NAME(                                                                                              \
        PREPROCESSOR_JOIN(LLMTagDeclaration_RedpointEOS_, TagDeclName).GetUniqueName(),                                \
        false /* bIsStatTag */,                                                                                        \
        ELLMTagSet::None,                                                                                              \
        ELLMTracker::Default);                                                                                         \
    UE_MEMSCOPE(PREPROCESSOR_JOIN(LLMTagDeclaration_RedpointEOS_, TagDeclName).GetUniqueName());

#define REDPOINT_EOS_DECLARE_API_CALL_LLM_TAG(CallNamespace, CallName)                                                 \
    static FORCEINLINE const TCHAR *LLMCallName()                                                                      \
    {                                                                                                                  \
        return TEXT("RedpointEOS/API/EOS_") TEXT(PREPROCESSOR_TO_STRING(CallNamespace)) TEXT("_")                      \
            TEXT(PREPROCESSOR_TO_STRING(CallName));                                                                    \
    }                                                                                                                  \
    static FLLMTagDeclaration LLMTagDeclaration_APICall;
#define REDPOINT_EOS_DEFINE_API_CALL_LLM_TAG(CallName, ...)                                                            \
    FLLMTagDeclaration F##CallName::LLMTagDeclaration_APICall(F##CallName::LLMCallName(), ##__VA_ARGS__);
#define REDPOINT_EOS_USE_API_CALL_LLM_TAG()                                                                            \
    FLLMScope SCOPE_NAME(                                                                                              \
        LLMTagDeclaration_APICall.GetUniqueName(),                                                                     \
        false /* bIsStatTag */,                                                                                        \
        ELLMTagSet::None,                                                                                              \
        ELLMTracker::Default);                                                                                         \
    UE_MEMSCOPE(LLMTagDeclaration_APICall.GetUniqueName());

#else

#define REDPOINT_EOS_DECLARE_LLM_TAG(UniqueNameWithUnderscores)
#define REDPOINT_EOS_DEFINE_LLM_TAG(UniqueNameWithUnderscores, ...)
#define REDPOINT_EOS_USE_LLM_TAG(TagDeclName)

#define REDPOINT_EOS_DECLARE_API_CALL_LLM_TAG(CallNamespace, CallName)
#define REDPOINT_EOS_DEFINE_API_CALL_LLM_TAG(CallName, ...)
#define REDPOINT_EOS_USE_API_CALL_LLM_TAG()

#endif

REDPOINT_EOS_DECLARE_LLM_TAG(AntiCheat)
REDPOINT_EOS_DECLARE_LLM_TAG(API)
REDPOINT_EOS_DECLARE_LLM_TAG(Async)
REDPOINT_EOS_DECLARE_LLM_TAG(Auth)
REDPOINT_EOS_DECLARE_LLM_TAG(Compat)
REDPOINT_EOS_DECLARE_LLM_TAG(Config)
REDPOINT_EOS_DECLARE_LLM_TAG(ConfigInspector)
REDPOINT_EOS_DECLARE_LLM_TAG(Core)
REDPOINT_EOS_DECLARE_LLM_TAG(Editor)
REDPOINT_EOS_DECLARE_LLM_TAG(EditorTests)
REDPOINT_EOS_DECLARE_LLM_TAG(Framework)
REDPOINT_EOS_DECLARE_LLM_TAG(Friends)
REDPOINT_EOS_DECLARE_LLM_TAG(Gauntlet)
REDPOINT_EOS_DECLARE_LLM_TAG(Identity)
REDPOINT_EOS_DECLARE_LLM_TAG(Interfaces)
REDPOINT_EOS_DECLARE_LLM_TAG(LicenseValidation)
REDPOINT_EOS_DECLARE_LLM_TAG(Networking)
REDPOINT_EOS_DECLARE_LLM_TAG(Orchestrator)
REDPOINT_EOS_DECLARE_LLM_TAG(Platform)
REDPOINT_EOS_DECLARE_LLM_TAG(Platform_Android)
REDPOINT_EOS_DECLARE_LLM_TAG(Platform_Desktop)
REDPOINT_EOS_DECLARE_LLM_TAG(PlatformIntegration_Apple)
REDPOINT_EOS_DECLARE_LLM_TAG(PlatformIntegration_Discord)
REDPOINT_EOS_DECLARE_LLM_TAG(PlatformIntegration_Epic)
REDPOINT_EOS_DECLARE_LLM_TAG(PlatformIntegration_GOG)
REDPOINT_EOS_DECLARE_LLM_TAG(PlatformIntegration_GooglePlay)
REDPOINT_EOS_DECLARE_LLM_TAG(PlatformIntegration_ItchIo)
REDPOINT_EOS_DECLARE_LLM_TAG(PlatformIntegration_Meta)
REDPOINT_EOS_DECLARE_LLM_TAG(PlatformIntegration_OSSv1)
REDPOINT_EOS_DECLARE_LLM_TAG(PlatformIntegration_Steam)
REDPOINT_EOS_DECLARE_LLM_TAG(Platform_IOS)
REDPOINT_EOS_DECLARE_LLM_TAG(Presence)
REDPOINT_EOS_DECLARE_LLM_TAG(RecentPlayers)
REDPOINT_EOS_DECLARE_LLM_TAG(Rooms)
REDPOINT_EOS_DECLARE_LLM_TAG(Rooms_Database)
REDPOINT_EOS_DECLARE_LLM_TAG(Rooms_Features)
REDPOINT_EOS_DECLARE_LLM_TAG(Rooms_Filters)
REDPOINT_EOS_DECLARE_LLM_TAG(Rooms_Hooks)
REDPOINT_EOS_DECLARE_LLM_TAG(Rooms_Providers)
REDPOINT_EOS_DECLARE_LLM_TAG(Rooms_Requests)
REDPOINT_EOS_DECLARE_LLM_TAG(Rooms_Synchronisation)
REDPOINT_EOS_DECLARE_LLM_TAG(Rooms_Utils)
REDPOINT_EOS_DECLARE_LLM_TAG(SDK)
REDPOINT_EOS_DECLARE_LLM_TAG(Shared)
REDPOINT_EOS_DECLARE_LLM_TAG(Storage)
REDPOINT_EOS_DECLARE_LLM_TAG(Tests)
REDPOINT_EOS_DECLARE_LLM_TAG(UserCache)
REDPOINT_EOS_DECLARE_LLM_TAG(Utils)
REDPOINT_EOS_DECLARE_LLM_TAG(VoiceChat)
