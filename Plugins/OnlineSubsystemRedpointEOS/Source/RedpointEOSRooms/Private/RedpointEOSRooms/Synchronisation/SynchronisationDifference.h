// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Room.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3610585159, Redpoint::EOS::Rooms::Synchronisation)
{

class REDPOINTEOSROOMS_API ISynchronisationDifference
{
public:
    ISynchronisationDifference() = default;
    UE_NONCOPYABLE(ISynchronisationDifference);
    virtual ~ISynchronisationDifference() = default;

    /**
     * Returns the type of difference this object represents.
     */
    virtual FName GetDifferenceType() const = 0;

    /**
     * Attempt to cast the specified value to the difference type.
     */
    template <typename TTarget> static TSharedPtr<TTarget> Get(const TSharedRef<ISynchronisationDifference> &InValue)
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

        if (InValue->GetDifferenceType().IsEqual(TTarget::GetDifferenceTypeStatic()))
        {
            return StaticCastSharedRef<TTarget>(InValue);
        }
        return nullptr;
    }
};

typedef TSharedRef<ISynchronisationDifference> FSynchronisationDifferenceRef;
typedef TSharedPtr<ISynchronisationDifference> FSynchronisationDifferencePtr;

}

namespace Redpoint::EOS::Rooms::Synchronisation
{
REDPOINT_EOS_FILE_NS_EXPORT(3610585159, Redpoint::EOS::Rooms::Synchronisation, ISynchronisationDifference)
REDPOINT_EOS_FILE_NS_EXPORT(3610585159, Redpoint::EOS::Rooms::Synchronisation, FSynchronisationDifferenceRef)
REDPOINT_EOS_FILE_NS_EXPORT(3610585159, Redpoint::EOS::Rooms::Synchronisation, FSynchronisationDifferencePtr)
}

REDPOINT_EOS_CODE_GUARD_END()