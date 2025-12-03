// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Synchronisation/SynchronisationDifference.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3679356872, Redpoint::EOS::Rooms::Synchronisation)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Synchronisation;

class REDPOINTEOSROOMS_API FRoomOwnerSynchronisationDifference : public ISynchronisationDifference
{
private:
    FRoomOwnerRef NewOwner;

public:
    FRoomOwnerSynchronisationDifference(FRoomOwnerRef InNewOwner);
    UE_NONCOPYABLE(FRoomOwnerSynchronisationDifference);
    virtual ~FRoomOwnerSynchronisationDifference() override = default;

    static FName GetDifferenceTypeStatic();
    virtual FName GetDifferenceType() const override;
    FRoomOwnerRef GetNewOwner() const;
};

}

namespace Redpoint::EOS::Rooms::Synchronisation
{
REDPOINT_EOS_FILE_NS_EXPORT(3679356872, Redpoint::EOS::Rooms::Synchronisation, FRoomOwnerSynchronisationDifference)
}

REDPOINT_EOS_CODE_GUARD_END()