// Copyright June Rhodes. All Rights Reserved.

#include "AutoSubsystems/OnlineChatSubsystem.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

#include "OnlineSubsystem.h"
#include "OnlineSubsystemBlueprintsModule.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineSubsystemUtils.h"

FName UOnlineChatSubsystem::OSSNameToAssignDuringConstruction = NAME_Default;

TSharedPtr<class IOnlineChat, ESPMode::ThreadSafe> UOnlineChatSubsystem::GetHandle()
{
    if (this->HasAnyFlags(RF_BeginDestroyed))
    {
        auto BDCheck_GameInstance = this->GetGameInstance();
        if (!IsValid(BDCheck_GameInstance))
        {
            return nullptr;
        }

        auto BDCheck_World = BDCheck_GameInstance->GetWorld();
        auto BDCheck_Pinned = this->HandleWk.Pin();

        if ((BDCheck_Pinned == nullptr || !BDCheck_Pinned.IsValid()) && !IsValid(BDCheck_World))
        {
            // The world and associated online subsystem interfaces have already been cleaned up.
            // The only caller of GetHandle() in this context is event deregistration, which doesn't
            // need to be done if the online subsystem itself is already gone.
            return nullptr;
        }
    }

    auto GameInstance = this->GetGameInstance();
    if (!IsValid(GameInstance))
    {
        return nullptr;
    }

    auto World = GameInstance->GetWorld();
    if (World == nullptr)
    {
        // The world has already been freed; this GetHandle call might be happening in a callback
        // (where it is already handled as a "CallFailed" if GetHandle returns a nullptr).
        return nullptr;
    }

    auto CurrentWorldContext = GEngine->GetWorldContextFromWorld(World);
    if (CurrentWorldContext == nullptr ||
        !CurrentWorldContext->ContextHandle.IsEqual(this->WorldContext->ContextHandle))
    {
        // There is no world context for the current world. This can happen during
        // play-in-editor shutdown when using voice.
        return nullptr;
    }

    TSharedPtr<class IOnlineChat, ESPMode::ThreadSafe> Pinned = this->HandleWk.Pin();
    if (Pinned == nullptr || !Pinned.IsValid())
    {
        // Even though Online::GetSubsystem takes a UWorld* as it's argument, it only ever uses it
        // to get the world context, so that it can provide a different online subsystem per
        // play-in-editor context.
        //
        // There's one UGameInstance* per play-in-editor context, which also means game instance
        // subsystems get created per play-in-editor context. In contrast, the UWorld* gets reused
        // between contexts, which means that if we inherited from UWorldSubsystem, our lifetime
        // would span contexts and in turn make accessing the correct online subsystem instance
        // much harder.
        //
        // So we get the UWorld* of the UGameInstance*, which will be the same UWorld that's shared
        // between all other instances, but we'll have the right FWorldContext
        IOnlineSubsystem *Subsystem = nullptr;
        if (this->OSSName.IsEqual(NAME_Default))
        {
            Subsystem = Online::GetSubsystem(World);
        }
        else if (this->OSSName.IsEqual(FName(TEXT("Native"))))
        {
            Subsystem = IOnlineSubsystem::GetByPlatform();
        }
        else
        {
            Subsystem = Online::GetSubsystem(World, this->OSSName);
        }
        if (Subsystem == nullptr)
        {
            // No subsystem is available.
            return nullptr;
        }

        // Finish setup, store the reference to the interface.
        auto __Handle__ = Subsystem->GetChatInterface();
        this->HandleWk = __Handle__;
        Pinned = __Handle__;
    }

    return Pinned;
}

bool UOnlineChatSubsystem::IsHandleValid(const TSharedPtr<class IOnlineChat, ESPMode::ThreadSafe> &InHandle)
{
    return InHandle.IsValid();
}

