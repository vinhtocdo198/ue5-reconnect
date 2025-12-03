// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatform/Services/NullRuntimePlatformRelationshipService.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1649350364, Redpoint::EOS::Platform::Services)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::UserCache;

FString FNullRuntimePlatformRelationshipService::GetCacheKey() const
{
    return TEXT("");
}

bool FNullRuntimePlatformRelationshipService::AllowRelationshipCaching() const
{
    return false;
}

void FNullRuntimePlatformRelationshipService::GetRelationships(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    ERelationshipType RelationshipTypes,
    const FOnGetRelationshipComplete &OnComplete)
{
    OnComplete.ExecuteIfBound(Errors::NotImplemented(), TMap<ERelationshipType, TArray<FExternalUserInfoRef>>());
}

IRuntimePlatformRelationshipService::FOnRelationshipsChanged &FNullRuntimePlatformRelationshipService::
    OnRelationshipsChanged(const FRuntimePlatformServiceCallContextRef &CallContext)
{
    return this->PlatformStorage[CallContext->PlatformHandle].OnRelationshipsChangedDelegate;
}

TSharedRef<IRuntimePlatformRelationshipService> NullRelationshipService =
    MakeShared<FNullRuntimePlatformRelationshipService>();

}

#endif

REDPOINT_EOS_CODE_GUARD_END()