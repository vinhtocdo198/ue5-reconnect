// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "EOSUserInterface_SignInOrCreateAccount_Choice.h"
#include "Engine/World.h"
#include "Misc/Variant.h"
#include "RedpointEOSAPI/Connect/Login.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAuth/AuthenticationGraphEOSCandidate.h"
#include "RedpointEOSAuth/AuthenticationGraphRefreshEOSCredentials.h"
#include "RedpointEOSAuth/AuthenticationGraphUserInterface.h"
#include "RedpointEOSAuth/CrossPlatform/CrossPlatformAccountProvider.h"
#include "RedpointEOSAuth/ForwardDecls.h"
#include "RedpointEOSAuth/OnlineExternalCredentials.h"
#include "RedpointEOSAuth/OnlineProvidedCredentials.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSCore/Id/UniqueNetIdEOS.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformAuthService.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(191830136, Redpoint::EOS::Auth)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::Config;
using namespace ::Redpoint::EOS::Platform::Services;

class REDPOINTEOSAUTH_API FAuthenticationGraphState : public TSharedFromThis<FAuthenticationGraphState>
{
    friend FAuthenticationGraph;

private:
    /**
     * The selected EOS candidate - FLoginWithSelectedEOSAccountNode will call CreateUser if the candidate only has a
     * continuation token.
     */
    FAuthenticationGraphEOSCandidate SelectedEOSCandidate;
    bool HasSelectedEOSCandidateFlag;

    /**
     * The current user interface widget displayed to the user.
     */
    TSoftObjectPtr<UUserWidget> CurrentWidget;
    bool bWasMouseCursorShown;

    /** The last cached world instance. */
    TSoftObjectPtr<UWorld> LastCachedWorld;

    /** A list of nodes to execute when the user signs out, and to execute if the authentication graph fails. */
    TArray<TSharedPtr<FAuthenticationGraphNode>> LogoutNodes;

    // If true, we've tried to get the auth service already (we only attempt once).
    bool bHasObtainedAuthService;

    // The auth service that we obtained, or nullptr if no matching auth service was selected.
    TSharedPtr<IRuntimePlatformAuthService> AuthService;

public:
    FAuthenticationGraphState(
        const FPlatformHandle &InPlatformHandle,
        const TSharedRef<IAuthenticationGraphUserInterface> &InUserInterface,
        int32 InLocalUserNum,
        FName InWorldContextHandle,
        const TSharedRef<IConfig> &InConfig);
    ~FAuthenticationGraphState();
    UE_NONCOPYABLE(FAuthenticationGraphState);

    /** The user interface handler, which allows authentication to pass widgets to the developer for custom rendering in
     * VR. */
    TSharedRef<IAuthenticationGraphUserInterface> UserInterface;

    /** The current configuration. */
    TSharedPtr<IConfig> Config;

    /** The EOS SDK platform handle. */
    FPlatformHandle PlatformHandle;

    /** The local user number of the user that is being authenticated. */
    int32 LocalUserNum;

    /**
     * If this authentication process is being run with an already authenticated user, this is their existing user ID.
     */
    TSharedPtr<const FUniqueNetIdEOS> ExistingUserId;

    /**
     * If this authentication process is being run with an already authenticated user, these are the external
     * credentials that were originally used to authenticate them. This field can be used by cross-platform account
     * providers to link the local platform credentials against a cross-platform account.
     */
    // NOLINTNEXTLINE(unreal-unsafe-storage-of-oss-pointer)
    TSharedPtr<const IOnlineExternalCredentials> ExistingExternalCredentials;

    /**
     * If this authentication process is being run with an already authenticated user, this is their cross-platform
     * account ID.
     */
    TSharedPtr<const CrossPlatform::FCrossPlatformAccountId> ExistingCrossPlatformAccountId;

    /**
     * If this authentication process is being run with an already authenticated user, these are the existing
     * authentication attributes (useful when linking and wanting to merge auth attributes).
     */
    TMap<FString, FString> ExistingAuthAttributes;

    /**
     * These credentials are not used by any of the default authentication flows, but if you want to pass custom data
     * into a custom authentication graph, you can use the ::Login method on the identity interface and the credentials
     * you provide to that function will be available here.
     */
    FOnlineProvidedCredentials ProvidedCredentials;

    /** A list of error messages that occurred during authentication. */
    TArray<FString> ErrorMessages;

