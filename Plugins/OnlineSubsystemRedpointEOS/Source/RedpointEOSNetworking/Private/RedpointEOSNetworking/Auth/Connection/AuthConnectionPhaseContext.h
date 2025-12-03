// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSNetworking/Auth/AuthPhaseContext.h"
#include "RedpointEOSNetworking/Auth/Connection/AuthConnectionPhase.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1581436555, Redpoint::EOS::Networking::Auth::Connection)
{
using namespace ::Redpoint::EOS::Networking::Auth;
using namespace ::Redpoint::EOS::Networking::Auth::Connection;

class FAuthConnectionPhaseContext : public FAuthPhaseContext<IAuthConnectionPhase, FAuthConnectionPhaseContext>,
                                    public TSharedFromThis<FAuthConnectionPhaseContext>
{
protected:
    virtual FString GetIdentifier() const override;
    virtual FString GetPhaseGroup() const override;

public:
    FAuthConnectionPhaseContext(URedpointEOSControlChannel *InControlChannel);
    UE_NONCOPYABLE(FAuthConnectionPhaseContext);
    virtual ~FAuthConnectionPhaseContext() override = default;

    /** Marks the connection as "trusted" on the client, which allows sending sensitive data on the legacy network
     * authentication path. */
    void MarkConnectionAsTrustedOnClient();
};

}

namespace Redpoint::EOS::Networking::Auth::Connection
{
REDPOINT_EOS_FILE_NS_EXPORT(1581436555, Redpoint::EOS::Networking::Auth::Connection, FAuthConnectionPhaseContext)
}

REDPOINT_EOS_CODE_GUARD_END()