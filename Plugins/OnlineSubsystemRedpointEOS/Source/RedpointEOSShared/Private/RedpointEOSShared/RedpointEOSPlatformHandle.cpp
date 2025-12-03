// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSShared/RedpointEOSPlatformHandle.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

#define REDPOINT_EOS_CALL_NOT_SUPPORTED()                                                                              \
    checkf(                                                                                                            \
        false,                                                                                                         \
        TEXT(                                                                                                          \
            "This call is not supported. The Redpoint EOS plugin only contains an IEOSPlatformHandle implementation "  \
            "to support the Fab plugin, and is not compatible with other plugins that depend on EOSShared. If you're " \
            "seeing this assert, make sure you disable the OnlineSubsystemEOS plugin (by Epic) and any other plugins " \
            "that depend on EOSShared."))

namespace REDPOINT_EOS_FILE_NS_ID(3627867287, Redpoint::EOS::Shared)
{

FRedpointEOSPlatformHandle::FRedpointEOSPlatformHandle(EOS_HPlatform InPlatformHandle)
    : IEOSPlatformHandle(InPlatformHandle)
{
}

void FRedpointEOSPlatformHandle::Tick()
{
    REDPOINT_EOS_CALL_NOT_SUPPORTED();
}

FString FRedpointEOSPlatformHandle::GetConfigName() const
{
    REDPOINT_EOS_CALL_NOT_SUPPORTED();
    return TEXT("");
}

FString FRedpointEOSPlatformHandle::GetOverrideCountryCode() const
{
    REDPOINT_EOS_CALL_NOT_SUPPORTED();
    return TEXT("");
}

FString FRedpointEOSPlatformHandle::GetOverrideLocaleCode() const
{
    REDPOINT_EOS_CALL_NOT_SUPPORTED();
    return TEXT("");
}

void FRedpointEOSPlatformHandle::LogInfo(int32 Indent) const
{
    REDPOINT_EOS_CALL_NOT_SUPPORTED();
}

void FRedpointEOSPlatformHandle::LogAuthInfo(const EOS_EpicAccountId LoggedInAccount, int32 Indent) const
{
    REDPOINT_EOS_CALL_NOT_SUPPORTED();
}

void FRedpointEOSPlatformHandle::LogUserInfo(
    const EOS_EpicAccountId LoggedInAccount,
    const EOS_EpicAccountId TargetAccount,
    int32 Indent) const
{
    REDPOINT_EOS_CALL_NOT_SUPPORTED();
}

void FRedpointEOSPlatformHandle::LogPresenceInfo(
    const EOS_EpicAccountId LoggedInAccount,
    const EOS_EpicAccountId TargetAccount,
    int32 Indent) const
{
    REDPOINT_EOS_CALL_NOT_SUPPORTED();
}

void FRedpointEOSPlatformHandle::LogFriendsInfo(const EOS_EpicAccountId LoggedInAccount, int32 Indent) const
{
    REDPOINT_EOS_CALL_NOT_SUPPORTED();
}

void FRedpointEOSPlatformHandle::LogConnectInfo(const EOS_ProductUserId LoggedInAccount, int32 Indent) const
{
    REDPOINT_EOS_CALL_NOT_SUPPORTED();
}

}

#undef REDPOINT_EOS_CALL_NOT_SUPPORTED

#endif

REDPOINT_EOS_CODE_GUARD_END()