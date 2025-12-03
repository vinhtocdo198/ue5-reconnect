// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "IPAddress.h"
#include "Online/CoreOnline.h"
#include "RedpointEOSAPI/P2P/SocketId.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3518527788, Redpoint::EOS::Networking::Socket)
{
using namespace ::Redpoint::EOS::API::P2P;

class REDPOINTEOSNETWORKING_API FInternetAddrRedpointEOS : public FInternetAddr
{
private:
    UE::Online::FAccountId UserId;
    FSocketId SymmetricSocketName;
    uint8 SymmetricChannel;

public:
    FInternetAddrRedpointEOS(
        UE::Online::FAccountId InUserId,
        const FSocketId &InSymmetricSocketName,
        uint8 InSymmetricChannel);
    FInternetAddrRedpointEOS();
    UE_NONCOPYABLE(FInternetAddrRedpointEOS);
    virtual ~FInternetAddrRedpointEOS() override = default;

    void SetFromParameters(
        UE::Online::FAccountId InUserId,
        const FSocketId &InSymmetricSocketName,
        uint8 InSymmetricChannel);
    UE::Online::FAccountId GetUserId() const;

    FSocketId GetSymmetricSocketId() const;

    virtual void SetIp(uint32 InAddr) override;
    virtual void SetIp(const TCHAR *InAddr, bool &bIsValid) override;
    virtual void GetIp(uint32 &OutAddr) const override;
    virtual void SetPort(int32 InPort) override;
    virtual int32 GetPort() const override;
    virtual void SetSymmetricChannel(uint8 InSymmetricChannel);
    virtual uint8 GetSymmetricChannel() const;
    virtual void SetRawIp(const TArray<uint8> &RawAddr) override;
    virtual TArray<uint8> GetRawIp() const override;
    virtual void SetAnyAddress() override;
    virtual void SetBroadcastAddress() override;
    virtual void SetLoopbackAddress() override;
    virtual FString ToString(bool bAppendPort) const override;
    virtual uint32 GetTypeHash() const override;
    virtual bool IsValid() const override;
    virtual TSharedRef<FInternetAddr> Clone() const override;

    virtual bool operator==(const FInternetAddr &Other) const override;

    virtual FName GetProtocolType() const override;
};

}

namespace Redpoint::EOS::Networking::Socket
{
REDPOINT_EOS_FILE_NS_EXPORT(3518527788, Redpoint::EOS::Networking::Socket, FInternetAddrRedpointEOS)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()