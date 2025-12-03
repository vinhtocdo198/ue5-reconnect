// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Providers/RoomProviderActionOperation.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4005217892, Redpoint::EOS::Rooms::Providers::Lobby)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Providers;
using namespace ::Redpoint::EOS::API;

class FLobbyRoomProviderTargetMemberOperation : public IRoomProviderActionOperation
{
#if !NO_LOGGING
protected:
    TSharedRef<FJsonObject> DebugJson;
    virtual TSharedRef<FJsonObject> &GetMutableDebugJson() override;
#endif

private:
    FRoomUserId ActorUserId;
    FString LobbyId;
    FRoomUserId TargetUserId;

public:
    FLobbyRoomProviderTargetMemberOperation(
        const FRoomUserId &InActorUserId,
        const FString &InLobbyId,
        const FRoomUserId &InTargetUserId);
    UE_NONCOPYABLE(FLobbyRoomProviderTargetMemberOperation);
    virtual ~FLobbyRoomProviderTargetMemberOperation() override = default;

    [[nodiscard]] virtual FRoomUserId GetActorUserId() const override;

    [[nodiscard]] virtual TTuple<TSharedPtr<IRoomInfo>, FError> GetInfo() override;

    [[nodiscard]] virtual TTuple<TMap<FString, FRoomAttribute>, FError> GetAttributes() override;

    [[nodiscard]] virtual TTuple<TArray<FRoomUserId>, FError> GetMembers() override;
    [[nodiscard]] virtual TTuple<TMap<FString, FRoomAttribute>, FError> GetMemberAttributes(
        const FRoomUserId &MemberUserId) override;

    [[nodiscard]] virtual TTuple<TSharedPtr<IRoomMemberInfo>, FError> GetMemberInfo(
        const FRoomUserId &MemberUserId) override;

    const FString &GetLobbyId() const;
    const FRoomUserId &GetTargetUserId() const;
};

}

namespace Redpoint::EOS::Rooms::Providers::Lobby
{
REDPOINT_EOS_FILE_NS_EXPORT(4005217892, Redpoint::EOS::Rooms::Providers::Lobby, FLobbyRoomProviderTargetMemberOperation)
}

REDPOINT_EOS_CODE_GUARD_END()