UOnlineChatSubsystem::UOnlineChatSubsystem()
{
    if (this->HasAnyFlags(RF_ClassDefaultObject))
    {
        return;
    }

    this->OSSName = UOnlineChatSubsystem::OSSNameToAssignDuringConstruction;
    UOnlineChatSubsystem::OSSNameToAssignDuringConstruction = NAME_Default;

    auto GameInstance = this->GetGameInstance();
    check(IsValid(GameInstance));

    auto World = GameInstance->GetWorld();
    check(IsValid(World));

    // Store the WorldContext so we can assert that it never changes later during our
    // operation (required for semantic correctness).
    this->WorldContext = GEngine->GetWorldContextFromWorld(World);
    check(WorldContext != nullptr);
    UE_LOG(
        LogOnlineBlueprint,
        Verbose,
        TEXT("OnlineChatSubsystem subsystem initialized in context %s"),
        *WorldContext->ContextHandle.ToString());

    // Perform the initial cache of the online subsystem pointer. Calling GetHandle()
    // is enough to cache the weak pointer.
    this->HandleWk = nullptr;
    this->GetHandle();

    // Now, register events.

    TSharedPtr<class IOnlineChat, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
        }
        this->DelegateHandle_OnChatRoomCreated =
            Handle->AddOnChatRoomCreatedDelegate_Handle(FOnChatRoomCreated::FDelegate::CreateWeakLambda(
                this,
                [this](const FUniqueNetId &Param1, const FChatRoomId &Param2, bool Param3, const FString &Param4) {
                    this->OnChatRoomCreated
                        .Broadcast(ConvertDangerousUniqueNetIdToRepl(Param1), Param2, Param3, Param4);
                }));
        this->DelegateHandle_OnChatRoomConfigured =
            Handle->AddOnChatRoomConfiguredDelegate_Handle(FOnChatRoomConfigured::FDelegate::CreateWeakLambda(
                this,
                [this](const FUniqueNetId &Param1, const FChatRoomId &Param2, bool Param3, const FString &Param4) {
                    this->OnChatRoomConfigured
                        .Broadcast(ConvertDangerousUniqueNetIdToRepl(Param1), Param2, Param3, Param4);
                }));
        this->DelegateHandle_OnChatRoomJoinPublic =
            Handle->AddOnChatRoomJoinPublicDelegate_Handle(FOnChatRoomJoinPublic::FDelegate::CreateWeakLambda(
                this,
                [this](const FUniqueNetId &Param1, const FChatRoomId &Param2, bool Param3, const FString &Param4) {
                    this->OnChatRoomJoinPublic
                        .Broadcast(ConvertDangerousUniqueNetIdToRepl(Param1), Param2, Param3, Param4);
                }));
        this->DelegateHandle_OnChatRoomJoinPrivate =
            Handle->AddOnChatRoomJoinPrivateDelegate_Handle(FOnChatRoomJoinPrivate::FDelegate::CreateWeakLambda(
                this,
                [this](const FUniqueNetId &Param1, const FChatRoomId &Param2, bool Param3, const FString &Param4) {
                    this->OnChatRoomJoinPrivate
                        .Broadcast(ConvertDangerousUniqueNetIdToRepl(Param1), Param2, Param3, Param4);
                }));
        this->DelegateHandle_OnChatRoomExit =
            Handle->AddOnChatRoomExitDelegate_Handle(FOnChatRoomExit::FDelegate::CreateWeakLambda(
                this,
                [this](const FUniqueNetId &Param1, const FChatRoomId &Param2, bool Param3, const FString &Param4) {
                    this->OnChatRoomExit.Broadcast(ConvertDangerousUniqueNetIdToRepl(Param1), Param2, Param3, Param4);
                }));
        this->DelegateHandle_OnChatRoomMemberJoin =
            Handle->AddOnChatRoomMemberJoinDelegate_Handle(FOnChatRoomMemberJoin::FDelegate::CreateWeakLambda(
                this,
                [this](const FUniqueNetId &Param1, const FChatRoomId &Param2, const FUniqueNetId &Param3) {
                    this->OnChatRoomMemberJoin.Broadcast(
                        ConvertDangerousUniqueNetIdToRepl(Param1),
                        Param2,
                        ConvertDangerousUniqueNetIdToRepl(Param3));
                }));
        this->DelegateHandle_OnChatRoomMemberExit =
            Handle->AddOnChatRoomMemberExitDelegate_Handle(FOnChatRoomMemberExit::FDelegate::CreateWeakLambda(
                this,
                [this](const FUniqueNetId &Param1, const FChatRoomId &Param2, const FUniqueNetId &Param3) {
                    this->OnChatRoomMemberExit.Broadcast(
                        ConvertDangerousUniqueNetIdToRepl(Param1),
                        Param2,
                        ConvertDangerousUniqueNetIdToRepl(Param3));
                }));
        this->DelegateHandle_OnChatRoomMemberUpdate =
            Handle->AddOnChatRoomMemberUpdateDelegate_Handle(FOnChatRoomMemberUpdate::FDelegate::CreateWeakLambda(
                this,
                [this](const FUniqueNetId &Param1, const FChatRoomId &Param2, const FUniqueNetId &Param3) {
                    this->OnChatRoomMemberUpdate.Broadcast(
                        ConvertDangerousUniqueNetIdToRepl(Param1),
                        Param2,
                        ConvertDangerousUniqueNetIdToRepl(Param3));
                }));
