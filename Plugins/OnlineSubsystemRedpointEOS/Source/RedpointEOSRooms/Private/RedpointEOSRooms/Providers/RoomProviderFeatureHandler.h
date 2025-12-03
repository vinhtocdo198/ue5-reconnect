// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Providers/RoomProviderFeatureHandlerBase.h"
#include "RedpointEOSRooms/Providers/RoomProviderWriteOperation.h"
#include "RedpointEOSRooms/Room.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(215967329, Redpoint::EOS::Rooms::Providers)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Providers;
using namespace ::Redpoint::EOS::Rooms::Features;

#define HANDLER_SIMPLE_FLAG(Name)                                                                                      \
private:                                                                                                               \
    bool b##Name##Value;                                                                                               \
                                                                                                                       \
public:                                                                                                                \
    FRoomProviderFeatureHandler &Name(bool bValue)                                                                     \
    {                                                                                                                  \
        this->b##Name##Value = bValue;                                                                                 \
        return *this;                                                                                                  \
    }                                                                                                                  \
    virtual bool Name() const override                                                                                 \
    {                                                                                                                  \
        return this->b##Name##Value;                                                                                   \
    }

#define HANDLER_APPLY_FEATURE_REQUEST_TO_WRITE_OPERATION(Name)                                                         \
private:                                                                                                               \
    FApplyFeatureRequestToWriteOperation Name##Value;                                                                  \
                                                                                                                       \
public:                                                                                                                \
    virtual FApplyFeatureRequestToWriteOperationBase Name() const override                                             \
    {                                                                                                                  \
        return FApplyFeatureRequestToWriteOperationBase::CreateLambda(                                                 \
            [CapturedCallback = this->Name##Value](                                                                    \
                const FRoomProviderWriteOperationRef &WriteOp,                                                         \
                const TSharedRef<IRoomFeatureRequestBase> &FeatureRequest) {                                           \
                if (CapturedCallback.IsBound())                                                                        \
                {                                                                                                      \
                    return CapturedCallback.Execute(                                                                   \
                        WriteOp,                                                                                       \
                        StaticCastSharedRef<TRoomFeatureRequest>(FeatureRequest));                                     \
                }                                                                                                      \
                return Redpoint::EOS::API::Errors::Success();                                                          \
            });                                                                                                        \
    }                                                                                                                  \
    template <typename FunctorType> FRoomProviderFeatureHandler &Name(FunctorType &&InFunctor)                         \
    {                                                                                                                  \
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);                                                                     \
                                                                                                                       \
        this->Name##Value = FApplyFeatureRequestToWriteOperation::CreateLambda(InFunctor);                             \
        return *this;                                                                                                  \
    }

#define HANDLER_APPLY_OMITTED_FEATURE_REQUEST_TO_WRITE_OPERATION(Name)                                                 \
private:                                                                                                               \
    FApplyOmittedFeatureRequestToWriteOperationBase Name##Value;                                                       \
                                                                                                                       \
public:                                                                                                                \
    virtual FApplyOmittedFeatureRequestToWriteOperationBase Name() const override                                      \
    {                                                                                                                  \
        return FApplyOmittedFeatureRequestToWriteOperationBase::CreateLambda(                                          \
            [CapturedCallback = this->Name##Value](const FRoomProviderWriteOperationRef &WriteOp) {                    \
                if (CapturedCallback.IsBound())                                                                        \
                {                                                                                                      \
                    return CapturedCallback.Execute(WriteOp);                                                          \
                }                                                                                                      \
                return Redpoint::EOS::API::Errors::Success();                                                          \
            });                                                                                                        \
    }                                                                                                                  \
    template <typename FunctorType> FRoomProviderFeatureHandler &Name(FunctorType &&InFunctor)                         \
    {                                                                                                                  \
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);                                                                     \
                                                                                                                       \
        this->Name##Value = FApplyOmittedFeatureRequestToWriteOperationBase::CreateLambda(InFunctor);                  \
        return *this;                                                                                                  \
    }

#define HANDLER_APPLY_FEATURE_REQUEST_TO_UPDATE_OPERATION(Name)                                                        \
private:                                                                                                               \
    FApplyFeatureRequestToUpdateOperation Name##Value;                                                                 \
                                                                                                                       \
