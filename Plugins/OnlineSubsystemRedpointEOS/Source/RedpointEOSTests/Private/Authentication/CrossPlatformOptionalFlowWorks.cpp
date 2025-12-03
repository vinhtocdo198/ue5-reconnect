// Copyright June Rhodes. All Rights Reserved.

#if 0 // @todo Restore these tests

#include "Interfaces/OnlineIdentityInterface.h"
#include "Misc/AutomationTest.h"
#include "OnlineSubsystemRedpointEOSModule.h"
#include "RedpointEOSAuth/Nodes/PromptToSignInOrCreateAccountNode.h"
#include "RedpointEOSAuth/Nodes/PromptToSignInOrCreateAccountNode_AutomationSetting.h"
#include "RedpointEOSTestsModule.h"
#include "TestHelpers.h"

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Authentication_CrossPlatformOptionalFlowWorks,
    "Redpoint.EOS.Authentication.CrossPlatformOptionalFlowWorks",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::ProductFilter);

class REDPOINTEOSTESTS_API FEOSConfigAuthTestOptionalLogin : public FEOSConfigEASLogin
{
public:
    FEOSConfigAuthTestOptionalLogin(){};
    UE_NONCOPYABLE(FEOSConfigAuthTestOptionalLogin);
    virtual ~FEOSConfigAuthTestOptionalLogin(){};

    virtual FName GetAuthenticationGraph() const override
    {
        return FName(TEXT("AutomatedTestingOSS"));
    }
    virtual FName GetCrossPlatformAccountProvider() const override
    {
        return FName(TEXT("AutomatedTesting"));
    }
    virtual bool GetRequireCrossPlatformAccount() const override
    {
        return false;
    }
    virtual bool IsAutomatedTesting() const override
    {
        return true;
    }
};

