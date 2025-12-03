// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSCore/Utils/IndexedTable.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2990577681, Redpoint::EOS::Core::Utils)
{

FIndexedColumnDefinitionBase::FIndexedColumnDefinitionBase(const FName &InColumnName)
    : ColumnName(InColumnName)
{
}

const FName &FIndexedColumnDefinitionBase::GetColumnName() const
{
    return this->ColumnName;
}

FIndexedTable &FIndexedTable::AddIndexedColumn(
    const FIndexedColumnDefinitionBase &InColumnDefinition,
    bool bEnforceUniqueness)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    this->IndexedColumns.Add(InColumnDefinition.GetColumnName(), &InColumnDefinition);
    if (bEnforceUniqueness)
    {
        this->UniqueColumns.Add(InColumnDefinition.GetColumnName());
    }
    return *this;
}

FIndexedTable &FIndexedTable::AddRow(const std::initializer_list<FIndexedColumnValueBaseRef> &InRowData)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    TArray<FIndexedColumnValueBaseRef> RowData;
    for (const auto &Value : InRowData)
    {
        RowData.Add(Value);
    }
    this->AddRow(RowData);
    return *this;
}

FIndexedTable &FIndexedTable::AddRow(const TArray<FIndexedColumnValueBaseRef> &InRowData)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    for (const auto &Value : InRowData)
    {
        if (this->IndexedColumns.Contains(Value->GetColumnName()) &&
            this->UniqueColumns.Contains(Value->GetColumnName()))
        {
            checkf(
                !this->IndexedRows.Contains(Value),
                TEXT("Value for unique column '%s' already maps to an existing row in FIndexedTable; all unique "
                     "columns must have unique values."),
                *Value->GetColumnName().ToString());
        }
    }

    auto NewRow = MakeShared<FIndexedRow>();
    for (const auto &Value : InRowData)
    {
        checkf(
            !NewRow->Contains(Value->GetColumnName()),
            TEXT("Multiple values specified for the same column '%s' in AddRow call!"),
            *Value->GetColumnName().ToString());
        NewRow->Add(Value->GetColumnName(), Value);
    }

    this->Rows.Add(NewRow);
    for (const auto &Value : InRowData)
    {
        const auto *IndexedColumn = this->IndexedColumns.Find(Value->GetColumnName());
        if (IndexedColumn != nullptr)
        {
            this->IndexedRows.Add(Value, NewRow);
        }
    }
    return *this;
}

FIndexedTable &FIndexedTable::RemoveRow(const FIndexedRowConstPtr &InRow)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    if (!InRow.IsValid())
    {
        return *this;
    }

    FIndexedRowConstRef RowRef = InRow.ToSharedRef();
    for (const auto &KV : *RowRef)
    {
        FIndexedColumnValueBaseConstRef Key = KV.Value;
        FIndexedRowConstRef Row = RowRef;
        this->IndexedRows.Remove(Key, Row);
    }
    this->Rows.Remove(RowRef);
    return *this;
}

FIndexedRowConstPtr FIndexedTable::FindFirstByValue(const FIndexedColumnValueBaseConstRef &InValue) const
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    const FIndexedRowConstRef *RowPtr = this->IndexedRows.Find(InValue);
    if (RowPtr == nullptr)
    {
        return nullptr;
    }
    return *RowPtr;
}

FIndexedRowConstPtr FIndexedTable::FindFirstByPredicate(
    const TFunction<bool(const FIndexedRowConstRef &)> &InPredicate) const
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    for (const auto &Row : this->Rows)
    {
        if (InPredicate(Row))
        {
            return Row;
        }
    }
    return nullptr;
}

TArray<FIndexedRowConstRef> FIndexedTable::FindByValue(const FIndexedColumnValueBaseConstRef &InValue) const
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    TArray<FIndexedRowConstRef> Results;
    this->IndexedRows.MultiFind(InValue, Results);
    return Results;
}

TArray<FIndexedRowConstRef> FIndexedTable::FindByPredicate(
    const TFunction<bool(const FIndexedRowConstRef &)> &InPredicate) const
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    TArray<FIndexedRowConstRef> Results;
    for (const auto &Row : this->Rows)
    {
        if (InPredicate(Row))
        {
            Results.Add(Row);
        }
    }
    return Results;
}

const TSet<FIndexedRowConstRef> &FIndexedTable::GetAllRows() const
{
    return this->Rows;
}

void FIndexedTable::Reset()
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    this->Rows.Reset();
    this->IndexedRows.Reset();
    this->IndexedColumns.Reset();
    this->UniqueColumns.Reset();
}

}

REDPOINT_EOS_CODE_GUARD_END()