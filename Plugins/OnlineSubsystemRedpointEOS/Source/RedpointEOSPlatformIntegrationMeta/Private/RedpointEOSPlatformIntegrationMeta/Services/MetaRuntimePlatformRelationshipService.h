// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED
#include "RedpointEOSPlatform/Services/RuntimePlatformRelationshipService.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(2563615876, Redpoint::EOS::Platform::Integration::Meta::Services)
{
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::Platform::Types;

class FMetaRuntimePlatformRelationshipService : public IRuntimePlatformRelationshipService,
                                                public TSharedFromThis<FMetaRuntimePlatformRelationshipService>
{
private:
    IRuntimePlatformRelationshipService::FOnRelationshipsChanged OnRelationshipsChangedDelegate;

public:
    FMetaRuntimePlatformRelationshipService() = default;
    UE_NONCOPYABLE(FMetaRuntimePlatformRelationshipService);
    virtual ~FMetaRuntimePlatformRelationshipService() override = default;

    virtual FString GetCacheKey() const override;
    virtual bool AllowRelationshipCaching() const override;
    virtual void GetRelationships(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        ERelationshipType RelationshipTypes,
        const FOnGetRelationshipComplete &OnComplete) override;
    virtual FOnRelationshipsChanged &OnRelationshipsChanged(
        const FRuntimePlatformServiceCallContextRef &CallContext) override;
};

}

namespace Redpoint::EOS::Platform::Integration::Meta::Services
{
REDPOINT_EOS_FILE_NS_EXPORT(
    2563615876,
    Redpoint::EOS::Platform::Integration::Meta::Services,
    FMetaRuntimePlatformRelationshipService)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()