void FOnlineSubsystemEOS_Authentication_CrossPlatformOptionalFlowWorks::RunAsyncTest(const TFunction<void()> &OnDone)
{
    FOnlineSubsystemRedpointEOSModule &OSSModule =
        FModuleManager::GetModuleChecked<FOnlineSubsystemRedpointEOSModule>(FName(TEXT("OnlineSubsystemRedpointEOS")));

    auto Subsystem = OSSModule.CreateSubsystem(
        FName(*FString::Printf(TEXT("%s"), *this->GetTestName())),
        MakeShared<FEOSConfigAuthTestOptionalLogin>());
    check(Subsystem);

    this->RegisteredSubsystems.Add(Subsystem);

    auto Identity = Subsystem->GetIdentityInterface();
    check(Identity);

    this->AddExpectedError("CPAT-02");
    this->AddExpectedError("CPAT-03", EAutomationExpectedErrorFlags::MatchType::Contains, 2);
    this->AddExpectedError("CPAT-04");
    this->AddExpectedError("CPAT-05");

    auto CancelLogin = RegisterOSSCallback(
        this,
        Identity,
        0,
        &IOnlineIdentity::AddOnLoginCompleteDelegate_Handle,
        &IOnlineIdentity::ClearOnLoginCompleteDelegate_Handle,
        TFunction<void(int32, bool, const FUniqueNetId &, const FString &)>(
            [this,
             // NOLINTNEXTLINE(unreal-unsafe-storage-of-oss-pointer)
             Subsystem,
             // NOLINTNEXTLINE(unreal-unsafe-storage-of-oss-pointer)
             Identity,
             OnDone](int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId &UserId, const FString &Error) {
                if (!bWasSuccessful)
                {
                    this->AddError(TEXT("Failed to authenticate!"));
                    OnDone();
                    return;
                }

                auto Account = Identity->GetUserAccount(UserId);
                if (!this->TestTrue(TEXT("Expected account to exist"), Account.IsValid()))
                {
                    OnDone();
                    return;
                }
                FString UsedJwt;
                Account->GetAuthAttribute(TEXT("automatedTesting.jwt"), UsedJwt);

                // Do the "upgrade flow".
                auto CancelUpgrade = RegisterOSSCallback(
                    this,
                    Identity,
                    0,
                    &IOnlineIdentity::AddOnLoginCompleteDelegate_Handle,
                    &IOnlineIdentity::ClearOnLoginCompleteDelegate_Handle,
                    TFunction<void(int32, bool, const FUniqueNetId &, const FString &)>(
                        [this,
                         // NOLINTNEXTLINE(unreal-unsafe-storage-of-oss-pointer)
                         Subsystem,
                         // NOLINTNEXTLINE(unreal-unsafe-storage-of-oss-pointer)
                         Identity,
                         UsedJwt,
                         OnDone](
                            int32 LocalUserNum,
                            bool bWasSuccessful,
                            const FUniqueNetId &UserId,
                            const FString &Error) {
                            if (!bWasSuccessful)
                            {
                                this->AddError(TEXT("Failed to authenticate (upgrade)!"));
                                OnDone();
                                return;
                            }

                            // Logout so we can do another login later.
                            auto CancelLogout = RegisterOSSCallback(
                                this,
                                Identity,
                                0,
                                &IOnlineIdentity::AddOnLogoutCompleteDelegate_Handle,
                                &IOnlineIdentity::ClearOnLogoutCompleteDelegate_Handle,
                                TFunction<void(int32, bool)>([this,
                                                              // NOLINTNEXTLINE(unreal-unsafe-storage-of-oss-pointer)
                                                              Subsystem,
                                                              // NOLINTNEXTLINE(unreal-unsafe-storage-of-oss-pointer)
                                                              Identity,
                                                              UsedJwt,
                                                              OnDone](int32 LocalUserNum, bool bWasSuccessful) {
                                    if (!bWasSuccessful)
                                    {
                                        this->AddError(TEXT("Failed to logout!"));
                                        OnDone();
                                        return;
                                    }

                                    // Now do a login in sign-in mode.
                                    auto CancelLogin = RegisterOSSCallback(
                                        this,
                                        Identity,
                                        0,
                                        &IOnlineIdentity::AddOnLoginCompleteDelegate_Handle,
                                        &IOnlineIdentity::ClearOnLoginCompleteDelegate_Handle,
                                        TFunction<void(int32, bool, const FUniqueNetId &, const FString &)>(
                                            [this,
                                             // NOLINTNEXTLINE(unreal-unsafe-storage-of-oss-pointer)
                                             Subsystem,
                                             // NOLINTNEXTLINE(unreal-unsafe-storage-of-oss-pointer)
                                             Identity,
                                             OnDone](
                                                int32 LocalUserNum,
                                                bool bWasSuccessful,
                                                const FUniqueNetId &UserId,
                                                const FString &Error) {
                                                if (!bWasSuccessful)
                                                {
                                                    this->AddError(TEXT("Failed to authenticate!"));
                                                    OnDone();
                                                    return;
                                                }

                                                OnDone();
                                            }));
                                    Redpoint::EOS::Auth::Nodes::FPromptToSignInOrCreateAccountNode_AutomationSetting::
                                        UserChoice = EEOSUserInterface_SignInOrCreateAccount_Choice::SignIn;
                                    FOnlineAccountCredentials Creds;
                                    Creds.Type = TEXT("AUTOMATED_TESTING_OSS");
                                    Creds.Id = FString::Printf(TEXT("CrossPlatJWT:%s"), *UsedJwt);
                                    Creds.Token = TEXT("");
                                    if (!Identity->Login(0, Creds))
                                    {
                                        CancelLogin();
                                        this->AddError(TEXT("Login call failed to start!"));
                                        OnDone();
                                    }
                                }));
                            if (!Identity->Logout(0))
                            {
                                CancelLogout();
                                this->AddError(TEXT("Logout call failed to start!"));
                                OnDone();
                            }
                        }));
                FOnlineAccountCredentials Creds;
                Creds.Type = TEXT("AUTOMATED_TESTING_OSS");
                Creds.Id = FString::Printf(TEXT("JWT:%s"), *UsedJwt);
                Creds.Token = TEXT("");
                if (!Identity->Login(0, Creds))
                {
                    CancelUpgrade();
                    this->AddError(TEXT("Login (upgrade) call failed to start!"));
                    OnDone();
                }
            }));

    // First step is to create account.
    Redpoint::EOS::Auth::Nodes::FPromptToSignInOrCreateAccountNode_AutomationSetting::UserChoice =
        EEOSUserInterface_SignInOrCreateAccount_Choice::CreateAccount;
    FOnlineAccountCredentials Creds;
    Creds.Type = TEXT("AUTOMATED_TESTING_OSS");
    Creds.Id = FString::Printf(TEXT("CreateOnDemand:%s"), *this->GetTestName());
    Creds.Token = FString::Printf(TEXT("%d"), TestHelpers::Port(0));
    if (!Identity->Login(0, Creds))
    {
        CancelLogin();
        this->AddError(TEXT("Login call failed to start!"));
        OnDone();
    }
}

#endif