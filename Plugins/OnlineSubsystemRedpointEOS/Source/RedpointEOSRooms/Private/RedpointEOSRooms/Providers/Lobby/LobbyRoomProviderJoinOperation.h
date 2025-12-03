// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Lobby/JoinLobby.h"
#include "RedpointEOSAPI/Lobby/JoinLobbyById.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Providers/RoomProviderWriteOperation.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3664703955, Redpoint::EOS::Rooms::Providers::Lobby)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Providers;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::API::Lobby;

class FLobbyRoomProviderJoinOperation : public IRoomProviderWriteOperation
{
#if !NO_LOGGING
protected:
    TSharedRef<FJsonObject> DebugJson;
    virtual TSharedRef<FJsonObject> &GetMutableDebugJson() override;
#endif

private:
    FRoomUserId ActorUserId;
    FJoinLobbyById::Options JoinByIdLobbyOptions;
    FString JoinLobbyId;
    TSharedPtr<TReleasable<EOS_HLobbyDetails>> JoinLobbyDetails;

    FLobbyRoomProviderJoinOperation(const FRoomUserId &InActorUserId);

public:
    FLobbyRoomProviderJoinOperation(const FRoomUserId &InActorUserId, const FString &InJoinLobbyId);
    FLobbyRoomProviderJoinOperation(
        const FRoomUserId &InActorUserId,
        const TReleasableRef<EOS_HLobbyDetails> &InJoinLobbyDetails);
    UE_NONCOPYABLE(FLobbyRoomProviderJoinOperation);
    virtual ~FLobbyRoomProviderJoinOperation() override = default;

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

    bool IsJoinById() const;
    FJoinLobbyById::Options &GetJoinLobbyByIdOptions();
    FJoinLobby::Options GetJoinLobbyOptions() const;
};

}

namespace Redpoint::EOS::Rooms::Providers::Lobby
{
REDPOINT_EOS_FILE_NS_EXPORT(3664703955, Redpoint::EOS::Rooms::Providers::Lobby, FLobbyRoomProviderJoinOperation)
}

REDPOINT_EOS_CODE_GUARD_END()