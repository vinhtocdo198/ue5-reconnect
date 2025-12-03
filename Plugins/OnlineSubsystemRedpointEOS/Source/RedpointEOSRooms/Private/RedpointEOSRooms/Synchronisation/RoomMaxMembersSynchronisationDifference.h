// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Synchronisation/SynchronisationDifference.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2565147049, Redpoint::EOS::Rooms::Synchronisation)
{
using namespace ::Redpoint::EOS::Rooms::Synchronisation;

class REDPOINTEOSROOMS_API FRoomMaxMembersSynchronisationDifference : public ISynchronisationDifference
{
private:
    uint32 NewMaxMembers;

public:
    FRoomMaxMembersSynchronisationDifference(uint32 InNewMaxMembers);
    UE_NONCOPYABLE(FRoomMaxMembersSynchronisationDifference);
    virtual ~FRoomMaxMembersSynchronisationDifference() override = default;

    static FName GetDifferenceTypeStatic();
    virtual FName GetDifferenceType() const override;
    uint32 GetNewMaxMembers() const;
};

}

namespace Redpoint::EOS::Rooms::Synchronisation
{
REDPOINT_EOS_FILE_NS_EXPORT(2565147049, Redpoint::EOS::Rooms::Synchronisation, FRoomMaxMembersSynchronisationDifference)
}

REDPOINT_EOS_CODE_GUARD_END()