// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/EventHandle.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API
{

FEventHandle::FEventHandle(FPlatformHandle InPlatformHandle)
    : PlatformHandle(InPlatformHandle)
    , NotificationId()
    , HeapState(nullptr)
    , UnregisterCallback()
{
}

FEventHandle::FEventHandle(
    FPlatformHandle InPlatformHandle,
    EOS_NotificationId InNotificationId,
    INativeHeapStateCleanup *InHeapState,
    TDelegate<void(FPlatformHandle, EOS_NotificationId)> InUnregisterCallback)
    : PlatformHandle(InPlatformHandle)
    , NotificationId(InNotificationId)
    , HeapState(InHeapState)
    , UnregisterCallback(InUnregisterCallback)
{
}

FEventHandle::~FEventHandle()
{
    REDPOINT_EOS_USE_LLM_TAG(API);

    this->UnregisterCallback.ExecuteIfBound(this->PlatformHandle, this->NotificationId);
    if (this->HeapState != nullptr)
    {
        delete this->HeapState;
    }
}

}

REDPOINT_EOS_CODE_GUARD_END()