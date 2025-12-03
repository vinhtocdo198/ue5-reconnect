// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSRooms/Features/InvitesRoomFeature.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1089720277, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class FInvitesRoomFeatureImpl : public IInvitesRoomFeature
{
public:
    bool bRejoinAfterKickRequiresInviteValue;

    virtual bool RejoinAfterKickRequiresInviteValue() const override;
    virtual TSharedPtr<IRoomFeatureRequestBase> GetFeatureRequestForUpdate() const override;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(1089720277, Redpoint::EOS::Rooms::Features, FInvitesRoomFeatureImpl)
}

REDPOINT_EOS_CODE_GUARD_END()