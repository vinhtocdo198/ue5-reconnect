// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Synchronisation/SynchronisationDifference.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4071186716, Redpoint::EOS::Rooms::Synchronisation)
{
using namespace ::Redpoint::EOS::Rooms::Synchronisation;

class REDPOINTEOSROOMS_API FRoomFeatureSynchronisationDifference : public ISynchronisationDifference
{
public:
    enum class EChangeType : uint8
    {
        Added,
        Updated,
        Removed,
    };

private:
    FName FeatureName;
    EChangeType ChangeType;

public:
    FRoomFeatureSynchronisationDifference(FName InFeatureName, EChangeType InChangeType);
    UE_NONCOPYABLE(FRoomFeatureSynchronisationDifference);
    virtual ~FRoomFeatureSynchronisationDifference() override = default;

    static FName GetDifferenceTypeStatic();
    virtual FName GetDifferenceType() const override;
    FName GetFeatureName() const;
    EChangeType GetChangeType() const;
};

}

namespace Redpoint::EOS::Rooms::Synchronisation
{
REDPOINT_EOS_FILE_NS_EXPORT(4071186716, Redpoint::EOS::Rooms::Synchronisation, FRoomFeatureSynchronisationDifference)
}

REDPOINT_EOS_CODE_GUARD_END()