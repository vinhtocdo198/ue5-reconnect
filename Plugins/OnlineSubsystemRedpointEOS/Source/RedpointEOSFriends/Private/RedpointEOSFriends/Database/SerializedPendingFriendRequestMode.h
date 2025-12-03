// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3509148548, Redpoint::EOS::Friends::Database)
{

enum class ESerializedPendingFriendRequestMode : uint8
{
    /**
     * We couldn't reach the target player during the original SendInvite call, so
     * we've scheduled to try again later.
     */
    OutboundPendingSend,

    /**
     * No longer used, but kept so that the numeric values of the enum remain the same.
     */
    __NoLongerUsed_OutboundPendingReceive,

    /**
     * The target user has received the request (and has told us so). We're waiting
     * on the user at the other end to make an accept/reject decision.
     */
    OutboundPendingResponse,

    /**
     * We've had a friend request arrive, but the user is yet to pick whether to
     * accept or reject it.
     */
    InboundPendingResponse,

    /**
     * We've accepted a friend request, but we haven't been able to tell the sender
     * that we've accepted it yet (due to them being offline).
     */
    InboundPendingSendAcceptance,

    /**
     * We've rejected a friend request, but we haven't been able to tell the sender
     * that we've rejected it yet (due to them being offline).
     */
    InboundPendingSendRejection,

    /**
     * We've deleted a friend, but we haven't been able to tell them to remove us
     * from their friends list yet (due to them being offline).
     */
    OutboundPendingSendDeletion,
};

}

namespace Redpoint::EOS::Friends::Database
{
REDPOINT_EOS_FILE_NS_EXPORT(3509148548, Redpoint::EOS::Friends::Database, ESerializedPendingFriendRequestMode);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()