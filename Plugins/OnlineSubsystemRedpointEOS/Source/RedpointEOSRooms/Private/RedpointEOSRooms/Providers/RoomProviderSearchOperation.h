// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Providers/RoomProviderOperation.h"
#include "RedpointEOSRooms/RoomAttributeConstraint.h"
#include "RedpointEOSRooms/RoomId.h"
#include "RedpointEOSRooms/RoomUserId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4160750306, Redpoint::EOS::Rooms::Providers)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Providers;
using namespace ::Redpoint::EOS::API;

class IRoomProviderSearchOperation : public IRoomProviderOperation
{
public:
    IRoomProviderSearchOperation() = default;
    UE_NONCOPYABLE(IRoomProviderSearchOperation);
    virtual ~IRoomProviderSearchOperation() override = default;

    /**
     * Returns the user ID of the user performing this operation.
     */
    [[nodiscard]] virtual FRoomUserId GetActorUserId() const = 0;

    [[nodiscard]] virtual FError WithRoomId(const FRoomIdRef &RoomId) = 0;
    [[nodiscard]] virtual FError WithRoomMemberId(const FRoomUserId &RoomMemberId) = 0;
    [[nodiscard]] virtual FError WithAttributeConstraint(
        const FString &Key,
        const FRoomAttributeConstraint &Constraint) = 0;
};

typedef TSharedRef<IRoomProviderSearchOperation> FRoomProviderSearchOperationRef;
typedef TSharedPtr<IRoomProviderSearchOperation> FRoomProviderSearchOperationPtr;

}

namespace Redpoint::EOS::Rooms::Providers
{
REDPOINT_EOS_FILE_NS_EXPORT(4160750306, Redpoint::EOS::Rooms::Providers, IRoomProviderSearchOperation)
REDPOINT_EOS_FILE_NS_EXPORT(4160750306, Redpoint::EOS::Rooms::Providers, FRoomProviderSearchOperationRef)
REDPOINT_EOS_FILE_NS_EXPORT(4160750306, Redpoint::EOS::Rooms::Providers, FRoomProviderSearchOperationPtr)
}

REDPOINT_EOS_CODE_GUARD_END()