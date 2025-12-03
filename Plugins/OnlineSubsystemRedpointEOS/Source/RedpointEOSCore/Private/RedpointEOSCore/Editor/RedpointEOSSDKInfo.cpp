// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSCore/Editor/RedpointEOSSDKInfo.h"

#if WITH_EDITOR
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSCore/Logging.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_EDITOR

namespace REDPOINT_EOS_FILE_NS_ID(1550610155, Redpoint::EOS::Core::Editor)
{
using namespace ::Redpoint::EOS::Core;

TOptional<FRedpointEOSSDKInfo> FRedpointEOSSDKInfo::Instance;

const FRedpointEOSSDKInfo &FRedpointEOSSDKInfo::Get()
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    if (Instance.IsSet())
    {
        return Instance.GetValue();
    }

    FString PluginBaseDir = IPluginManager::Get().FindPlugin("OnlineSubsystemRedpointEOS")->GetBaseDir();
    FString RedpointEOSSDKInfoPath =
        PluginBaseDir / TEXT("Source") / TEXT("RedpointEOSSDK") / TEXT("RedpointEOSSDKInfo.json");
    if (!FPaths::FileExists(RedpointEOSSDKInfoPath))
    {
        UE_LOG(
            LogRedpointEOSCore,
            Error,
            TEXT("Unable to find EOS SDK information file at '%s'. Please re-install the plugin. Various editor "
                 "functions will not work until this is fixed."),
            *RedpointEOSSDKInfoPath);
        Instance = FRedpointEOSSDKInfo();
        return Instance.GetValue();
    }

    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *RedpointEOSSDKInfoPath))
    {
        UE_LOG(
            LogRedpointEOSCore,
            Error,
            TEXT("Unable to read the EOS SDK information file at '%s'. Please re-install the plugin. Various editor "
                 "functions will not work until this is fixed."),
            *RedpointEOSSDKInfoPath);
        Instance = FRedpointEOSSDKInfo();
        return Instance.GetValue();
    }

    TSharedPtr<FJsonObject> Json;
    TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);
    if (!FJsonSerializer::Deserialize(JsonReader, Json) || !Json.IsValid())
    {
        UE_LOG(
            LogRedpointEOSCore,
            Error,
            TEXT("The EOS SDK information file at '%s' is not valid JSON. Please re-install the plugin. Various editor "
                 "functions will not work until this is fixed."),
            *RedpointEOSSDKInfoPath);
        Instance = FRedpointEOSSDKInfo();
        return Instance.GetValue();
    }

    FString TrimmedEngineDir = FPaths::EngineDir();
    {
        int32 End = TrimmedEngineDir.Len();
        while (End > 0 && (TrimmedEngineDir[End - 1] == '/' || TrimmedEngineDir[End - 1] == '\\'))
        {
            End--;
        }
        TrimmedEngineDir.RemoveAt(End, TrimmedEngineDir.Len() - End);
    }

    FString TrimmedProjectDir;
    if (FPaths::ProjectDir().IsEmpty())
    {
        TrimmedProjectDir = TrimmedEngineDir;
    }
    else
    {
        TrimmedProjectDir = FPaths::ProjectDir();
        int32 End = TrimmedProjectDir.Len();
        while (End > 0 && (TrimmedProjectDir[End - 1] == '/' || TrimmedProjectDir[End - 1] == '\\'))
        {
            End--;
        }
        TrimmedProjectDir.RemoveAt(End, TrimmedProjectDir.Len() - End);
    }

    TMap<FString, FString> SearchPathReplacements;
    SearchPathReplacements.Add(TEXT("${V:EngineDirectory}"), TrimmedEngineDir);
    SearchPathReplacements.Add(TEXT("${V:ProjectDirectory}"), TrimmedProjectDir);
    SearchPathReplacements.Add(TEXT("${V:ModuleBase}"), PluginBaseDir / TEXT("Source") / TEXT("RedpointEOSSDK"));
    SearchPathReplacements.Add(TEXT("${V:ModuleDirectory}"), PluginBaseDir / TEXT("Source") / TEXT("RedpointEOSSDK"));
    SearchPathReplacements.Add(TEXT("${V:PlatformSuffix}"), TEXT(""));
    SearchPathReplacements.Add(TEXT("${V:Platform}"), TEXT(""));
#if PLATFORM_WINDOWS
    SearchPathReplacements.Add(
        TEXT("${F:CommonApplicationData}"),
        FPlatformMisc::GetEnvironmentVariable(TEXT("PROGRAMDATA")));
    SearchPathReplacements.Add(TEXT("${F:UserProfile}"), FPlatformMisc::GetEnvironmentVariable(TEXT("USERPROFILE")));
    SearchPathReplacements.Add(TEXT("${SYSTEMDRIVE}"), FPlatformMisc::GetEnvironmentVariable(TEXT("SYSTEMDRIVE")));
#else
    SearchPathReplacements.Add(TEXT("${HOME}"), FPlatformMisc::GetEnvironmentVariable(TEXT("HOME")));
