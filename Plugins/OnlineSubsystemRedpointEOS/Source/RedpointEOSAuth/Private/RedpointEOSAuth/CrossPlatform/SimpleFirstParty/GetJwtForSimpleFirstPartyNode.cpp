// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/CrossPlatform/SimpleFirstParty/GetJwtForSimpleFirstPartyNode.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSAuth/Logging.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSCore/Legacy/EOSString.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonTypes.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1215427675, Redpoint::EOS::Auth::CrossPlatform::SimpleFirstParty)
{
using namespace ::Redpoint::EOS::Auth;

void FGetJwtForSimpleFirstPartyNode::OnHttpResponse(
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
        State->ErrorMessages.Add(TEXT("Unable to connect to login URL for authentication."));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }

    if (Response->GetResponseCode() != EHttpResponseCodes::Ok)
    {
        State->ErrorMessages.Add(TEXT("Non-200 response from login URL when obtaining JWT for authentication."));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }

    TSharedPtr<FJsonValue> JsonValue;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
    if (!FJsonSerializer::Deserialize(Reader, JsonValue))
    {
        State->ErrorMessages.Add(FString::Printf(
            TEXT("Response data from login URL wasn't JSON, got: '%s'."),
            *Response->GetContentAsString()));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }

    const TSharedPtr<FJsonObject> *JsonObject = nullptr;
    if (!JsonValue.IsValid() || !JsonValue->TryGetObject(JsonObject))
    {
        State->ErrorMessages.Add(FString::Printf(
            TEXT("Response data from login URL wasn't expected JSON format, got: '%s'."),
            *Response->GetContentAsString()));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }

    if (!(*JsonObject)->HasTypedField<EJson::String>(TEXT("access_token")) ||
        !(*JsonObject)->HasTypedField<EJson::String>(TEXT("user_id")))
    {
        State->ErrorMessages.Add(FString::Printf(
            TEXT("Response data from login URL wasn't expected JSON format, got: '%s'."),
            *Response->GetContentAsString()));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }

    FString AccessToken = (*JsonObject)->GetStringField(TEXT("access_token"));
    FString UserId = (*JsonObject)->GetStringField(TEXT("user_id"));
    if (FCString::Atoi64(*UserId) == 0)
    {
        State->ErrorMessages.Add(
            TEXT("Got response from login URL, but it indicated a user ID of 0 which is not valid. "
                 "User IDs are non-zero 64-bit integers."));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }
    State->Metadata.Add(TEXT("FIRST_PARTY_ACCESS_TOKEN"), AccessToken);
    State->Metadata.Add(TEXT("FIRST_PARTY_USER_ID"), UserId);

    UE_LOG(
        LogRedpointEOSAuth,
        Verbose,
        TEXT("Obtained data for first party login, user ID: '%s', access token: '%s'"),
        *UserId,
        *AccessToken);
    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
    return;
}

void FGetJwtForSimpleFirstPartyNode::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
    FString FirstPartyUrl = State->Config->GetSimpleFirstPartyLoginUrl();
    if (FirstPartyUrl.IsEmpty())
    {
        State->ErrorMessages.Add(TEXT("First party login URL not configured in Project Settings."));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }

    auto HttpRequest = FHttpModule::Get().CreateRequest();

    HttpRequest->SetVerb("POST");
    HttpRequest->SetHeader("Content-Type", "application/x-www-form-urlencoded");
    HttpRequest->SetURL(FirstPartyUrl);
    HttpRequest->SetContentAsString(FString::Printf(
        TEXT("username=%s&password=%s"),
        *FPlatformHttp::UrlEncode(State->ProvidedCredentials.Id),
        *FPlatformHttp::UrlEncode(State->ProvidedCredentials.Token)));
    HttpRequest->OnProcessRequestComplete()
        .BindSP(this, &FGetJwtForSimpleFirstPartyNode::OnHttpResponse, State, OnDone);
    if (!HttpRequest->ProcessRequest())
    {
        State->ErrorMessages.Add(TEXT("Unable to start login request for first party login."));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()