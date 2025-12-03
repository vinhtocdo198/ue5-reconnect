// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Error/Decl.h"
#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/RoomFeature.h"
#include "RedpointEOSRooms/Features/RoomFeatureRequest.h"
#include "RedpointEOSRooms/Providers/RoomProviderWriteOperation.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#define HANDLER_APPLY_FEATURE_REQUEST_SET_TO_WRITE_OPERATION(Name)                                                     \
private:                                                                                                               \
    FApplyFeatureRequestSetToWriteOperation Name##Value;                                                               \
                                                                                                                       \
public:                                                                                                                \
    FApplyFeatureRequestSetToWriteOperation Name() const                                                               \
    {                                                                                                                  \
        return FApplyFeatureRequestSetToWriteOperation::CreateLambda(                                                  \
            [CapturedCallback = this->Name##Value](                                                                    \
                const FRoomProviderWriteOperationRef &WriteOp,                                                         \
                const TMap<FName, TSharedRef<IRoomFeatureRequestBase>> &FeatureRequests,                               \
                const TSet<FName> &DeletedFeatureRequests) {                                                           \
                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);                                                             \
                                                                                                                       \
                if (CapturedCallback.IsBound())                                                                        \
                {                                                                                                      \
                    return CapturedCallback.Execute(WriteOp, FeatureRequests, DeletedFeatureRequests);                 \
                }                                                                                                      \
                return Redpoint::EOS::API::Errors::Success();                                                          \
            });                                                                                                        \
    }                                                                                                                  \
    template <typename FunctorType> FRoomProviderFeatureSetHandler &Name(FunctorType &&InFunctor)                      \
    {                                                                                                                  \
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);                                                                     \
                                                                                                                       \
        this->Name##Value = FApplyFeatureRequestSetToWriteOperation::CreateLambda(InFunctor);                          \
        return *this;                                                                                                  \
    }

#define HANDLER_APPLY_FEATURE_REQUEST_SET_TO_UPDATE_OPERATION(Name)                                                    \
private:                                                                                                               \
    FApplyFeatureRequestSetToUpdateOperation Name##Value;                                                              \
                                                                                                                       \
public:                                                                                                                \
    FApplyFeatureRequestSetToUpdateOperation Name() const                                                              \
    {                                                                                                                  \
        return FApplyFeatureRequestSetToUpdateOperation::CreateLambda(                                                 \
            [CapturedCallback = this->Name##Value](                                                                    \
                const FRoomProviderWriteOperationRef &WriteOp,                                                         \
                const TMap<FName, TSharedRef<IRoomFeatureRequestBase>> &FeatureRequests,                               \
                const TMap<FName, TSharedRef<IRoomFeature>> &ExistingFeatures,                                         \
                const TSet<FName> &DeletedFeatureRequests) {                                                           \
                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);                                                             \
                                                                                                                       \
                if (CapturedCallback.IsBound())                                                                        \
                {                                                                                                      \
                    return CapturedCallback                                                                            \
                        .Execute(WriteOp, FeatureRequests, ExistingFeatures, DeletedFeatureRequests);                  \
                }                                                                                                      \
                return Redpoint::EOS::API::Errors::Success();                                                          \
            });                                                                                                        \
    }                                                                                                                  \
    template <typename FunctorType> FRoomProviderFeatureSetHandler &Name(FunctorType &&InFunctor)                      \
    {                                                                                                                  \
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);                                                                     \
                                                                                                                       \
        this->Name##Value = FApplyFeatureRequestSetToUpdateOperation::CreateLambda(InFunctor);                         \
        return *this;                                                                                                  \
    }

    ;
namespace REDPOINT_EOS_FILE_NS_ID(148044540, Redpoint::EOS::Rooms::Providers)
{
using namespace ::Redpoint::EOS::Rooms::Providers;
using namespace ::Redpoint::EOS::Rooms::Features;
using namespace ::Redpoint::EOS::API;

class FRoomProviderFeatureSetHandler
{

public:
    typedef TDelegate<FError(
        const FRoomProviderWriteOperationRef &WriteOp,
        const TMap<FName, TSharedRef<IRoomFeatureRequestBase>> &FeatureRequests,
        const TSet<FName> &DeletedFeatureRequests)>
        FApplyFeatureRequestSetToWriteOperation;
    typedef TDelegate<FError(
        const FRoomProviderWriteOperationRef &WriteOp,
        const TMap<FName, TSharedRef<IRoomFeatureRequestBase>> &FeatureRequests,
        const TMap<FName, TSharedRef<IRoomFeature>> &ExistingFeatures,
        const TSet<FName> &DeletedFeatureRequests)>
        FApplyFeatureRequestSetToUpdateOperation;

public:
    FRoomProviderFeatureSetHandler() = default;
    UE_NONCOPYABLE(FRoomProviderFeatureSetHandler);
    virtual ~FRoomProviderFeatureSetHandler() = default;

    HANDLER_APPLY_FEATURE_REQUEST_SET_TO_WRITE_OPERATION(ApplyFeatureRequestSetOnCreateRoomOperation);
    HANDLER_APPLY_FEATURE_REQUEST_SET_TO_WRITE_OPERATION(ApplyFeatureRequestSetOnPostCreateRoomOperation);
    HANDLER_APPLY_FEATURE_REQUEST_SET_TO_UPDATE_OPERATION(ApplyFeatureRequestSetOnUpdateRoomOperation);
    HANDLER_APPLY_FEATURE_REQUEST_SET_TO_WRITE_OPERATION(ApplyFeatureRequestSetOnJoinRoomOperation);
    HANDLER_APPLY_FEATURE_REQUEST_SET_TO_WRITE_OPERATION(ApplyFeatureRequestSetOnPostJoinRoomOperation);
};

#undef HANDLER_APPLY_FEATURE_REQUEST_SET_TO_WRITE_OPERATION
#undef HANDLER_APPLY_FEATURE_REQUEST_SET_TO_UPDATE_OPERATION

}

namespace Redpoint::EOS::Rooms::Providers
{
REDPOINT_EOS_FILE_NS_EXPORT(148044540, Redpoint::EOS::Rooms::Providers, FRoomProviderFeatureSetHandler)
}

REDPOINT_EOS_CODE_GUARD_END()