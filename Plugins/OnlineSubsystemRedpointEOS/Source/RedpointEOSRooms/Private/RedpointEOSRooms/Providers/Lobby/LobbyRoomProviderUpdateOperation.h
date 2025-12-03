// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Lobby/UpdateLobbyModification.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Providers/Lobby/LobbyRoomProviderReadOperation.h"
#include "RedpointEOSRooms/Providers/RoomProviderWriteOperation.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1439984043, Redpoint::EOS::Rooms::Providers::Lobby)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Providers;
using namespace ::Redpoint::EOS::Rooms::Providers::Lobby;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::API::Lobby;

class FLobbyRoomProviderUpdateOperation : public FLobbyRoomProviderReadOperation, public IRoomProviderWriteOperation
{
#if !NO_LOGGING
protected:
    virtual TSharedRef<FJsonObject> &GetMutableDebugJson() override;
#endif

private:
    FRoomUserId ActorUserId;
    FString LobbyId;
    FName RoomNamespace;
    TSharedPtr<TReleasable<EOS_HLobbyModification>> LobbyModification;
    EOS_EResult UpdateLobbyModificationResultCode;

public:
#if !NO_LOGGING
    TSharedRef<FJsonObject> AddUpdateToDebugJson(const FString &OpName, const EOS_EResult &ResultCode);
    TSharedRef<FJsonObject> AddUpdateToDebugJson(const FString &OpName, const FError &ErrorCode);
#endif

    FLobbyRoomProviderUpdateOperation(
        const FRoomUserId &InActorUserId,
        const FString &InLobbyId,
        const FName &InRoomNamespace,
        TReleasableRef<EOS_HLobbyDetails> InLobbyDetails,
        TReleasableRef<EOS_HLobbyModification> InLobbyModification);
    FLobbyRoomProviderUpdateOperation(
        const FRoomUserId &InActorUserId,
        const FString &InLobbyId,
        const FName &InRoomNamespace,
        TReleasableRef<EOS_HLobbyDetails> InLobbyDetails,
        EOS_EResult InUpdateLobbyModificationResultCode);
    UE_NONCOPYABLE(FLobbyRoomProviderUpdateOperation);
    virtual ~FLobbyRoomProviderUpdateOperation() override = default;

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

    const FString &GetLobbyId() const;
    const TSharedPtr<TReleasable<EOS_HLobbyModification>> &GetLobbyModification() const;
    const EOS_EResult &GetUpdateLobbyModificationResultCode() const;
    const FName &GetRoomNamespace();
};

}

namespace Redpoint::EOS::Rooms::Providers::Lobby
{
REDPOINT_EOS_FILE_NS_EXPORT(1439984043, Redpoint::EOS::Rooms::Providers::Lobby, FLobbyRoomProviderUpdateOperation)
}

REDPOINT_EOS_CODE_GUARD_END()