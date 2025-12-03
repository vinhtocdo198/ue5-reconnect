// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Online/CoreOnline.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSPlatform/Services/ForwardDecls.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformServiceCallContext.h"
#include "RedpointEOSPlatform/Types/RelationshipType.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(4139651278, Redpoint::EOS::Platform::Services)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::Platform::Types;
using namespace ::Redpoint::EOS::UserCache;

class REDPOINTEOSPLATFORM_API IRuntimePlatformRelationshipService
{
public:
    IRuntimePlatformRelationshipService() = default;
    UE_NONCOPYABLE(IRuntimePlatformRelationshipService);
    virtual ~IRuntimePlatformRelationshipService() = default;

    /**
     * For an external system that wants to identify this relationship service for the purposes of caching the results
     * of GetRelationships, returns a string unique to this implementation (typically the name of the service).
     */
    virtual FString GetCacheKey() const = 0;

    /**
     * Returns whether this runtime platform integration permits relationships to be cached in Player Data Storage for
     * retrieval on other platforms.
     */
    virtual bool AllowRelationshipCaching() const = 0;

    /**
     * The delegate used when the GetRelationships operation completes.
     */
    typedef TDelegate<
        void(const FError &Error, const TMap<ERelationshipType, TArray<FExternalUserInfoRef>> &RelationshipsByType)>
        FOnGetRelationshipComplete;

    /**
     * Get the relationships (such as friends) between a local user and remote users. Multiple relationship types can be
     * requested.
     */
    virtual void GetRelationships(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        ERelationshipType RelationshipTypes,
        const FOnGetRelationshipComplete &OnComplete) = 0;

    /**
     * The delegate type used for the OnRelationshipsChanged event.
     */
    typedef TMulticastDelegate<void(
        const TMap<ERelationshipType, TArray<FExternalUserInfoRef>> &AddedRelationships,
        const TMap<ERelationshipType, TArray<FExternalUserInfoRef>> &RemovedRelationships)>
        FOnRelationshipsChanged;

    /**
     * This event is fired when a local user's relationships on the runtime platform change for an external reason (such
     * as a remote user accepting a friend request).
     */
    virtual FOnRelationshipsChanged &OnRelationshipsChanged(
        const FRuntimePlatformServiceCallContextRef &CallContext) = 0;
};

}

namespace Redpoint::EOS::Platform::Services
{
REDPOINT_EOS_FILE_NS_EXPORT(4139651278, Redpoint::EOS::Platform::Services, IRuntimePlatformRelationshipService)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()