    /**
     * The provider for cross-platform accounts.
     */
    TSharedPtr<const CrossPlatform::ICrossPlatformAccountProvider> CrossPlatformAccountProvider;

    /**
     * The authenticated cross-platform account ID, present when a cross-platform provider is enabled and the user has
     * been signed into a cross-platform account.
     *
     * Note that this is NOT the field that is used to determine if the user has logged into the game with a
     * cross-platform account. This field is just used internally between cross-platform authentication graph nodes to
     * track the cross-platform account ID.
     *
     * The cross-platform account ID must still eventually end up associated with an EOS candidate and stored in the
     * AssociatedCrossPlatformAccountId field. Then, if the user selects this EOS candidate, it will be placed into the
     * ResultCrossPlatformAccountId field which is what FOnlineIdentityEOS reads to actually set the user's logged in
     * state.
     */
    TSharedPtr<const CrossPlatform::FCrossPlatformAccountId> AuthenticatedCrossPlatformAccountId;

    /**
     * Casts and returns the account ID to the given type if the account ID belongs to the expected provider.
     */
    template <typename T> TSharedPtr<const T> GetAuthenticatedCrossPlatformAccountId(FName ProviderName) const
    {
        if (this->CrossPlatformAccountProvider.IsValid() &&
            this->CrossPlatformAccountProvider->GetName().IsEqual(ProviderName))
        {
            return StaticCastSharedPtr<const T>(this->AuthenticatedCrossPlatformAccountId);
        }
        return nullptr;
    }

    /**
     * DEPRECATED: Please do not use this function in new code.
     */
    EOS_EpicAccountId GetAuthenticatedEpicAccountId() const;

    /**
     * The available external credentials collected by the authentication graph. These credentials may be used by a
     * cross-platform account provider to try to implicitly sign the user into a cross-platform account.
     */
    TArray<TSharedRef<IOnlineExternalCredentials>> AvailableExternalCredentials;

    /**
     * Keeps track of the credential names we've already tried with the EAS Developer Authentication Tool so we don't
     * bother making duplicate requests.
     */
    TArray<FString> AttemptedDeveloperAuthenticationCredentialNames;

    /** The name of the current world context handle. */
    FName WorldContextHandle;

    /** A list of EOS candidates that the authentication process can select from. */
    TArray<FAuthenticationGraphEOSCandidate> EOSCandidates;

    /** The authenticated user ID from a successful login process. */
    TSharedPtr<const FUniqueNetIdEOS> ResultUserId;

    /** The external credentials that were used to authenticate the user in ResultUserId. */
    // NOLINTNEXTLINE(unreal-unsafe-storage-of-oss-pointer)
    TSharedPtr<IOnlineExternalCredentials> ResultExternalCredentials;

    /** The authenticated cross-platform account ID from a successful login process. */
    TSharedPtr<const CrossPlatform::FCrossPlatformAccountId> ResultCrossPlatformAccountId;

    /** The callback used to refresh the credentials and call EOS_Connect_Login for the authenticated user. */
    FAuthenticationGraphRefreshEOSCredentials ResultRefreshCallback;

    /** The native subsystem to use for external UI and e-commerce. */
    FName ResultNativeSubsystemName;

    /** The last choice made with regards to signing in or creating an account. */
    EEOSUserInterface_SignInOrCreateAccount_Choice LastSignInChoice;

#if !UE_BUILD_SHIPPING
    /** The email address to use for automated testing login. */
    FString AutomatedTestingEmailAddress;

    /** The password to use for automated testing login. */
    FString AutomatedTestingPassword;
#endif

    /** Storage of custom metadata between nodes. You can store whatever you like in this field. */
    TMap<FString, FVariant> Metadata;

    /** Storage of an EAS external continuance token, as it can not be stored portably inside Metadata. */
    EOS_ContinuanceToken EASExternalContinuanceToken;

private:
    /** This field is now accessed through GetResultUserAuthAttributes, AddResultUserAuthAttribute and
     * AddResultUserAuthAttributes. */
    TMap<FString, FString> ResultUserAuthAttributes;

public:
    /** A list of nodes in the graph registered by name. Used by the JumpToNamedNode node. */
    TMap<FName, TSharedPtr<FAuthenticationGraphNode>> RegisteredNodes;

    /** Gets a reference to the current active world, or null if no world could be found. */
    UWorld *GetWorld();

