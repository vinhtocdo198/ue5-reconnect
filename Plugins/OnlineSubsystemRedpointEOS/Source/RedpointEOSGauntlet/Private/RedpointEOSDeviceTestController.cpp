// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSDeviceTestController.h"

#include "Engine/Engine.h"
#include "Engine/GameEngine.h"
#include "HAL/PlatformMisc.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlinePartyInterface.h"
#include "Misc/CommandLine.h"
#include "OnlineSubsystem.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

DECLARE_LOG_CATEGORY_EXTERN(LogRedpointEOSGauntlet, VeryVerbose, All);
DEFINE_LOG_CATEGORY(LogRedpointEOSGauntlet);

static ::Redpoint::EOS::Async::TTask<int32>::With<::Redpoint::EOS::Async::ETaskBinding::Static> SignIntoOSSv1(
    int32 LocalUserNum)
{
    using namespace ::Redpoint::EOS::Async;

    auto *OSSv1 = IOnlineSubsystem::Get();
    if (OSSv1 == nullptr)
    {
        UE_LOG(LogRedpointEOSGauntlet, Error, TEXT("OSSv1 not available."));
        co_return 1;
    }
    if (!OSSv1->GetIdentityInterface().IsValid())
    {
        UE_LOG(LogRedpointEOSGauntlet, Error, TEXT("OSSv1 identity interface not available."));
        co_return 1;
    }

    auto LoginTask = TTask<bool>::Deferred();

    auto LoginEventHandle = OSSv1->GetIdentityInterface()->AddOnLoginCompleteDelegate_Handle(
        LocalUserNum,
        FOnLoginCompleteDelegate::CreateLambda(
            [LocalUserNum,
             LoginTask](int32 CbLocalUserNum, bool bWasSuccessful, const FUniqueNetId &UserId, const FString &Error) {
                if (CbLocalUserNum == LocalUserNum)
                {
                    if (!bWasSuccessful)
                    {
                        UE_LOG(LogRedpointEOSGauntlet, Error, TEXT("OSSv1 login failure: %s"), *Error);
                    }
                    LoginTask.SetValue(bWasSuccessful);
                }
            }));

    if (!OSSv1->GetIdentityInterface()->AutoLogin(LocalUserNum))
    {
        OSSv1->GetIdentityInterface()->ClearOnLoginCompleteDelegate_Handle(LocalUserNum, LoginEventHandle);
        UE_LOG(LogRedpointEOSGauntlet, Error, TEXT("OSSv1 login call failed to start."));
        co_return 1;
    }

    auto bLoginSuccessful = co_await LoginTask;

    OSSv1->GetIdentityInterface()->ClearOnLoginCompleteDelegate_Handle(LocalUserNum, LoginEventHandle);

    if (!bLoginSuccessful)
    {
        UE_LOG(LogRedpointEOSGauntlet, Error, TEXT("OSSv1 login call failed to start."));
        co_return 1;
    }

    co_return 0;
}

static ::Redpoint::EOS::Async::TTask<void> Delay(float Seconds)
{
    using namespace ::Redpoint::EOS::Async;

    auto Result = TTask<void>::Deferred();
    FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateLambda([Result](float) -> bool {
            Result.SetValue();
            return false;
        }),
        Seconds);
    return Result;
}

static ::Redpoint::EOS::Async::TTask<int32>::With<::Redpoint::EOS::Async::ETaskBinding::Static>
WaitForPartyToBePresent()
{
    using namespace ::Redpoint::EOS::Async;

    UE_LOG(LogRedpointEOSGauntlet, Verbose, TEXT("[EVENT] Waiting for the user to be in a party..."));

    do
    {
        auto *OSSv1 = IOnlineSubsystem::Get();
        if (OSSv1 == nullptr)
        {
            UE_LOG(LogRedpointEOSGauntlet, Error, TEXT("OSSv1 not available."));
            co_return 1;
        }
        if (!OSSv1->GetPartyInterface().IsValid())
        {
            UE_LOG(LogRedpointEOSGauntlet, Error, TEXT("OSSv1 party interface not available."));
            co_return 1;
        }
        if (!OSSv1->GetIdentityInterface().IsValid())
        {
            UE_LOG(LogRedpointEOSGauntlet, Error, TEXT("OSSv1 identity interface not available."));
            co_return 1;
        }

        TArray<TSharedRef<const FOnlinePartyId>> JoinedParties;
        OSSv1->GetPartyInterface()->GetJoinedParties(
            *OSSv1->GetIdentityInterface()->GetUniquePlayerId(0),
            JoinedParties);
        if (JoinedParties.Num() > 0)
        {
            UE_LOG(LogRedpointEOSGauntlet, Verbose, TEXT("[EVENT] User has been joined to a party!"));

            co_await Delay(5.0);

            co_return 0;
        }

        co_await Delay(1.0f);
    } while (true);
}

