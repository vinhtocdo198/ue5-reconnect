// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(368097829, Redpoint::EOS::Platform::Types)
{
using namespace ::Redpoint::EOS::API;

/**
 * Store arbitrary data per platform handle. This is used by runtime platform services that are instantiated globally
 * but need to store data per call context.
 */
template <typename T> class TPlatformStorage
{
private:
    class FInstance : public TSharedFromThis<FInstance>
    {
    private:
        TMap<FPlatformHandle, T> Storage;

    public:
        FInstance() = default;
        UE_NONCOPYABLE(FInstance);
        ~FInstance() = default;

        T &Get(const FPlatformHandle &PlatformHandle)
        {
            if (!this->Storage.Contains(PlatformHandle))
            {
                this->Storage.Add(PlatformHandle, T());
                PlatformHandle->OnShutdown().AddSP(this->AsShared(), &FInstance::OnPlatformShutdown, PlatformHandle);
            }
            return this->Storage[PlatformHandle];
        }

    private:
        void OnPlatformShutdown(FPlatformInstance &, FPlatformHandle PlatformHandle)
        {
            this->Storage.Remove(PlatformHandle);
        }
    };

    TSharedRef<FInstance> Instance;

public:
    TPlatformStorage()
        : Instance(MakeShared<FInstance>())
    {
    }
    TPlatformStorage(const TPlatformStorage &) = default;
    TPlatformStorage(TPlatformStorage &&) = default;
    ~TPlatformStorage() = default;

    T &Get(const FPlatformHandle &PlatformHandle)
    {
        return this->Instance->Get(PlatformHandle);
    }

    T &operator[](const FPlatformHandle &PlatformHandle)
    {
        return this->Instance->Get(PlatformHandle);
    }
};

}

namespace Redpoint::EOS::Platform::Types
{
REDPOINT_EOS_FILE_NS_EXPORT(368097829, Redpoint::EOS::Platform::Types, TPlatformStorage)
}

REDPOINT_EOS_CODE_GUARD_END()