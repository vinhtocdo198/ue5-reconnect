// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAuth/AuthenticationGraph.h"
#include "RedpointEOSAuth/CrossPlatform/CrossPlatformAccountProvider.h"
#include "RedpointEOSAuth/OnlineProvidedCredentials.h"
#include "RedpointEOSAuth/ResolveAuthenticationGraphDelegate.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(379028669, Redpoint::EOS::Auth)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Auth;

class REDPOINTEOSAUTH_API FAuthenticationGraphRegistry : public TSharedFromThis<FAuthenticationGraphRegistry>
{
private:
    static TSharedPtr<FAuthenticationGraphRegistry> Instance;

    TMap<FName, FResolveAuthenticationGraphDelegate> RegisteredResolvers;
    TMap<FName, TSharedPtr<FAuthenticationGraph>> RegisteredGraphs;
    TMap<FName, FText> RegisteredDescriptions;
    TMap<FName, TSharedPtr<CrossPlatform::ICrossPlatformAccountProvider>> RegisteredCrossPlatformAccountProviders;

public:
    FAuthenticationGraphRegistry() = default;
    UE_NONCOPYABLE(FAuthenticationGraphRegistry);
    virtual ~FAuthenticationGraphRegistry(){};

    /**
     * Called by FOnlineSubsystemRedpointEOSModule when the plugin starts up to register all the built-in authentication
     * graphs.
     */
    static void RegisterDefaults();

    /**
     * Registers an authentication graph with a given name. You can then choose the authentication graph appropriate for
     * your platform in Project Settings.
     */
    static void Register(
        FName InAuthenticationGraphName,
        const FText &InAuthenticationGraphDescription,
        const TSharedRef<FAuthenticationGraph> &InAuthenticationGraph);

    /**
     * Registers an authentication graph resolver with a given name. This allows the authentication graph to be
     * dynamically chosen based on platform, availability of graphs and the input credentials.
     */
    static void Register(
        FName InAuthenticationGraphName,
        const FText &InAuthenticationGraphDescription,
        const FResolveAuthenticationGraphDelegate &InAuthenticationGraphResolver);

    /**
     * Registers a placeholder authentication graph so it will appear in the Project Settings dropdown, where it does
     * not actually have an implementation available on Win64/Mac.
     */
    static void RegisterPlaceholder(FName InAuthenticationGraphName, const FText &InAuthenticationGraphDescription);

    /**
     * Registers a cross-platform account provider with a given name. You can then choose the cross-platform account
     * provider appropriate for your platform in Project Settings.
     */
    static void RegisterCrossPlatformAccountProvider(
        FName InAuthenticationGraphName,
        const TSharedRef<CrossPlatform::ICrossPlatformAccountProvider> &InCrossPlatformAccountProvider);

    /**
     * Returns whether or not a resolver or graph is registered with the given name.
     */
    static bool Has(FName InAuthenticationGraphName);

    /**
     * Returns whether or not a cross-platform account provider is registered with the given name.
     */
    static bool HasCrossPlatformAccountProvider(FName InAuthenticationGraphName);

    /**
     * Retrieves the authentication graph that was registered with the given name, or returns null if no such graph
     * exists.
     */
    static TSharedPtr<const FAuthenticationGraph> Get(
        FName InAuthenticationGraphName,
        const TSharedRef<Redpoint::EOS::Config::IConfig> &InConfig,
        const FOnlineProvidedCredentials &InProvidedCredentials,
        const TSoftObjectPtr<class UWorld> &InWorld,
        const FPlatformHandle &InPlatformHandle,
        int32 LocalUserNum);

    /**
     * Retrieves the cross-platform account provider that was registered with the given name, or returns null if no such
     * cross-platform account provider exists.
     */
    static TSharedPtr<const CrossPlatform::ICrossPlatformAccountProvider> GetCrossPlatformAccountProvider(
        FName InCrossPlatformAccountProviderName);

    /** Returns a list of registered authentication graph names and their descriptions. */
    static TMap<FName, FText> GetNames();

    /** Returns a list of cross-platform account providers. */
    static TArray<FName> GetCrossPlatformAccountProviderNames();
};

}

namespace Redpoint::EOS::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(379028669, Redpoint::EOS::Auth, FAuthenticationGraphRegistry)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()