// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointEOSInterfaces/Interfaces/OnlineLobbyInterface.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

class FOnlineLobbyInterfaceTest : public IOnlineLobby,
                                  public TSharedFromThis<FOnlineLobbyInterfaceTest, ESPMode::ThreadSafe>
{
private:
    TSharedPtr<class FTestBackend> Backend;

public:
    FOnlineLobbyInterfaceTest(TSharedPtr<class FTestBackend> InBackend)
        : Backend(InBackend) {};
    virtual ~FOnlineLobbyInterfaceTest() override = default;

    virtual FDateTime GetUtcNow() override
        PURE_VIRTUAL(FOnlineLobbyInterfaceTest::GetUtcNow, return FDateTime::MinValue(););

    virtual TSharedPtr<FOnlineLobbyId> ParseSerializedLobbyId(const FString &InLobbyId) override;

    virtual TSharedPtr<FOnlineLobbyTransaction> MakeCreateLobbyTransaction(const FUniqueNetId &UserId) override;

    virtual TSharedPtr<FOnlineLobbyTransaction> MakeUpdateLobbyTransaction(
        const FUniqueNetId &UserId,
        const FOnlineLobbyId &LobbyId) override;

    virtual TSharedPtr<FOnlineLobbyMemberTransaction> MakeUpdateLobbyMemberTransaction(
        const FUniqueNetId &UserId,
        const FOnlineLobbyId &LobbyId,
        const FUniqueNetId &MemberId) override;

    virtual bool CreateLobby(
        const FUniqueNetId &UserId,
        const FOnlineLobbyTransaction &Transaction,
        FOnLobbyCreateOrConnectComplete OnComplete = FOnLobbyCreateOrConnectComplete()) override;

    virtual bool UpdateLobby(
        const FUniqueNetId &UserId,
        const FOnlineLobbyId &LobbyId,
        const FOnlineLobbyTransaction &Transaction,
        FOnLobbyOperationComplete OnComplete = FOnLobbyOperationComplete()) override;

    virtual bool DeleteLobby(
        const FUniqueNetId &UserId,
        const FOnlineLobbyId &LobbyId,
        FOnLobbyOperationComplete OnComplete = FOnLobbyOperationComplete()) override;

    virtual bool ConnectLobby(
        const FUniqueNetId &UserId,
        const FOnlineLobbyId &LobbyId,
        FOnLobbyCreateOrConnectComplete OnComplete = FOnLobbyCreateOrConnectComplete()) override;

    virtual bool DisconnectLobby(
        const FUniqueNetId &UserId,
        const FOnlineLobbyId &LobbyId,
        FOnLobbyOperationComplete OnComplete = FOnLobbyOperationComplete()) override;

    virtual bool UpdateMemberSelf(
        const FUniqueNetId &UserId,
        const FOnlineLobbyId &LobbyId,
        const FOnlineLobbyMemberTransaction &Transaction,
        FOnLobbyOperationComplete OnComplete = FOnLobbyOperationComplete()) override;

    virtual bool GetMemberCount(const FUniqueNetId &UserId, const FOnlineLobbyId &LobbyId, int32 &OutMemberCount)
        override;

    virtual bool GetMemberUserId(
        const FUniqueNetId &UserId,
        const FOnlineLobbyId &LobbyId,
        int32 MemberIndex,
        TSharedPtr<const FUniqueNetId> &OutMemberId) override;

    virtual bool GetMemberMetadataValue(
        const FUniqueNetId &UserId,
        const FOnlineLobbyId &LobbyId,
        const FUniqueNetId &MemberId,
        const FString &MetadataKey,
        FVariantData &OutMetadataValue) override;

    virtual bool Search(
        const FUniqueNetId &UserId,
        const FOnlineLobbySearchQuery &Query,
        FOnLobbySearchComplete OnComplete = FOnLobbySearchComplete()) override;

    virtual bool GetLobbyMetadataValue(
        const FUniqueNetId &UserId,
        const FOnlineLobbyId &LobbyId,
        const FString &MetadataKey,
        FVariantData &OutMetadataValue) override;

    virtual bool KickMember(
        const FUniqueNetId &UserId,
        const FOnlineLobbyId &LobbyId,
        const FUniqueNetId &MemberId,
        FOnLobbyOperationComplete OnComplete = FOnLobbyOperationComplete()) override
    {
        checkf(
            false,
            TEXT("KickMember should never be called during tests, as this function is only used if something "
                 "unexpected happens in a real environment. This method being called during testing indicates a bug!"));
        return false;
    }
};

#endif

REDPOINT_EOS_CODE_GUARD_END()