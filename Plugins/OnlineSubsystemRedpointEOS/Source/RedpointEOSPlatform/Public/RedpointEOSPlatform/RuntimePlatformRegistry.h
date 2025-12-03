// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSPlatform/RuntimePlatform.h"
#include "RedpointEOSPlatform/RuntimePlatformIntegration.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(3273657706, Redpoint::EOS::Core, FModule);

namespace REDPOINT_EOS_FILE_NS_ID(1189415427, Redpoint::EOS::Platform)
{
using namespace ::Redpoint::EOS::Platform;

class REDPOINTEOSPLATFORM_API FRuntimePlatformRegistry : public TSharedFromThis<FRuntimePlatformRegistry>
{
    friend Redpoint::EOS::Core::FModule;

private:
    TSharedPtr<IRuntimePlatform> RuntimePlatform;
    TMap<FName, TSharedPtr<IRuntimePlatformIntegration>> RuntimePlatformIntegrations;
    TArray<TSharedRef<IRuntimePlatformIntegration>> RuntimePlatformIntegrationsArray;

    bool IsRuntimePlatformValid() const;
    void UnloadRuntimePlatform();

public:
    TSharedRef<IRuntimePlatform> GetRuntimePlatform() const;
    void SetRuntimePlatform(const TSharedRef<IRuntimePlatform> &InRuntimePlatform);
    void AddRuntimePlatformIntegration(
        const FName &InName,
        const TSharedRef<IRuntimePlatformIntegration> &InRuntimePlatformIntegration);
    void RemoveRuntimePlatformIntegration(const FName &InName);
    const TArray<TSharedRef<IRuntimePlatformIntegration>> &GetRuntimePlatformIntegrations() const;
};

}

namespace Redpoint::EOS::Platform
{
REDPOINT_EOS_FILE_NS_EXPORT(1189415427, Redpoint::EOS::Platform, FRuntimePlatformRegistry)
}

REDPOINT_EOS_CODE_GUARD_END()