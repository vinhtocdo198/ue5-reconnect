// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/OnlineExternalCredentialsRefreshComplete.h"
#include "RedpointEOSAuth/SupportedLinkType.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "UObject/SoftObjectPtr.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3436413633, Redpoint::EOS::Auth)
{
using namespace ::Redpoint::EOS::Auth;

/**
 * Similar to FOnlineAccountCredentials, but also capable of storing a refresh callback that is used to be obtain a new
 * token value when the existing token expires.
 *
 * This is a heap-allocated class unlike FOnlineAccountCredentials; you should construct implementations with
 * MakeShared<>.
 */
class REDPOINTEOSAUTH_API IOnlineExternalCredentials : public TSharedFromThis<IOnlineExternalCredentials>
{
public:
    IOnlineExternalCredentials() = default;
    virtual ~IOnlineExternalCredentials() = default;
    UE_NONCOPYABLE(IOnlineExternalCredentials);

    /** Returns the display name of the credential provider (e.g. "Steam"). Shown to the end user. */
    virtual FText GetProviderDisplayName() const = 0;

    /** Returns the type of credential. */
    virtual FString GetType() const = 0;

    /** Returns the ID (username or display name) for the credential. */
    virtual FString GetId() const = 0;

    /** Returns the token for authentication.. */
    virtual FString GetToken() const = 0;

    /**
     * Returns the display name for the local user account. Used on platforms where the EOS backend can not obtain the
     * user's display name by any other means.
     */
    virtual FString GetUserDisplayName() const
    {
        return TEXT("");
    }

    /**
     * Returns the type of linking that the specified cross-platform account provider can do with this credential type.
     * Primarily used with the Epic Games account provider.
     *
     * - A return value of `None` indicates that this credential can't be associated with a cross-platform account in
     * any meaningful way.
     * - A return value of `ExternalAuth` indicates that this credential can be directly associated with the
     * cross-platform account.
     * - A return value of `PersistentAuth` indicates that the refresh token for the cross-platform account needs to be
     * stored on the local device under a key or filename that is specific to the user's identity associated with this
     * credential (e.g. the Meta Quest user ID). StoreData and RetrieveData will be used by the cross-platform account
     * provider to store and retrieve data associated with this specific user's identity as needed.
     */
    virtual ESupportedLinkType GetProviderSupportedLinkType(FName PlatformName) const
    {
        return ESupportedLinkType::ExternalAuth;
    }

    /**
     * Store an FString by key
     * Stored values are associated with the user's ID to support the usecase of a shared devices with
     * multiple user accounts
     */
    virtual void StoreData(const FString &Key, const FString &Data) const
    {
    }

    /**
     * Read an FString by key
     * Stored values are associated with the user's ID to support the usecase of a shared devices with
     * multiple user accounts
     */
    virtual TOptional<FString> ReadData(const FString &Key) const
    {
        return TOptional<FString>();
    }

    /** Returns the authentication attributes for the user. */
    virtual TMap<FString, FString> GetAuthAttributes() const = 0;

    /**
     * The native subsystem name to use for external UI and e-commerce. Return NAME_None if there is no native
     * subsystem for these interfaces with this credential type. */
    virtual FName GetNativeSubsystemName() const = 0;

    /**
     * Refresh the credentials. OnComplete will be called when you can call GetType/GetId/GetToken again to get the new
     * values.
     */
    virtual void Refresh(
        TSoftObjectPtr<UWorld> InWorld,
        int32 LocalUserNum,
        FOnlineExternalCredentialsRefreshComplete OnComplete) = 0;
};

}

namespace Redpoint::EOS::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(3436413633, Redpoint::EOS::Auth, IOnlineExternalCredentials)
}

REDPOINT_EOS_CODE_GUARD_END()