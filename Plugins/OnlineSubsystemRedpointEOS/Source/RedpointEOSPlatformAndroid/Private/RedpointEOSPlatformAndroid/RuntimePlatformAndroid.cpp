// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformAndroid/RuntimePlatformAndroid.h"

#include "Android/AndroidApplication.h"
#include "Android/AndroidJNI.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "HAL/PlatformFileManager.h"
#include "HAL/PlatformProcess.h"
#include "Misc/App.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "RedpointEOSCore/Legacy/EOSString.h"
#include "RedpointEOSPlatformAndroid/Logging.h"

#define LOCTEXT_NAMESPACE "FOnlineSubsystemRedpointEOSModule"

namespace Redpoint::EOS::Platform::Android
{

const char *FRuntimePlatformAndroid::InternalPath = nullptr;
const char *FRuntimePlatformAndroid::ExternalPath = nullptr;

void FRuntimePlatformAndroid::Load()
{
    this->RTCOptions = EOS_Platform_RTCOptions{};
    this->RTCOptions.ApiVersion = EOS_PLATFORM_RTCOPTIONS_API_LATEST;
}

void FRuntimePlatformAndroid::Unload()
{
}

bool FRuntimePlatformAndroid::IsValid()
{
    return true;
}

void *FRuntimePlatformAndroid::GetSystemInitializeOptions()
{
    if (this->Opts.IsValid())
    {
        return this->Opts.Get();
    }

    this->Opts = MakeShared<EOS_Android_InitializeOptions>();
    this->Opts->ApiVersion = EOS_ANDROID_INITIALIZEOPTIONS_API_LATEST;
    this->Opts->Reserved = nullptr;
    checkf(
        FRuntimePlatformAndroid::InternalPath != nullptr && FRuntimePlatformAndroid::ExternalPath != nullptr,
        TEXT("InternalPath/ExternalPath has not been set on Android, this is a bug in the Redpoint EOS Online Framework "
             "plugin!"));
    this->Opts->OptionalInternalDirectory = FRuntimePlatformAndroid::InternalPath;
    this->Opts->OptionalExternalDirectory = FRuntimePlatformAndroid::ExternalPath;
    return this->Opts.Get();
}

EOS_Platform_RTCOptions *FRuntimePlatformAndroid::GetRTCOptions()
{
    return &this->RTCOptions;
}

FString FRuntimePlatformAndroid::GetCacheDirectory()
{
    FString Path = FPaths::ProjectPersistentDownloadDir() / TEXT("EOSCache");

    UE_LOG(LogRedpointEOSPlatformAndroid, Verbose, TEXT("Using the following path as the cache directory: %s"), *Path);
    IPlatformFile &PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (!PlatformFile.DirectoryExists(*Path))
    {
        PlatformFile.CreateDirectory(*Path);
    }
    return Path;
}

#if !UE_BUILD_SHIPPING
FString FRuntimePlatformAndroid::GetPathToEASAutomatedTestingCredentials()
{
    return FString::Printf(TEXT("%s/Binaries/Android/Credentials.json"), FApp::GetProjectName());
}
#endif

bool FRuntimePlatformAndroid::UseCrossPlatformFriendStorage() const
{
    return true;
}

bool FRuntimePlatformAndroid::IsDelegatedSubsystemForciblyExcluded(FName InSubsystemName) const
{
    bool bIsRequestingOculus =
        InSubsystemName.IsEqual(FName(TEXT("Oculus"))) || InSubsystemName.IsEqual(FName(TEXT("RedpointOculus")));
    bool bIsRequestingGoogle =
        InSubsystemName.IsEqual(FName(TEXT("Google"))) || InSubsystemName.IsEqual(FName(TEXT("RedpointGoogle")));

    if (bIsRequestingOculus || bIsRequestingGoogle)
    {
        // If this is a relevant delegated subsystem, check to see if this was packaged for Oculus.
        bool bIsOculus = false;
        JNIEnv *Env = FAndroidApplication::GetJavaEnv();
        checkf(Env != nullptr, TEXT("JavaEnv must be available on module startup!"));
        {
            jmethodID RedpointIsOculusMethod = FJavaWrapper::FindMethod(
                Env,
                FJavaWrapper::GameActivityClassID,
                "Thunk_Redpoint_IsOculus",
                "()Z",
                true);
            bIsOculus = FJavaWrapper::CallBooleanMethod(Env, FJavaWrapper::GameActivityThis, RedpointIsOculusMethod);
        }

        if ((bIsRequestingOculus && !bIsOculus) || (bIsRequestingGoogle && bIsOculus))
        {
            // Exclude this delegated subsystem because it can't be used safely based on how this Android application
            // was packaged.
            return true;
        }
    }

    return false;
}

}

JNI_METHOD void Java_com_epicgames_unreal_GameActivity_nativeSetEOSCacheDirectories(
    JNIEnv *env,
    jobject /* this */,
    jstring InternalPath,
    jstring ExternalPath)
{
    using namespace ::Redpoint::EOS::Platform::Android;

    FString InternalPathStr = FJavaHelper::FStringFromParam(env, InternalPath);
    FString ExternalPathStr = FJavaHelper::FStringFromParam(env, ExternalPath);

    FPlatformMisc::LowLevelOutputDebugStringf(TEXT("Received internal directory path from JNI: %s"), *InternalPathStr);
    FPlatformMisc::LowLevelOutputDebugStringf(TEXT("Received external directory path from JNI: %s"), *ExternalPathStr);

    checkf(
        FRuntimePlatformAndroid::InternalPath == nullptr && FRuntimePlatformAndroid::ExternalPath == nullptr,
        TEXT("SetEOSCacheDirectories already called, this is a bug in Redpoint EOS Online Framework plugin."));

    checkf(
        EOSString_Android_InitializeOptions_Directory::AllocateToCharBuffer(
            InternalPathStr,
            FRuntimePlatformAndroid::InternalPath) == EOS_EResult::EOS_Success,
        TEXT("Unable to store path %s as internal directory on startup; this is a bug in the Redpoint EOS Online Framework "
             "plugin."),
        *InternalPathStr);
    checkf(
        EOSString_Android_InitializeOptions_Directory::AllocateToCharBuffer(
            ExternalPathStr,
            FRuntimePlatformAndroid::ExternalPath) == EOS_EResult::EOS_Success,
        TEXT("Unable to store path %s as external directory on startup; this is a bug in the Redpoint EOS Online Framework "
             "plugin."),
        *ExternalPathStr);

    FPlatformMisc::LowLevelOutputDebugStringf(
        TEXT("Will use the following internal cache directory on Android: %s"),
        ANSI_TO_TCHAR(FRuntimePlatformAndroid::InternalPath));
    FPlatformMisc::LowLevelOutputDebugStringf(
        TEXT("Will use the following external cache directory on Android: %s"),
        ANSI_TO_TCHAR(FRuntimePlatformAndroid::ExternalPath));
}

#undef LOCTEXT_NAMESPACE