public:                                                                                                                \
    virtual FApplyFeatureRequestToUpdateOperationBase Name() const override                                            \
    {                                                                                                                  \
        return FApplyFeatureRequestToUpdateOperationBase::CreateLambda(                                                \
            [CapturedCallback = this->Name##Value](                                                                    \
                const FRoomProviderWriteOperationRef &WriteOp,                                                         \
                const TSharedPtr<IRoomFeatureRequestBase> &FeatureRequestOrNullIfDeleted,                              \
                const TSharedPtr<IRoomFeature> &ExistingFeatureOrNullIfNotPresent) {                                   \
                if (CapturedCallback.IsBound())                                                                        \
                {                                                                                                      \
                    return CapturedCallback.Execute(                                                                   \
                        WriteOp,                                                                                       \
                        StaticCastSharedPtr<TRoomFeatureRequest>(FeatureRequestOrNullIfDeleted),                       \
                        StaticCastSharedPtr<TRoomFeatureImpl>(ExistingFeatureOrNullIfNotPresent));                     \
                }                                                                                                      \
                return Redpoint::EOS::API::Errors::Success();                                                          \
            });                                                                                                        \
    }                                                                                                                  \
    template <typename FunctorType> FRoomProviderFeatureHandler &Name(FunctorType &&InFunctor)                         \
    {                                                                                                                  \
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);                                                                     \
                                                                                                                       \
        this->Name##Value = FApplyFeatureRequestToUpdateOperation::CreateLambda(InFunctor);                            \
        return *this;                                                                                                  \
    }

#define HANDLER_SERIALIZE_FEATURE_REQUEST_TO_ROOM_ATTRIBUTES(Name)                                                     \
private:                                                                                                               \
    FSerializeFeatureRequestToRoomAttributes Name##Value;                                                              \
                                                                                                                       \
public:                                                                                                                \
    virtual FSerializeFeatureRequestToRoomAttributesBase Name() const override                                         \
    {                                                                                                                  \
        return FSerializeFeatureRequestToRoomAttributesBase::CreateLambda(                                             \
            [CapturedCallback = this->Name##Value](                                                                    \
                const FRoomProviderWriteOperationRef &WriteOp,                                                         \
                const TSharedRef<IRoomFeatureRequestBase> &FeatureRequest) {                                           \
                if (CapturedCallback.IsBound())                                                                        \
                {                                                                                                      \
                    return CapturedCallback.Execute(                                                                   \
                        WriteOp,                                                                                       \
                        StaticCastSharedRef<TRoomFeatureRequest>(FeatureRequest));                                     \
                }                                                                                                      \
                return TMap<FString, FRoomAttribute>();                                                                \
            });                                                                                                        \
    }                                                                                                                  \
    template <typename FunctorType> FRoomProviderFeatureHandler &Name(FunctorType &&InFunctor)                         \
    {                                                                                                                  \
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);                                                                     \
                                                                                                                       \
        this->Name##Value = FSerializeFeatureRequestToRoomAttributes::CreateLambda(InFunctor);                         \
        return *this;                                                                                                  \
    }

template <
    typename TRoomFeature,
    typename TRoomFeatureImpl,
    typename TRoomFeatureRequest,
    typename TRoomFeatureSearchState>
class FRoomProviderFeatureHandler : public FRoomProviderFeatureHandlerBase
{
    // @todo: static assert on template parameters

public:
    typedef TDelegate<TMap<FString, FRoomAttribute>(
        const FRoomProviderWriteOperationRef &WriteOp,
        const TSharedRef<TRoomFeatureRequest> &FeatureRequest)>
        FSerializeFeatureRequestToRoomAttributes;

    typedef TDelegate<TSharedPtr<TRoomFeatureImpl>(
        const FRoomProviderReadOperationRef &ReadOp,
        const TSharedRef<FRoomProvider> &RoomProvider,
        const FRoomRef &Room)>
        FConstructFeatureImplementation;
    typedef TDelegate<void(
        const FRoomProviderReadOperationRef &ReadOp,
        const TSharedRef<FRoomProvider> &RoomProvider,
        const FRoomRef &Room,
        const TSharedRef<TRoomFeatureImpl> &Feature)>
        FUpdateFeatureImplementationDuringPreSynchronisation;
    typedef TDelegate<FFeaturePostSynchronisationResult(
        const FRoomProviderReadOperationRef &ReadOp,
        const TSharedRef<FRoomProvider> &RoomProvider,
        const FRoomRef &Room,
        const TSharedRef<TRoomFeatureImpl> &Feature,
        const TMap<FString, FRoomAttribute> &FeatureAttributes,
        const TMap<FRoomUserId, TMap<FString, FRoomAttribute>> &FeatureMemberAttributes)>
        FUpdateFeatureImplementationDuringPostSynchronisation;