    /**
     * Adds an EOS candidate based on IOnlineExternalCredentials. Automatically handles wiring up the refresh callback.
     *
     * @param Data                      The result of the EOS_Connect_Login call.
     * @param ExternalCredentials       The external credentials to use.
     * @param InType                    The type of credential that this is.
     * @param InCrossPlatformAccountId  If this is a cross-platform credential (via InType), this is the cross-platform
     *                                  account ID.
     */
    FAuthenticationGraphEOSCandidate AddEOSConnectCandidateFromExternalCredentials(
        const Redpoint::EOS::API::Connect::FLogin::Result &Data,
        const TSharedRef<IOnlineExternalCredentials> &ExternalCredentials,
        EAuthenticationGraphEOSCandidateType InType = EAuthenticationGraphEOSCandidateType::Generic,
        TSharedPtr<const CrossPlatform::FCrossPlatformAccountId> InCrossPlatformAccountId = nullptr);

    /**
     * Adds an EOS candidate manually.
     *
     * @param ProviderDisplayName       The display name of the provider of this credential.
     * @param UserAuthAttributes        The user authentication attributes to provide if this credential is used to sign
     *                                  in.
     * @param Data                      The result of the EOS_Connect_Login call.
     * @param RefreshCallback           The callback used to refresh login credentials when needed.
     * @param InNativeSubsystemName     The native subsystem to use for external UI and e-commerce.
     * @param InType                    The type of credential that this is.
     * @param InCrossPlatformAccountId  If this is a cross-platform credential (via InType), this is the cross-platform
     *                                  account ID.
     */
    FAuthenticationGraphEOSCandidate AddEOSConnectCandidate(
        FText ProviderDisplayName,
        TMap<FString, FString> UserAuthAttributes,
        const Redpoint::EOS::API::Connect::FLogin::Result &Data,
        FAuthenticationGraphRefreshEOSCredentials RefreshCallback,
        const FName &InNativeSubsystemName,
        EAuthenticationGraphEOSCandidateType InType = EAuthenticationGraphEOSCandidateType::Generic,
        TSharedPtr<const CrossPlatform::FCrossPlatformAccountId> InCrossPlatformAccountId = nullptr);

    /** Select the specified EOS candidate as the one to authenticate with. */
    void SelectEOSCandidate(const FAuthenticationGraphEOSCandidate &Candidate);

    /** Returns whether or not an EOS candidate has been selected. */
    bool HasSelectedEOSCandidate();

    /** Returns the currently selected EOS candidate. Only valid if HasSelectedEOSCandidate returns true. */
    FAuthenticationGraphEOSCandidate GetSelectedEOSCandidate();

    /** Internally used by UserInterfaceRef. Do not call this function directly. */
    bool HasCurrentUserInterfaceWidget();

    /** Internally used by UserInterfaceRef. Do not call this function directly. */
    void SetCurrentUserInterfaceWidget(class UUserWidget *InWidget);

    /** Internally used by UserInterfaceRef. Do not call this function directly. */
    void ClearCurrentUserInterfaceWidget();

    /** Add a logout node to the authentication graph. This node will run when the user signs out, or if the
     * authentication graph fails, in order from last-added to first-added. */
    void AddLogoutNode(const TSharedRef<FAuthenticationGraphNode> &Node);

    /**
     * Returns the runtime platform auth service that should be used for authentication.
     */
    TSharedPtr<IRuntimePlatformAuthService> GetAuthService();

    /**
     * For static usage outside of the authentication graph state instance.
     */
    static TSharedPtr<IRuntimePlatformAuthService> GetAuthServiceStatic(
        const FPlatformHandle &InPlatformHandle,
        const TSharedRef<IConfig> &InConfig,
        const TSoftObjectPtr<UWorld> &InWorld,
        int32 InLocalUserNum);

    /** The attributes will be propagated to the FUserOnlineAccountEOS and available through GetAuthAttribute. */
    const TMap<FString, FString> &GetResultUserAuthAttributes() const;

    /** Add a single user auth attribute to the result. */
    void AddResultUserAuthAttribute(const FString &Key, const FString &Value);

    /** Add a set of user auth attributes to the result, merging them into the current state. */
    void AddResultUserAuthAttributes(const TMap<FString, FString> &Attributes);
};

}

namespace Redpoint::EOS::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(191830136, Redpoint::EOS::Auth, FAuthenticationGraphState)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()