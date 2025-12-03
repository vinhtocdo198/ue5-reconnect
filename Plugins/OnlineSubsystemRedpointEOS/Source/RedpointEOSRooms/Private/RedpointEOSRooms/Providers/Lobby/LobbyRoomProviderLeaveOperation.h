// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Providers/RoomProviderActionOperation.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4122519946, Redpoint::EOS::Rooms::Providers::Lobby)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Providers;
using namespace ::Redpoint::EOS::API;

class FLobbyRoomProviderLeaveOperation : public IRoomProviderActionOperation
{
#if !NO_LOGGING
protected:
    TSharedRef<FJsonObject> DebugJson;
    virtual TSharedRef<FJsonObject> &GetMutableDebugJson() override;
#endif

private:
    FRoomUserId ActorUserId;
    FString LobbyId;

public:
    FLobbyRoomProviderLeaveOperation(const FRoomUserId &InActorUserId, const FString &InLobbyId);
    UE_NONCOPYABLE(FLobbyRoomProviderLeaveOperation);
    virtual ~FLobbyRoomProviderLeaveOperation() override = default;

    [[nodiscard]] virtual FRoomUserId GetActorUserId() const override;

    [[nodiscard]] virtual TTuple<TSharedPtr<IRoomInfo>, FError> GetInfo() override;

    [[nodiscard]] virtual TTuple<TMap<FString, FRoomAttribute>, FError> GetAttributes() override;

    [[nodiscard]] virtual TTuple<TArray<FRoomUserId>, FError> GetMembers() override;
    [[nodiscard]] virtual TTuple<TMap<FString, FRoomAttribute>, FError> GetMemberAttributes(
        const FRoomUserId &MemberUserId) override;

    [[nodiscard]] virtual TTuple<TSharedPtr<IRoomMemberInfo>, FError> GetMemberInfo(
        const FRoomUserId &MemberUserId) override;

    const FString &GetLobbyId() const;
};

}

namespace Redpoint::EOS::Rooms::Providers::Lobby
{
REDPOINT_EOS_FILE_NS_EXPORT(4122519946, Redpoint::EOS::Rooms::Providers::Lobby, FLobbyRoomProviderLeaveOperation)
}

REDPOINT_EOS_CODE_GUARD_END()