    typedef TDelegate<TSharedPtr<TRoomFeatureSearchState>(
        const FRoomProviderReadOperationRef &ReadOp,
        const TSharedRef<FRoomProvider> &RoomProvider,
        const TMap<FString, FRoomAttribute> &FeatureAttributes)>
        FConstructFeatureSearchState;

    typedef TDelegate<
        FError(const FRoomProviderWriteOperationRef &WriteOp, const TSharedRef<TRoomFeatureRequest> &FeatureRequest)>
        FApplyFeatureRequestToWriteOperation;
    typedef TDelegate<FError(
        const FRoomProviderWriteOperationRef &WriteOp,
        const TSharedPtr<TRoomFeatureRequest> &FeatureRequestOrNullIfDeleted,
        const TSharedPtr<TRoomFeatureImpl> &ExistingFeatureOrNullIfNotPresent)>
        FApplyFeatureRequestToUpdateOperation;

    typedef TDelegate<bool(
        const FRoomProviderReadOperationRef &ReadOp,
        const TSharedRef<FRoomProvider> &RoomProvider,
        const FRoomRef &Room,
        const TSharedRef<TRoomFeatureImpl> &Feature,
        FRoomUserId RoomMemberId,
        const TMap<FString, FRoomAttribute> &FeatureMemberAttributes)>
        FShouldKickReadyRoomMemberAfterJoin;

    typedef TDelegate<FExclusivityKey(
        const TSharedRef<FRoomProvider> &RoomProvider,
        const TSharedPtr<IRoom> &RoomOnUpdate,
        const TSharedRef<TRoomFeatureRequest> &FeatureRequest)>
        FGetRoomExclusivityKeyFromFeatureRequest;
    typedef TDelegate<FExclusivityKey(
        const TSharedRef<FRoomProvider> &RoomProvider,
        const TSharedRef<IRoom> &Room,
        const TSharedRef<TRoomFeatureImpl> &Feature)>
        FGetRoomExclusivityKeyFromFeature;
    typedef TDelegate<FExclusivityKey(
        const TSharedRef<FRoomProvider> &RoomProvider,
        const TSharedRef<TRoomFeatureSearchState> &Feature)>
        FGetRoomExclusivityKeyFromFeatureSearchState;

    FRoomProviderFeatureHandler() = default;
    UE_NONCOPYABLE(FRoomProviderFeatureHandler);
    virtual ~FRoomProviderFeatureHandler() override = default;

    virtual FName GetFeatureName() const override
    {
        return TRoomFeature::GetFeatureName();
    }

    HANDLER_SIMPLE_FLAG(AllowOnCreate);
    HANDLER_SIMPLE_FLAG(AllowAddOnUpdate);
    HANDLER_SIMPLE_FLAG(AllowModifyOnUpdate);
    HANDLER_SIMPLE_FLAG(AllowDeleteOnUpdate);
    HANDLER_SIMPLE_FLAG(AllowOnJoin);

    HANDLER_SERIALIZE_FEATURE_REQUEST_TO_ROOM_ATTRIBUTES(SerializeFeatureRequestToRoomAttributes);
    HANDLER_SERIALIZE_FEATURE_REQUEST_TO_ROOM_ATTRIBUTES(SerializeFeatureRequestToRoomMemberAttributes);

private:
    FConstructFeatureImplementation ConstructFeatureImplementationValue;

public:
    virtual FConstructFeatureImplementationBase ConstructFeatureImplementation() const override
    {
        return FConstructFeatureImplementationBase::CreateLambda(
            [CapturedCallback = this->ConstructFeatureImplementationValue](
                const FRoomProviderReadOperationRef &ReadOp,
                const TSharedRef<FRoomProvider> &RoomProvider,
                const TSharedRef<IRoom> &Room) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                if (CapturedCallback.IsBound())
                {
                    return StaticCastSharedPtr<IRoomFeature>(CapturedCallback.Execute(ReadOp, RoomProvider, Room));
                }
                return TSharedPtr<IRoomFeature>();
            });
    }
    template <typename FunctorType> FRoomProviderFeatureHandler &ConstructFeatureImplementation(FunctorType &&InFunctor)
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

        this->ConstructFeatureImplementationValue = FConstructFeatureImplementation::CreateLambda(InFunctor);
        return *this;
    }

