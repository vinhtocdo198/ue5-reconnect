// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Synchronisation/SynchronisationDifference.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1699210340, Redpoint::EOS::Rooms::Synchronisation)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Synchronisation;

class REDPOINTEOSROOMS_API FRoomAttributeSynchronisationDifference : public ISynchronisationDifference
{
public:
    enum class EChangeType : uint8
    {
        Added,
        Removed,
        Modified,
    };

private:
    FString AttributeKey;
    EChangeType ChangeType;
    FRoomAttribute OldAttributeValue;
    FRoomAttribute NewAttributeValue;

public:
    FRoomAttributeSynchronisationDifference(
        FString InAttributeKey,
        EChangeType InChangeType,
        const FRoomAttribute &InOldAttributeValue,
        const FRoomAttribute &InNewAttributeValue);
    UE_NONCOPYABLE(FRoomAttributeSynchronisationDifference);
    virtual ~FRoomAttributeSynchronisationDifference() override = default;

    static FName GetDifferenceTypeStatic();
    virtual FName GetDifferenceType() const override;
    FString GetAttributeKey() const;
    EChangeType GetChangeType() const;
    const FRoomAttribute &GetOldAttributeValue() const;
    const FRoomAttribute &GetNewAttributeValue() const;
};

}

namespace Redpoint::EOS::Rooms::Synchronisation
{
REDPOINT_EOS_FILE_NS_EXPORT(1699210340, Redpoint::EOS::Rooms::Synchronisation, FRoomAttributeSynchronisationDifference)
}

REDPOINT_EOS_CODE_GUARD_END()