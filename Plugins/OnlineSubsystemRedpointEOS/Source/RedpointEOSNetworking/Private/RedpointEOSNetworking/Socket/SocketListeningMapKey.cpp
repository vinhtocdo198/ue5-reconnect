// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/Socket/SocketListeningMapKey.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSCore/Id/Id.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3549223374, Redpoint::EOS::Networking::Socket)
{
using namespace ::Redpoint::EOS::Core::Id;

FSocketListeningMapKey::FSocketListeningMapKey(
    const UE::Online::FAccountId InLocalUserId,
    const FString InSymmetricSocketId)
    : LocalUserId(InLocalUserId)
    , SymmetricSocketId(InSymmetricSocketId)
{
}

FSocketListeningMapKey::FSocketListeningMapKey(const FSocketListeningMapKey &Other)
    : LocalUserId(Other.LocalUserId)
    , SymmetricSocketId(Other.SymmetricSocketId)
{
}

uint32 GetTypeHash(const FSocketListeningMapKey &A)
{
    return GetTypeHash(A.ToString());
}

FString FSocketListeningMapKey::ToString() const
{
    auto LocalUserIdString = GetUserIdString(this->LocalUserId);

    return FString::Printf(TEXT("%s:%s"), *LocalUserIdString, *this->SymmetricSocketId);
}

const UE::Online::FAccountId &FSocketListeningMapKey::GetLocalUserId() const
{
    return this->LocalUserId;
}

const FSocketId &FSocketListeningMapKey::GetSymmetricSocketId() const
{
    return this->SymmetricSocketId;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()