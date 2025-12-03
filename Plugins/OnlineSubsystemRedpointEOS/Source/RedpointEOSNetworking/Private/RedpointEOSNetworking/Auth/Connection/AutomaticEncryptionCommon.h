// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSControlChannel.h"
#include "RedpointEOSNetworking/Auth/Connection/AuthConnectionPhase.h"
#include "RedpointEOSNetworking/Auth/Connection/AuthConnectionPhaseContext.h"
#include "RedpointLibHydrogen.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2729212981, Redpoint::EOS::Networking::Auth::Connection)
{
using namespace ::Redpoint::EOS::Networking::Auth::Connection;

class IAutomaticEncryptionCommon : public IAuthConnectionPhase
{
public:
    virtual ~IAutomaticEncryptionCommon() override = default;

    static void RegisterRoutes(URedpointEOSControlChannel *ControlChannel);

    virtual void On_NMT_EOS_RequestClientEphemeralKey(
        const TSharedRef<FAuthConnectionPhaseContext> &Context,
        FInBunch &Bunch) = 0;
    virtual void On_NMT_EOS_DeliverClientEphemeralKey(
        const TSharedRef<FAuthConnectionPhaseContext> &Context,
        FInBunch &Bunch) = 0;
    virtual void On_NMT_EOS_SymmetricKeyExchange(
        const TSharedRef<FAuthConnectionPhaseContext> &Context,
        FInBunch &Bunch) = 0;
    virtual void On_NMT_EOS_EnableEncryption(
        const TSharedRef<FAuthConnectionPhaseContext> &Context,
        FInBunch &Bunch) = 0;
};

}

namespace Redpoint::EOS::Networking::Auth::Connection
{
REDPOINT_EOS_FILE_NS_EXPORT(2729212981, Redpoint::EOS::Networking::Auth::Connection, IAutomaticEncryptionCommon)
}

REDPOINT_EOS_CODE_GUARD_END()