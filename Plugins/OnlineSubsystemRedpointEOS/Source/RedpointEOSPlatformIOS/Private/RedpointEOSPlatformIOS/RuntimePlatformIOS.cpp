// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIOS/RuntimePlatformIOS.h"

#include "GenericPlatform/GenericPlatformFile.h"
#include "HAL/PlatformFileManager.h"
#include "HAL/PlatformProcess.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/App.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Legacy/EOSString.h"
#include "RedpointEOSPlatformIOS/Logging.h"

#define LOCTEXT_NAMESPACE "FOnlineSubsystemRedpointEOSModule"

namespace Redpoint::EOS::Platform::IOS
{

void FRuntimePlatformIOS::Load()
{
    this->RTCOptions = EOS_Platform_RTCOptions{};
    this->RTCOptions.ApiVersion = EOS_PLATFORM_RTCOPTIONS_API_LATEST;
}

void FRuntimePlatformIOS::Unload()
{
}

bool FRuntimePlatformIOS::IsValid()
{
    return true;
}

void *FRuntimePlatformIOS::GetSystemInitializeOptions()
{
    return nullptr;
}

EOS_Platform_RTCOptions *FRuntimePlatformIOS::GetRTCOptions()
{
    return &this->RTCOptions;
}

FString FRuntimePlatformIOS::GetCacheDirectory()
{
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
    NSString *cachePath = [paths objectAtIndex:0];
    BOOL isDir = NO;
    NSError *error;
    if (![[NSFileManager defaultManager] fileExistsAtPath:cachePath isDirectory:&isDir] && isDir == NO)
    {
        [[NSFileManager defaultManager] createDirectoryAtPath:cachePath
                                  withIntermediateDirectories:YES
                                                   attributes:nil
                                                        error:&error];
    }
    const char *URLStrUTF8 = [cachePath cStringUsingEncoding:NSASCIIStringEncoding];
    FString Path = FString(UTF8_TO_TCHAR(URLStrUTF8)) / TEXT("EOSCache");

    UE_LOG(LogRedpointEOSPlatformIOS, Verbose, TEXT("Using the following path as the cache directory: %s"), *Path);
    IPlatformFile &PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (!PlatformFile.DirectoryExists(*Path))
    {
        PlatformFile.CreateDirectory(*Path);
    }
    return Path;
}

#if !UE_BUILD_SHIPPING
FString FRuntimePlatformIOS::GetPathToEASAutomatedTestingCredentials()
{
    return FString::Printf(TEXT("%s/Binaries/IOS/Credentials.json"), FApp::GetProjectName());
}
#endif

bool FRuntimePlatformIOS::UseCrossPlatformFriendStorage() const
{
    return true;
}

#undef LOCTEXT_NAMESPACE

}