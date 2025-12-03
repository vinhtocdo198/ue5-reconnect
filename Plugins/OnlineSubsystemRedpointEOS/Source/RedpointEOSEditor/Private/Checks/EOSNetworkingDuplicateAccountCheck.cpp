// Copyright June Rhodes. All Rights Reserved.

#include "EOSNetworkingDuplicateAccountCheck.h"

const TArray<FEOSCheckEntry> FEOSNetworkingDuplicateAccountCheck::ProcessLogMessage(
    EOS_ELogLevel InLogLevel,
    const FString &Category,
    const FString &Message) const
{
    return IEOSCheck::EmptyEntries;
}

void FEOSNetworkingDuplicateAccountCheck::HandleAction(const FString &CheckId, const FString &ActionId) const
{
}