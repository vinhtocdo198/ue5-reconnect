// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(554134602, Redpoint::EOS::Core::Utils)
{

template <typename TEntry, typename TRegistry> class TGenericRegistry
{
private:
    TArray<TSharedRef<TEntry>> EntryArray;
    TMap<TSharedRef<TEntry>, TSharedRef<TEntry>> EntryMap;
    TMap<TSharedRef<TEntry>, int> PriorityMap;

protected:
    using RegistryEntry = TEntry;
    using RegistryRegistry = TRegistry;

    TGenericRegistry() = default;
    UE_NONCOPYABLE(TGenericRegistry);
    virtual ~TGenericRegistry() = default;

    virtual TSharedRef<TEntry> MakeEntry(const TSharedRef<TEntry> &InEntry)
    {
        return InEntry;
    }

    static TSharedRef<TRegistry> GetInstance()
    {
        REDPOINT_EOS_USE_LLM_TAG(Core);

        if (!RegistryRegistry::Instance.IsValid())
        {
            RegistryRegistry::Instance = MakeShareable(new TRegistry());
        }
        return RegistryRegistry::Instance.ToSharedRef();
    }

    void InternalAdd(const TSharedRef<TEntry> &InValue, int InPriority)
    {
        REDPOINT_EOS_USE_LLM_TAG(Core);

        auto Value = this->MakeEntry(InValue);
        this->EntryArray.Add(Value);
        this->PriorityMap.Add(Value, InPriority);
        this->EntryMap.Add(InValue, Value);

        this->EntryArray.StableSort([this](const TSharedRef<TEntry> &A, const TSharedRef<TEntry> &B) {
            return !(this->PriorityMap[A] < this->PriorityMap[B]);
        });
    }

    void InternalRemove(const TSharedRef<TEntry> &InValue)
    {
        REDPOINT_EOS_USE_LLM_TAG(Core);

        auto *OriginalValue = this->EntryMap.Find(InValue);
        if (OriginalValue != nullptr)
        {
            this->EntryArray.Remove(*OriginalValue);
            this->PriorityMap.Remove(*OriginalValue);
            this->EntryMap.Remove(InValue);
        }
    }

public:
    /**
     * Add an entry to the registry. Entries with a higher priority are returned first from ::Get().
     */
    static void Register(const TSharedRef<TEntry> &InValue, int InPriority)
    {
        TRegistry::GetInstance()->InternalAdd(InValue, InPriority);
    }

    /**
     * Remove a previous entry from the registry.
     */
    static void Unregister(const TSharedRef<TEntry> &InValue)
    {
        TRegistry::GetInstance()->InternalRemove(InValue);
    }

    /**
     * Retrieve all the registered values from the registry.
     */
    static const TArray<TSharedRef<TEntry>> &Get()
    {
        return TRegistry::GetInstance()->EntryArray;
    }
};

}

namespace Redpoint::EOS::Core::Utils
{
REDPOINT_EOS_FILE_NS_EXPORT(554134602, Redpoint::EOS::Core::Utils, TGenericRegistry)
}

#define REDPOINT_EOS_DECLARE_GENERIC_REGISTRY_BODY()                                                                   \
    friend ::Redpoint::EOS::Core::Utils::TGenericRegistry<RegistryEntry, RegistryRegistry>;                            \
                                                                                                                       \
private:                                                                                                               \
    static TSharedPtr<RegistryRegistry> Instance;

#define REDPOINT_EOS_DECLARE_GENERIC_REGISTRY(Name, Type)                                                              \
    class Name : public ::Redpoint::EOS::Core::Utils::TGenericRegistry<Type, Name>                                     \
    {                                                                                                                  \
        REDPOINT_EOS_DECLARE_GENERIC_REGISTRY_BODY()                                                                   \
    };
#define REDPOINT_EOS_DECLARE_GENERIC_REGISTRY_EXTERN(Api, Name, Type)                                                  \
    class Api Name : public ::Redpoint::EOS::Core::Utils::TGenericRegistry<Type, Name>                                 \
    {                                                                                                                  \
        REDPOINT_EOS_DECLARE_GENERIC_REGISTRY_BODY()                                                                   \
    };
#define REDPOINT_EOS_DEFINE_GENERIC_REGISTRY(Name) TSharedPtr<typename Name::RegistryRegistry> Name::Instance;

REDPOINT_EOS_CODE_GUARD_END()