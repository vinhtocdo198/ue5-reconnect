// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/NativeHeapStateCleanup.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API
{

class REDPOINTEOSAPI_API FEventHandle
{
private:
    FPlatformHandle PlatformHandle;
    EOS_NotificationId NotificationId;
    INativeHeapStateCleanup *HeapState;
    TDelegate<void(FPlatformHandle, EOS_NotificationId)> UnregisterCallback;

public:
    FEventHandle(FPlatformHandle InPlatformHandle);
    FEventHandle(
        FPlatformHandle InPlatformHandle,
        EOS_NotificationId InNotificationId,
        INativeHeapStateCleanup *InHeapState,
        TDelegate<void(FPlatformHandle, EOS_NotificationId)> InUnregisterCallback);
    UE_NONCOPYABLE(FEventHandle);
    ~FEventHandle();
};

typedef TSharedRef<FEventHandle> FEventHandleRef;
typedef TSharedPtr<FEventHandle> FEventHandlePtr;

}

REDPOINT_EOS_CODE_GUARD_END()