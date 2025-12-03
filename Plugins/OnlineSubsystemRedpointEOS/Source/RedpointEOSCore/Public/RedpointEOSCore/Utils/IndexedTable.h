// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/Map.h"
#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2990577681, Redpoint::EOS::Core::Utils)
{

class REDPOINTEOSCORE_API FIndexedColumnDefinitionBase
{
private:
    FName ColumnName;

public:
    FIndexedColumnDefinitionBase(const FName &InColumnName);
    UE_NONCOPYABLE(FIndexedColumnDefinitionBase);
    virtual ~FIndexedColumnDefinitionBase() = default;

    const FName &GetColumnName() const;
};

class REDPOINTEOSCORE_API FIndexedColumnValueBase
{
public:
    FIndexedColumnValueBase() = default;
    UE_NONCOPYABLE(FIndexedColumnValueBase);
    virtual ~FIndexedColumnValueBase() = default;

    virtual const FName &GetColumnName() const = 0;
    virtual bool Matches(const FIndexedColumnValueBase &InOther) const = 0;
    virtual uint32 GetKeyHash() const = 0;
};

typedef TSharedRef<FIndexedColumnValueBase> FIndexedColumnValueBaseRef;
typedef TSharedRef<const FIndexedColumnValueBase> FIndexedColumnValueBaseConstRef;
typedef TMap<FName, FIndexedColumnValueBaseRef> FIndexedRow;
typedef TSharedPtr<const FIndexedRow> FIndexedRowConstPtr;
typedef TSharedRef<const FIndexedRow> FIndexedRowConstRef;

template <typename T> class FIndexedColumnDefinition : public FIndexedColumnDefinitionBase
{
public:
    typedef TFunction<bool(const T &InA, const T &InB)> FMatchesFunction;
    typedef TFunction<uint32(const T &InA)> FGetKeyHashFunction;

private:
    FMatchesFunction MatchesFunction;
    FGetKeyHashFunction GetKeyHashFunction;

public:
    FIndexedColumnDefinition(
        const FName &InColumnName,
        const FMatchesFunction &InMatchesFunction,
        const FGetKeyHashFunction &InGetKeyHashFunction)
        : FIndexedColumnDefinitionBase(InColumnName)
        , MatchesFunction(InMatchesFunction)
        , GetKeyHashFunction(InGetKeyHashFunction)
    {
    }
    UE_NONCOPYABLE(FIndexedColumnDefinition);
    virtual ~FIndexedColumnDefinition() override = default;

    class FIndexedColumnValue : public FIndexedColumnValueBase
    {
    private:
        const T Value;
        const FIndexedColumnDefinition &ColumnDefinition;

    public:
        FIndexedColumnValue(const T &InValue, const FIndexedColumnDefinition &InColumnDefinition)
            : Value(InValue)
            , ColumnDefinition(InColumnDefinition)
        {
        }
        UE_NONCOPYABLE(FIndexedColumnValue);
        virtual ~FIndexedColumnValue() override = default;

        virtual const FName &GetColumnName() const override
        {
            return this->ColumnDefinition.GetColumnName();
        }

        virtual bool Matches(const FIndexedColumnValueBase &InOther) const override
        {
            return this->ColumnDefinition.MatchesFunction(Value, ((const FIndexedColumnValue &)InOther).Value);
        }

        virtual uint32 GetKeyHash() const override
        {
            return this->ColumnDefinition.GetKeyHashFunction(Value);
        }

        const T &GetValue() const
        {
            return this->Value;
        }
    };

    TSharedRef<FIndexedColumnValue> Create(const T &InValue) const
    {
        REDPOINT_EOS_USE_LLM_TAG(Core);

        return MakeShared<FIndexedColumnValue>(InValue, *this);
    }

    const T &Get(const FIndexedRowConstRef &InRow) const
    {
        const auto *Value = InRow->Find(this->GetColumnName());
        checkf(Value != nullptr, TEXT("Indexed row does not have column '%s'."), *this->GetColumnName().ToString());
        return StaticCastSharedRef<const FIndexedColumnValue>(*Value)->GetValue();
    }
};

struct REDPOINTEOSCORE_API FIndexedTableHashableKeyFuncs
    : BaseKeyFuncs<TPair<FIndexedColumnValueBaseConstRef, FIndexedRowConstRef>, FIndexedColumnValueBaseConstRef, true>
{
    typedef typename TTypeTraits<FIndexedColumnValueBaseConstRef>::ConstPointerType KeyInitType;
    typedef const TPairInitializer<
        typename TTypeTraits<FIndexedColumnValueBaseConstRef>::ConstInitType,
        typename TTypeTraits<FIndexedRowConstRef>::ConstInitType> &ElementInitType;

    static FORCEINLINE KeyInitType GetSetKey(ElementInitType Element)
    {
        return Element.Key;
    }

    static FORCEINLINE bool Matches(KeyInitType A, KeyInitType B)
    {
        return A->GetColumnName().IsEqual(B->GetColumnName()) && A->Matches(*B);
    }

    template <typename ComparableKey> static FORCEINLINE bool Matches(KeyInitType A, ComparableKey B)
    {
        return A->GetColumnName().IsEqual(B->GetColumnName()) && A->Matches(*B);
    }

    static FORCEINLINE uint32 GetKeyHash(KeyInitType K)
    {
        return HashCombine(GetTypeHash(K->GetColumnName()), K->GetKeyHash());
    }

    template <typename ComparableKey> static FORCEINLINE uint32 GetKeyHash(ComparableKey K)
    {
        return HashCombine(GetTypeHash(K.GetColumnName()), K->GetKeyHash());
    }
};

class REDPOINTEOSCORE_API FIndexedTable
{
private:
    TSet<FIndexedRowConstRef> Rows;
    TMultiMap<FIndexedColumnValueBaseConstRef, FIndexedRowConstRef, FDefaultSetAllocator, FIndexedTableHashableKeyFuncs>
        IndexedRows;
    TMap<FName, const FIndexedColumnDefinitionBase *> IndexedColumns;
    TSet<FName> UniqueColumns;

public:
    FIndexedTable() = default;
    UE_NONCOPYABLE(FIndexedTable);
    virtual ~FIndexedTable() = default;

    FIndexedTable &AddIndexedColumn(
        const FIndexedColumnDefinitionBase &InColumnDefinition,
        bool bEnforceUniqueness = false);
    FIndexedTable &AddRow(const std::initializer_list<FIndexedColumnValueBaseRef> &InRowData);
    FIndexedTable &AddRow(const TArray<FIndexedColumnValueBaseRef> &InRowData);
    FIndexedTable &RemoveRow(const FIndexedRowConstPtr &InRow);

    FIndexedRowConstPtr FindFirstByValue(const FIndexedColumnValueBaseConstRef &InValue) const;
    FIndexedRowConstPtr FindFirstByPredicate(const TFunction<bool(const FIndexedRowConstRef &)> &InPredicate) const;

    TArray<FIndexedRowConstRef> FindByValue(const FIndexedColumnValueBaseConstRef &InValue) const;
    TArray<FIndexedRowConstRef> FindByPredicate(const TFunction<bool(const FIndexedRowConstRef &)> &InPredicate) const;

    const TSet<FIndexedRowConstRef> &GetAllRows() const;

    void Reset();
};

}

