// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Providers/RoomProviderReadOperation.h"
#include "RedpointEOSRooms/RoomAttribute.h"
#include "RedpointEOSRooms/RoomUserId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2340692160, Redpoint::EOS::Rooms::Providers)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Providers;
using namespace ::Redpoint::EOS::API;

class IRoomProviderWriteOperation : public IRoomProviderReadOperation
{
public:
    IRoomProviderWriteOperation() = default;
    UE_NONCOPYABLE(IRoomProviderWriteOperation);
    virtual ~IRoomProviderWriteOperation() override = default;

    [[nodiscard]] virtual FError SetMaxMembers(uint32 MaxMembers) = 0;

    [[nodiscard]] virtual FError AddAttribute(const FString &Key, const FRoomAttribute &Value) = 0;
    [[nodiscard]] virtual FError RemoveAttribute(const FString &Key) = 0;

    [[nodiscard]] virtual FError AddMemberAttribute(const FString &Key, const FRoomAttribute &Value) = 0;
    [[nodiscard]] virtual FError RemoveMemberAttribute(const FString &Key) = 0;
};

typedef TSharedRef<IRoomProviderWriteOperation> FRoomProviderWriteOperationRef;
typedef TSharedPtr<IRoomProviderWriteOperation> FRoomProviderWriteOperationPtr;

}

namespace Redpoint::EOS::Rooms::Providers
{
REDPOINT_EOS_FILE_NS_EXPORT(2340692160, Redpoint::EOS::Rooms::Providers, IRoomProviderWriteOperation)
REDPOINT_EOS_FILE_NS_EXPORT(2340692160, Redpoint::EOS::Rooms::Providers, FRoomProviderWriteOperationRef)
REDPOINT_EOS_FILE_NS_EXPORT(2340692160, Redpoint::EOS::Rooms::Providers, FRoomProviderWriteOperationPtr)
}

REDPOINT_EOS_CODE_GUARD_END()