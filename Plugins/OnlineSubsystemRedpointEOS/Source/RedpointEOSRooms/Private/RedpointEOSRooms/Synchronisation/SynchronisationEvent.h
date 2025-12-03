// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Room.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3599987289, Redpoint::EOS::Rooms::Synchronisation)
{
using namespace ::Redpoint::EOS::Rooms::Synchronisation;

class REDPOINTEOSROOMS_API ISynchronisationEvent
{
public:
    ISynchronisationEvent() = default;
    UE_NONCOPYABLE(ISynchronisationEvent);
    virtual ~ISynchronisationEvent() = default;

    /**
     * Returns the type of event this object represents.
     */
    virtual FName GetEventType() const = 0;
};

template <typename TEventResult> class FSynchronisationEvent : public ISynchronisationEvent
{
private:
    FName EventType;
    TEventResult EventResult;

public:
    FSynchronisationEvent(FName InEventType, TEventResult InEventResult)
        : EventType(InEventType)
        , EventResult(InEventResult) {};
    UE_NONCOPYABLE(FSynchronisationEvent);
    virtual ~FSynchronisationEvent() override = default;

    virtual FName GetEventType() const override
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

        return this->EventType;
    }
    const TEventResult &GetEventResult() const
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

        return this->EventResult;
    }
};

typedef TSharedRef<ISynchronisationEvent> FSynchronisationEventRef;
typedef TSharedPtr<ISynchronisationEvent> FSynchronisationEventPtr;

}

namespace Redpoint::EOS::Rooms::Synchronisation
{
REDPOINT_EOS_FILE_NS_EXPORT(3599987289, Redpoint::EOS::Rooms::Synchronisation, ISynchronisationEvent)
REDPOINT_EOS_FILE_NS_EXPORT(3599987289, Redpoint::EOS::Rooms::Synchronisation, FSynchronisationEvent)
REDPOINT_EOS_FILE_NS_EXPORT(3599987289, Redpoint::EOS::Rooms::Synchronisation, FSynchronisationEventRef)
REDPOINT_EOS_FILE_NS_EXPORT(3599987289, Redpoint::EOS::Rooms::Synchronisation, FSynchronisationEventPtr)
}

REDPOINT_EOS_CODE_GUARD_END()