static ::Redpoint::EOS::Async::TTask<int32>::With<::Redpoint::EOS::Async::ETaskBinding::Static>
CreatePartyInviteFriendsAndWaitForJoin(FString InviteFriendDisplayName)
{
    using namespace ::Redpoint::EOS::Async;

    UE_LOG(LogRedpointEOSGauntlet, Verbose, TEXT("Creating a party..."));

    auto *OSSv1 = IOnlineSubsystem::Get();
    if (OSSv1 == nullptr)
    {
        UE_LOG(LogRedpointEOSGauntlet, Error, TEXT("OSSv1 not available."));
        co_return 1;
    }
    if (!OSSv1->GetPartyInterface().IsValid())
    {
        UE_LOG(LogRedpointEOSGauntlet, Error, TEXT("OSSv1 party interface not available."));
        co_return 1;
    }
    if (!OSSv1->GetIdentityInterface().IsValid())
    {
        UE_LOG(LogRedpointEOSGauntlet, Error, TEXT("OSSv1 identity interface not available."));
        co_return 1;
    }
    if (!OSSv1->GetFriendsInterface().IsValid())
    {
        UE_LOG(LogRedpointEOSGauntlet, Error, TEXT("OSSv1 friends interface not available."));
        co_return 1;
    }

    auto IdentityInterface = OSSv1->GetIdentityInterface();
    auto PartySystem = OSSv1->GetPartyInterface();
    auto FriendsInterface = OSSv1->GetFriendsInterface();

    FPartyConfiguration PartyConfiguration;
    PartyConfiguration.JoinRequestAction = EJoinRequestAction::AutoApprove;
    PartyConfiguration.PresencePermissions = PartySystemPermissions::EPermissionType::Anyone;
    PartyConfiguration.InvitePermissions = PartySystemPermissions::EPermissionType::Anyone;
    PartyConfiguration.bChatEnabled = false;
    PartyConfiguration.bShouldRemoveOnDisconnection = false;
    PartyConfiguration.bIsAcceptingMembers = true;
    PartyConfiguration.NotAcceptingMembersReason = 0;
    PartyConfiguration.MaxMembers = 4;

    auto CreatePartyResult = TTask<TTuple<TSharedPtr<const FOnlinePartyId>, ECreatePartyCompletionResult>>::Deferred();
    if (!PartySystem->CreateParty(
            *IdentityInterface->GetUniquePlayerId(0),
            PartySystem->GetPrimaryPartyTypeId(),
            PartyConfiguration,
            FOnCreatePartyComplete::CreateLambda([CreatePartyResult](
                                                     const FUniqueNetId &LocalUserId,
                                                     const TSharedPtr<const FOnlinePartyId> &PartyId,
                                                     const ECreatePartyCompletionResult Result) {
                CreatePartyResult.SetValue(
                    TTuple<TSharedPtr<const FOnlinePartyId>, ECreatePartyCompletionResult>(PartyId, Result));
            })))
    {
        UE_LOG(LogRedpointEOSGauntlet, Error, TEXT("OSSv1 create party call failed to start."));
        co_return 1;
    }

    auto [PartyId, CreatePartyResultCode] = co_await CreatePartyResult;

    if (CreatePartyResultCode != ECreatePartyCompletionResult::Succeeded)
    {
        UE_LOG(LogRedpointEOSGauntlet, Error, TEXT("Failed to create party!"));
        co_return 1;
    }

    TArray<TSharedRef<FOnlineFriend>> Friends;
    FriendsInterface->GetFriendsList(0, TEXT(""), Friends);
    if (Friends.Num() == 0)
    {
        UE_LOG(LogRedpointEOSGauntlet, Error, TEXT("Player doesn't have any friends, this test requires friends!"));
        co_return 1;
    }

    bool bFoundFriend = false;
    for (const auto &Friend : Friends)
    {
        UE_LOG(
            LogRedpointEOSGauntlet,
            Verbose,
            TEXT("Detected friend ID '%s' with display name '%s'."),
            *Friend->GetUserId()->ToString(),
            *Friend->GetDisplayName());

        if (Friend->GetDisplayName() != InviteFriendDisplayName)
        {
            continue;
        }

        auto SendInviteResult = TTask<ESendPartyInvitationCompletionResult>::Deferred();

        if (!PartySystem->SendInvitation(
                *IdentityInterface->GetUniquePlayerId(0),
                *PartyId,
                FPartyInvitationRecipient(*Friend->GetUserId()),
                FOnSendPartyInvitationComplete::CreateLambda([SendInviteResult](
                                                                 const FUniqueNetId &,
                                                                 const FOnlinePartyId &,
                                                                 const FUniqueNetId &,
                                                                 const ESendPartyInvitationCompletionResult Result) {
                    SendInviteResult.SetValue(Result);
                })))
        {
            UE_LOG(LogRedpointEOSGauntlet, Error, TEXT("SendInvitation call failed to start!"));
            co_return 1;
        }

        if (co_await SendInviteResult != ESendPartyInvitationCompletionResult::Succeeded)
        {
            UE_LOG(LogRedpointEOSGauntlet, Error, TEXT("Failed to send invite to friend!"));
            co_return 1;
        }

        bFoundFriend = true;
    }

    if (!bFoundFriend)
    {
        UE_LOG(
            LogRedpointEOSGauntlet,
            Error,
            TEXT("Player doesn't have a friend with display name '%s'!"),
            *InviteFriendDisplayName);
        co_return 1;
    }

    UE_LOG(LogRedpointEOSGauntlet, Verbose, TEXT("[EVENT] Waiting for another player to join the party..."));

    do
    {
        TArray<TSharedRef<const FOnlinePartyId>> JoinedParties;
        auto MemberCount = PartySystem->GetPartyMemberCount(*IdentityInterface->GetUniquePlayerId(0), *PartyId);
        if (MemberCount != 1)
        {
            if (MemberCount == 0)
            {
                UE_LOG(LogRedpointEOSGauntlet, Error, TEXT("Somehow we left the party!"));
                co_return 1;
            }
            else
            {
                UE_LOG(
                    LogRedpointEOSGauntlet,
                    Verbose,
                    TEXT("[EVENT] Detected another user has successfully joined the party!"));

                co_await Delay(5.0);

                co_return 0;
            }
        }

        co_await Delay(1.0f);
    } while (true);
}

