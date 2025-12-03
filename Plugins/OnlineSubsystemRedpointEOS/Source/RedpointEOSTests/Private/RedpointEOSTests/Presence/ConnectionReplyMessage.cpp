// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSIdentity/IdentityUserId.h"
#include "RedpointEOSPresence/PresenceSystem.h"
#include "RedpointEOSTests/TestUtilities/AutomationMacros.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "RedpointEOSTests/TestUtilities/LambdaCommand.h"

namespace REDPOINT_EOS_FILE_NS_ID(395217008, Redpoint::EOS::Tests::Presence)
{
using namespace ::Redpoint::EOS;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Identity;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Presence_ConnectionReplyMessage,
    "Redpoint.EOS.Presence.ConnectionReplyMessage",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_Presence_ConnectionReplyMessage::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
    using namespace ::Redpoint::EOS::Presence;
    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::Rooms;

    TSharedRef<FChainedTaskExecutor> Executor =
        MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1, 1}));

    // Data storage.
    TSharedRef<TSharedPtr<IPresenceMessageConnection>> EstablishedClientConnection =
        MakeShared<TSharedPtr<IPresenceMessageConnection>>();
    TSharedRef<TSharedPtr<IPresenceMessageConnection>> EstablishedHostConnection =
        MakeShared<TSharedPtr<IPresenceMessageConnection>>();
    TSharedRef<TOptional<TTuple<FName, TSharedPtr<FJsonValue>>>> ReceivedMessageOnHost =
        MakeShared<TOptional<TTuple<FName, TSharedPtr<FJsonValue>>>>();
    TSharedRef<TOptional<TTuple<FName, TSharedPtr<FJsonValue>>>> ReceivedMessageOnClient =
        MakeShared<TOptional<TTuple<FName, TSharedPtr<FJsonValue>>>>();

    // Register event handler so that we can grab the host connection when it is made.
    Executor->Then<FLambdaChainedTask>([ReceivedMessageOnHost, EstablishedHostConnection](
                                           const FChainedTaskContextRef &Context,
                                           const FLambdaChainedTask::FOnComplete &OnDone) {
        auto PresenceSystem =
            ((FOnlineSubsystemEOS &)Context->Instance(1)).GetPlatformHandle()->GetSystem<IPresenceSystem>();

        PresenceSystem->OnInboundMessageConnectionEstablished(FName(TEXT("AutomatedTests")))
            .AddLambda([ReceivedMessageOnHost,
                        EstablishedHostConnection](const TSharedRef<IPresenceMessageConnection> &MessageConnection) {
                *EstablishedHostConnection = MessageConnection;
                MessageConnection->OnMessageReceivedOrClose().AddLambda(
                    [ReceivedMessageOnHost](
                        bool bIsClosed,
                        const FName &MessageType,
                        const TSharedPtr<FJsonValue> &Message,
                        const IPresenceMessageConnection::FAckMessage &AckMessage) {
                        if (!bIsClosed)
                        {
                            *ReceivedMessageOnHost = TTuple<FName, TSharedPtr<FJsonValue>>(MessageType, Message);
                        }
                        AckMessage.ExecuteIfBound();
                    });
            });

        OnDone.ExecuteIfBound(true);
    });

    // Establish a connection.
    Executor->Then<FLambdaChainedTask>([EstablishedClientConnection, ReceivedMessageOnClient](
                                           const FChainedTaskContextRef &Context,
                                           const FLambdaChainedTask::FOnComplete &OnDone) {
        auto PresenceSystem =
            ((FOnlineSubsystemEOS &)Context->Instance(0)).GetPlatformHandle()->GetSystem<IPresenceSystem>();

        // Open a connection to the second user, then close it.
        PresenceSystem->TryOpenMessageConnection(
            GetAccountId(Context->User(0)),
            GetAccountId(Context->User(1)),
            FName(TEXT("AutomatedTests")),
            IPresenceSystem::FOnTryOpenMessageConnection::CreateLambda(
                [Context, OnDone, EstablishedClientConnection, ReceivedMessageOnClient](
                    const FError &ErrorCode,
                    const TSharedPtr<IPresenceMessageConnection> &MessageConnection) {
                    Context->Assert().TestTrue(
                        FString::Printf(
                            TEXT("Expected TryOpenMessageConnection operation to succeed."),
                            *ErrorCode.ToLogString()),
                        ErrorCode.WasSuccessful());
                    if (Context->Assert().TestTrue(
                            TEXT("Expected MessageConnection to not be null."),
                            MessageConnection.IsValid()))
                    {
                        *EstablishedClientConnection = MessageConnection.ToSharedRef();
                        MessageConnection->OnMessageReceivedOrClose().AddLambda(
                            [ReceivedMessageOnClient](
                                bool bIsClosed,
                                const FName &MessageType,
                                const TSharedPtr<FJsonValue> &Message,
                                const IPresenceMessageConnection::FAckMessage &AckMessage) {
                                if (!bIsClosed)
                                {
                                    *ReceivedMessageOnClient =
                                        TTuple<FName, TSharedPtr<FJsonValue>>(MessageType, Message);
                                }
                                AckMessage.ExecuteIfBound();
                            });
                        OnDone.ExecuteIfBound(true);
                    }
                    else
                    {
                        OnDone.ExecuteIfBound(false);
                    }
                }));
    });

    // Send a message from the client to the host.
    Executor->Then<FLambdaChainedTask>([EstablishedClientConnection, ReceivedMessageOnHost](
                                           const FChainedTaskContextRef &Context,
                                           const FLambdaChainedTask::FOnComplete &OnDone) {
        (*EstablishedClientConnection)
            ->SendMessage(
                NAME_Game,
                MakeShared<FJsonValueString>("Hello World"),
                IPresenceMessageConnection::FOnSendMessageComplete::CreateLambda(
                    [ReceivedMessageOnHost, Context, OnDone](const FError &Error) {
                        Context->Assert().TestTrue(
                            FString::Printf(TEXT("Expected SendMessage operation to succeed."), *Error.ToLogString()),
                            Error.WasSuccessful());
                        if (Context->Assert().TestTrue(
                                TEXT("Expected ReceivedMessageOnHost to be set."),
                                ReceivedMessageOnHost->IsSet()))
                        {
                            Context->Assert().TestTrue(
                                TEXT("ReceivedMessageOnHost MessageType"),
                                ReceivedMessageOnHost->GetValue().Key == NAME_Game);
                            if (Context->Assert().TestTrue(
                                    TEXT("ReceivedMessageOnHost MessageContent Type"),
                                    ReceivedMessageOnHost->GetValue().Value->Type == EJson::String))
                            {
                                Context->Assert().TestTrue(
                                    TEXT("ReceivedMessageOnHost MessageContent"),
                                    ReceivedMessageOnHost->GetValue().Value->AsString() == TEXT("Hello World"));
                            }
                        }
                        OnDone.ExecuteIfBound(true);
                    }));
    });

    // Send a message from the host to the client.
    Executor->Then<FLambdaChainedTask>([EstablishedHostConnection, ReceivedMessageOnClient](
                                           const FChainedTaskContextRef &Context,
                                           const FLambdaChainedTask::FOnComplete &OnDone) {
        (*EstablishedHostConnection)
            ->SendMessage(
                NAME_Game,
                MakeShared<FJsonValueString>("Client World"),
                IPresenceMessageConnection::FOnSendMessageComplete::CreateLambda(
                    [ReceivedMessageOnClient, Context, OnDone](const FError &Error) {
                        Context->Assert().TestTrue(
                            FString::Printf(TEXT("Expected SendMessage operation to succeed."), *Error.ToLogString()),
                            Error.WasSuccessful());
                        if (Context->Assert().TestTrue(
                                TEXT("Expected ReceivedMessageOnClient to be set."),
                                ReceivedMessageOnClient->IsSet()))
                        {
                            Context->Assert().TestTrue(
                                TEXT("ReceivedMessageOnClient MessageType"),
                                ReceivedMessageOnClient->GetValue().Key == NAME_Game);
                            if (Context->Assert().TestTrue(
                                    TEXT("ReceivedMessageOnClient MessageContent Type"),
                                    ReceivedMessageOnClient->GetValue().Value->Type == EJson::String))
                            {
                                Context->Assert().TestTrue(
                                    TEXT("ReceivedMessageOnClient MessageContent"),
                                    ReceivedMessageOnClient->GetValue().Value->AsString() == TEXT("Client World"));
                            }
                        }
                        OnDone.ExecuteIfBound(true);
                    }));
    });

    // Close the connection on the host side.
    Executor->Then<FLambdaChainedTask>([EstablishedHostConnection](
                                           const FChainedTaskContextRef &Context,
                                           const FLambdaChainedTask::FOnComplete &OnDone) {
        (*EstablishedHostConnection)
            ->Close(IPresenceMessageConnection::FOnCloseComplete::CreateLambda(
                [Context, OnDone](const FError &CloseErrorCode) {
                    Context->Assert().TestTrue(
                        FString::Printf(TEXT("Expected Close operation to succeed."), *CloseErrorCode.ToLogString()),
                        CloseErrorCode.WasSuccessful());
                    OnDone.ExecuteIfBound(true);
                }));
    });

    // Run the test.
    Executor->Execute();
}

}