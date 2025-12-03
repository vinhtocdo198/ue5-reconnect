// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Releasable.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Providers/RoomProviderSearchOperation.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2332866522, Redpoint::EOS::Rooms::Providers::Lobby)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Providers;
using namespace ::Redpoint::EOS::API;

class FLobbyRoomProviderSearchOperation : public IRoomProviderSearchOperation
{
#if !NO_LOGGING
protected:
    TSharedRef<FJsonObject> DebugJson;
    virtual TSharedRef<FJsonObject> &GetMutableDebugJson() override;
#endif

private:
    FRoomUserId ActorUserId;
    TSharedPtr<TReleasable<EOS_HLobbySearch>> LobbySearch;
    EOS_EResult CreateSearchResultCode;

#if !NO_LOGGING
    TSharedRef<FJsonObject> AddSearchToDebugJson(const FString &OpName, const EOS_EResult &ResultCode);
    TSharedRef<FJsonObject> AddSearchToDebugJson(const FString &OpName, const FError &ErrorCode);
#endif

public:
    FLobbyRoomProviderSearchOperation(const FRoomUserId &InActorUserId, TReleasableRef<EOS_HLobbySearch> InLobbySearch);
    FLobbyRoomProviderSearchOperation(const FRoomUserId &InActorUserId, EOS_EResult InCreateSearchResultCode);
    UE_NONCOPYABLE(FLobbyRoomProviderSearchOperation);
    virtual ~FLobbyRoomProviderSearchOperation() override = default;

    [[nodiscard]] virtual FRoomUserId GetActorUserId() const override;

    [[nodiscard]] virtual FError WithRoomId(const FRoomIdRef &RoomId) override;
    [[nodiscard]] virtual FError WithRoomMemberId(const FRoomUserId &RoomMemberId) override;
    [[nodiscard]] virtual FError WithAttributeConstraint(const FString &Key, const FRoomAttributeConstraint &Constraint)
        override;

    const TSharedPtr<TReleasable<EOS_HLobbySearch>> &GetLobbySearch() const;
    const EOS_EResult &GetCreateSearchResultCode() const;

#if !NO_LOGGING
    virtual void InitializeDebugJson() override;
#endif
};

}

namespace Redpoint::EOS::Rooms::Providers::Lobby
{
REDPOINT_EOS_FILE_NS_EXPORT(2332866522, Redpoint::EOS::Rooms::Providers::Lobby, FLobbyRoomProviderSearchOperation)
}

REDPOINT_EOS_CODE_GUARD_END()