private:
    FUpdateFeatureImplementationDuringPreSynchronisation UpdateFeatureImplementationDuringPreSynchronisationValue;

public:
    virtual FUpdateFeatureImplementationDuringPreSynchronisationBase
    UpdateFeatureImplementationDuringPreSynchronisation() const override
    {
        return FUpdateFeatureImplementationDuringPreSynchronisationBase::CreateLambda(
            [CapturedCallback = this->UpdateFeatureImplementationDuringPreSynchronisationValue](
                const FRoomProviderReadOperationRef &ReadOp,
                const TSharedRef<FRoomProvider> &RoomProvider,
                const TSharedRef<IRoom> &Room,
                const TSharedRef<IRoomFeature> &Feature) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                if (CapturedCallback.IsBound())
                {
                    CapturedCallback
                        .Execute(ReadOp, RoomProvider, Room, StaticCastSharedRef<TRoomFeatureImpl>(Feature));
                }
            });
    }
    template <typename FunctorType>
    FRoomProviderFeatureHandler &UpdateFeatureImplementationDuringPreSynchronisation(FunctorType &&InFunctor)
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

        this->UpdateFeatureImplementationDuringPreSynchronisationValue =
            FUpdateFeatureImplementationDuringPreSynchronisation::CreateLambda(InFunctor);
        return *this;
    }

private:
    FUpdateFeatureImplementationDuringPostSynchronisation UpdateFeatureImplementationDuringPostSynchronisationValue;

public:
    virtual FUpdateFeatureImplementationDuringPostSynchronisationBase
    UpdateFeatureImplementationDuringPostSynchronisation() const override
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

        return FUpdateFeatureImplementationDuringPostSynchronisationBase::CreateLambda(
            [CapturedCallback = this->UpdateFeatureImplementationDuringPostSynchronisationValue](
                const FRoomProviderReadOperationRef &ReadOp,
                const TSharedRef<FRoomProvider> &RoomProvider,
                const TSharedRef<IRoom> &Room,
                const TSharedRef<IRoomFeature> &Feature,
                const TMap<FString, FRoomAttribute> &FeatureAttributes,
                const TMap<FRoomUserId, TMap<FString, FRoomAttribute>> &FeatureMemberAttributes) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                if (CapturedCallback.IsBound())
                {
                    return CapturedCallback.Execute(
                        ReadOp,
                        RoomProvider,
                        Room,
                        StaticCastSharedRef<TRoomFeatureImpl>(Feature),
                        FeatureAttributes,
                        FeatureMemberAttributes);
                }
                return FFeaturePostSynchronisationResult(
                    FFeaturePostSynchronisationResult::ERequireRoomUpdate::No,
                    FFeaturePostSynchronisationResult::EStateUpdated::No);
            });
    }
    template <typename FunctorType>
    FRoomProviderFeatureHandler &UpdateFeatureImplementationDuringPostSynchronisation(FunctorType &&InFunctor)
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

        this->UpdateFeatureImplementationDuringPostSynchronisationValue =
            FUpdateFeatureImplementationDuringPostSynchronisation::CreateLambda(InFunctor);
        return *this;
    }

private:
    FTestFeaturePresenceFromLocalRoomState TestFeaturePresenceFromLocalRoomStateValue;

public:
    virtual FTestFeaturePresenceFromLocalRoomState TestFeaturePresenceFromLocalRoomState() const override
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

        return this->TestFeaturePresenceFromLocalRoomStateValue;
    }
    template <typename FunctorType>
    FRoomProviderFeatureHandler &TestFeaturePresenceFromLocalRoomState(FunctorType &&InFunctor)
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

        this->TestFeaturePresenceFromLocalRoomStateValue =
            FTestFeaturePresenceFromLocalRoomState::CreateLambda(InFunctor);
        return *this;
    }

private:
    FConstructFeatureSearchState ConstructFeatureSearchStateValue;