#if REDPOINT_EOS_UE_5_5_OR_LATER
        this->DelegateHandle_OnChatRoomMessageSent =
            Handle->AddOnChatRoomMessageSentDelegate_Handle(FOnChatRoomMessageSent::FDelegate::CreateWeakLambda(
                this,
                [this](const FUniqueNetId &Param1, const FChatRoomId &Param2, bool Param3, const FString &Param4) {
                    this->OnChatRoomMessageSent
                        .Broadcast(ConvertDangerousUniqueNetIdToRepl(Param1), Param2, Param3, Param4);
                }));
#endif
        this->DelegateHandle_OnChatRoomMessageReceived =
            Handle->AddOnChatRoomMessageReceivedDelegate_Handle(FOnChatRoomMessageReceived::FDelegate::CreateWeakLambda(
                this,
                [this](const FUniqueNetId &Param1, const FChatRoomId &Param2, const TSharedRef<FChatMessage> &Param3) {
                    this->OnChatRoomMessageReceived.Broadcast(
                        ConvertDangerousUniqueNetIdToRepl(Param1),
                        Param2,
                        FChatMessageBP::FromNative(*Param3));
                }));
#if REDPOINT_EOS_UE_5_5_OR_LATER
        this->DelegateHandle_OnChatPrivateMessageSent =
            Handle->AddOnChatPrivateMessageSentDelegate_Handle(FOnChatPrivateMessageSent::FDelegate::CreateWeakLambda(
                this,
                [this](const FUniqueNetId &Param1, const FUniqueNetId &Param2, bool Param3, const FString &Param4) {
                    this->OnChatPrivateMessageSent.Broadcast(
                        ConvertDangerousUniqueNetIdToRepl(Param1),
                        ConvertDangerousUniqueNetIdToRepl(Param2),
                        Param3,
                        Param4);
                }));
#endif
        this->DelegateHandle_OnChatPrivateMessageReceived = Handle->AddOnChatPrivateMessageReceivedDelegate_Handle(
            FOnChatPrivateMessageReceived::FDelegate::CreateWeakLambda(
                this,
                [this](const FUniqueNetId &Param1, const TSharedRef<FChatMessage> &Param2) {
                    this->OnChatPrivateMessageReceived.Broadcast(
                        ConvertDangerousUniqueNetIdToRepl(Param1),
                        FChatMessageBP::FromNative(*Param2));
                }));
    }
}

void UOnlineChatSubsystem::BeginDestroy()
{
    if (this->HasAnyFlags(RF_ClassDefaultObject))
    {
        Super::BeginDestroy();
        return;
    }

    TSharedPtr<class IOnlineChat, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
        }
        Handle->ClearOnChatRoomCreatedDelegate_Handle(this->DelegateHandle_OnChatRoomCreated);
        Handle->ClearOnChatRoomConfiguredDelegate_Handle(this->DelegateHandle_OnChatRoomConfigured);
        Handle->ClearOnChatRoomJoinPublicDelegate_Handle(this->DelegateHandle_OnChatRoomJoinPublic);
        Handle->ClearOnChatRoomJoinPrivateDelegate_Handle(this->DelegateHandle_OnChatRoomJoinPrivate);
        Handle->ClearOnChatRoomExitDelegate_Handle(this->DelegateHandle_OnChatRoomExit);
        Handle->ClearOnChatRoomMemberJoinDelegate_Handle(this->DelegateHandle_OnChatRoomMemberJoin);
        Handle->ClearOnChatRoomMemberExitDelegate_Handle(this->DelegateHandle_OnChatRoomMemberExit);
        Handle->ClearOnChatRoomMemberUpdateDelegate_Handle(this->DelegateHandle_OnChatRoomMemberUpdate);
#if REDPOINT_EOS_UE_5_5_OR_LATER
        Handle->ClearOnChatRoomMessageSentDelegate_Handle(this->DelegateHandle_OnChatRoomMessageSent);
#endif
        Handle->ClearOnChatRoomMessageReceivedDelegate_Handle(this->DelegateHandle_OnChatRoomMessageReceived);
