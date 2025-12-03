// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditor/Config/ConfigurationFilePathHelpers.h"

#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "RedpointEOSEditor/Config/PlatformHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3436934203, Redpoint::EOS::Editor::Config)
{

TMap<FString, FString> FilePathCache;

FString GetFilePath(EConfigurationFileType File, FName Platform)
{
    FString CacheKey;
    switch (File)
    {
    case EConfigurationFileType::Engine:
        CacheKey = FString::Printf(TEXT("Engine-%s"), *Platform.ToString());
        break;
    case EConfigurationFileType::Game:
        CacheKey = FString::Printf(TEXT("Game-%s"), *Platform.ToString());
        break;
    case EConfigurationFileType::Editor:
        CacheKey = FString::Printf(TEXT("Editor-%s"), *Platform.ToString());
        break;
    case EConfigurationFileType::DedicatedServer:
        CacheKey = FString::Printf(TEXT("DedicatedServer-%s"), *Platform.ToString());
        break;
    case EConfigurationFileType::TrustedClient:
        CacheKey = FString::Printf(TEXT("TrustedClient-%s"), *Platform.ToString());
        break;
    default:
        checkf(false, TEXT("Unsupported EConfigurationFileType value!"));
        break;
    }
    if (FilePathCache.Contains(CacheKey))
    {
        return FilePathCache[CacheKey];
    }

    FString SourceConfigDir = FConfigCacheIni::NormalizeConfigIniPath(FPaths::SourceConfigDir());
    checkf(!SourceConfigDir.IsEmpty(), TEXT("FPaths::SourceConfigDir must not be empty!"));

    if (File == EConfigurationFileType::DedicatedServer)
    {
        FString Result = FString::Printf(TEXT("%sDedicatedServerEngine.ini"), *SourceConfigDir);
        FilePathCache.Add(CacheKey, Result);
        return Result;
    }
    if (File == EConfigurationFileType::TrustedClient)
    {
        FString Result = FPaths::Combine(
            SourceConfigDir,
            FString(".."),
            FString("Build"),
            FString("NoRedist"),
            FString("TrustedEOSClient.ini"));
        FilePathCache.Add(CacheKey, Result);
        return Result;
    }

    FString FileName;
    switch (File)
    {
    case EConfigurationFileType::Engine:
        FileName = TEXT("Engine");
        break;
    case EConfigurationFileType::Game:
        FileName = TEXT("Game");
        break;
    case EConfigurationFileType::Editor:
        FileName = TEXT("Editor");
        break;
    case EConfigurationFileType::DedicatedServer:
    case EConfigurationFileType::TrustedClient:
    default:
        checkf(false, TEXT("Unsupported EConfigurationFileType value!"));
        break;
    }

    if (Platform.IsNone())
    {
        FString Result = FPaths::Combine(SourceConfigDir, FString::Printf(TEXT("Default%s.ini"), *FileName));
        FilePathCache.Add(CacheKey, Result);
        return Result;
    }
    else if (!GetConfidentialPlatformNames().Contains(Platform))
    {
        FString Result = FPaths::Combine(
            SourceConfigDir,
            Platform.ToString(),
            FString::Printf(TEXT("%s%s.ini"), *Platform.ToString(), *FileName));
        FilePathCache.Add(CacheKey, Result);
        return Result;
    }
    else
    {
#if REDPOINT_EOS_UE_5_4_OR_LATER
        FString ProjectPlatformExtensionDir = FPaths::ProjectPlatformExtensionDir(*Platform.ToString());
        checkf(
            !ProjectPlatformExtensionDir.IsEmpty(),
            TEXT("FPaths::ProjectPlatformExtensionDir(%s) must not be empty!"),
            *Platform.ToString());
        FString Result = FPaths::Combine(
            ProjectPlatformExtensionDir,
            FString("Config"),
            FString::Printf(TEXT("%s%s.ini"), *Platform.ToString(), *FileName));
#else
        FString ProjectPlatformExtensionsDir = FPaths::ProjectPlatformExtensionsDir();
        checkf(
            !ProjectPlatformExtensionsDir.IsEmpty(),
            TEXT("FPaths::ProjectPlatformExtensionsDir must not be empty!"));
        FString Result = FPaths::Combine(
            ProjectPlatformExtensionsDir,
            Platform.ToString(),
            FString("Config"),
            FString::Printf(TEXT("%s%s.ini"), *Platform.ToString(), *FileName));
#endif
        FilePathCache.Add(CacheKey, Result);
        return Result;
    }
}

}

REDPOINT_EOS_CODE_GUARD_END()
