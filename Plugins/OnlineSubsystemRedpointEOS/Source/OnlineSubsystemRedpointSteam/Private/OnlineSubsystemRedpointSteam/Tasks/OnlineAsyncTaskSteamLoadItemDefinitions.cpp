// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointSteam/Tasks/OnlineAsyncTaskSteamLoadItemDefinitions.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemRedpointEOS/Public/EOSError.h"
#include "OnlineSubsystemRedpointSteam/Logging.h"
#include "OnlineSubsystemRedpointSteam/OnlineSubsystem/OnlineSubsystemRedpointSteam.h"

#if defined(REDPOINT_EOS_STEAM_ENABLED)

namespace REDPOINT_EOS_FILE_NS_ID(3496575725, Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks)
{
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam;

void FOnlineAsyncTaskSteamLoadItemDefinitionsCallbackHandler::OnSteamInventoryDefinitionUpdate(
    SteamInventoryDefinitionUpdate_t *InEv)
{
    UE_LOG(
        LogRedpointSteam,
        Verbose,
        TEXT("%s: Received the SteamInventoryDefinitionUpdate_t event from Steamworks SDK."),
        ANSI_TO_TCHAR(__FUNCTION__));
    this->Owner->NotifyComplete();
}

void FOnlineAsyncTaskSteamLoadItemDefinitions::NotifyComplete()
{
    this->bIsComplete = true;
    this->bWasSuccessful = true;
}

FOnlineAsyncTaskSteamLoadItemDefinitions::FOnlineAsyncTaskSteamLoadItemDefinitions(
    FSteamLoadItemDefinitionsComplete InDelegate,
    TArray<FSteamItemDefinitionPriceRaw> InPrices)
    : bInit(false)
    , Delegate(MoveTemp(InDelegate))
    , FailureContext()
    , CallbackHandler(MakeShareable(new FOnlineAsyncTaskSteamLoadItemDefinitionsCallbackHandler(this)))
    , ResultDefinitions()
    , LoadedPrices(InPrices)
    , ParseItemDefinitionsLogs()
{
}

void FOnlineAsyncTaskSteamLoadItemDefinitions::Tick()
{
    ISteamInventory *SteamInventoryPtr = SteamInventory();
    if (SteamInventoryPtr == nullptr)
    {
        UE_LOG(
            LogRedpointSteam,
            Error,
            TEXT("%s: The SteamInventory() service is not available."),
            ANSI_TO_TCHAR(__FUNCTION__));
        this->bIsComplete = true;
        this->bWasSuccessful = false;
        return;
    }

    if (!this->bInit)
    {
        uint32 UnusedCount = 0;
        if (SteamInventoryPtr->GetItemDefinitionIDs(nullptr, &UnusedCount))
        {
            // The item definitions are already loaded, and we don't need to run LoadItemDefinitions.
            UE_LOG(
                LogRedpointSteam,
                Verbose,
                TEXT("%s: Skipping LoadItemDefinitions call because item definitions are already loaded."),
                ANSI_TO_TCHAR(__FUNCTION__));
            this->bIsComplete = true;
            this->bWasSuccessful = true;
        }
        // @note: Steam documentation says LoadItemDefinitions can only return true, but check the result just in case.
        else if (!SteamInventoryPtr->LoadItemDefinitions())
        {
            // We were unable to call LoadItemDefinitions for some reason (but the Steam API won't tell us why).
            UE_LOG(
                LogRedpointSteam,
                Error,
                TEXT("%s: The initial call to LoadItemDefinitions failed, even through the Steam documentation "
                     "indicates this function can never fail."),
                ANSI_TO_TCHAR(__FUNCTION__));
            this->bIsComplete = true;
            this->bWasSuccessful = false;
        }

        this->bInit = true;
    }

    // Otherwise wait for our FOnlineAsyncTaskSteamLoadItemDefinitionsCallbackHandler
    // to send us a notification that the work has finished.

    if (this->bIsComplete && this->bWasSuccessful)
    {
        // Get the item definition IDs.
        uint32 ItemDefinitionCount = 0;
        if (!SteamInventoryPtr->GetItemDefinitionIDs(nullptr, &ItemDefinitionCount))
        {
            UE_LOG(
                LogRedpointSteam,
                Error,
                TEXT("%s: The call to GetItemDefinitionIDs failed even after Steam told us the item definitions were "
                     "loaded. This will cause the whole FOnlineAsyncTaskSteamLoadItemDefinitions operation to report "
                     "failure."),
                ANSI_TO_TCHAR(__FUNCTION__));
            this->FailureContext = TEXT("GetItemDefinitionIDs failed (count retrieval).");
            this->bWasSuccessful = false;
            return;
        }
        UE_LOG(
            LogRedpointSteam,
            VeryVerbose,
            TEXT("Steamworks API provided %u item definitions."),
            ItemDefinitionCount);
        if (ItemDefinitionCount == 0)
        {
            this->Delegate.ExecuteIfBound(OnlineRedpointEOS::Errors::Success(), FSteamItemDefinitionMap());
            return;
        }
        uint32 AllocatedItemDefinitionCount = ItemDefinitionCount;
        SteamItemDef_t *ItemDefinitions =
            (SteamItemDef_t *)FMemory::Malloc(sizeof(SteamItemDef_t) * ItemDefinitionCount);
        if (!SteamInventoryPtr->GetItemDefinitionIDs(ItemDefinitions, &ItemDefinitionCount))
        {
            UE_LOG(
                LogRedpointSteam,
                Error,
                TEXT("%s: The second call to GetItemDefinitionIDs to fill out the array with size %u failed, and Steam "
                     "reported that the size of the array should actually be %u. This will cause the whole "
                     "FOnlineAsyncTaskSteamLoadItemDefinitions operation to report failure."),
                ANSI_TO_TCHAR(__FUNCTION__),
                AllocatedItemDefinitionCount,
                ItemDefinitionCount);
            this->FailureContext = TEXT("GetItemDefinitionIDs failed (fill operation).");
            this->bWasSuccessful = false;
            return;
        }

        // If we don't have prices provided when LoadItemDefinitions started, generate an
        // array without prices that we can use for parsing item definitions.
        TArray<FSteamItemDefinitionPriceRaw> ItemDefinitionsMaybeWithPrices = this->LoadedPrices;
        if (ItemDefinitionsMaybeWithPrices.Num() == 0)
        {
            for (uint32 i = 0; i < ItemDefinitionCount; i++)
            {
                auto ItemDefinitionWithoutPrice = FSteamItemDefinitionPriceRaw();
                ItemDefinitionWithoutPrice.SteamItemDefinitionId = ItemDefinitions[i];
                ItemDefinitionWithoutPrice.CurrentPrice = 0;
                ItemDefinitionWithoutPrice.BasePrice = 0;
                ItemDefinitionsMaybeWithPrices.Add(ItemDefinitionWithoutPrice);
            }
        }

        // Parse the item definitions and generate the map.
        TArray<FSteamItemDefinition> ItemDefinitionsArray = this->ParseItemDefinitions(ItemDefinitionsMaybeWithPrices);
        TMap<int32, FSteamItemDefinition> ItemDefinitionsMap;
        for (const auto &Entry : ItemDefinitionsArray)
        {
            ItemDefinitionsMap.Add(Entry.SteamItemDefinitionId, Entry);
        }
        this->ResultDefinitions = ItemDefinitionsMap;
    }
}

TArray<FSteamItemDefinition> FOnlineAsyncTaskSteamLoadItemDefinitions::ParseItemDefinitions(
    TArray<FSteamItemDefinitionPriceRaw> DefinitionsOptionallyWithPrices)
{
    ISteamInventory *SteamInventoryPtr = SteamInventory();
    if (SteamInventoryPtr == nullptr)
    {
        this->ParseItemDefinitionsLogs.Add(TTuple<ELogVerbosity::Type, FString>(
            ELogVerbosity::Error,
            FString::Printf(TEXT("%s: The SteamInventory() service is not available."), ANSI_TO_TCHAR(__FUNCTION__))));
        return TArray<FSteamItemDefinition>();
    }

    TArray<FSteamItemDefinition> Result;
    for (int32 i = 0; i < DefinitionsOptionallyWithPrices.Num(); i++)
    {
        // Get the full list of available properties for this item.
        uint32 BufferSizeRequired = 0;
        FString PropertyList;
        {
            if (!SteamInventoryPtr->GetItemDefinitionProperty(
                    DefinitionsOptionallyWithPrices[i].SteamItemDefinitionId,
                    nullptr,
                    nullptr,
                    &BufferSizeRequired))
            {
                this->ParseItemDefinitionsLogs.Add(TTuple<ELogVerbosity::Type, FString>(
                    ELogVerbosity::Error,
                    OnlineRedpointEOS::Errors::UnexpectedError(
                        ANSI_TO_TCHAR(__FUNCTION__),
                        TEXT("GetItemDefinitionProperty call failed (0x1)."))
                        .ToLogString()));
                continue;
            }
            char *PropertyListBuffer = (char *)FMemory::MallocZeroed(sizeof(char) * (BufferSizeRequired + 1));
            if (!SteamInventoryPtr->GetItemDefinitionProperty(
                    DefinitionsOptionallyWithPrices[i].SteamItemDefinitionId,
                    nullptr,
                    PropertyListBuffer,
                    &BufferSizeRequired))
            {
                FMemory::Free(PropertyListBuffer);
                this->ParseItemDefinitionsLogs.Add(TTuple<ELogVerbosity::Type, FString>(
                    ELogVerbosity::Error,
                    OnlineRedpointEOS::Errors::UnexpectedError(
                        ANSI_TO_TCHAR(__FUNCTION__),
                        TEXT("GetItemDefinitionProperty call failed (0x2)."))
                        .ToLogString()));
                continue;
            }
            PropertyList = ANSI_TO_TCHAR(PropertyListBuffer);
            FMemory::Free(PropertyListBuffer);
        }

        // Iterate through all of the properties, retrieving all of their values.
        TMap<FString, FString> Properties;
        {
            TArray<FString> PropertyNames;
            PropertyList.ParseIntoArray(PropertyNames, TEXT(","));
            for (const auto &PropertyName : PropertyNames)
            {
                auto SteamPropertyName = StringCast<ANSICHAR>(*PropertyName);
                BufferSizeRequired = 0;
                if (!SteamInventoryPtr->GetItemDefinitionProperty(
                        DefinitionsOptionallyWithPrices[i].SteamItemDefinitionId,
                        SteamPropertyName.Get(),
                        nullptr,
                        &BufferSizeRequired))
                {
                    this->ParseItemDefinitionsLogs.Add(TTuple<ELogVerbosity::Type, FString>(
                        ELogVerbosity::Error,
                        OnlineRedpointEOS::Errors::UnexpectedError(
                            ANSI_TO_TCHAR(__FUNCTION__),
                            TEXT("GetItemDefinitionProperty call failed (0x3)."))
                            .ToLogString()));
                    continue;
                }
                char *PropertyValueBuffer = (char *)FMemory::MallocZeroed(sizeof(char) * (BufferSizeRequired + 1));
                if (!SteamInventoryPtr->GetItemDefinitionProperty(
                        DefinitionsOptionallyWithPrices[i].SteamItemDefinitionId,
                        SteamPropertyName.Get(),
                        PropertyValueBuffer,
                        &BufferSizeRequired))
                {
                    FMemory::Free(PropertyValueBuffer);
                    this->ParseItemDefinitionsLogs.Add(TTuple<ELogVerbosity::Type, FString>(
                        ELogVerbosity::Error,
                        OnlineRedpointEOS::Errors::UnexpectedError(
                            ANSI_TO_TCHAR(__FUNCTION__),
                            TEXT("GetItemDefinitionProperty call failed (0x4)."))
                            .ToLogString()));
                    continue;
                }
                FString PropertyValue = UTF8_TO_TCHAR(PropertyValueBuffer);
                FMemory::Free(PropertyValueBuffer);
                Properties.Add(PropertyName, PropertyValue);
            }
        }
        this->ParseItemDefinitionsLogs.Add(TTuple<ELogVerbosity::Type, FString>(
            ELogVerbosity::VeryVerbose,
            FString::Printf(
                TEXT("Steamworks API item definition %u: id=%d, currentPrice=%llu, basePrice=%llu, propertyCount=%d"),
                i,
                DefinitionsOptionallyWithPrices[i].SteamItemDefinitionId,
                DefinitionsOptionallyWithPrices[i].CurrentPrice,
                DefinitionsOptionallyWithPrices[i].BasePrice,
                Properties.Num())));
        for (const auto &KV : Properties)
        {
            this->ParseItemDefinitionsLogs.Add(TTuple<ELogVerbosity::Type, FString>(
                ELogVerbosity::VeryVerbose,
                FString::Printf(TEXT("Steamworks API item definition %u property: %s=%s"), i, *KV.Key, *KV.Value)));
        }

        Result.Add(FSteamItemDefinition{
            DefinitionsOptionallyWithPrices[i].SteamItemDefinitionId,
            DefinitionsOptionallyWithPrices[i].CurrentPrice,
            DefinitionsOptionallyWithPrices[i].BasePrice,
            Properties});
    }
    return Result;
}

void FOnlineAsyncTaskSteamLoadItemDefinitions::Finalize()
{
    for (const auto &Log : this->ParseItemDefinitionsLogs)
    {
        switch (Log.Key)
        {
        case ELogVerbosity::Error:
            UE_LOG(LogRedpointSteam, Error, TEXT("%s"), *Log.Value);
            break;
        case ELogVerbosity::Warning:
            UE_LOG(LogRedpointSteam, Warning, TEXT("%s"), *Log.Value);
            break;
        case ELogVerbosity::Verbose:
            UE_LOG(LogRedpointSteam, Verbose, TEXT("%s"), *Log.Value);
            break;
        case ELogVerbosity::VeryVerbose:
            UE_LOG(LogRedpointSteam, VeryVerbose, TEXT("%s"), *Log.Value);
            break;
        }
    }

    if (!this->bWasSuccessful)
    {
        UE_LOG(
            LogRedpointSteam,
            Error,
            TEXT(
                "%s: FOnlineAsyncTaskSteamLoadItemDefinitions::bWasSuccessful was false at the end of the async task."),
            ANSI_TO_TCHAR(__FUNCTION__));

        this->Delegate.ExecuteIfBound(
            OnlineRedpointEOS::Errors::UnexpectedError(
                TEXT("FOnlineAsyncTaskSteamLoadItemDefinitions::Finalize"),
                FString::Printf(
                    TEXT("Failed to retrieve item definitions from Steam for e-commerce (%s)."),
                    *this->FailureContext)),
            FSteamItemDefinitionMap());
        return;
    }

    this->Delegate.ExecuteIfBound(OnlineRedpointEOS::Errors::Success(), this->ResultDefinitions);
}

}

#endif