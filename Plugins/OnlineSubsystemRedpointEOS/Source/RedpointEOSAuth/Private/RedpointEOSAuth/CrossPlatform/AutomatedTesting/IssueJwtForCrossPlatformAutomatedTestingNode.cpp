// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/CrossPlatform/AutomatedTesting/IssueJwtForCrossPlatformAutomatedTestingNode.h"

#include "Misc/App.h"
#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSAuth/Logging.h"
#include "RedpointEOSConfig/Config.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(788135354, Redpoint::EOS::Auth::CrossPlatform::AutomatedTesting)
{

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

#if !defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING

void FIssueJwtForCrossPlatformAutomatedTestingNode::OnHttpResponse(
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    FHttpRequestPtr Request,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    FHttpResponsePtr Response,
    bool bConnectedSuccessfully,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    TSharedRef<FAuthenticationGraphState> State,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    FAuthenticationGraphNodeOnDone OnDone)
{
    if (!bConnectedSuccessfully)
    {
        State->ErrorMessages.Add(
            TEXT("Unable to connect to licensing.redpoint.games to obtain JWT for automated testing."));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }

    if (Response->GetResponseCode() != EHttpResponseCodes::Ok)
    {
        State->ErrorMessages.Add(
            TEXT("Non-200 response from licensing.redpoint.games when obtaining JWT for automated testing."));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }

    FString JWT = Response->GetContentAsString();
    JWT = JWT.TrimStartAndEnd();
    State->Metadata.Add(TEXT("AUTOMATED_TESTING_JWT"), JWT);
    State->AddResultUserAuthAttribute(TEXT("automatedTesting.crossPlatformJWT"), JWT);
    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
    return;
}

void FIssueJwtForCrossPlatformAutomatedTestingNode::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
    if (State->AutomatedTestingEmailAddress.StartsWith("CrossPlatJWT:"))
    {
        State->Metadata.Add(
            TEXT("AUTOMATED_TESTING_JWT"),
            State->AutomatedTestingEmailAddress.Mid(FString(TEXT("CrossPlatJWT:")).Len()));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
        return;
    }
    if (State->ProvidedCredentials.Type == TEXT("AUTOMATED_TESTING_OSS") &&
        State->ProvidedCredentials.Id.StartsWith("CrossPlatJWT:"))
    {
        State->Metadata.Add(
            TEXT("AUTOMATED_TESTING_JWT"),
            State->ProvidedCredentials.Id.Mid(FString(TEXT("CrossPlatJWT:")).Len()));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
        return;
    }

    auto HttpRequest = FHttpModule::Get().CreateRequest();

    auto TestName = State->AutomatedTestingEmailAddress.Mid(FString(TEXT("CreateOnDemand:")).Len());

    UE_LOG(
        LogRedpointEOSAuth,
        Verbose,
        TEXT("Requesting automation JWT with parameters: HostName=%s_%s_%s, TestName=%s, SubsystemId=%d"),
        TEXT(PREPROCESSOR_TO_STRING(PLATFORM_HEADER_NAME)),
        *FPlatformHttp::UrlEncode(FPlatformMisc::GetLoginId()),
        *FPlatformHttp::UrlEncode(FApp::GetInstanceId().ToString()),
        *FPlatformHttp::UrlEncode(TestName),
        FCString::Atoi(*(State->AutomatedTestingPassword)));

    HttpRequest->SetVerb("POST");
    HttpRequest->SetHeader("Content-Type", "application/x-www-form-urlencoded");
    HttpRequest->SetURL("https://licensing.redpoint.games/api/eos-automated-testing/issue");
    HttpRequest->SetContentAsString(FString::Printf(
        TEXT("hostName=%s_%s_%s&testName=%s&subsystemId=%d"),
        TEXT(PREPROCESSOR_TO_STRING(PLATFORM_HEADER_NAME)),
        *FPlatformHttp::UrlEncode(FPlatformMisc::GetLoginId()),
        *FPlatformHttp::UrlEncode(FApp::GetInstanceId().ToString()),
        *FPlatformHttp::UrlEncode(TestName),
        FCString::Atoi(*(State->AutomatedTestingPassword))));
    HttpRequest->OnProcessRequestComplete()
        .BindSP(this, &FIssueJwtForCrossPlatformAutomatedTestingNode::OnHttpResponse, State, OnDone);
    if (!HttpRequest->ProcessRequest())
    {
        State->ErrorMessages.Add(TEXT("Unable to start HTTP request to obtain JWT for automated testing."));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }
}

#endif

#endif

}

REDPOINT_EOS_CODE_GUARD_END()