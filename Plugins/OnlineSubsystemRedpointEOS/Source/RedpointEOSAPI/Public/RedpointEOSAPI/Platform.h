// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::Config
{
/**
 * Forward declaration of IConfig so we can store a pointer to it without RedpointEOSAPI depending on RedpointEOSConfig.
 */
class IConfig;
}

namespace Redpoint::EOS::API
{

class FPlatformInstance;

typedef TMulticastDelegate<void(FPlatformInstance &Instance)> FOnPlatformInstanceEvent;
typedef TMulticastDelegate<void(
    FPlatformInstance &Instance,
    EOS_EApplicationStatus OldApplicationStatus,
    EOS_EApplicationStatus NewApplicationStatus)>
    FOnPlatformInstanceApplicationStatusChangedEvent;
typedef TMulticastDelegate<
    void(FPlatformInstance &Instance, EOS_ENetworkStatus OldNetworkStatus, EOS_ENetworkStatus NewNetworkStatus)>
    FOnPlatformInstanceNetworkStatusChangedEvent;

typedef TSharedRef<FPlatformInstance> FPlatformHandle;

/**
 * Represents the environmental state that is associated with a platform instance. Services associated with a platform
 * instance receive this information when constructed.
 */
class REDPOINTEOSAPI_API FPlatformInstanceEnvironment
{
public:
    FPlatformInstanceEnvironment(
        const FName &InInstanceName,
        bool bInIsDedicatedServer,
        TSharedRef<Redpoint::EOS::Config::IConfig> InConfig);

    const FName InstanceName;
    const bool bIsDedicatedServer;
    const TSharedRef<Redpoint::EOS::Config::IConfig> Config;
};

typedef TSharedRef<const FPlatformInstanceEnvironment> FPlatformInstanceEnvironmentRef;
typedef TSharedPtr<const FPlatformInstanceEnvironment> FPlatformInstanceEnvironmentPtr;

/**
 * An empty interface with a virtual destructor that services should implement so that FPlatformInstance can track them
 * and clean up them appropriately.
 *
 * Do not add any other methods to this interface!
 */
class REDPOINTEOSAPI_API IPlatformInstanceSystem
{
public:
    IPlatformInstanceSystem() = default;
    UE_NONCOPYABLE(IPlatformInstanceSystem);
    virtual ~IPlatformInstanceSystem() = default;
};
[[deprecated("Use IPlatformInstanceSystem directly.")]] typedef IPlatformInstanceSystem IPlatformInstanceService;

#define REDPOINT_EOSSDK_PLATFORM_GET(Name)                                                                             \
    template <> EOS_H##Name Get<EOS_H##Name>()                                                                         \
    {                                                                                                                  \
        if (this->bIsShutdown || this->Instance == nullptr)                                                            \
        {                                                                                                              \
            return nullptr;                                                                                            \
        }                                                                                                              \
        return EOS_Platform_Get##Name##Interface(this->Instance);                                                      \
    }

/**
 * A safe class that wraps EOS_HPlatform and allows the APIs in the Redpoint::EOS::API namespace to only attempt
 * invocations if the platform instance is still valid.
 */
class REDPOINTEOSAPI_API FPlatformInstance : public TSharedFromThis<FPlatformInstance>
{
private:
    bool bIsShutdown;
    EOS_HPlatform Instance;
    FPlatformInstanceEnvironmentRef Environment;
    TMap<FName, TSharedPtr<IPlatformInstanceSystem>> InstanceSystems;
    FCriticalSection InstanceSystemsLock;

    FOnPlatformInstanceEvent OnBeforeTickEvent;
    FOnPlatformInstanceEvent OnAfterTickEvent;
    FOnPlatformInstanceEvent OnShutdownEvent;
    FOnPlatformInstanceApplicationStatusChangedEvent OnApplicationStatusChangedEvent;
    FOnPlatformInstanceNetworkStatusChangedEvent OnNetworkStatusChangedEvent;

    FPlatformInstance(FPlatformInstanceEnvironmentRef InEnvironment);

    TSharedRef<IPlatformInstanceSystem> GetSystemInternal(
        FName InSystemName,
        TFunction<TSharedRef<IPlatformInstanceSystem>(
            FPlatformHandle InPlatformHandle,
            FPlatformInstanceEnvironmentRef InEnvironment)> InConstructSystem);

public:
    FPlatformInstance(EOS_HPlatform InInstance, FPlatformInstanceEnvironmentRef InEnvironment);
    UE_NONCOPYABLE(FPlatformInstance);
    virtual ~FPlatformInstance();

    static TSharedRef<FPlatformInstance> CreateDeadWithNoInstance(FPlatformInstanceEnvironmentRef InEnvironment);

    void ForceShutdown();
    bool IsShutdown() const;

    const FPlatformInstanceEnvironmentRef &GetEnvironment() const
    {
        return this->Environment;
    }

    FOnPlatformInstanceEvent &OnBeforeTick()
    {
        return this->OnBeforeTickEvent;
    }

    FOnPlatformInstanceEvent &OnAfterTick()
    {
        return this->OnAfterTickEvent;
    }

