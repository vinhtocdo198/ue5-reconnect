// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

/**
 * Forward declarations of classes and types in other modules that RedpointEOSPlatform can not depend on due to how low
 * it is in the module hierarchy.
 */

// @todo: This should be removed once the legacy service is gone.
class FOnlineSessionSettings;

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(3531376250, Redpoint::EOS::UserCache, FUserInfo);
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_TYPEDEF(3531376250, Redpoint::EOS::UserCache, FUserInfoRef, TSharedRef<FUserInfo>);
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_TYPEDEF(3531376250, Redpoint::EOS::UserCache, FUserInfoPtr, TSharedPtr<FUserInfo>);
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(1941373948, Redpoint::EOS::UserCache, FExternalUserInfo);
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_TYPEDEF(
    1941373948,
    Redpoint::EOS::UserCache,
    FExternalUserInfoRef,
    TSharedRef<FExternalUserInfo>);
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_TYPEDEF(
    1941373948,
    Redpoint::EOS::UserCache,
    FExternalUserInfoPtr,
    TSharedPtr<FExternalUserInfo>);
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(1841536699, Redpoint::EOS::Presence, FPresenceUserState);
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(3258393336, Redpoint::EOS::Presence, FPresenceUserAdvertisedRoom);
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_ENUM(3356904104, Redpoint::EOS::Presence, EPresenceUserActivityState);
#endif
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(518246272, Redpoint::EOS::Rooms, FRoomId);
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_TYPEDEF(518246272, Redpoint::EOS::Rooms, FRoomIdRef, TSharedRef<const FRoomId>);
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(191830136, Redpoint::EOS::Auth, FAuthenticationGraphState);

REDPOINT_EOS_CODE_GUARD_END()