// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSOrchestrator/AgonesOrchestrator.h"

#if REDPOINT_EOS_ORCHESTRATORS_ENABLED
#include "Interfaces/IHttpResponse.h"
#include "RedpointEOSOrchestrator/Logging.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_ORCHESTRATORS_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(1636037717, Redpoint::EOS::Orchestrator)
{
using namespace ::Redpoint::EOS::Orchestrator;

FAgonesOrchestrator::FAgonesOrchestrator()
    : AgonesHttpServerPort(0)
    , HealthCheckTimerHandle()
    , bHasSentReady(false)
    , bHasSentAllocate(false)
    , bHasSentShutdown(false)
{
}

FAgonesOrchestrator::~FAgonesOrchestrator()
{
    if (this->HealthCheckTimerHandle.IsValid())
    {
        FTSTicker::GetCoreTicker().RemoveTicker(this->HealthCheckTimerHandle);
        this->HealthCheckTimerHandle.Reset();
    }
}

void FAgonesOrchestrator::HandleHealthCheck(
    FHttpRequestPtr HttpRequest,
    const FHttpResponsePtr HttpResponse,
    const bool bSucceeded)
{
    if (!bSucceeded)
    {
        UE_LOG(
            LogRedpointEOSOrchestrator,
            Error,
            TEXT("Agones: Health check failed (generic connection error when trying to contact Agones HTTP sidecar)"));
        return;
    }

    if (!EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
    {
        UE_LOG(
            LogRedpointEOSOrchestrator,
            Error,
            TEXT("Agones: Health check failed (HTTP error code %d from Agones HTTP sidecar)"),
            HttpResponse->GetResponseCode());
        return;
    }
}

bool FAgonesOrchestrator::SendHealthCheck(float DeltaSeconds)
{
    FHttpModule *Http = &FHttpModule::Get();
    FHttpRequestRef Request = Http->CreateRequest();
    Request->SetURL(FString::Printf(TEXT("http://localhost:%d/health"), this->AgonesHttpServerPort));
    Request->SetVerb("POST");
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
    Request->SetHeader(TEXT("Accepts"), TEXT("application/json"));
    Request->SetContentAsString("{}");

    Request->OnProcessRequestComplete().BindSP(this, &FAgonesOrchestrator::HandleHealthCheck);
    Request->ProcessRequest();

    this->HealthCheckTimerHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateSP(this, &FAgonesOrchestrator::SendHealthCheck),
        1.0f);
    return false;
}

bool FAgonesOrchestrator::RetryGetPortMappings(float DeltaSeconds, FOnGetPortMappingsComplete OnComplete)
{
    AsCallback(this->GetPortMappings(), [OnComplete](TArray<FOrchestratorPortMapping> Mappings) {
        OnComplete.ExecuteIfBound(Mappings);
    });
    return false;
}

void FAgonesOrchestrator::HandleGameServer(
    FHttpRequestPtr HttpRequest,
    const FHttpResponsePtr HttpResponse,
    const bool bSucceeded,
    FOnGetPortMappingsComplete OnComplete)
{
    if (!bSucceeded)
    {
        UE_LOG(
            LogRedpointEOSOrchestrator,
            Warning,
            TEXT("Agones: Unable to get game server ports from Agones HTTP sidecar (generic connection failure), "
                 "retrying in 5 seconds..."));
        FTSTicker::GetCoreTicker().AddTicker(
            FTickerDelegate::CreateSP(this, &FAgonesOrchestrator::RetryGetPortMappings, OnComplete),
            5.0f);
        return;
    }

    if (!EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
    {
        UE_LOG(
            LogRedpointEOSOrchestrator,
            Warning,
            TEXT("Agones: Unable to get game server ports from Agones HTTP sidecar (HTTP result code %d), retrying in "
                 "5 seconds..."),
            HttpResponse->GetResponseCode());
        FTSTicker::GetCoreTicker().AddTicker(
            FTickerDelegate::CreateSP(this, &FAgonesOrchestrator::RetryGetPortMappings, OnComplete),
            5.0f);
        return;
    }

    const FString Json = HttpResponse->GetContentAsString();
    TSharedPtr<FJsonObject> JsonObject;
    const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Json);
    if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject.IsValid())
    {
        UE_LOG(
            LogRedpointEOSOrchestrator,
            Verbose,
            TEXT("Agones: Unable to get game server ports from Agones HTTP sidecar (invalid JSON), retrying in 5 "
                 "seconds..."));
        FTSTicker::GetCoreTicker().AddTicker(
            FTickerDelegate::CreateSP(this, &FAgonesOrchestrator::RetryGetPortMappings, OnComplete),
            5.0f);
        return;
    }

    auto StatusObj = JsonObject->GetObjectField(TEXT("status"));
    if (!StatusObj.IsValid() || !StatusObj->HasField(TEXT("ports")))
    {
        UE_LOG(
            LogRedpointEOSOrchestrator,
            Verbose,
            TEXT("Agones: Unable to get game server ports from Agones HTTP sidecar (missing 'status.ports' field in "
                 "JSON), retrying in 5 "
                 "seconds..."));
        UE_LOG(
            LogRedpointEOSOrchestrator,
            Verbose,
            TEXT("Agones: The received game server JSON from the Agones HTTP sidecar was: %s"),
            *Json);
        FTSTicker::GetCoreTicker().AddTicker(
            FTickerDelegate::CreateSP(this, &FAgonesOrchestrator::RetryGetPortMappings, OnComplete),
            5.0f);
        return;
    }

    UE_LOG(
        LogRedpointEOSOrchestrator,
        Verbose,
        TEXT("Agones: Received game server JSON from Agones HTTP sidecar: %s"),
        *Json);

    const TArray<TSharedPtr<FJsonValue>> &PortsArray =
        JsonObject->GetObjectField(TEXT("status"))->GetArrayField(TEXT("ports"));
    TArray<FOrchestratorPortMapping> Results;
    for (const auto &PortValue : PortsArray)
    {
        const auto &PortObject = PortValue->AsObject();

        FName PortName = FName(*PortObject->GetStringField(TEXT("name")));
        int32 PortPort = PortObject->GetIntegerField(TEXT("port"));

        Results.Add(FOrchestratorPortMapping(PortName, (int16)PortPort));
    }
    OnComplete.ExecuteIfBound(Results);
}

