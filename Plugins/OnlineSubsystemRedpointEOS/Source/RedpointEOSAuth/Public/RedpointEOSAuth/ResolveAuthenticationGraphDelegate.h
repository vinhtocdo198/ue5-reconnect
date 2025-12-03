// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Delegates/Delegate.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAuth/ForwardDecls.h"
#include "RedpointEOSAuth/OnlineProvidedCredentials.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSConfig/Config.h"
#include "UObject/NameTypes.h"
#include "UObject/SoftObjectPtr.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3107270426, Redpoint::EOS::Auth)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Auth;

typedef TDelegate<FName(
    const TSharedRef<FAuthenticationGraphRegistry> &InRegistry,
    const TSharedRef<Redpoint::EOS::Config::IConfig> &InConfig,
    const FOnlineProvidedCredentials &InProvidedCredentials,
    const TSoftObjectPtr<UWorld> &InWorld,
    const FPlatformHandle &InPlatformHandle,
    int32 InLocalUserNum)>
    FResolveAuthenticationGraphDelegate;

}

namespace Redpoint::EOS::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(3107270426, Redpoint::EOS::Auth, FResolveAuthenticationGraphDelegate)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()