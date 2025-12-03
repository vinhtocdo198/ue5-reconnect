// Copyright June Rhodes. All Rights Reserved.

#include "Types/OSBLeaderboardTypes.h"

FOnlineStatsRowBP FOnlineStatsRowBP::FromNative(FOnlineStatsRow &Row)
{
    FOnlineStatsRowBP Result;
    Result.PlayerNickname = Row.NickName;
    Result.PlayerId = FUniqueNetIdRepl(Row.PlayerId);
    Result.Rank = Row.Rank;
    for (auto KV : Row.Columns)
    {
#if REDPOINT_EOS_UE_5_7_OR_LATER
#error The key type of property Columns must be changed to FString as of Unreal Engine 5.7.
#elif REDPOINT_EOS_UE_5_5_OR_LATER
        Result.Columns.Add(FName(*KV.Key), FVariantDataBP::FromNative(KV.Value));
#else
        Result.Columns.Add(KV.Key, FVariantDataBP::FromNative(KV.Value));
#endif
    }
    return Result;
}

FColumnMetaDataBP FColumnMetaDataBP::FromNative(FColumnMetaData &Column)
{
    FColumnMetaDataBP Result;
#if REDPOINT_EOS_UE_5_7_OR_LATER
#error The type of property ColumnName must be changed to FString as of Unreal Engine 5.7.
#elif REDPOINT_EOS_UE_5_5_OR_LATER
    Result.ColumnName = FName(*Column.ColumnName);
#else
    Result.ColumnName = Column.ColumnName;
#endif
    Result.DataType = (EOnlineKeyValuePairDataType_)Column.DataType;
    return Result;
}

FColumnMetaData FColumnMetaDataBP::ToNative() const
{
#if REDPOINT_EOS_UE_5_7_OR_LATER
#error The type of property ColumnName must be changed to FString as of Unreal Engine 5.7.
#elif REDPOINT_EOS_UE_5_5_OR_LATER
    return FColumnMetaData(this->ColumnName.ToString(), (EOnlineKeyValuePairDataType::Type)this->DataType);
#else
    return FColumnMetaData(this->ColumnName, (EOnlineKeyValuePairDataType::Type)this->DataType);
#endif
}

void UOnlineLeaderboardRead::SetLeaderboardName(FName LeaderboardName)
{
#if REDPOINT_EOS_UE_5_7_OR_LATER
#error The type of parameter LeaderboardName must be changed to FString as of Unreal Engine 5.7.
#elif REDPOINT_EOS_UE_5_5_OR_LATER
    this->Leaderboard->LeaderboardName = LeaderboardName.ToString();
#else
    this->Leaderboard->LeaderboardName = LeaderboardName;
#endif
}

FName UOnlineLeaderboardRead::GetLeaderboardName() const
{
#if REDPOINT_EOS_UE_5_7_OR_LATER
#error The return type must be changed to FString as of Unreal Engine 5.7.
#elif REDPOINT_EOS_UE_5_5_OR_LATER
    return FName(*this->Leaderboard->LeaderboardName);
#else
    return this->Leaderboard->LeaderboardName;
#endif
}

void UOnlineLeaderboardRead::SetSortedColumn(FName SortedColumn)
{
#if REDPOINT_EOS_UE_5_7_OR_LATER
#error The type of parameter SortedColumn must be changed to FString as of Unreal Engine 5.7.
#elif REDPOINT_EOS_UE_5_5_OR_LATER
    this->Leaderboard->SortedColumn = SortedColumn.ToString();
#else
    this->Leaderboard->SortedColumn = SortedColumn;
#endif
}

FName UOnlineLeaderboardRead::GetSortedColumn() const
{
#if REDPOINT_EOS_UE_5_7_OR_LATER
#error The return type must be changed to FString as of Unreal Engine 5.7.
#elif REDPOINT_EOS_UE_5_5_OR_LATER
    return FName(*this->Leaderboard->SortedColumn);
#else
    return this->Leaderboard->SortedColumn;
#endif
}