void FAgonesOrchestrator::HandleLabel(
    FHttpRequestPtr HttpRequest,
    const FHttpResponsePtr HttpResponse,
    const bool bSucceeded,
    FSimpleDelegate OnComplete)
{
    if (!bSucceeded)
    {
        // Only warn about this, since Agones might not be new enough to support it.
        UE_LOG(
            LogRedpointEOSOrchestrator,
            Warning,
            TEXT("Agones: Unable to label game server with session ID; make sure Agones is on a new enough version"));
    }

    FHttpModule *Http = &FHttpModule::Get();
    FHttpRequestRef Request = Http->CreateRequest();
    Request->SetURL(FString::Printf(TEXT("http://localhost:%d/ready"), this->AgonesHttpServerPort));
    Request->SetVerb("POST");
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
    Request->SetHeader(TEXT("Accepts"), TEXT("application/json"));
    Request->SetContentAsString("{}");

    Request->OnProcessRequestComplete().BindSP(this, &FAgonesOrchestrator::HandleReady, MoveTemp(OnComplete));
    Request->ProcessRequest();
}

void FAgonesOrchestrator::HandleReady(
    FHttpRequestPtr HttpRequest,
    const FHttpResponsePtr HttpResponse,
    const bool bSucceeded,
    FSimpleDelegate OnComplete)
{
    if (!bSucceeded)
    {
        UE_LOG(
            LogRedpointEOSOrchestrator,
            Error,
            TEXT("Agones: Marking game server as ready failed (generic connection error when trying to contact Agones "
                 "HTTP sidecar)"));
        this->bHasSentReady = false;
        OnComplete.ExecuteIfBound();
        return;
    }

    if (!EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
    {
        UE_LOG(
            LogRedpointEOSOrchestrator,
            Error,
            TEXT("Agones: Marking game server as ready failed (HTTP error code %d from Agones HTTP sidecar)"),
            HttpResponse->GetResponseCode());
        this->bHasSentReady = false;
        OnComplete.ExecuteIfBound();
        return;
    }

    UE_LOG(LogRedpointEOSOrchestrator, Verbose, TEXT("Agones: Marked game server as ready for connections"));
    OnComplete.ExecuteIfBound();
}

void FAgonesOrchestrator::HandleAllocate(
    FHttpRequestPtr HttpRequest,
    const FHttpResponsePtr HttpResponse,
    const bool bSucceeded,
    FSimpleDelegate OnComplete)
{
    if (!bSucceeded)
    {
        UE_LOG(
            LogRedpointEOSOrchestrator,
            Error,
            TEXT("Agones: Marking game server as allocated failed (generic connection error when trying to contact "
                 "Agones HTTP sidecar)"));
        this->bHasSentAllocate = false;
        OnComplete.ExecuteIfBound();
        return;
    }

    if (!EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
    {
        UE_LOG(
            LogRedpointEOSOrchestrator,
            Error,
            TEXT("Agones: Marking game server as ready allocated (HTTP error code %d from Agones HTTP sidecar)"),
            HttpResponse->GetResponseCode());
        this->bHasSentAllocate = false;
        OnComplete.ExecuteIfBound();
        return;
    }

    UE_LOG(LogRedpointEOSOrchestrator, Verbose, TEXT("Agones: Marked game server as allocated"));
    OnComplete.ExecuteIfBound();
}

void FAgonesOrchestrator::HandleShutdown(
    FHttpRequestPtr HttpRequest,
    const FHttpResponsePtr HttpResponse,
    const bool bSucceeded,
    FSimpleDelegate OnComplete)
{
    if (!bSucceeded)
    {
        UE_LOG(
            LogRedpointEOSOrchestrator,
            Error,
            TEXT("Agones: Shutting down game server failed (generic connection error when trying to contact "
                 "Agones HTTP sidecar)"));
        this->bHasSentAllocate = false;
        OnComplete.ExecuteIfBound();
        return;
    }

    if (!EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
    {
        UE_LOG(
            LogRedpointEOSOrchestrator,
            Error,
            TEXT("Agones: Shutting down game server failed (HTTP error code %d from Agones HTTP sidecar)"),
            HttpResponse->GetResponseCode());
        this->bHasSentAllocate = false;
        OnComplete.ExecuteIfBound();
        return;
    }

    UE_LOG(LogRedpointEOSOrchestrator, Verbose, TEXT("Agones: Notified that the game server should shutdown"));
    OnComplete.ExecuteIfBound();
}

bool FAgonesOrchestrator::IsEnabled() const
{
    return !FPlatformMisc::GetEnvironmentVariable(TEXT("AGONES_SDK_HTTP_PORT")).IsEmpty();
}

void FAgonesOrchestrator::Init()
{
    FString Port = FPlatformMisc::GetEnvironmentVariable(TEXT("AGONES_SDK_HTTP_PORT"));
    checkf(!Port.IsEmpty(), TEXT("AGONES_SDK_HTTP_PORT should not be empty; already checked that it wasn't!"));
    this->AgonesHttpServerPort = (int16)FCString::Atoi(*Port);
    checkf(this->AgonesHttpServerPort != 0, TEXT("AGONES_SDK_HTTP_PORT must not be 0!"));

    checkf(!this->HealthCheckTimerHandle.IsValid(), TEXT("FAgonesOrchestrator::Init must not be called twice!"));
    this->HealthCheckTimerHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateSP(this, &FAgonesOrchestrator::SendHealthCheck),
        1.0f);
}

TTask<TArray<FOrchestratorPortMapping>> FAgonesOrchestrator::GetPortMappings()
{
    checkf(this->AgonesHttpServerPort != 0, TEXT("%s: Init must have been called!"), ANSI_TO_TCHAR(__FUNCTION__));

    FHttpModule *Http = &FHttpModule::Get();
    FHttpRequestRef Request = Http->CreateRequest();
    Request->SetURL(FString::Printf(TEXT("http://localhost:%d/gameserver"), this->AgonesHttpServerPort));
    Request->SetVerb("GET");
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
    Request->SetHeader(TEXT("Accepts"), TEXT("application/json"));
    Request->SetContentAsString("");

    auto DeferredTask = TTask<TArray<FOrchestratorPortMapping>>::Deferred();

    Request->OnProcessRequestComplete().BindSP(
        this,
        &FAgonesOrchestrator::HandleGameServer,
        FOnGetPortMappingsComplete::CreateLambda([DeferredTask](const TArray<FOrchestratorPortMapping> &Mapping) {
            DeferredTask.SetValue(Mapping);
        }));
    verifyf(Request->ProcessRequest(), TEXT("%s: Expected HTTP request to start."), ANSI_TO_TCHAR(__FUNCTION__));

    return DeferredTask;
}

TTask<void> FAgonesOrchestrator::ServingTraffic(const FName &SessionName, const FString &SessionId)
{
    UE_LOG(
        LogRedpointEOSOrchestrator,
        Verbose,
        TEXT("CreateSession: Notifying dedicated server orchestrator that we are now "
             "serving traffic..."));

    checkf(this->AgonesHttpServerPort != 0, TEXT("%s: Init must have been called!"), ANSI_TO_TCHAR(__FUNCTION__));

    if (this->bHasSentReady)
    {
        // We've already told Agones we're ready.
        UE_LOG(
            LogRedpointEOSOrchestrator,
            Warning,
            TEXT("Agones: Already informed the allocator that this game server is ready. Once the match ends, you "
                 "should terminate the game server instead of re-using the same process."));
        return TTask<void>::FromResult();
    }

    this->bHasSentReady = true;

    FHttpModule *Http = &FHttpModule::Get();
    FHttpRequestRef Request = Http->CreateRequest();
    Request->SetURL(FString::Printf(TEXT("http://localhost:%d/metadata/label"), this->AgonesHttpServerPort));
    Request->SetVerb("PUT");
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
    Request->SetHeader(TEXT("Accepts"), TEXT("application/json"));
    Request->SetContentAsString(FString::Printf(
        TEXT("{\"key\": \"sessionid-%s\", \"value\": \"%s\"}"),
        *SessionName.ToString().ToLower(),
        *SessionId));

    auto DeferredTask = TTask<void>::Deferred();

    Request->OnProcessRequestComplete().BindSP(
        this,
        &FAgonesOrchestrator::HandleLabel,
        FSimpleDelegate::CreateLambda([DeferredTask]() {
            DeferredTask.SetValue();
        }));
    verifyf(Request->ProcessRequest(), TEXT("%s: Expected HTTP request to start."), ANSI_TO_TCHAR(__FUNCTION__));

    return DeferredTask;
}

TTask<void> FAgonesOrchestrator::SessionStarted()
{
    checkf(this->AgonesHttpServerPort != 0, TEXT("%s: Init must have been called!"), ANSI_TO_TCHAR(__FUNCTION__));

    if (this->bHasSentAllocate)
    {
        // We've already told Agones we're ready.
        UE_LOG(
            LogRedpointEOSOrchestrator,
            Warning,
            TEXT("Agones: Already informed the allocator that this game server is allocated. Once the match ends, you "
                 "should call EndSession or DestroySession to get Agones to terminate the game server instead of "
                 "re-using it."));
        return TTask<void>::FromResult();
    }

    this->bHasSentAllocate = true;

    FHttpModule *Http = &FHttpModule::Get();
    FHttpRequestRef Request = Http->CreateRequest();
    Request->SetURL(FString::Printf(TEXT("http://localhost:%d/allocate"), this->AgonesHttpServerPort));
    Request->SetVerb("POST");
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
    Request->SetHeader(TEXT("Accepts"), TEXT("application/json"));
    Request->SetContentAsString("{}");

    auto DeferredTask = TTask<void>::Deferred();

    Request->OnProcessRequestComplete().BindSP(
        this,
        &FAgonesOrchestrator::HandleAllocate,
        FSimpleDelegate::CreateLambda([DeferredTask]() {
            DeferredTask.SetValue();
        }));
    verifyf(Request->ProcessRequest(), TEXT("%s: Expected HTTP request to start."), ANSI_TO_TCHAR(__FUNCTION__));

    return DeferredTask;
}

bool FAgonesOrchestrator::ShouldDestroySessionOnEndSession()
{
    return true;
}

TTask<void> FAgonesOrchestrator::Shutdown()
{
    checkf(this->AgonesHttpServerPort != 0, TEXT("%s: Init must have been called!"), ANSI_TO_TCHAR(__FUNCTION__));

    if (this->bHasSentShutdown)
    {
        // We've already told Agones we're shutting down.
        UE_LOG(
            LogRedpointEOSOrchestrator,
            Warning,
            TEXT("Agones: Already informed the allocator that this game server is to be shutdown."));
        return TTask<void>::FromResult();
    }

    this->bHasSentShutdown = true;

    FHttpModule *Http = &FHttpModule::Get();
    FHttpRequestRef Request = Http->CreateRequest();
    Request->SetURL(FString::Printf(TEXT("http://localhost:%d/shutdown"), this->AgonesHttpServerPort));
    Request->SetVerb("POST");
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
    Request->SetHeader(TEXT("Accepts"), TEXT("application/json"));
    Request->SetContentAsString("{}");

    auto DeferredTask = TTask<void>::Deferred();

    Request->OnProcessRequestComplete().BindSP(
        this,
        &FAgonesOrchestrator::HandleShutdown,
        FSimpleDelegate::CreateLambda([DeferredTask]() {
            DeferredTask.SetValue();
        }));
    verifyf(Request->ProcessRequest(), TEXT("%s: Expected HTTP request to start."), ANSI_TO_TCHAR(__FUNCTION__));

    return DeferredTask;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()