#if REDPOINT_EOS_UE_5_5_OR_LATER
        Handle->ClearOnChatPrivateMessageSentDelegate_Handle(this->DelegateHandle_OnChatPrivateMessageSent);
#endif
        Handle->ClearOnChatPrivateMessageReceivedDelegate_Handle(this->DelegateHandle_OnChatPrivateMessageReceived);
    }

    Super::BeginDestroy();
}

bool UOnlineChatSubsystem::IsSubsystemAvailable()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    return this->IsHandleValid(this->GetHandle());
}

bool UOnlineChatSubsystem::CreateRoom(
    const FUniqueNetIdRepl &UserId,
    const FString &RoomId,
    const FString &Nickname,
    const FChatRoomConfigBP &ChatRoomConfig)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }
    if (!(UserId.IsValid() && UserId.GetUniqueNetId().IsValid()))
    {
        return false;
    }

    auto __Result =
        Handle->CreateRoom(UserId.GetUniqueNetId().ToSharedRef().Get(), RoomId, Nickname, ChatRoomConfig.ToNative());

    return __Result;
}

bool UOnlineChatSubsystem::ConfigureRoom(
    const FUniqueNetIdRepl &UserId,
    const FString &RoomId,
    const FChatRoomConfigBP &ChatRoomConfig)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }
    if (!(UserId.IsValid() && UserId.GetUniqueNetId().IsValid()))
    {
        return false;
    }

    auto __Result =
        Handle->ConfigureRoom(UserId.GetUniqueNetId().ToSharedRef().Get(), RoomId, ChatRoomConfig.ToNative());

    return __Result;
}

bool UOnlineChatSubsystem::JoinPublicRoom(
    const FUniqueNetIdRepl &UserId,
    const FString &RoomId,
    const FString &Nickname,
    const FChatRoomConfigBP &ChatRoomConfig)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }
    if (!(UserId.IsValid() && UserId.GetUniqueNetId().IsValid()))
    {
        return false;
    }

    auto __Result = Handle->JoinPublicRoom(
        UserId.GetUniqueNetId().ToSharedRef().Get(),
        RoomId,
        Nickname,
        ChatRoomConfig.ToNative());

    return __Result;
}

bool UOnlineChatSubsystem::JoinPrivateRoom(
    const FUniqueNetIdRepl &UserId,
    const FString &RoomId,
    const FString &Nickname,
    const FChatRoomConfigBP &ChatRoomConfig)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }
    if (!(UserId.IsValid() && UserId.GetUniqueNetId().IsValid()))
    {
        return false;
    }

    auto __Result = Handle->JoinPrivateRoom(
        UserId.GetUniqueNetId().ToSharedRef().Get(),
        RoomId,
        Nickname,
        ChatRoomConfig.ToNative());

    return __Result;
}

bool UOnlineChatSubsystem::ExitRoom(const FUniqueNetIdRepl &UserId, const FString &RoomId)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }
    if (!(UserId.IsValid() && UserId.GetUniqueNetId().IsValid()))
    {
        return false;
    }

    auto __Result = Handle->ExitRoom(UserId.GetUniqueNetId().ToSharedRef().Get(), RoomId);

    return __Result;
}

bool UOnlineChatSubsystem::SendRoomChat(const FUniqueNetIdRepl &UserId, const FString &RoomId, const FString &MsgBody)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }
    if (!(UserId.IsValid() && UserId.GetUniqueNetId().IsValid()))
    {
        return false;
    }

    auto __Result = Handle->SendRoomChat(UserId.GetUniqueNetId().ToSharedRef().Get(), RoomId, MsgBody);

    return __Result;
}

bool UOnlineChatSubsystem::SendPrivateChat(
    const FUniqueNetIdRepl &UserId,
    const FUniqueNetIdRepl &RecipientId,
    const FString &MsgBody)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }
    if (!(UserId.IsValid() && UserId.GetUniqueNetId().IsValid()))
    {
        return false;
    }
    if (!(RecipientId.IsValid() && RecipientId.GetUniqueNetId().IsValid()))
    {
        return false;
    }

    auto __Result = Handle->SendPrivateChat(
        UserId.GetUniqueNetId().ToSharedRef().Get(),
        RecipientId.GetUniqueNetId().ToSharedRef().Get(),
        MsgBody);

    return __Result;
}

