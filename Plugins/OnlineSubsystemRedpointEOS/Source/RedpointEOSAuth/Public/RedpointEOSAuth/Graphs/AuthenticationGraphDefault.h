// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAuth/AuthenticationGraph.h"
#include "RedpointEOSAuth/OnlineProvidedCredentials.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSConfig/Config.h"
#include "UObject/SoftObjectPtr.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1044168076, Redpoint::EOS::Auth::Graphs)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::Config;

#define EOS_AUTH_GRAPH_DEFAULT NAME_Default
#define EOS_AUTH_GRAPH_DEFAULT_WITH_CROSS_PLATFORM_FALLBACK FName(TEXT("DefaultWithCrossPlatformFallback"))

/**
 * This class provides the default resolver for authentication graphs. If you don't explicitly set an authentication
 * graph for your project, this resolver will choose for you.
 */
class REDPOINTEOSAUTH_API FAuthenticationGraphDefault
{
private:
    static FName Resolve(
        const TSharedRef<FAuthenticationGraphRegistry> &InRegistry,
        const TSharedRef<Redpoint::EOS::Config::IConfig> &InConfig,
        const FOnlineProvidedCredentials &InProvidedCredentials,
        const TSoftObjectPtr<UWorld> &InWorld,
        const FPlatformHandle &InPlatformHandle,
        int32 InLocalUserNum);
    static FName ResolveWithCrossPlatform(
        const TSharedRef<FAuthenticationGraphRegistry> &InRegistry,
        const TSharedRef<Redpoint::EOS::Config::IConfig> &InConfig,
        const FOnlineProvidedCredentials &InProvidedCredentials,
        const TSoftObjectPtr<UWorld> &InWorld,
        const FPlatformHandle &InPlatformHandle,
        int32 InLocalUserNum);

public:
    static void Register();
};

}

namespace Redpoint::EOS::Auth::Graphs
{
REDPOINT_EOS_FILE_NS_EXPORT(1044168076, Redpoint::EOS::Auth::Graphs, FAuthenticationGraphDefault)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()