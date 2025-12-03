// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Lobby/CreateLobby.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Providers/RoomProviderWriteOperation.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4170781936, Redpoint::EOS::Rooms::Providers::Lobby)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Providers;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::API::Lobby;

class FLobbyRoomProviderCreateOperation : public IRoomProviderWriteOperation
{
#if !NO_LOGGING
protected:
    TSharedRef<FJsonObject> DebugJson;
    virtual TSharedRef<FJsonObject> &GetMutableDebugJson() override;
#endif

private:
    FRoomUserId ActorUserId;
    FName RoomNamespace;
    FCreateLobby::Options CreateLobbyOptions;

public:
    FLobbyRoomProviderCreateOperation(const FRoomUserId &InActorUserId, const FName &InRoomNamespace);
    UE_NONCOPYABLE(FLobbyRoomProviderCreateOperation);
    virtual ~FLobbyRoomProviderCreateOperation() override = default;

    [[nodiscard]] virtual FRoomUserId GetActorUserId() const override;

    [[nodiscard]] virtual TTuple<TSharedPtr<IRoomInfo>, FError> GetInfo() override;
    [[nodiscard]] virtual FError SetMaxMembers(uint32 MaxMembers) override;

    [[nodiscard]] virtual TTuple<TMap<FString, FRoomAttribute>, FError> GetAttributes() override;
    [[nodiscard]] virtual FError AddAttribute(const FString &Key, const FRoomAttribute &Value) override;
    [[nodiscard]] virtual FError RemoveAttribute(const FString &Key) override;

    [[nodiscard]] virtual TTuple<TArray<FRoomUserId>, FError> GetMembers() override;
    [[nodiscard]] virtual TTuple<TMap<FString, FRoomAttribute>, FError> GetMemberAttributes(
        const FRoomUserId &MemberUserId) override;
    [[nodiscard]] virtual FError AddMemberAttribute(const FString &Key, const FRoomAttribute &Value) override;
    [[nodiscard]] virtual FError RemoveMemberAttribute(const FString &Key) override;

    [[nodiscard]] virtual TTuple<TSharedPtr<IRoomMemberInfo>, FError> GetMemberInfo(
        const FRoomUserId &MemberUserId) override;

    FCreateLobby::Options &GetCreateLobbyOptions();
    const FName &GetRoomNamespace();
};

}

namespace Redpoint::EOS::Rooms::Providers::Lobby
{
REDPOINT_EOS_FILE_NS_EXPORT(4170781936, Redpoint::EOS::Rooms::Providers::Lobby, FLobbyRoomProviderCreateOperation)
}

REDPOINT_EOS_CODE_GUARD_END()