public:
    virtual FConstructFeatureSearchStateBase ConstructFeatureSearchState() const override
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

        return FConstructFeatureSearchStateBase::CreateLambda(
            [CapturedCallback = this->ConstructFeatureSearchStateValue](
                const FRoomProviderReadOperationRef &ReadOp,
                const TSharedRef<FRoomProvider> &RoomProvider,
                const TMap<FString, FRoomAttribute> &FeatureAttributes) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                if (CapturedCallback.IsBound())
                {
                    return StaticCastSharedPtr<IRoomFeatureSearchStateBase>(
                        CapturedCallback.Execute(ReadOp, RoomProvider, FeatureAttributes));
                }
                return TSharedPtr<IRoomFeatureSearchStateBase>();
            });
    }
    template <typename FunctorType> FRoomProviderFeatureHandler &ConstructFeatureSearchState(FunctorType &&InFunctor)
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

        this->ConstructFeatureSearchStateValue = FConstructFeatureSearchState::CreateLambda(InFunctor);
        return *this;
    }

private:
    FTestFeaturePresenceFromSearchReadOp TestFeaturePresenceFromSearchReadOpValue;

public:
    virtual FTestFeaturePresenceFromSearchReadOp TestFeaturePresenceFromSearchReadOp() const override
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

        return this->TestFeaturePresenceFromSearchReadOpValue;
    }
    template <typename FunctorType>
    FRoomProviderFeatureHandler &TestFeaturePresenceFromSearchReadOp(FunctorType &&InFunctor)
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

        this->TestFeaturePresenceFromSearchReadOpValue = FTestFeaturePresenceFromSearchReadOp::CreateLambda(InFunctor);
        return *this;
    }

public:
    HANDLER_APPLY_FEATURE_REQUEST_TO_WRITE_OPERATION(ApplyFeatureRequestOnCreateRoomOperation);
    HANDLER_APPLY_OMITTED_FEATURE_REQUEST_TO_WRITE_OPERATION(ApplyOmittedFeatureRequestOnCreateRoomOperation);
    HANDLER_APPLY_FEATURE_REQUEST_TO_WRITE_OPERATION(ApplyFeatureRequestOnPostCreateRoomOperation);
    HANDLER_APPLY_OMITTED_FEATURE_REQUEST_TO_WRITE_OPERATION(ApplyOmittedFeatureRequestOnPostCreateRoomOperation);
    HANDLER_APPLY_FEATURE_REQUEST_TO_UPDATE_OPERATION(ApplyFeatureRequestOnUpdateRoomOperation);
    HANDLER_APPLY_FEATURE_REQUEST_TO_WRITE_OPERATION(ApplyFeatureRequestOnJoinRoomOperation);
    HANDLER_APPLY_FEATURE_REQUEST_TO_WRITE_OPERATION(ApplyFeatureRequestOnPostJoinRoomOperation);

private:
    FShouldKickReadyRoomMemberAfterJoin ShouldKickReadyRoomMemberAfterJoinValue;

public:
    virtual FShouldKickReadyRoomMemberAfterJoinBase ShouldKickReadyRoomMemberAfterJoin() const override
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

        return FShouldKickReadyRoomMemberAfterJoinBase::CreateLambda(
            [CapturedCallback = this->ShouldKickReadyRoomMemberAfterJoinValue](
                const FRoomProviderReadOperationRef &ReadOp,
                const TSharedRef<FRoomProvider> &RoomProvider,
                const TSharedRef<IRoom> &Room,
                const TSharedRef<IRoomFeature> &Feature,
                FRoomUserId RoomMemberId,
                const TMap<FString, FRoomAttribute> &FeatureMemberAttributes) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                if (CapturedCallback.IsBound())
                {
                    return CapturedCallback.Execute(
                        ReadOp,
                        RoomProvider,
                        Room,
                        StaticCastSharedRef<TRoomFeatureImpl>(Feature),
                        RoomMemberId,
                        FeatureMemberAttributes);
                }
                return false;
            });
    }
    template <typename FunctorType>
    FRoomProviderFeatureHandler &ShouldKickReadyRoomMemberAfterJoin(FunctorType &&InFunctor)
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

        this->ShouldKickReadyRoomMemberAfterJoinValue = FShouldKickReadyRoomMemberAfterJoin::CreateLambda(InFunctor);
        return *this;
    }

private:
    FGetRoomExclusivityKeyFromFeatureRequest GetRoomExclusivityKeyFromFeatureRequestValue;

