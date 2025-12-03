// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Providers/RoomProviderFeatureHandlerBase.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3588820163, Redpoint::EOS::Rooms::Database)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Providers;

class FExclusivityKeySetFuncs : public BaseKeyFuncs<
                                    FRoomProviderFeatureHandlerBase::FExclusivityKey,
                                    FRoomProviderFeatureHandlerBase::FExclusivityKey,
                                    false>
{
public:
    typedef typename TTypeTraits<FRoomProviderFeatureHandlerBase::FExclusivityKey>::ConstPointerType KeyInitType;
    typedef typename TCallTraits<FRoomProviderFeatureHandlerBase::FExclusivityKey>::ParamType ElementInitType;

    static FORCEINLINE KeyInitType GetSetKey(ElementInitType Element)
    {
        return Element;
    }

    static FORCEINLINE bool Matches(KeyInitType A, KeyInitType B)
    {
        return A.GetKey() == B.GetKey();
    }

    static FORCEINLINE uint32 GetKeyHash(KeyInitType Key)
    {
        return GetTypeHash(Key.GetKey());
    }
};

}

namespace Redpoint::EOS::Rooms::Database
{
REDPOINT_EOS_FILE_NS_EXPORT(3588820163, Redpoint::EOS::Rooms::Database, FExclusivityKeySetFuncs)
}

REDPOINT_EOS_CODE_GUARD_END()