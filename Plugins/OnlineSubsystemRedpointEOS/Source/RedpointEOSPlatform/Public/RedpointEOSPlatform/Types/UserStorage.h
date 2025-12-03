// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformServiceCallContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(256078695, Redpoint::EOS::Platform::Types)
{
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::API;

/**
 * Store arbitrary data per platform handle and per local user num. This is used by runtime platform services that are
 * instantiated globally but need to store data per call context.
 */
template <typename T> class TUserStorage
{
private:
    class FInstance : public TSharedFromThis<FInstance>
    {
    private:
        using Key = TTuple<FPlatformHandle, int32>;
        TMap<Key, T> Storage;

    public:
        FInstance() = default;
        UE_NONCOPYABLE(FInstance);
        ~FInstance() = default;

        T &Get(const FPlatformHandle &PlatformHandle, int32 LocalUserNum)
        {
            auto K = Key(PlatformHandle, LocalUserNum);
            if (!this->Storage.Contains(K))
            {
                this->Storage.Add(K, T());
                PlatformHandle->OnShutdown()
                    .AddSP(this->AsShared(), &FInstance::OnPlatformShutdown, PlatformHandle, LocalUserNum);
            }
            return this->Storage[K];
        }

    private:
        void OnPlatformShutdown(FPlatformInstance &, FPlatformHandle PlatformHandle, int32 LocalUserNum)
        {
            this->Storage.Remove(Key(PlatformHandle, LocalUserNum));
        }
    };

    TSharedRef<FInstance> Instance;

public:
    TUserStorage()
        : Instance(MakeShared<FInstance>())
    {
    }
    TUserStorage(const TUserStorage &) = default;
    TUserStorage(TUserStorage &&) = default;
    ~TUserStorage() = default;

    T &Get(const FPlatformHandle &PlatformHandle, int32 LocalUserNum)
    {
        return this->Instance->Get(PlatformHandle, LocalUserNum);
    }

    T &operator[](const FRuntimePlatformServiceCallContextRef &CallContext)
    {
        return this->Instance->Get(CallContext->PlatformHandle, CallContext->LocalUserNum);
    }
};

}

namespace Redpoint::EOS::Platform::Types
{
REDPOINT_EOS_FILE_NS_EXPORT(256078695, Redpoint::EOS::Platform::Types, TUserStorage)
}

REDPOINT_EOS_CODE_GUARD_END()