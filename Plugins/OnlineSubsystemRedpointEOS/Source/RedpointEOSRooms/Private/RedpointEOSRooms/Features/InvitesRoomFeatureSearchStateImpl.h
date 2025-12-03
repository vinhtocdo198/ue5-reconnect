// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/InvitesRoomFeatureSearchState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3535887801, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API FInvitesRoomFeatureSearchStateImpl : public IInvitesRoomFeatureSearchState
{
public:
    bool bRejoinAfterKickRequiresInvite;

    FInvitesRoomFeatureSearchStateImpl();
    UE_NONCOPYABLE(FInvitesRoomFeatureSearchStateImpl);
    virtual ~FInvitesRoomFeatureSearchStateImpl() override = default;

    virtual bool RejoinAfterKickRequiresInvite() const override;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(3535887801, Redpoint::EOS::Rooms::Features, FInvitesRoomFeatureSearchStateImpl)
}

REDPOINT_EOS_CODE_GUARD_END()