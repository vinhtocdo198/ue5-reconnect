// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "GameFramework/OnlineReplStructs.h"
#include "Online/CoreOnline.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/UniqueNetIdEOS.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3130306466, Redpoint::EOS::Core::Id)
{

bool REDPOINTEOSCORE_API IsValidId(const UE::Online::FAccountId &AccountId);
bool REDPOINTEOSCORE_API IsDedicatedServerId(const UE::Online::FAccountId &AccountId);
bool REDPOINTEOSCORE_API IsProductUserId(const UE::Online::FAccountId &AccountId);
EOS_ProductUserId REDPOINTEOSCORE_API GetProductUserId(const UE::Online::FAccountId &AccountId);
FString REDPOINTEOSCORE_API GetUserIdString(const UE::Online::FAccountId &AccountId);
FString REDPOINTEOSCORE_API GetUserIdDebugString(const UE::Online::FAccountId &AccountId);
UE::Online::FAccountId REDPOINTEOSCORE_API GetAccountId(const FUniqueNetId &UserId);
UE::Online::FAccountId REDPOINTEOSCORE_API GetAccountId(const TSharedRef<const FUniqueNetId> &UserId);
UE::Online::FAccountId REDPOINTEOSCORE_API GetAccountId(const EOS_ProductUserId &UserId);
UE::Online::FAccountId REDPOINTEOSCORE_API GetAccountId(const FUniqueNetIdRepl &UserId);
UE::Online::FAccountId REDPOINTEOSCORE_API GetDedicatedServerAccountId();
UE::Online::FAccountId REDPOINTEOSCORE_API GetInvalidAccountId();
TSharedPtr<const FUniqueNetIdEOS> REDPOINTEOSCORE_API GetUniqueNetId(const UE::Online::FAccountId &AccountId);
TSharedRef<const FUniqueNetIdEOS> REDPOINTEOSCORE_API GetUniqueNetIdRef(const UE::Online::FAccountId &AccountId);
FUniqueNetIdRepl REDPOINTEOSCORE_API GetUniqueNetIdRepl(const UE::Online::FAccountId &AccountId);
TOptional<UE::Online::FAccountId> REDPOINTEOSCORE_API TryParseAccountId(const FString &UserIdString);
void REDPOINTEOSCORE_API SerializeAccountId(FArchive &Archive, UE::Online::FAccountId &AccountId);
void REDPOINTEOSCORE_API DumpAccountIdRegistry();

EOS_EpicAccountId REDPOINTEOSCORE_API GetEpicGamesAccountId(const FString &EpicGamesAccountId);
FString REDPOINTEOSCORE_API GetEpicGamesUserIdString(const EOS_EpicAccountId &AccountId);

}

namespace Redpoint::EOS::Core::Id
{
REDPOINT_EOS_FILE_NS_EXPORT(3130306466, Redpoint::EOS::Core::Id, IsValidId)
REDPOINT_EOS_FILE_NS_EXPORT(3130306466, Redpoint::EOS::Core::Id, IsDedicatedServerId)
REDPOINT_EOS_FILE_NS_EXPORT(3130306466, Redpoint::EOS::Core::Id, IsProductUserId)
REDPOINT_EOS_FILE_NS_EXPORT(3130306466, Redpoint::EOS::Core::Id, GetProductUserId)
REDPOINT_EOS_FILE_NS_EXPORT(3130306466, Redpoint::EOS::Core::Id, GetUserIdString)
REDPOINT_EOS_FILE_NS_EXPORT(3130306466, Redpoint::EOS::Core::Id, GetUserIdDebugString)
REDPOINT_EOS_FILE_NS_EXPORT(3130306466, Redpoint::EOS::Core::Id, GetAccountId)
REDPOINT_EOS_FILE_NS_EXPORT(3130306466, Redpoint::EOS::Core::Id, GetDedicatedServerAccountId)
REDPOINT_EOS_FILE_NS_EXPORT(3130306466, Redpoint::EOS::Core::Id, GetInvalidAccountId)
REDPOINT_EOS_FILE_NS_EXPORT(3130306466, Redpoint::EOS::Core::Id, GetUniqueNetId)
REDPOINT_EOS_FILE_NS_EXPORT(3130306466, Redpoint::EOS::Core::Id, GetUniqueNetIdRef)
REDPOINT_EOS_FILE_NS_EXPORT(3130306466, Redpoint::EOS::Core::Id, GetUniqueNetIdRepl)
REDPOINT_EOS_FILE_NS_EXPORT(3130306466, Redpoint::EOS::Core::Id, TryParseAccountId)
REDPOINT_EOS_FILE_NS_EXPORT(3130306466, Redpoint::EOS::Core::Id, SerializeAccountId)
REDPOINT_EOS_FILE_NS_EXPORT(3130306466, Redpoint::EOS::Core::Id, DumpAccountIdRegistry)
REDPOINT_EOS_FILE_NS_EXPORT(3130306466, Redpoint::EOS::Core::Id, GetEpicGamesAccountId)
REDPOINT_EOS_FILE_NS_EXPORT(3130306466, Redpoint::EOS::Core::Id, GetEpicGamesUserIdString)
}

REDPOINT_EOS_CODE_GUARD_END()