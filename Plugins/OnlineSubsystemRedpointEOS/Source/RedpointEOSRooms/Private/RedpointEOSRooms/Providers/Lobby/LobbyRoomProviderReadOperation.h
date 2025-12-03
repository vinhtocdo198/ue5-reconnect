// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/LobbyDetails/CopyInfo.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Providers/RoomProviderReadOperation.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(4289788093, Redpoint::EOS::Rooms::Providers::Lobby, FLobbyRoomProvider)

namespace REDPOINT_EOS_FILE_NS_ID(8442807, Redpoint::EOS::Rooms::Providers::Lobby)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Providers;
using namespace ::Redpoint::EOS::Rooms::Providers::Lobby;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::API::LobbyDetails;

class FLobbyRoomProviderReadOperation : public IRoomProviderReadOperation
{
    friend FLobbyRoomProvider;

#if !NO_LOGGING
protected:
    TSharedRef<FJsonObject> DebugJson;
    virtual TSharedRef<FJsonObject> &GetMutableDebugJson() override;
#endif

private:
    FRoomUserId ActorUserId;
    TReleasableRef<EOS_HLobbyDetails> LobbyDetails;
    bool bIsSearchResult;

    class FLobbyRoomInfo : public IRoomInfo
    {
        friend FLobbyRoomProvider;

    private:
        FCopyInfo::Result Result;

    public:
        FLobbyRoomInfo(const FCopyInfo::Result &InResult);
        UE_NONCOPYABLE(FLobbyRoomInfo);
        virtual ~FLobbyRoomInfo() override = default;

        [[nodiscard]] virtual FRoomIdRef GetRoomId() const override;
        [[nodiscard]] virtual bool IsOwnerDedicatedServer() const override;
        [[nodiscard]] virtual FRoomUserId GetOwnerUserId() const override;
        [[nodiscard]] virtual FString GetOwnerDedicatedServerClientId() const override;
        [[nodiscard]] virtual uint32 GetMaxMembers() const override;

#if !NO_LOGGING
        virtual void InitializeDebugJson(const TSharedRef<FJsonObject> &InfoObject) const override;
#endif
    };

public:
    FLobbyRoomProviderReadOperation(
        const FRoomUserId &InActorUserId,
        TReleasableRef<EOS_HLobbyDetails> InLobbyDetails,
        bool bInIsSearchResult);
    UE_NONCOPYABLE(FLobbyRoomProviderReadOperation);
    virtual ~FLobbyRoomProviderReadOperation() override = default;

    [[nodiscard]] virtual FRoomUserId GetActorUserId() const override;

    [[nodiscard]] virtual TTuple<TSharedPtr<IRoomInfo>, FError> GetInfo() override;

    [[nodiscard]] virtual TTuple<TMap<FString, FRoomAttribute>, FError> GetAttributes() override;

    [[nodiscard]] virtual TTuple<TArray<FRoomUserId>, FError> GetMembers() override;
    [[nodiscard]] virtual TTuple<TMap<FString, FRoomAttribute>, FError> GetMemberAttributes(
        const FRoomUserId &MemberUserId) override;

    [[nodiscard]] virtual TTuple<TSharedPtr<IRoomMemberInfo>, FError> GetMemberInfo(
        const FRoomUserId &MemberUserId) override;

    const TReleasableRef<EOS_HLobbyDetails> &GetLobbyDetails() const;
};

}

namespace Redpoint::EOS::Rooms::Providers::Lobby
{
REDPOINT_EOS_FILE_NS_EXPORT(8442807, Redpoint::EOS::Rooms::Providers::Lobby, FLobbyRoomProviderReadOperation)
}

REDPOINT_EOS_CODE_GUARD_END()