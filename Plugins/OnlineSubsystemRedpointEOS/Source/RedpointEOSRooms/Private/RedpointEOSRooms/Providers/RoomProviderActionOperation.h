// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Providers/RoomProviderReadOperation.h"
#include "RedpointEOSRooms/RoomAttribute.h"
#include "RedpointEOSRooms/RoomUserId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1491814861, Redpoint::EOS::Rooms::Providers)
{
using namespace ::Redpoint::EOS::Rooms::Providers;
using namespace ::Redpoint::EOS::API;

class IRoomProviderActionOperation : public IRoomProviderReadOperation
{
public:
    IRoomProviderActionOperation() = default;
    UE_NONCOPYABLE(IRoomProviderActionOperation);
    virtual ~IRoomProviderActionOperation() override = default;
};

typedef TSharedRef<IRoomProviderActionOperation> FRoomProviderActionOperationRef;
typedef TSharedPtr<IRoomProviderActionOperation> FRoomProviderActionOperationPtr;

}

namespace Redpoint::EOS::Rooms::Providers
{
REDPOINT_EOS_FILE_NS_EXPORT(1491814861, Redpoint::EOS::Rooms::Providers, IRoomProviderActionOperation)
REDPOINT_EOS_FILE_NS_EXPORT(1491814861, Redpoint::EOS::Rooms::Providers, FRoomProviderActionOperationRef)
REDPOINT_EOS_FILE_NS_EXPORT(1491814861, Redpoint::EOS::Rooms::Providers, FRoomProviderActionOperationPtr)
}

REDPOINT_EOS_CODE_GUARD_END()