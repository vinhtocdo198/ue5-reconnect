// Copyright June Rhodes. All Rights Reserved.

#pragma once

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && defined(REDPOINT_EOS_STEAM_ENABLED)

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(
    724733551,
    Redpoint::EOS::Platform::Integration::Steam::Auth,
    FAuthTicketCallbackHandler)
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(
    1893130842,
    Redpoint::EOS::Platform::Integration::Steam,
    FRuntimePlatformIntegrationSteam)
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_STRUCT(
    4123366627,
    Redpoint::EOS::Platform::Integration::Steam::Auth,
    FSteamCredentialInfo)
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(
    4001727958,
    Redpoint::EOS::Platform::Integration::Steam::Auth,
    FSteamCredentialObtainer)
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(
    4063608760,
    Redpoint::EOS::Platform::Integration::Steam::Auth,
    FSteamExternalCredentials)

#endif