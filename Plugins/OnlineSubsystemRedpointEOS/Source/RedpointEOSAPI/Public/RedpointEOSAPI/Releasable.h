// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAPI/ReleasableBase.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API
{

template <typename T> class TReleasable : public TSharedFromThis<TReleasable<T>>, public IReleasableBase
{
private:
    bool Valid;
    FPlatformHandle PlatformHandle;
    T ValueStorage;
    TDelegate<void(T Value)> ReleaseCallback;

    virtual FPlatformHandle GetPlatformHandle() const override
    {
        return PlatformHandle;
    }

public:
    TReleasable(const FPlatformHandle &InPlatformHandle)
        : Valid(false)
        , PlatformHandle(InPlatformHandle)
        , ValueStorage()
        , ReleaseCallback()
    {
    }
    TReleasable(const FPlatformHandle &InPlatformHandle, const T &InValue, TDelegate<void(T Value)> InReleaseCallback)
        : Valid(true)
        , PlatformHandle(InPlatformHandle)
        , ValueStorage(InValue)
        , ReleaseCallback(InReleaseCallback)
    {
    }
    TReleasable(const IReleasableBase &InReleasableBase, const T &InValue, TDelegate<void(T Value)> InReleaseCallback)
        : Valid(true)
        , PlatformHandle(GetPlatformHandleFromBase(InReleasableBase))
        , ValueStorage(InValue)
        , ReleaseCallback(InReleaseCallback)
    {
    }
    UE_NONCOPYABLE(TReleasable);
    virtual ~TReleasable() override
    {
        if (!this->PlatformHandle->IsShutdown() && this->Valid)
        {
            checkf(IsInGameThread(), TEXT("Calls to the EOS SDK can only be made on the game thread!"));

            this->ReleaseCallback.ExecuteIfBound(this->ValueStorage);
        }
    }

    bool IsValid() const
    {
        return !this->PlatformHandle->IsShutdown() && this->Valid;
    }
    const T &Value() const
    {
        checkf(this->Valid, TEXT("Value for TReleasable is not valid!"));
        return this->ValueStorage;
    }
};

template <typename T> using TReleasableRef = TSharedRef<TReleasable<T>>;

}

REDPOINT_EOS_CODE_GUARD_END()