// Copyright June Rhodes. All Rights Reserved.

#include "Containers/Ticker.h"
#include "RedpointEOSAPI/Connect/GetLoggedInUsers.h"
#include "RedpointEOSAPI/Platform/Tick.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSCore/InstancePool.h"
#include "RedpointEOSIdentity/IdentityProcessGlobalUserRegistry.h"
#include "RedpointEOSIdentity/IdentitySystem.h"
#include "RedpointEOSIdentity/Requests/LoginRequest.h"
#include "TestHelpers.h"

namespace REDPOINT_EOS_FILE_NS_ID(2414244312, Redpoint::EOS::Tests::Identity)
{
using namespace ::Redpoint::EOS;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::API::Connect;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Identity_LoginAsUser,
    "Redpoint.EOS.Identity.LoginAsUser",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_Identity_LoginAsUser::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Core;
    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::Identity;

    API::FPlatformRefCountedHandle Handle = IInstancePool::Pool().CreateWithConfig(
        FName(*FString::Printf(TEXT("Automation_LoginAsUser_%s"), *FGuid::NewGuid().ToString())),
        MakeShared<FEOSConfigEASLogin>(),
        false);

    FTSTicker::FDelegateHandle TickHandle =
        FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Handle](float DeltaSeconds) {
            API::Platform::FTick::Execute(Handle->Instance());
            return true;
        }));

    // Delay a little bit to allow the platform to startup fully to prevent shutdown from hitching.
    FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateLambda([this, Handle, TickHandle, OnDone](float DeltaSeconds) {
            TSharedRef<IIdentitySystem> IdentitySystem = Handle->Instance()->GetSystem<IIdentitySystem>();

            IdentitySystem->Login(
                Requests::FLoginRequest(
                    0,
                    TEXT("AUTOMATED_TESTING"),
                    TEXT("CreateOnDemand:OnlineSubsystemEOS.Identity.LoginAsUser"),
                    TEXT("Context_0")),
                IIdentitySystem::FOnLoginComplete::CreateLambda([this, Handle, TickHandle, OnDone, IdentitySystem](
                                                                    FError ErrorCode,
                                                                    FIdentityUserPtr NewUser) {
                    this->TestTrue(
                        FString::Printf(TEXT("Expected login to succeed: %s"), *ErrorCode.ToLogString()),
                        ErrorCode.WasSuccessful());
                    this->TestTrue(TEXT("Expected login to return a user object"), NewUser.IsValid());
                    if (NewUser.IsValid())
                    {
                        this->TestNotEqual(
                            TEXT("Expected new user to not have a dedicated server user ID"),
                            NewUser->GetUserId(),
                            GetDedicatedServerAccountId());
                        this->TestEqual(TEXT("Expected new user to have a user slot of 0"), NewUser->GetUserSlot(), 0);
                        this->TestEqual(
                            TEXT("Expected process global handle to find correct user"),
                            FIdentityProcessGlobalUserRegistry::Get(NewUser->GetProcessGlobalUserHandle())
                                .Value.Pin()
                                .Get(),
                            NewUser.Get());
                        this->TestTrue(
                            TEXT(
                                "Expected new user to have a 'friendCode' auth attribute (provided by presence hook)."),
                            NewUser->GetAuthenticationAttributeKeys().Contains("friendCode"));
                        FString FriendCode;
                        this->TestTrue(
                            TEXT("Expected new user's 'friendCode' auth attribute to not be empty (provided by "
                                 "presence hook)."),
                            NewUser->TryGetAuthenticationAttributeValue(TEXT("friendCode"), FriendCode) &&
                                !FriendCode.IsEmpty());
                    }

                    IdentitySystem->Logout(
                        Requests::FLogoutRequest(0),
                        IIdentitySystem::FOnLogoutComplete::CreateLambda(
                            [this, TickHandle, Handle, OnDone](FError ErrorCode) {
                                this->TestTrue(
                                    FString::Printf(TEXT("Expected logout to succeed: %s"), *ErrorCode.ToLogString()),
                                    ErrorCode.WasSuccessful());
                                FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);

                                TArray<EOS_ProductUserId> ExistingUsersResult;
                                EOS_EResult ExistingUsersResultCode;
                                FGetLoggedInUsers::Execute(
                                    Handle->Instance(),
                                    ExistingUsersResultCode,
                                    ExistingUsersResult);

                                this->TestEqual(
                                    TEXT("Expected there to be no logged in users afterwards."),
                                    ExistingUsersResult.Num(),
                                    0);
                                Handle->Instance()->ForceShutdown();
                                OnDone();
                            }));
                }));

            return false;
        }),
        1.0f);
}

}