EOnlineAsyncTaskState_ UOnlineLeaderboardRead::GetReadState() const
{
    return (EOnlineAsyncTaskState_)this->Leaderboard->ReadState;
}

void UOnlineLeaderboardRead::SetColumns(const TArray<FColumnMetaDataBP> &InColumns)
{
    this->Leaderboard->ColumnMetadata.Empty();
    for (auto Column : InColumns)
    {
        this->Leaderboard->ColumnMetadata.Add(Column.ToNative());
    }
}

TArray<FColumnMetaDataBP> UOnlineLeaderboardRead::GetColumns() const
{
    TArray<FColumnMetaDataBP> Columns;
    for (auto Column : this->Leaderboard->ColumnMetadata)
    {
        Columns.Add(FColumnMetaDataBP::FromNative(Column));
    }
    return Columns;
}

TArray<FOnlineStatsRowBP> UOnlineLeaderboardRead::GetRows() const
{
    TArray<FOnlineStatsRowBP> Rows;
    for (auto Row : this->Leaderboard->Rows)
    {
        Rows.Add(FOnlineStatsRowBP::FromNative(Row));
    }
    return Rows;
}

FOnlineStatsRowBP UOnlineLeaderboardRead::FindPlayerRecord(const FUniqueNetIdRepl &UserId, bool &OutFound) const
{
    if (!UserId.IsValid())
    {
        OutFound = false;
        return FOnlineStatsRowBP();
    }

    FOnlineStatsRow *Record = this->Leaderboard->FindPlayerRecord(*UserId.GetUniqueNetId());
    if (Record == nullptr)
    {
        OutFound = false;
        return FOnlineStatsRowBP();
    }

    OutFound = true;
    return FOnlineStatsRowBP::FromNative(*Record);
}

FOnlineLeaderboardReadRef &UOnlineLeaderboardRead::GetLeaderboard()
{
    return this->Leaderboard;
}

void UOnlineLeaderboardWrite::SetLeaderboardNames(const TArray<FName> &InLeaderboardNames)
{
    this->LeaderboardNames = InLeaderboardNames;
}

void UOnlineLeaderboardWrite::SetRatedStat(FName InRatedStat)
{
    this->RatedStat = InRatedStat;
}

void UOnlineLeaderboardWrite::SetStats(const TMap<FString, FVariantDataBP> &InStats)
{
    this->Stats = InStats;
}

TSharedRef<FOnlineLeaderboardWrite> UOnlineLeaderboardWrite::ToNative()
{
    TSharedRef<FOnlineLeaderboardWrite> Leaderboard = MakeShared<FOnlineLeaderboardWrite>();
#if REDPOINT_EOS_UE_5_7_OR_LATER
#error The LeaderboardNames property type must be changed to TArray<FString> as of Unreal Engine 5.7.
#elif REDPOINT_EOS_UE_5_5_OR_LATER
    for (const auto &Name : this->LeaderboardNames)
    {
        Leaderboard->LeaderboardNames.Add(Name.ToString());
    }
    Leaderboard->RatedStat = this->RatedStat.ToString();
    for (const auto &KV : this->Stats)
    {
        if (KV.Value.Type == EOnlineKeyValuePairDataType_::Float)
        {
            Leaderboard->SetFloatStat(KV.Key, KV.Value.AsFloat);
        }
        else if (KV.Value.Type == EOnlineKeyValuePairDataType_::Int64)
        {
            Leaderboard->SetIntStat(KV.Key, static_cast<int32>(KV.Value.AsInt64));
        }
    }
#else
    Leaderboard->LeaderboardNames = this->LeaderboardNames;
    Leaderboard->RatedStat = this->RatedStat;
    for (const auto &KV : this->Stats)
    {
        if (KV.Value.Type == EOnlineKeyValuePairDataType_::Float)
        {
            Leaderboard->SetFloatStat(FName(*KV.Key), KV.Value.AsFloat);
        }
        else if (KV.Value.Type == EOnlineKeyValuePairDataType_::Int64)
        {
            Leaderboard->SetIntStat(FName(*KV.Key), static_cast<int32>(KV.Value.AsInt64));
        }
    }
#endif
    return Leaderboard;
}