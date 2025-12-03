// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "PacketHandler.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1870867386, Redpoint::EOS::Networking)
{

class FEmptyHandshakeHandlerComponent : public HandlerComponent, public TSharedFromThis<FEmptyHandshakeHandlerComponent>
{
public:
    FEmptyHandshakeHandlerComponent();
    UE_NONCOPYABLE(FEmptyHandshakeHandlerComponent);
    virtual ~FEmptyHandshakeHandlerComponent() override = default;
    virtual void NotifyHandshakeBegin() override;
    virtual bool IsValid() const override;
    virtual void Initialize() override;
    virtual int32 GetReservedPacketBits() const override;
    void SetAsInitialized();
};

}

namespace Redpoint::EOS::Networking
{
REDPOINT_EOS_FILE_NS_EXPORT(1870867386, Redpoint::EOS::Networking, FEmptyHandshakeHandlerComponent)
}

REDPOINT_EOS_CODE_GUARD_END()