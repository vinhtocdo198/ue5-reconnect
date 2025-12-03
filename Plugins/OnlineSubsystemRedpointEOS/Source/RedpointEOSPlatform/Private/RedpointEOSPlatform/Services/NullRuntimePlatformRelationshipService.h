// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformRelationshipService.h"
#include "RedpointEOSPlatform/Types/PlatformStorage.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1649350364, Redpoint::EOS::Platform::Services)
{
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::Platform::Types;

class FNullRuntimePlatformRelationshipService : public IRuntimePlatformRelationshipService
{
private:
    struct FPlatformStorage
    {
        FOnRelationshipsChanged OnRelationshipsChangedDelegate;
    };
    TPlatformStorage<FPlatformStorage> PlatformStorage;

public:
    FNullRuntimePlatformRelationshipService() = default;
    UE_NONCOPYABLE(FNullRuntimePlatformRelationshipService);
    virtual ~FNullRuntimePlatformRelationshipService() override = default;

    virtual FString GetCacheKey() const override;
    virtual bool AllowRelationshipCaching() const override;

    virtual void GetRelationships(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        ERelationshipType RelationshipTypes,
        const FOnGetRelationshipComplete &OnComplete) override;

    virtual FOnRelationshipsChanged &OnRelationshipsChanged(
        const FRuntimePlatformServiceCallContextRef &CallContext) override;
};

extern TSharedRef<IRuntimePlatformRelationshipService> NullRelationshipService;

}

namespace Redpoint::EOS::Platform::Services
{
REDPOINT_EOS_FILE_NS_EXPORT(1649350364, Redpoint::EOS::Platform::Services, FNullRuntimePlatformRelationshipService)
REDPOINT_EOS_FILE_NS_EXPORT(1649350364, Redpoint::EOS::Platform::Services, NullRelationshipService)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()