static ::Redpoint::EOS::Async::TTask<int32>::With<::Redpoint::EOS::Async::ETaskBinding::Static> RunCommandlet(
    FString Params)
{
    using namespace ::Redpoint::EOS::Async;

    FString TestRole;
    if (!FParse::Value(*Params, TEXT("-eostestrole="), TestRole))
    {
        UE_LOG(LogRedpointEOSGauntlet, Error, TEXT("Missing -eostestrole= parameter."));
        co_return 1;
    }

    UE_LOG(LogRedpointEOSGauntlet, Verbose, TEXT("Signing into OSSv1 as local user 0..."));

    auto ExitCode = co_await SignIntoOSSv1(0);
    if (ExitCode != 0)
    {
        co_return ExitCode;
    }

    if (TestRole == TEXT("InviteToParty"))
    {
        FString InviteFriendDisplayName;
        if (!FParse::Value(*Params, TEXT("-invitefrienddisplayname="), InviteFriendDisplayName))
        {
            UE_LOG(LogRedpointEOSGauntlet, Error, TEXT("Missing -invitefrienddisplayname= parameter."));
            co_return 1;
        }

        ExitCode = co_await CreatePartyInviteFriendsAndWaitForJoin(InviteFriendDisplayName);
        if (ExitCode != 0)
        {
            co_return ExitCode;
        }
    }
    else if (TestRole == TEXT("WaitForPartyJoin"))
    {
        ExitCode = co_await WaitForPartyToBePresent();
        if (ExitCode != 0)
        {
            co_return ExitCode;
        }
    }

    UE_LOG(LogRedpointEOSGauntlet, Verbose, TEXT("Testing completed successfully."));
    co_return 0;
}

void URedpointEOSDeviceTestController::OnInit()
{
    using namespace ::Redpoint::EOS::Async;

    this->CompletionTask = MakeShared<TTask<int32>>(RunCommandlet(FCommandLine::Get()));
}

void URedpointEOSDeviceTestController::OnTick(float TimeDelta)
{
    if (this->CompletionTask.IsValid() && this->CompletionTask->IsReady())
    {
        UGauntletTestController::EndTest(this->CompletionTask->await_resume());
    }
}

REDPOINT_EOS_CODE_GUARD_END()