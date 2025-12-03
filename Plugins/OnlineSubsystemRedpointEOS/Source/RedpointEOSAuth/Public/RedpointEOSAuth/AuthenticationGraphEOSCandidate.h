// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/UnrealString.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSAuth/AuthenticationGraphEOSCandidateType.h"
#include "RedpointEOSAuth/AuthenticationGraphRefreshEOSCredentials.h"
#include "RedpointEOSAuth/CrossPlatform/CrossPlatformAccountId.h"
#include "RedpointEOSAuth/OnlineExternalCredentials.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1868914345, Redpoint::EOS::Auth)
{
using namespace ::Redpoint::EOS::Auth;

/**
 * An EOS candidate capable of being used to authenticate the user.
 */
struct REDPOINTEOSAUTH_API FAuthenticationGraphEOSCandidate
{
public:
    /** If this credential is displayed to the user as an option, what does it show to them? */
    FText DisplayName;

    /** The user auth attributes to set on FOnlineUserAccountEOS if this candidate is used. */
    TMap<FString, FString> UserAuthAttributes;

    /** The EOS product user ID; will only be present if this candidate has a valid EOS account. */
    EOS_ProductUserId ProductUserId;

    /** The continuance token if this account can be linked or created. */
    EOS_ContinuanceToken ContinuanceToken;

    /** The candidate type */
    EAuthenticationGraphEOSCandidateType Type;

    /** If this is a cross-platform candidate, this is the cross-platform account ID. */
    TSharedPtr<const CrossPlatform::FCrossPlatformAccountId> AssociatedCrossPlatformAccountId;

    /**
     * If this candidate is selected, this is the delegate that will be used by the identity subsystem to refresh
     * credentials when needed. The implementation of this delegate *must* call EOS_Connect_Login.
     */
    FAuthenticationGraphRefreshEOSCredentials RefreshCallback;

    /**
     * The external credentials used to sign this candidate in.
     */
    // NOLINTNEXTLINE(unreal-unsafe-storage-of-oss-pointer)
    TSharedPtr<IOnlineExternalCredentials> ExternalCredentials;

    /**
     * The name of the subsystem that should be used for external UI
     * and e-commerce if the user is authenticated with this candidate.
     */
    FName NativeSubsystemName;
};

}

namespace Redpoint::EOS::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(1868914345, Redpoint::EOS::Auth, FAuthenticationGraphEOSCandidate)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()