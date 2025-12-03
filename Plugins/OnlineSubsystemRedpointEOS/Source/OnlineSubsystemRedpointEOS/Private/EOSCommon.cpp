// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/Shared/EOSCommon.h"

#if WITH_EDITOR
#include "Modules/ModuleManager.h"
#include "OnlineSubsystemRedpointEOS/Public/OnlineSubsystemRedpointEOSModule.h"
#endif

EOS_ENABLE_STRICT_WARNINGS

DEFINE_LOG_CATEGORY(LogRedpointEOSStat);
DEFINE_LOG_CATEGORY(LogRedpointEOSFriends);
DEFINE_LOG_CATEGORY(LogRedpointEOSFileTransfer);
DEFINE_LOG_CATEGORY(LogRedpointEOSSessionListening);
#if defined(EOS_IS_FREE_EDITION)
DEFINE_LOG_CATEGORY(LogRedpointEOSLicenseValidation);
#endif

DEFINE_STAT(STAT_EOSIdentifiableCallbackDispatcher);

#if defined(EOS_ENABLE_TRACE)
DEFINE_STAT(STAT_EOSOpInvoke);
DEFINE_STAT(STAT_EOSOpCallback);
DEFINE_STAT(STAT_EOSOpKeepAliveInvoke);
DEFINE_STAT(STAT_EOSOpKeepAliveCallback);
DEFINE_STAT(STAT_EOSEvRegister);
DEFINE_STAT(STAT_EOSEvCallback);
DEFINE_STAT(STAT_EOSEvDeregister);

DEFINE_STAT(STAT_EOSOnlineSubsystemInit);
DEFINE_STAT(STAT_EOSOnlineSubsystemTick);
DEFINE_STAT(STAT_EOSOnlineSubsystemShutdown);
#endif

EOS_DISABLE_STRICT_WARNINGS
