// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationItchIo/ItchIoSystem.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_ITCH_IO_ENABLED
#include "Dom/JsonObject.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_ITCH_IO_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(2060926592, Redpoint::EOS::Platform::Integration::ItchIo)
{

FName FItchIoSystem::GetSystemName()
{
    return FName(TEXT("ItchIoSystem"));
}

TSharedRef<FItchIoSystem> FItchIoSystem::ConstructSystem(
    FPlatformHandle InPlatformHandle,
    FPlatformInstanceEnvironmentRef InPlatformInstanceEnvironment)
{
    return MakeShared<FItchIoSystem>();
}

TTask<FError> FItchIoSystem::TryLoginLocalUserFromApiKey()
{
    FString ItchIoApiKey = FPlatformMisc::GetEnvironmentVariable(TEXT("ITCHIO_API_KEY"));
    if (ItchIoApiKey.IsEmpty())
    {
        return TTask<FError>::FromResult(Errors::AccessDenied(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("Can't sign in with itch.io, ITCHIO_API_KEY environment variable is missing.")));
    }

    auto DeferredTask = TTask<FError>::Deferred();

    // Make a request to the itch.io API to get information about the user.
    auto Request = FHttpModule::Get().CreateRequest();
    Request->SetVerb(TEXT("GET"));
    Request->SetHeader(TEXT("Authorization"), ItchIoApiKey);
    Request->SetURL(TEXT("https://itch.io/api/1/jwt/me"));
    Request->OnProcessRequestComplete().BindSPLambda(
        this,
        [this, DeferredTask](const FHttpRequestPtr &_, const FHttpResponsePtr &Response, bool bConnectedSuccessfully) {
            if (!bConnectedSuccessfully || !Response.IsValid() ||
                !EHttpResponseCodes::IsOk(Response->GetResponseCode()))
            {
                DeferredTask.SetValue(Errors::AccessDenied(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    TEXT("Unable to get user information from itch.io API.")));
                return;
            }

            TSharedPtr<FJsonObject> ResponseJson;
            TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
            if (!FJsonSerializer::Deserialize(JsonReader, ResponseJson))
            {
                DeferredTask.SetValue(Errors::UnexpectedError(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    TEXT("Unable to decode response from itch.io API.")));
                return;
            }

            TSharedPtr<FJsonObject> UserInfo = ResponseJson->GetObjectField(TEXT("user"));
            if (!UserInfo.IsValid())
            {
                DeferredTask.SetValue(Errors::UnexpectedError(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    TEXT("Unable to decode response from itch.io API.")));
                return;
            }

            int32 UserId = UserInfo->GetIntegerField(TEXT("id"));
            FString CoverUrl = UserInfo->GetStringField(TEXT("cover_url"));
            FString Username = UserInfo->GetStringField(TEXT("username"));
            FString DisplayName = UserInfo->GetStringField(TEXT("display_name"));

            this->CurrentLocalUser = FItchIoLocalUser(UserId, Username, DisplayName, CoverUrl);
            DeferredTask.SetValue(Errors::Success());
            return;
        });
    verifyf(Request->ProcessRequest(), TEXT("%s: Expected HTTP request to start."), ANSI_TO_TCHAR(__FUNCTION__));

    return DeferredTask;
}

TOptional<FItchIoLocalUser> FItchIoSystem::GetLocalUser()
{
    return this->CurrentLocalUser;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()