    FOnPlatformInstanceEvent &OnShutdown()
    {
        return this->OnShutdownEvent;
    }

    FOnPlatformInstanceApplicationStatusChangedEvent &OnApplicationStatusChanged()
    {
        return this->OnApplicationStatusChangedEvent;
    }

    FOnPlatformInstanceNetworkStatusChangedEvent &OnNetworkStatusChanged()
    {
        return this->OnNetworkStatusChangedEvent;
    }

    EOS_HPlatform Handle() const
    {
        return this->Instance;
    }

    template <typename TSystem> TSharedRef<TSystem> GetSystem()
    {
        return StaticCastSharedRef<TSystem>(this->GetSystemInternal(
            TSystem::GetSystemName(),
            [this](FPlatformHandle InPlatformHandle, FPlatformInstanceEnvironmentRef InEnvironment) {
                return TSystem::ConstructSystem(this->AsShared(), InEnvironment);
            }));
    }

    template <typename TService> [[deprecated("Use GetSystem<TSystem>() instead.")]] TSharedRef<TService> GetService()
    {
        return GetSystem<TService>();
    }

    template <typename TInterface> TInterface Get();
    REDPOINT_EOSSDK_PLATFORM_GET(Metrics);
    REDPOINT_EOSSDK_PLATFORM_GET(Auth);
    REDPOINT_EOSSDK_PLATFORM_GET(Connect);
    REDPOINT_EOSSDK_PLATFORM_GET(Ecom);
    REDPOINT_EOSSDK_PLATFORM_GET(UI);
    REDPOINT_EOSSDK_PLATFORM_GET(Friends);
    REDPOINT_EOSSDK_PLATFORM_GET(Presence);
    REDPOINT_EOSSDK_PLATFORM_GET(Sessions);
    REDPOINT_EOSSDK_PLATFORM_GET(Lobby);
    REDPOINT_EOSSDK_PLATFORM_GET(UserInfo);
    REDPOINT_EOSSDK_PLATFORM_GET(P2P);
    REDPOINT_EOSSDK_PLATFORM_GET(RTC);
    REDPOINT_EOSSDK_PLATFORM_GET(RTCAdmin);
    REDPOINT_EOSSDK_PLATFORM_GET(PlayerDataStorage);
    REDPOINT_EOSSDK_PLATFORM_GET(TitleStorage);
    REDPOINT_EOSSDK_PLATFORM_GET(Achievements);
    REDPOINT_EOSSDK_PLATFORM_GET(Stats);
    REDPOINT_EOSSDK_PLATFORM_GET(Leaderboards);
    REDPOINT_EOSSDK_PLATFORM_GET(Mods);
    REDPOINT_EOSSDK_PLATFORM_GET(AntiCheatClient);
    REDPOINT_EOSSDK_PLATFORM_GET(AntiCheatServer);
    REDPOINT_EOSSDK_PLATFORM_GET(ProgressionSnapshot);
    REDPOINT_EOSSDK_PLATFORM_GET(Reports);
    REDPOINT_EOSSDK_PLATFORM_GET(Sanctions);
    REDPOINT_EOSSDK_PLATFORM_GET(KWS);
    REDPOINT_EOSSDK_PLATFORM_GET(CustomInvites);
    REDPOINT_EOSSDK_PLATFORM_GET(IntegratedPlatform);
    template <> EOS_HRTCAudio Get<EOS_HRTCAudio>()
    {
        if (this->bIsShutdown || this->Instance == nullptr)
        {
            return nullptr;
        }
        EOS_HRTC RTC = EOS_Platform_GetRTCInterface(this->Instance);
        if (RTC == nullptr)
        {
            return nullptr;
        }
        return EOS_RTC_GetAudioInterface(RTC);
    }
    template <> EOS_HPlatform Get<EOS_HPlatform>()
    {
        if (this->bIsShutdown || this->Instance == nullptr)
        {
            return nullptr;
        }
        return this->Instance;
    }
};

#undef REDPOINT_EOSSDK_PLATFORM_GET

namespace Private
{
class REDPOINTEOSAPI_API FPlatformRefCountedHandleInternal
{
private:
    FPlatformHandle _Instance;

public:
    FPlatformRefCountedHandleInternal(const FPlatformHandle &InInstance);
    UE_NONCOPYABLE(FPlatformRefCountedHandleInternal);
    ~FPlatformRefCountedHandleInternal();

    const FPlatformHandle &Instance() const;
};
}

typedef TSharedRef<Private::FPlatformRefCountedHandleInternal> FPlatformRefCountedHandle;
typedef TSharedPtr<Private::FPlatformRefCountedHandleInternal> FPlatformOptionalRefCountedHandle;
typedef TWeakPtr<Private::FPlatformRefCountedHandleInternal> FPlatformWeakRefCountedHandle;

REDPOINTEOSAPI_API FPlatformRefCountedHandle MakeRefCountedPlatformHandle(const FPlatformHandle &Handle);

}

REDPOINT_EOS_CODE_GUARD_END()