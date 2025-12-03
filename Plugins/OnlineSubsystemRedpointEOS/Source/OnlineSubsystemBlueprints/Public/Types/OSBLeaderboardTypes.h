// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/OnlineReplStructs.h"
#include "Interfaces/OnlineLeaderboardInterface.h"
#include "OSBSessionTypes.h"
#include "Online/CoreOnline.h"

#include "OSBLeaderboardTypes.generated.h"

USTRUCT(BlueprintType)
struct ONLINESUBSYSTEMBLUEPRINTS_API FOnlineStatsRowBP
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Online|Leaderboard")
    FString PlayerNickname;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Online|Leaderboard")
    FUniqueNetIdRepl PlayerId;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Online|Leaderboard")
    int32 Rank = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Online|Leaderboard")
    TMap<FName, FVariantDataBP> Columns;

    static FOnlineStatsRowBP FromNative(FOnlineStatsRow &Row);
};

USTRUCT(BlueprintType)
struct ONLINESUBSYSTEMBLUEPRINTS_API FColumnMetaDataBP
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|Leaderboard")
    FName ColumnName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|Leaderboard")
    EOnlineKeyValuePairDataType_ DataType = EOnlineKeyValuePairDataType_::Empty;

    static FColumnMetaDataBP FromNative(FColumnMetaData &Row);
    FColumnMetaData ToNative() const;
};

UCLASS(BlueprintType, Meta = (IsBlueprintBase = "false"))
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineLeaderboardRead : public UObject
{
    GENERATED_BODY()

private:
    UOnlineLeaderboardRead()
        : UObject()
        , Leaderboard(MakeShared<FOnlineLeaderboardRead, ESPMode::ThreadSafe>())
    {
    }

    UOnlineLeaderboardRead(const FObjectInitializer &ObjectInitializer)
        : UObject(ObjectInitializer)
        , Leaderboard(MakeShared<FOnlineLeaderboardRead, ESPMode::ThreadSafe>())
    {
    }

    FOnlineLeaderboardReadRef Leaderboard;

    UFUNCTION(BlueprintCallable, Category = "Online|Leaderboard")
    void SetLeaderboardName(FName LeaderboardName);

    UFUNCTION(BlueprintPure, Category = "Online|Leaderboard")
    FName GetLeaderboardName() const;

    UFUNCTION(BlueprintCallable, Category = "Online|Leaderboard")
    void SetSortedColumn(FName SortedColumn);

    UFUNCTION(BlueprintPure, Category = "Online|Leaderboard")
    FName GetSortedColumn() const;

    UFUNCTION(BlueprintPure, Category = "Online|Leaderboard")
    EOnlineAsyncTaskState_ GetReadState() const;

    UFUNCTION(BlueprintCallable, Category = "Online|Leaderboard")
    void SetColumns(const TArray<FColumnMetaDataBP> &InColumns);

    UFUNCTION(BlueprintCallable, Category = "Online|Leaderboard")
    TArray<FColumnMetaDataBP> GetColumns() const;

    UFUNCTION(BlueprintCallable, Category = "Online|Leaderboard")
    TArray<FOnlineStatsRowBP> GetRows() const;

    UFUNCTION(BlueprintCallable, Category = "Online|Leaderboard")
    FOnlineStatsRowBP FindPlayerRecord(const FUniqueNetIdRepl &UserId, bool &OutFound) const;

public:
    FOnlineLeaderboardReadRef &GetLeaderboard();
};

UCLASS(BlueprintType, Meta = (IsBlueprintBase = "false"))
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineLeaderboardWrite : public UObject
{
    GENERATED_BODY()

private:
    TArray<FName> LeaderboardNames;
    FName RatedStat;
    TMap<FString, FVariantDataBP> Stats;

public:
    UFUNCTION(BlueprintCallable, Category = "Online|Leaderboard")
    void SetLeaderboardNames(const TArray<FName> &LeaderboardNames);

    UFUNCTION(BlueprintCallable, Category = "Online|Leaderboard")
    void SetRatedStat(FName RatedStat);

    UFUNCTION(BlueprintCallable, Category = "Online|Leaderboard")
    void SetStats(const TMap<FString, FVariantDataBP> &Stats);

    TSharedRef<FOnlineLeaderboardWrite> ToNative();
};