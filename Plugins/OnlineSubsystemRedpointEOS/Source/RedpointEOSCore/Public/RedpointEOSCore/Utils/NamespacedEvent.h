// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(138435011, Redpoint::EOS::Core::Utils)
{

template <typename TDelegate> class TNamespacedEvent
{
private:
    TMap<FName, TDelegate> Handles;

public:
    TNamespacedEvent() = default;
    TNamespacedEvent(const TNamespacedEvent &) = default;
    TNamespacedEvent(TNamespacedEvent &&) = default;
    ~TNamespacedEvent() = default;

    TDelegate &operator[](const FName &Namespace)
    {
        return Handles.FindOrAdd(Namespace);
    }

    template <typename... TArgs> void Broadcast(const FName &Namespace, TArgs... Args)
    {
        REDPOINT_EOS_USE_LLM_TAG(Core);

        auto *AllHandler = Handles.Find(NAME_All);
        if (AllHandler != nullptr)
        {
            AllHandler->Broadcast(Args...);
        }
        auto *SpecificHandler = Handles.Find(Namespace);
        if (SpecificHandler != nullptr)
        {
            SpecificHandler->Broadcast(Args...);
        }
    }

    void Reset()
    {
        this->Handles.Reset();
    }
};

}

namespace Redpoint::EOS::Core::Utils
{
REDPOINT_EOS_FILE_NS_EXPORT(138435011, Redpoint::EOS::Core::Utils, TNamespacedEvent)
}

REDPOINT_EOS_CODE_GUARD_END()