namespace Redpoint::EOS::Core::Utils
{
REDPOINT_EOS_FILE_NS_EXPORT(2990577681, Redpoint::EOS::Core::Utils, FIndexedColumnDefinitionBase)
REDPOINT_EOS_FILE_NS_EXPORT(2990577681, Redpoint::EOS::Core::Utils, FIndexedColumnValueBase)
REDPOINT_EOS_FILE_NS_EXPORT(2990577681, Redpoint::EOS::Core::Utils, FIndexedColumnValueBaseRef)
REDPOINT_EOS_FILE_NS_EXPORT(2990577681, Redpoint::EOS::Core::Utils, FIndexedColumnValueBaseConstRef)
REDPOINT_EOS_FILE_NS_EXPORT(2990577681, Redpoint::EOS::Core::Utils, FIndexedRow)
REDPOINT_EOS_FILE_NS_EXPORT(2990577681, Redpoint::EOS::Core::Utils, FIndexedRowConstPtr)
REDPOINT_EOS_FILE_NS_EXPORT(2990577681, Redpoint::EOS::Core::Utils, FIndexedRowConstRef)
REDPOINT_EOS_FILE_NS_EXPORT(2990577681, Redpoint::EOS::Core::Utils, FIndexedColumnDefinition)
REDPOINT_EOS_FILE_NS_EXPORT(2990577681, Redpoint::EOS::Core::Utils, FIndexedTableHashableKeyFuncs)
REDPOINT_EOS_FILE_NS_EXPORT(2990577681, Redpoint::EOS::Core::Utils, FIndexedTable)
}

REDPOINT_EOS_CODE_GUARD_END()