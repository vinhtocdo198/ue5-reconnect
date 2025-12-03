// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API
{

/**
 * In some rare circumstances, the EOS SDK incorrectly invokes a callback more than once even though
 * EOS_EResult_IsOperationComplete returns false and the result code is not EOS_Auth_PinGrantCode. When it does so, a
 * crash will occur because the heap state has been released after the first callback has been handled.
 *
 * These are not bugs in the Redpoint EOS plugin. These bugs should be reported to Epic Games via
 * https://eoshelp.epicgames.com/.
 */
class REDPOINTEOSAPI_API FCallbackValidator
{
private:
public:
    static void TrackCallback(void *HeapState, const FString &CallName);

    // For legacy EOSRunOperation calls which don't have the call name.
    static void TrackCallback(void *HeapState);

    [[nodiscard]] static bool VerifyCallback(void *HeapState, const TOptional<EOS_EResult> &ResultCode);

    [[nodiscard]] static bool VerifyCallbackWithoutRelease(void *HeapState, const TOptional<EOS_EResult> &ResultCode);

    static void TrackEvent(void *HeapState, const FString &EventName, EOS_NotificationId NotificationId);

    [[nodiscard]] static bool VerifyEvent(void *HeapState);

    static void UntrackEvent(EOS_NotificationId NotificationId);
};

}

REDPOINT_EOS_CODE_GUARD_END()