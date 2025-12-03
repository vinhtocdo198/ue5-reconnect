// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Synchronisation/SynchronisationDifference.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1929713852, Redpoint::EOS::Rooms::Synchronisation)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Synchronisation;

class REDPOINTEOSROOMS_API FRoomMemberAttributeSynchronisationDifference : public ISynchronisationDifference
{
public:
    enum class EChangeType : uint8
    {
        Added,
        Removed,
        Modified,
    };

private:
    FRoomUserId MemberUserId;
    FString AttributeKey;
    EChangeType ChangeType;
    FRoomAttribute OldAttributeValue;
    FRoomAttribute NewAttributeValue;

public:
    FRoomMemberAttributeSynchronisationDifference(
        FRoomUserId InMemberUserId,
        FString InAttributeKey,
        EChangeType InChangeType,
        const FRoomAttribute &InOldAttributeValue,
        const FRoomAttribute &InNewAttributeValue);
    UE_NONCOPYABLE(FRoomMemberAttributeSynchronisationDifference);
    virtual ~FRoomMemberAttributeSynchronisationDifference() override = default;

    static FName GetDifferenceTypeStatic();
    virtual FName GetDifferenceType() const override;
    FRoomUserId GetMemberUserId() const;
    FString GetAttributeKey() const;
    EChangeType GetChangeType() const;
    const FRoomAttribute &GetOldAttributeValue() const;
    const FRoomAttribute &GetNewAttributeValue() const;
};

}

namespace Redpoint::EOS::Rooms::Synchronisation
{
REDPOINT_EOS_FILE_NS_EXPORT(
    1929713852,
    Redpoint::EOS::Rooms::Synchronisation,
    FRoomMemberAttributeSynchronisationDifference)
}

REDPOINT_EOS_CODE_GUARD_END()