#endif

    FRedpointEOSSDKInfo NewInstance;

    const TSharedPtr<FJsonObject> *SupportedVersions = nullptr;
    const TSharedPtr<FJsonObject> *SearchPaths = nullptr;
    if (Json->TryGetObjectField(TEXT("SupportedVersions"), SupportedVersions) && SupportedVersions != nullptr &&
        SupportedVersions->IsValid())
    {
        for (const auto &KV : (*SupportedVersions)->Values)
        {
            auto &Entries = NewInstance.SupportedVersionsValue.FindOrAdd(KV.Key);
            if (KV.Value.IsValid() && KV.Value->Type == EJson::Array)
            {
                for (const auto &VersionNumber : KV.Value->AsArray())
                {
                    if (VersionNumber.IsValid() && VersionNumber->Type == EJson::String)
                    {
                        Entries.Add(VersionNumber->AsString());
                    }
                }
            }
        }
    }
    if (Json->TryGetObjectField(TEXT("SearchPaths"), SearchPaths) && SearchPaths != nullptr && SearchPaths->IsValid())
    {
        for (const auto &KV : (*SearchPaths)->Values)
        {
            auto &Entries = NewInstance.SearchPathsValue.FindOrAdd(KV.Key);
            if (KV.Value.IsValid() && KV.Value->Type == EJson::Object)
            {
                const TArray<TSharedPtr<FJsonValue>> *UnprocessedSearchPaths = nullptr;
#if PLATFORM_WINDOWS
                KV.Value->AsObject()->TryGetArrayField(TEXT("Windows"), UnprocessedSearchPaths);
#else
                KV.Value->AsObject()->TryGetArrayField(TEXT("Unix"), UnprocessedSearchPaths);
#endif
                if (UnprocessedSearchPaths != nullptr)
                {
                    for (const auto &UnprocessedSearchPath : *UnprocessedSearchPaths)
                    {
                        if (UnprocessedSearchPath.IsValid() && UnprocessedSearchPath->Type == EJson::String)
                        {
                            FString ProcessedSearchPath = UnprocessedSearchPath->AsString();
                            for (const auto &ReplacementKV : SearchPathReplacements)
                            {
                                ProcessedSearchPath =
                                    ProcessedSearchPath.Replace(*ReplacementKV.Key, *ReplacementKV.Value);
                            }
                            Entries.Add(ProcessedSearchPath);
                        }
                    }
                }
            }
        }
    }

    UE_LOG(LogRedpointEOSCore, Verbose, TEXT("EOS SDK information loaded from RedpointEOSSDKInfo.json:"));
    for (const auto &KV : NewInstance.SupportedVersionsValue)
    {
        UE_LOG(LogRedpointEOSCore, Verbose, TEXT("  %d %s supported versions:"), KV.Value.Num(), *KV.Key);
        for (const auto &Entry : KV.Value)
        {
            UE_LOG(LogRedpointEOSCore, Verbose, TEXT("    - %s"), *Entry);
        }
    }
    for (const auto &KV : NewInstance.SearchPathsValue)
    {
        UE_LOG(LogRedpointEOSCore, Verbose, TEXT("  %d %s search paths:"), KV.Value.Num(), *KV.Key);
        for (const auto &Entry : KV.Value)
        {
            UE_LOG(LogRedpointEOSCore, Verbose, TEXT("    - %s"), *Entry);
        }
    }

    Instance = NewInstance;
    return Instance.GetValue();
}

TArray<FString> FRedpointEOSSDKInfo::SearchPaths(const FString &InName) const
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    auto Name = InName;
#if REDPOINT_EOS_SDK_IS_UNVERSIONED_AT_BUILD_TIME
    if (Name == TEXT("EOSSDK"))
    {
        Name = TEXT("EOSSDK-Unversioned");
    }
#endif

    auto *Value = this->SearchPathsValue.Find(Name);
    if (Value == nullptr)
    {
        return TArray<FString>();
    }

    TArray<FString> ProcessedPaths;
    for (const auto &Path : *Value)
    {
        if (Name == TEXT("EOSSDK"))
        {
            // Use the linked EOS SDK version.
            ProcessedPaths.Add(Path.Replace(TEXT("${V:FullVersion}"), ANSI_TO_TCHAR(EOS_SDK_LINK_VERSION)));
        }
        else
        {
            // Use only the latest target SDK version. Since both Discord and OVR have a single version in the list,
            // this will work for now.
            //
            // @todo: Have a way for extra modules to set specific link versions for things like the Discord Social SDK.
            auto SupportedVersionArray = SupportedVersions(Name);
            if (SupportedVersionArray.Num() > 0)
            {
                ProcessedPaths.Add(Path.Replace(TEXT("${V:FullVersion}"), *SupportedVersionArray[0]));
            }
            else
            {
                ProcessedPaths.Add(Path.Replace(TEXT("${V:FullVersion}"), TEXT("")));
            }
        }
    }
    return ProcessedPaths;
}

TArray<FString> FRedpointEOSSDKInfo::SupportedVersions(const FString &Name) const
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    auto *Value = this->SupportedVersionsValue.Find(Name);
    if (Value != nullptr)
    {
        return *Value;
    }
    return TArray<FString>();
}
}

#endif

REDPOINT_EOS_CODE_GUARD_END()