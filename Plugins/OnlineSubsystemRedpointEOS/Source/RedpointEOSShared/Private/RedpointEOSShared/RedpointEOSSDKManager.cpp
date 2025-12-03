// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSShared/RedpointEOSSDKManager.h"

#if REDPOINT_EOS_UE_5_5_OR_LATER
#include "Modules/ModuleManager.h"
#include "RedpointEOSShared/RedpointEOSPlatformHandle.h"
#include "RedpointEOSSharedModule.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

#define REDPOINT_EOS_CALL_NOT_SUPPORTED()                                                                              \
    checkf(                                                                                                            \
        false,                                                                                                         \
        TEXT(                                                                                                          \
            "This call is not supported. The Redpoint EOS plugin only contains an IEOSSDKManager implementation to "   \
            "support the Fab plugin, and is not compatible with other plugins that depend on EOSShared. If you're "    \
            "seeing this assert, make sure you disable the OnlineSubsystemEOS plugin (by Epic) and any other plugins " \
            "that depend on EOSShared."))

namespace REDPOINT_EOS_FILE_NS_ID(647807917, Redpoint::EOS::Shared)
{
using namespace ::Redpoint::EOS::Shared;

static const FString EmptyString = TEXT("");

bool FRedpointEOSSDKManager::IsInitialized() const
{
    return true;
}

const FEOSSDKPlatformConfig *FRedpointEOSSDKManager::GetPlatformConfig(
    const FString &PlatformConfigName,
    bool bLoadIfMissing)
{
    REDPOINT_EOS_CALL_NOT_SUPPORTED();
    return nullptr;
}

bool FRedpointEOSSDKManager::AddPlatformConfig(
    const FEOSSDKPlatformConfig &PlatformConfig,
    bool bOverwriteExistingConfig)
{
    REDPOINT_EOS_CALL_NOT_SUPPORTED();
    return false;
}

const FString &FRedpointEOSSDKManager::GetDefaultPlatformConfigName()
{
    REDPOINT_EOS_CALL_NOT_SUPPORTED();
    return EmptyString;
}

void FRedpointEOSSDKManager::SetDefaultPlatformConfigName(const FString &PlatformConfigName)
{
    REDPOINT_EOS_CALL_NOT_SUPPORTED();
}

IEOSPlatformHandlePtr FRedpointEOSSDKManager::CreatePlatform(const FString &PlatformConfigName, FName InstanceName)
{
    REDPOINT_EOS_CALL_NOT_SUPPORTED();
    return nullptr;
}

IEOSPlatformHandlePtr FRedpointEOSSDKManager::CreatePlatform(EOS_Platform_Options &PlatformOptions)
{
#if WITH_EDITOR
    // Ensure the RedpointEOSEditor module is loaded, which is the module that registers with our module's
    // OnPlatformCreateRequested event handler.
    FModuleManager::Get().LoadModuleChecked<IModuleInterface>("RedpointEOSEditor");

    auto &SelfModule = FModuleManager::Get().GetModuleChecked<FRedpointEOSSharedModule>("RedpointEOSShared");
    checkf(
        SelfModule.OnPlatformCreateRequested.IsBound(),
        TEXT("Expected OnPlatformCreateRequested to be bound at this point!"));

    // Technically we don't safely track the EOS_HPlatform lifetime here because we don't have a common type to cross
    // the RedpointEOSShared <-> RedpointEOSEditor boundary. However, the editor module guarantees that handles it
    // creates in response to OnPlatformCreateRequested will live for the entire lifetime of the editor, since they
    // should only be used by Fab.
    return MakeShared<FRedpointEOSPlatformHandle>(SelfModule.OnPlatformCreateRequested.Execute(PlatformOptions));
#else
    // We do not support creating platform instances through the IEOSSDKManager interface when not running in the
    // editor.
    return nullptr;
#endif
}

TArray<IEOSPlatformHandlePtr> FRedpointEOSSDKManager::GetActivePlatforms()
{
    REDPOINT_EOS_CALL_NOT_SUPPORTED();
    return TArray<IEOSPlatformHandlePtr>();
}

FString FRedpointEOSSDKManager::GetProductName() const
{
    REDPOINT_EOS_CALL_NOT_SUPPORTED();
    return TEXT("");
}

FString FRedpointEOSSDKManager::GetProductVersion() const
{
    REDPOINT_EOS_CALL_NOT_SUPPORTED();
    return TEXT("");
}

FString FRedpointEOSSDKManager::GetCacheDirBase() const
{
    REDPOINT_EOS_CALL_NOT_SUPPORTED();
    return TEXT("");
}

FString FRedpointEOSSDKManager::GetOverrideCountryCode(const EOS_HPlatform Platform) const
{
    REDPOINT_EOS_CALL_NOT_SUPPORTED();
    return TEXT("");
}

FString FRedpointEOSSDKManager::GetOverrideLocaleCode(const EOS_HPlatform Platform) const
{
    REDPOINT_EOS_CALL_NOT_SUPPORTED();
    return TEXT("");
}

void FRedpointEOSSDKManager::LogInfo(int32 Indent) const
{
    REDPOINT_EOS_CALL_NOT_SUPPORTED();
}

void FRedpointEOSSDKManager::LogPlatformInfo(const EOS_HPlatform Platform, int32 Indent) const
{
    REDPOINT_EOS_CALL_NOT_SUPPORTED();
}

void FRedpointEOSSDKManager::LogAuthInfo(
    const EOS_HPlatform Platform,
    const EOS_EpicAccountId LoggedInAccount,
    int32 Indent) const
{
    REDPOINT_EOS_CALL_NOT_SUPPORTED();
}

void FRedpointEOSSDKManager::LogUserInfo(
    const EOS_HPlatform Platform,
    const EOS_EpicAccountId LoggedInAccount,
    const EOS_EpicAccountId TargetAccount,
    int32 Indent) const
{
    REDPOINT_EOS_CALL_NOT_SUPPORTED();
}

void FRedpointEOSSDKManager::LogPresenceInfo(
    const EOS_HPlatform Platform,
    const EOS_EpicAccountId LoggedInAccount,
    const EOS_EpicAccountId TargetAccount,
    int32 Indent) const
{
    REDPOINT_EOS_CALL_NOT_SUPPORTED();
}

void FRedpointEOSSDKManager::LogFriendsInfo(
    const EOS_HPlatform Platform,
    const EOS_EpicAccountId LoggedInAccount,
    int32 Indent) const
{
    REDPOINT_EOS_CALL_NOT_SUPPORTED();
}

void FRedpointEOSSDKManager::LogConnectInfo(
    const EOS_HPlatform Platform,
    const EOS_ProductUserId LoggedInAccount,
    int32 Indent) const
{
    REDPOINT_EOS_CALL_NOT_SUPPORTED();
}

void FRedpointEOSSDKManager::AddCallbackObject(TUniquePtr<FCallbackBase> CallbackObj)
{
    REDPOINT_EOS_CALL_NOT_SUPPORTED();
}

}

#undef REDPOINT_EOS_CALL_NOT_SUPPORTED

#endif

REDPOINT_EOS_CODE_GUARD_END()