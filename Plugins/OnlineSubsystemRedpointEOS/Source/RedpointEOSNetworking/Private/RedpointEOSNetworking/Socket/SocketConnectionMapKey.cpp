// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/Socket/SocketConnectionMapKey.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSCore/Id/Id.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(800251675, Redpoint::EOS::Networking::Socket)
{
using namespace ::Redpoint::EOS::API::P2P;
using namespace ::Redpoint::EOS::Core::Id;

FSocketConnectionMapKey::FSocketConnectionMapKey(
    UE::Online::FAccountId InLocalUserId,
    UE::Online::FAccountId InRemoteUserId,
    FString InSymmetricSocketId,
    uint8 InSymmetricChannel)
    : LocalUserId(InLocalUserId)
    , RemoteUserId(InRemoteUserId)
    , SymmetricSocketId(InSymmetricSocketId)
    , SymmetricChannel(InSymmetricChannel)
{
}

FSocketConnectionMapKey::FSocketConnectionMapKey(const FSocketConnectionMapKey &Other)
    : LocalUserId(Other.LocalUserId)
    , RemoteUserId(Other.RemoteUserId)
    , SymmetricSocketId(Other.SymmetricSocketId)
    , SymmetricChannel(Other.SymmetricChannel)
{
}

uint32 GetTypeHash(const FSocketConnectionMapKey &A)
{
    return GetTypeHash(A.ToString());
};

FString FSocketConnectionMapKey::ToString(bool bExcludeChannel) const
{
    auto LocalUserIdString = GetUserIdString(this->LocalUserId);
    auto RemoteUserIdString = GetUserIdString(this->RemoteUserId);

    if (bExcludeChannel)
    {
        return FString::Printf(TEXT("%s:%s:%s"), *LocalUserIdString, *RemoteUserIdString, *this->SymmetricSocketId);
    }

    return FString::Printf(
        TEXT("%s:%s:%s:%u"),
        *LocalUserIdString,
        *RemoteUserIdString,
        *this->SymmetricSocketId,
        this->SymmetricChannel);
}

bool FSocketConnectionMapKey::IsIdenticalExcludingChannel(const FSocketConnectionMapKey &Other) const
{
    return this->ToString(true) == Other.ToString(true);
}

const UE::Online::FAccountId &FSocketConnectionMapKey::GetLocalUserId() const
{
    return this->LocalUserId;
}

const UE::Online::FAccountId &FSocketConnectionMapKey::GetRemoteUserId() const
{
    return this->RemoteUserId;
}

const FSocketId &FSocketConnectionMapKey::GetSymmetricSocketId() const
{
    return this->SymmetricSocketId;
}

const uint8 &FSocketConnectionMapKey::GetSymmetricChannel() const
{
    return this->SymmetricChannel;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()