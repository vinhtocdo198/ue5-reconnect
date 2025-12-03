// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Providers/RoomProviderOperation.h"
#include "RedpointEOSRooms/RoomAttribute.h"
#include "RedpointEOSRooms/RoomId.h"
#include "RedpointEOSRooms/RoomUserId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1706010629, Redpoint::EOS::Rooms::Providers)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Providers;
using namespace ::Redpoint::EOS::API;

class IRoomProviderReadOperation : public IRoomProviderOperation
{
public:
    IRoomProviderReadOperation() = default;
    UE_NONCOPYABLE(IRoomProviderReadOperation);
    virtual ~IRoomProviderReadOperation() override = default;

    /**
     * Returns the user ID of the user performing this operation.
     */
    [[nodiscard]] virtual FRoomUserId GetActorUserId() const = 0;

    class IRoomInfo
    {
    public:
        IRoomInfo() = default;
        UE_NONCOPYABLE(IRoomInfo);
        virtual ~IRoomInfo() = default;

        [[nodiscard]] virtual FRoomIdRef GetRoomId() const = 0;
        [[nodiscard]] virtual bool IsOwnerDedicatedServer() const = 0;
        [[nodiscard]] virtual FRoomUserId GetOwnerUserId() const = 0;
        [[nodiscard]] virtual FString GetOwnerDedicatedServerClientId() const = 0;
        [[nodiscard]] virtual uint32 GetMaxMembers() const = 0;

#if !NO_LOGGING
        virtual void InitializeDebugJson(const TSharedRef<FJsonObject> &InfoObject) const {};
#endif
    };
    [[nodiscard]] virtual TTuple<TSharedPtr<IRoomInfo>, FError> GetInfo() = 0;

    [[nodiscard]] virtual TTuple<TMap<FString, FRoomAttribute>, FError> GetAttributes() = 0;

    [[nodiscard]] virtual TTuple<TArray<FRoomUserId>, FError> GetMembers() = 0;
    [[nodiscard]] virtual TTuple<TMap<FString, FRoomAttribute>, FError> GetMemberAttributes(
        const FRoomUserId &MemberUserId) = 0;

    class IRoomMemberInfo
    {
    public:
        IRoomMemberInfo() = default;
        UE_NONCOPYABLE(IRoomMemberInfo);
        virtual ~IRoomMemberInfo() = default;
    };
    [[nodiscard]] virtual TTuple<TSharedPtr<IRoomMemberInfo>, FError> GetMemberInfo(
        const FRoomUserId &MemberUserId) = 0;

#if !NO_LOGGING
    virtual void InitializeDebugJson() override;
#endif
};

typedef TSharedRef<IRoomProviderReadOperation> FRoomProviderReadOperationRef;
typedef TSharedPtr<IRoomProviderReadOperation> FRoomProviderReadOperationPtr;

}

namespace Redpoint::EOS::Rooms::Providers
{
REDPOINT_EOS_FILE_NS_EXPORT(1706010629, Redpoint::EOS::Rooms::Providers, IRoomProviderReadOperation)
REDPOINT_EOS_FILE_NS_EXPORT(1706010629, Redpoint::EOS::Rooms::Providers, FRoomProviderReadOperationRef)
REDPOINT_EOS_FILE_NS_EXPORT(1706010629, Redpoint::EOS::Rooms::Providers, FRoomProviderReadOperationPtr)
}

REDPOINT_EOS_CODE_GUARD_END()