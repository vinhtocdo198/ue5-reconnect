// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Engine/Texture.h"
#include "Online/CoreOnline.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSPlatform/Services/ForwardDecls.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformServiceCallContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(235436569, Redpoint::EOS::Platform::Services)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Platform::Services;

class REDPOINTEOSPLATFORM_API IRuntimePlatformAuthService
{
public:
    IRuntimePlatformAuthService() = default;
    UE_NONCOPYABLE(IRuntimePlatformAuthService);
    virtual ~IRuntimePlatformAuthService() = default;

    /**
     * Returns the platform name, which is used by platform-specific authentication graphs to identify the
     * implementation they are interested in. For example, this allows the "Steam Only" authentication graph to pick the
     * Steam-specific implementation instead of using GetSelectionScore.
     */
    virtual FName GetPlatformName() const = 0;

    /**
     * Returns a "score" that determines which IRuntimePlatformAuthService should be used when the default
     * authentication graph is being used. A negative score means that this platform can't currently authenticate (e.g.
     * if the implementation was Steam but the Steamworks API isn't available), in which case this implementation is
     * never selected. Otherwise, the highest non-zero score is the implementation selected for the default
     * authentication graph.
     */
    virtual int32 GetSelectionScore(const FRuntimePlatformServiceCallContextRef &CallContext) const = 0;

    /**
     * The delegate used when the EnsureSignedInOnLocalPlatform operation completes. ErrorCode should be Success() only
     * if the user is now signed in.
     */
    typedef TDelegate<void(const FError &ErrorCode)> FOnEnsureSignedInOnLocalPlatform;

    /**
     * Ensure that the local user is currently signed into a platform account. This should ensure that the current input
     * device (e.g. controller) specified by the local user number is signed into a platform account such as Steam,
     * PlayStation, etc. On some platforms, this call may prompt the user to select a local platform account or provide
     * credentials.
     *
     * If the local user can't be signed into a local platform account, authentication will not proceed under the
     * default authentication graph.
     */
    virtual void EnsureSignedInOnLocalPlatform(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FOnEnsureSignedInOnLocalPlatform &OnComplete) = 0;

    /**
     * The delegate used when the IsOnlinePlayPermitted operation completes. ErrorCode should be Success() if the user
     * is permitted to play online, and the relevant error message if not permitted.
     */
    typedef TDelegate<void(const FError &ErrorCode)> FOnIsOnlinePlayPermitted;

    /**
     * Checks with the current platform whether the local user is permitted play online (and proceed with signing into
     * Epic Online Services). Some platforms disallow this unless the user currently has an "online/multiplayer services
     * subscription".
     */
    virtual void IsOnlinePlayPermitted(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FOnIsOnlinePlayPermitted &OnComplete) = 0;

    /**
     * The delegate used when the IsOnlinePlayPermitted operation completes. ErrorCode should be Success() only if
     * credentials were successfully obtained.
     */
    typedef TDelegate<void(const FError &ErrorCode)> FOnGetCredentialsFromLocalPlatformDuringAuthenticationGraph;

    /**
     * The implementation to execute inside the authentication graph to get the credentials from the local platform.
     * This function should add an entry to State->AvailableExternalCredentials before invoking the callback if
     * credentials can be successfully obtained.
     */
    virtual void GetCredentialsFromLocalPlatformDuringAuthenticationGraph(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const TSharedRef<FAuthenticationGraphState> &State,
        const FOnGetCredentialsFromLocalPlatformDuringAuthenticationGraph &OnComplete) = 0;
};

}

namespace Redpoint::EOS::Platform::Services
{
REDPOINT_EOS_FILE_NS_EXPORT(235436569, Redpoint::EOS::Platform::Services, IRuntimePlatformAuthService)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()