bool UOnlineChatSubsystem::IsChatAllowed(const FUniqueNetIdRepl &UserId, const FUniqueNetIdRepl &RecipientId)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }
    if (!(UserId.IsValid() && UserId.GetUniqueNetId().IsValid()))
    {
        return false;
    }
    if (!(RecipientId.IsValid() && RecipientId.GetUniqueNetId().IsValid()))
    {
        return false;
    }

    auto __Result = Handle->IsChatAllowed(
        UserId.GetUniqueNetId().ToSharedRef().Get(),
        RecipientId.GetUniqueNetId().ToSharedRef().Get());

    return __Result;
}

void UOnlineChatSubsystem::GetJoinedRooms(const FUniqueNetIdRepl &UserId, TArray<FString> &OutRooms)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return;
    }
    if (!(UserId.IsValid() && UserId.GetUniqueNetId().IsValid()))
    {
        return;
    }

    TArray<FChatRoomId> __StoreTemp__OutRooms;

    Handle->GetJoinedRooms(UserId.GetUniqueNetId().ToSharedRef().Get(), __StoreTemp__OutRooms);
    OutRooms = ConvertArrayElements<FChatRoomId, FString>(__StoreTemp__OutRooms, [](const FChatRoomId &Val) {
        return Val;
    });
}

FChatRoomInfoBP UOnlineChatSubsystem::GetRoomInfo(const FUniqueNetIdRepl &UserId, const FString &RoomId)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return FChatRoomInfoBP();
    }
    if (!(UserId.IsValid() && UserId.GetUniqueNetId().IsValid()))
    {
        return FChatRoomInfoBP();
    }

    auto __Result =
        FChatRoomInfoBP::FromNative(Handle->GetRoomInfo(UserId.GetUniqueNetId().ToSharedRef().Get(), RoomId));

    return __Result;
}

bool UOnlineChatSubsystem::GetMembers(
    const FUniqueNetIdRepl &UserId,
    const FString &RoomId,
    TArray<FChatRoomMemberBP> &OutMembers)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }
    if (!(UserId.IsValid() && UserId.GetUniqueNetId().IsValid()))
    {
        return false;
    }

    TArray<TSharedRef<FChatRoomMember>> __StoreTemp__OutMembers;

    auto __Result = Handle->GetMembers(UserId.GetUniqueNetId().ToSharedRef().Get(), RoomId, __StoreTemp__OutMembers);
    OutMembers = ConvertArrayElements<TSharedRef<FChatRoomMember>, FChatRoomMemberBP>(
        __StoreTemp__OutMembers,
        [](const TSharedRef<FChatRoomMember> &Val) {
            return FChatRoomMemberBP::FromNative(*Val);
        });

    return __Result;
}

FChatRoomMemberBP UOnlineChatSubsystem::GetMember(
    const FUniqueNetIdRepl &UserId,
    const FString &RoomId,
    const FUniqueNetIdRepl &MemberId)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return FChatRoomMemberBP();
    }
    if (!(UserId.IsValid() && UserId.GetUniqueNetId().IsValid()))
    {
        return FChatRoomMemberBP();
    }
    if (!(MemberId.IsValid() && MemberId.GetUniqueNetId().IsValid()))
    {
        return FChatRoomMemberBP();
    }

    auto __Result = FChatRoomMemberBP::FromNative(Handle->GetMember(
        UserId.GetUniqueNetId().ToSharedRef().Get(),
        RoomId,
        MemberId.GetUniqueNetId().ToSharedRef().Get()));

    return __Result;
}

bool UOnlineChatSubsystem::GetLastMessages(
    const FUniqueNetIdRepl &UserId,
    const FString &RoomId,
    int32 NumMessages,
    TArray<FChatMessageBP> &OutMessages)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }
    if (!(UserId.IsValid() && UserId.GetUniqueNetId().IsValid()))
    {
        return false;
    }

    TArray<TSharedRef<FChatMessage>> __StoreTemp__OutMessages;

    auto __Result = Handle->GetLastMessages(
        UserId.GetUniqueNetId().ToSharedRef().Get(),
        RoomId,
        NumMessages,
        __StoreTemp__OutMessages);
    OutMessages = ConvertArrayElements<TSharedRef<FChatMessage>, FChatMessageBP>(
        __StoreTemp__OutMessages,
        [](const TSharedRef<FChatMessage> &Val) {
            return FChatMessageBP::FromNative(*Val);
        });

    return __Result;
}

void UOnlineChatSubsystem::DumpChatState()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return;
    }

    Handle->DumpChatState();
}

PRAGMA_ENABLE_DEPRECATION_WARNINGS