// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSCore/Utils/WorldTravel.h"

#include "Engine/Engine.h"
#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSCore/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2601969302, Redpoint::EOS::Core::Utils)
{
using namespace ::Redpoint::EOS::Core;

void FWorldTravel::TravelToMap(const TSoftObjectPtr<UWorld> &World, FURL URL, const FOnTravelToMapComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    if (GEngine == nullptr)
    {
        OnComplete.ExecuteIfBound(
            Errors::UnexpectedError(ANSI_TO_TCHAR(__FUNCTION__), TEXT("GEngine is not available.")));
        return;
    }

    auto *WorldContext = GEngine->GetWorldContextFromWorld(World.Get());
    if (WorldContext == nullptr)
    {
        OnComplete.ExecuteIfBound(Errors::UnexpectedError(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("There is no world context for the specified world.")));
        return;
    }

    return TravelToMap(*WorldContext, URL, OnComplete);
}

void FWorldTravel::TravelToMap(FWorldContext &WorldContext, FURL URL, const FOnTravelToMapComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    if (!IsValid(GEngine))
    {
        OnComplete.ExecuteIfBound(
            Errors::UnexpectedError(ANSI_TO_TCHAR(__FUNCTION__), TEXT("GEngine is not available.")));
        return;
    }

    auto Engine = TSoftObjectPtr<UEngine>(GEngine);

    auto Handled = MakeShared<bool>();
    auto WorldInitHandle = MakeShared<FDelegateHandle>();
    auto TravelFailureHandle = MakeShared<FDelegateHandle>();
    *WorldInitHandle = FWorldDelegates::OnWorldInitializedActors.AddLambda(
        [OnComplete,
         Engine,
         Handled,
         WorldInitHandle,
         TravelFailureHandle,
         WorldContextHandle = WorldContext.ContextHandle](const UWorld::FActorsInitializedParams &ActorInitParams) {
            if (*Handled)
            {
                return;
            }
            *Handled = true;
            checkf(
                IsValid(ActorInitParams.World),
                TEXT("FWorldTravel::TravelToMap: Expected world for OnWorldInitializedActors event to be valid!"));
            checkf(Engine.IsValid(), TEXT("FWorldTravel::TravelToMap: Expected engine object to still be valid!"));
            auto &NewWorldContext = Engine->GetWorldContextFromWorldChecked(ActorInitParams.World);
            if (!NewWorldContext.ContextHandle.IsEqual(WorldContextHandle))
            {
                UE_LOG(
                    LogRedpointEOSCore,
                    Verbose,
                    TEXT("FWorldTravel::TravelToMap: OnTravelFailure received for world context '%s', but we're "
                         "handling world context '%s'."),
                    *NewWorldContext.ContextHandle.ToString(),
                    *WorldContextHandle.ToString());
                return;
            }
            UE_LOG(LogRedpointEOSCore, Verbose, TEXT("FWorldTravel::TravelToMap: Received OnWorldInitializedActors."));
            OnComplete.ExecuteIfBound(Errors::Success());
            if (Engine.IsValid())
            {
                Engine->OnTravelFailure().Remove(*TravelFailureHandle);
            }
            FWorldDelegates::OnWorldInitializedActors.Remove(*WorldInitHandle);
        });
    *TravelFailureHandle = Engine->OnTravelFailure().AddLambda([OnComplete,
                                                                Engine,
                                                                Handled,
                                                                WorldInitHandle,
                                                                TravelFailureHandle,
                                                                WorldContextHandle = WorldContext.ContextHandle](
                                                                   UWorld *NewWorld,
                                                                   ETravelFailure::Type FailureType,
                                                                   const FString &ErrorMessage) {
        if (*Handled)
        {
            return;
        }
        *Handled = true;
        checkf(IsValid(NewWorld), TEXT("FWorldTravel::TravelToMap: Expected world after travel failure to be valid!"));
        checkf(Engine.IsValid(), TEXT("FWorldTravel::TravelToMap: Expected engine object to still be valid!"));
        auto &NewWorldContext = Engine->GetWorldContextFromWorldChecked(NewWorld);
        if (!NewWorldContext.ContextHandle.IsEqual(WorldContextHandle))
        {
            UE_LOG(
                LogRedpointEOSCore,
                Verbose,
                TEXT("FWorldTravel::TravelToMap: OnTravelFailure received for world context '%s', but we're "
                     "handling world context '%s'."),
                *NewWorldContext.ContextHandle.ToString(),
                *WorldContextHandle.ToString());
            return;
        }
        UE_LOG(
            LogRedpointEOSCore,
            Verbose,
            TEXT("FWorldTravel::TravelToMap: Received OnTravelFailure, failure code %d: %s"),
            FailureType,
            *ErrorMessage);
        OnComplete.ExecuteIfBound(Errors::UnexpectedError(
            ANSI_TO_TCHAR(__FUNCTION__),
            FString::Printf(TEXT("Failed to travel to new map, failure code %d: %s"), FailureType, *ErrorMessage)));
        FWorldDelegates::OnWorldInitializedActors.Remove(*WorldInitHandle);
        if (Engine.IsValid())
        {
            Engine->OnTravelFailure().Remove(*TravelFailureHandle);
        }
    });

    Engine->SetClientTravel(WorldContext.World(), *URL.ToString(), ETravelType::TRAVEL_Absolute);
}

}

REDPOINT_EOS_CODE_GUARD_END()