public:
    virtual FGetRoomExclusivityKeyFromFeatureRequestBase GetRoomExclusivityKeyFromFeatureRequest() const override
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

        return FGetRoomExclusivityKeyFromFeatureRequestBase::CreateLambda(
            [CapturedCallback = this->GetRoomExclusivityKeyFromFeatureRequestValue](
                const TSharedRef<FRoomProvider> &RoomProvider,
                const TSharedPtr<IRoom> &RoomOnUpdate,
                const TSharedRef<IRoomFeatureRequestBase> &FeatureRequest) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                if (CapturedCallback.IsBound())
                {
                    return CapturedCallback.Execute(
                        RoomProvider,
                        RoomOnUpdate,
                        StaticCastSharedRef<TRoomFeatureRequest>(FeatureRequest));
                }
                return FExclusivityKey(TEXT(""), EExclusivityKeyBehaviour::RejectNewRoom);
            });
    }
    template <typename FunctorType>
    FRoomProviderFeatureHandler &GetRoomExclusivityKeyFromFeatureRequest(FunctorType &&InFunctor)
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

        this->GetRoomExclusivityKeyFromFeatureRequestValue =
            FGetRoomExclusivityKeyFromFeatureRequest::CreateLambda(InFunctor);
        return *this;
    }

private:
    FGetRoomExclusivityKeyFromFeature GetRoomExclusivityKeyFromFeatureValue;

public:
    virtual FGetRoomExclusivityKeyFromFeatureBase GetRoomExclusivityKeyFromFeature() const override
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

        return FGetRoomExclusivityKeyFromFeatureBase::CreateLambda(
            [CapturedCallback = this->GetRoomExclusivityKeyFromFeatureValue](
                const TSharedRef<FRoomProvider> &RoomProvider,
                const TSharedRef<IRoom> &Room,
                const TSharedRef<IRoomFeature> &Feature) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                if (CapturedCallback.IsBound())
                {
                    return CapturedCallback.Execute(RoomProvider, Room, StaticCastSharedRef<TRoomFeatureImpl>(Feature));
                }
                return FExclusivityKey(TEXT(""), EExclusivityKeyBehaviour::RejectNewRoom);
            });
    }
    template <typename FunctorType>
    FRoomProviderFeatureHandler &GetRoomExclusivityKeyFromFeature(FunctorType &&InFunctor)
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

        this->GetRoomExclusivityKeyFromFeatureValue = FGetRoomExclusivityKeyFromFeature::CreateLambda(InFunctor);
        return *this;
    }

private:
    FGetRoomExclusivityKeyFromFeatureSearchState GetRoomExclusivityKeyFromFeatureSearchStateValue;

public:
    virtual FGetRoomExclusivityKeyFromFeatureSearchStateBase GetRoomExclusivityKeyFromFeatureSearchState()
        const override
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

        return FGetRoomExclusivityKeyFromFeatureSearchStateBase::CreateLambda(
            [CapturedCallback = this->GetRoomExclusivityKeyFromFeatureSearchStateValue](
                const TSharedRef<FRoomProvider> &RoomProvider,
                const TSharedRef<IRoomFeatureSearchStateBase> &FeatureSearchState) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                if (CapturedCallback.IsBound())
                {
                    return CapturedCallback.Execute(
                        RoomProvider,
                        StaticCastSharedRef<TRoomFeatureSearchState>(FeatureSearchState));
                }
                return FExclusivityKey(TEXT(""), EExclusivityKeyBehaviour::RejectNewRoom);
            });
    }
    template <typename FunctorType>
    FRoomProviderFeatureHandler &GetRoomExclusivityKeyFromFeatureSearchState(FunctorType &&InFunctor)
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

        this->GetRoomExclusivityKeyFromFeatureSearchStateValue =
            FGetRoomExclusivityKeyFromFeatureSearchState::CreateLambda(InFunctor);
        return *this;
    }
};

#undef HANDLER_SIMPLE_FLAG
#undef HANDLER_APPLY_FEATURE_REQUEST_TO_WRITE_OPERATION
#undef HANDLER_APPLY_OMITTED_FEATURE_REQUEST_TO_WRITE_OPERATION
#undef HANDLER_APPLY_FEATURE_REQUEST_TO_UPDATE_OPERATION

}

namespace Redpoint::EOS::Rooms::Providers
{
REDPOINT_EOS_FILE_NS_EXPORT(215967329, Redpoint::EOS::Rooms::Providers, FRoomProviderFeatureHandler)
}

REDPOINT_EOS_CODE_GUARD_END()