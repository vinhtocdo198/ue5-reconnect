// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Identity/OnlineIdentityInterfaceEOSImpl.h"

#include "OnlineSubsystemRedpointEOS/Error/ErrorConv.h"
#include "OnlineSubsystemRedpointEOS/Logging.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Identity/UserOnlineAccountEOSImpl.h"
#include "RedpointEOSCore/Id/Id.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2650835609, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Identity)
{
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Identity;

static const auto CLocalAccountsLocalUserNum = FIndexedColumnDefinition<int32>(
    FName(TEXT("LocalUserNum")),
    [](const int32 &A, const int32 &B) {
        return A == B;
    },
    [](const int32 &K) {
        return GetTypeHash(K);
    });
static const auto CLocalAccountsUniqueNetId = FIndexedColumnDefinition<FUniqueNetIdRef>(
    FName(TEXT("UniqueNetId")),
    [](const FUniqueNetIdRef &A, const FUniqueNetIdRef &B) {
        return *A == *B;
    },
    [](const FUniqueNetIdRef &K) {
        return GetTypeHash(*K);
    });
static const auto CLocalAccountsIdentityUser = FIndexedColumnDefinition<FIdentityUserRef>(
    FName(TEXT("IdentityUser")),
    [](const FIdentityUserRef &A, const FIdentityUserRef &B) {
        return A->GetUserId() == B->GetUserId();
    },
    [](const FIdentityUserRef &K) {
        return GetTypeHash(K->GetUserId());
    });
static const auto CLocalAccountsUserOnlineAccount = FIndexedColumnDefinition<TSharedRef<FUserOnlineAccountEOSImpl>>(
    FName(TEXT("UserOnlineAccount")),
    [](const TSharedRef<FUserOnlineAccountEOSImpl> &A, const TSharedRef<FUserOnlineAccountEOSImpl> &B) {
        return *A->GetUserId() == *B->GetUserId();
    },
    [](const TSharedRef<FUserOnlineAccountEOSImpl> &K) {
        return GetTypeHash(*K->GetUserId());
    });
static const auto CLocalAccountsPlatformUserId = FIndexedColumnDefinition<FPlatformUserId>(
    FName(TEXT("PlatformUserId")),
    [](const FPlatformUserId &A, const FPlatformUserId &B) {
        return A == B;
    },
    [](const FPlatformUserId &K) {
        return GetTypeHash(K);
    });

FOnlineIdentityInterfaceEOSImpl::FOnlineIdentityInterfaceEOSImpl(
    const FPlatformHandle &InPlatformHandle,
    const FName &InInstanceName,
    const TSharedRef<IConfig> &InConfig
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    ,
    const TSharedPtr<FOnlineSubsystemRedpointEAS> &InEpicGamesOnlineSubsystem
#endif
    )
    : PlatformHandle(InPlatformHandle)
    , InstanceName(InInstanceName)
    , Config(InConfig)
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    , EpicGamesOnlineSubsystem(InEpicGamesOnlineSubsystem)
#endif
    , IdentitySystem(InPlatformHandle->GetSystem<IIdentitySystem>())
    , OnUserSlotChangedHandle()
    , LocalAccounts()
{
    this->LocalAccounts.AddIndexedColumn(CLocalAccountsLocalUserNum, true)
        .AddIndexedColumn(CLocalAccountsUniqueNetId, true)
        .AddIndexedColumn(CLocalAccountsIdentityUser, true)
        .AddIndexedColumn(CLocalAccountsUserOnlineAccount, true)
        .AddIndexedColumn(CLocalAccountsPlatformUserId, true);
}

void FOnlineIdentityInterfaceEOSImpl::RegisterEvents()
{
    this->OnUserSlotChangedHandle =
        this->IdentitySystem->OnUserSlotChanged().AddSP(this, &FOnlineIdentityInterfaceEOSImpl::OnUserSlotChanged);
}

FOnlineIdentityInterfaceEOSImpl::~FOnlineIdentityInterfaceEOSImpl()
{
    this->IdentitySystem->OnUserSlotChanged().Remove(this->OnUserSlotChangedHandle);
}

bool FOnlineIdentityInterfaceEOSImpl::Login(int32 LocalUserNum, const FOnlineAccountCredentials &AccountCredentials)
{
    this->IdentitySystem->Login(
        Requests::FLoginRequest(LocalUserNum, AccountCredentials.Type, AccountCredentials.Id, AccountCredentials.Token),
        IIdentitySystem::FOnLoginComplete::CreateSP(
            this,
            &FOnlineIdentityInterfaceEOSImpl::OnLoginComplete,
            LocalUserNum));
    return true;
}

bool FOnlineIdentityInterfaceEOSImpl::Logout(int32 LocalUserNum)
{
    this->IdentitySystem->Logout(
        Requests::FLogoutRequest(LocalUserNum),
        IIdentitySystem::FOnLogoutComplete::CreateSP(
            this,
            &FOnlineIdentityInterfaceEOSImpl::OnLogoutComplete,
            LocalUserNum));
    return true;
}

bool FOnlineIdentityInterfaceEOSImpl::AutoLogin(int32 LocalUserNum)
{
    return this->Login(LocalUserNum, FOnlineAccountCredentials());
}

FUniqueNetIdRef FOnlineIdentityInterfaceEOSImpl::ConvertToUniqueNetId(FIdentityUserId InUserId) const
{
    TSharedPtr<const FUniqueNetIdEOS> UserId;
    if (Redpoint::EOS::Core::Id::IsDedicatedServerId(InUserId))
    {
        UserId = FUniqueNetIdEOS::DedicatedServerId();
    }
    else
    {
        UserId = Redpoint::EOS::Core::Id::GetUniqueNetId(InUserId);
    }
    if (!UserId.IsValid())
    {
        UserId = FUniqueNetIdEOS::InvalidId();
    }
    return UserId.ToSharedRef();
}

void FOnlineIdentityInterfaceEOSImpl::OnUserSlotChanged(
    FIdentityUserSlot UserSlot,
    EIdentityUserSlotState OldUserSlotState,
    FIdentityUserId OldUserId,
    EIdentityUserSlotState NewUserSlotState,
    FIdentityUserId NewUserId,
    EIdentityUserSlotChangeReason ChangeReason)
{
    using namespace Redpoint::EOS::Core::Id;

    auto UserId = this->ConvertToUniqueNetId(NewUserId);

    if (NewUserSlotState == EIdentityUserSlotState::SignedIn &&
        ChangeReason == EIdentityUserSlotChangeReason::ExplicitLink)
    {
        auto NewUser = this->IdentitySystem->GetUser(UserSlot);
        checkf(
            NewUser.IsValid(),
            TEXT(
                "Expected identity system to have user when new user slot state is EIdentityUserSlotState::SignedIn."));
        auto NewAccount = MakeShared<FUserOnlineAccountEOSImpl>(NewUser.ToSharedRef());

        // @note: We have to delete and re-add the row, since the FIdentityUserPtr is expected to be a different object
        // with new information post-link.
        this->LocalAccounts.RemoveRow(
            this->LocalAccounts.FindFirstByValue(CLocalAccountsLocalUserNum.Create(UserSlot)));
        this->LocalAccounts.AddRow(
            {CLocalAccountsLocalUserNum.Create(UserSlot),
             CLocalAccountsUniqueNetId.Create(NewAccount->GetUserId()),
             CLocalAccountsIdentityUser.Create(NewUser.ToSharedRef()),
             CLocalAccountsUserOnlineAccount.Create(NewAccount),
             CLocalAccountsPlatformUserId.Create(NewUser->GetProcessGlobalUserHandle())});
    }
    else if (NewUserSlotState == EIdentityUserSlotState::SignedIn)
    {
        auto NewUser = this->IdentitySystem->GetUser(UserSlot);
        checkf(
            NewUser.IsValid(),
            TEXT(
                "Expected identity system to have user when new user slot state is EIdentityUserSlotState::SignedIn."));
        auto NewAccount = MakeShared<FUserOnlineAccountEOSImpl>(NewUser.ToSharedRef());
        this->LocalAccounts.AddRow(
            {CLocalAccountsLocalUserNum.Create(UserSlot),
             CLocalAccountsUniqueNetId.Create(NewAccount->GetUserId()),
             CLocalAccountsIdentityUser.Create(NewUser.ToSharedRef()),
             CLocalAccountsUserOnlineAccount.Create(NewAccount),
             CLocalAccountsPlatformUserId.Create(NewUser->GetProcessGlobalUserHandle())});
    }
    else if (NewUserSlotState == EIdentityUserSlotState::NotSignedIn)
    {
        this->LocalAccounts.RemoveRow(
            this->LocalAccounts.FindFirstByValue(CLocalAccountsLocalUserNum.Create(UserSlot)));
    }

    UE_LOG(
        LogOnlineSubsystemRedpointEOS,
        Verbose,
        TEXT("Broadcasting 'OnLoginStatusChanged' event for local user %d."),
        UserSlot);
    this->TriggerOnLoginStatusChangedDelegates(
        UserSlot,
        OldUserSlotState == EIdentityUserSlotState::NotSignedIn ? ELoginStatus::NotLoggedIn : ELoginStatus::LoggedIn,
        NewUserSlotState == EIdentityUserSlotState::NotSignedIn ? ELoginStatus::NotLoggedIn : ELoginStatus::LoggedIn,
        *UserId);

    UE_LOG(
        LogOnlineSubsystemRedpointEOS,
        Verbose,
        TEXT("Broadcasting 'OnLoginChanged' event for local user %d."),
        UserSlot);
    this->TriggerOnLoginChangedDelegates(UserSlot);
}

void FOnlineIdentityInterfaceEOSImpl::OnLoginComplete(FError ErrorCode, FIdentityUserPtr NewUser, int LocalUserNum)
{
    using namespace Redpoint::EOS::Core::Id;

    auto UserId = NewUser.IsValid() ? this->ConvertToUniqueNetId(NewUser->GetUserId()) : FUniqueNetIdEOS::InvalidId();

    UE_LOG(
        LogOnlineSubsystemRedpointEOS,
        Verbose,
        TEXT("Broadcasting 'OnLoginComplete' event for local user %d."),
        LocalUserNum);
    this->TriggerOnLoginCompleteDelegates(
        LocalUserNum,
        NewUser.IsValid(),
        *UserId,
        ErrorCode.WasSuccessful() ? TEXT("") : ErrorCode.ToLogString());
}

void FOnlineIdentityInterfaceEOSImpl::OnLogoutComplete(FError ErrorCode, int LocalUserNum)
{
    UE_LOG(
        LogOnlineSubsystemRedpointEOS,
        Verbose,
        TEXT("Broadcasting 'OnLogoutComplete' event for local user %d."),
        LocalUserNum);
    this->TriggerOnLogoutCompleteDelegates(LocalUserNum, ErrorCode.WasSuccessful());
}

TSharedPtr<FUserOnlineAccount> FOnlineIdentityInterfaceEOSImpl::GetUserAccount(const FUniqueNetId &UserId) const
{
    auto Row = this->LocalAccounts.FindFirstByValue(CLocalAccountsUniqueNetId.Create(UserId.AsShared()));
    if (Row.IsValid())
    {
        return CLocalAccountsUserOnlineAccount.Get(Row.ToSharedRef());
    }
    return nullptr;
}

TArray<TSharedPtr<FUserOnlineAccount>> FOnlineIdentityInterfaceEOSImpl::GetAllUserAccounts() const
{
    TArray<TSharedPtr<FUserOnlineAccount>> Results;
    for (const auto &Row : this->LocalAccounts.GetAllRows())
    {
        Results.Add(CLocalAccountsUserOnlineAccount.Get(Row));
    }
    return Results;
}

FUniqueNetIdPtr FOnlineIdentityInterfaceEOSImpl::GetUniquePlayerId(int32 LocalUserNum) const
{
    auto Row = this->LocalAccounts.FindFirstByValue(CLocalAccountsLocalUserNum.Create(LocalUserNum));
    if (Row.IsValid())
    {
        return CLocalAccountsUniqueNetId.Get(Row.ToSharedRef());
    }
    return nullptr;
}

FUniqueNetIdPtr FOnlineIdentityInterfaceEOSImpl::CreateUniquePlayerId(uint8 *Bytes, int32 Size)
{
    FString Data;
    Data.Empty(Size);
    for (int i = 0; i < Size; i++)
    {
        if (Bytes[i] == 0)
        {
            break;
        }
        Data += (ANSICHAR)Bytes[i];
    }
    return FUniqueNetIdEOS::ParseFromString(Data);
}

FUniqueNetIdPtr FOnlineIdentityInterfaceEOSImpl::CreateUniquePlayerId(const FString &Str)
{
    return FUniqueNetIdEOS::ParseFromString(Str);
}

ELoginStatus::Type FOnlineIdentityInterfaceEOSImpl::GetLoginStatus(int32 LocalUserNum) const
{
    return this->LocalAccounts.FindFirstByValue(CLocalAccountsLocalUserNum.Create(LocalUserNum)).IsValid()
               ? ELoginStatus::LoggedIn
               : ELoginStatus::NotLoggedIn;
}

ELoginStatus::Type FOnlineIdentityInterfaceEOSImpl::GetLoginStatus(const FUniqueNetId &UserId) const
{
    return this->LocalAccounts.FindFirstByValue(CLocalAccountsUniqueNetId.Create(UserId.AsShared())).IsValid()
               ? ELoginStatus::LoggedIn
               : ELoginStatus::NotLoggedIn;
}

FString FOnlineIdentityInterfaceEOSImpl::GetPlayerNickname(int32 LocalUserNum) const
{
    auto Row = this->LocalAccounts.FindFirstByValue(CLocalAccountsLocalUserNum.Create(LocalUserNum));
    if (Row.IsValid())
    {
        return CLocalAccountsUserOnlineAccount.Get(Row.ToSharedRef())->GetDisplayName();
    }
    return TEXT("");
}

FString FOnlineIdentityInterfaceEOSImpl::GetPlayerNickname(const FUniqueNetId &UserId) const
{
    auto Row = this->LocalAccounts.FindFirstByValue(CLocalAccountsUniqueNetId.Create(UserId.AsShared()));
    if (Row.IsValid())
    {
        return CLocalAccountsUserOnlineAccount.Get(Row.ToSharedRef())->GetDisplayName();
    }
    return TEXT("");
}

FString FOnlineIdentityInterfaceEOSImpl::GetAuthToken(int32 LocalUserNum) const
{
    auto Row = this->LocalAccounts.FindFirstByValue(CLocalAccountsLocalUserNum.Create(LocalUserNum));
    if (Row.IsValid())
    {
        return CLocalAccountsUserOnlineAccount.Get(Row.ToSharedRef())->GetAccessToken();
    }
    return TEXT("");
}

void FOnlineIdentityInterfaceEOSImpl::RevokeAuthToken(
    const FUniqueNetId &LocalUserId,
    const FOnRevokeAuthTokenCompleteDelegate &Delegate)
{
    using namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::Error;

    Delegate.ExecuteIfBound(LocalUserId, AsOSSError(Errors::NotImplemented()));
}

void FOnlineIdentityInterfaceEOSImpl::ClearCachedAuthToken(const FUniqueNetId &UserId)
{
}

#if REDPOINT_EOS_UE_5_4_OR_LATER
void FOnlineIdentityInterfaceEOSImpl::GetUserPrivilege(
    const FUniqueNetId &LocalUserId,
    EUserPrivileges::Type Privilege,
    const FOnGetUserPrivilegeCompleteDelegate &Delegate,
    EShowPrivilegeResolveUI ShowResolveUI)
#else
void FOnlineIdentityInterfaceEOSImpl::GetUserPrivilege(
    const FUniqueNetId &LocalUserId,
    EUserPrivileges::Type Privilege,
    const FOnGetUserPrivilegeCompleteDelegate &Delegate)
#endif
{
    // @todo: When the EOS SDK surfaces this information, use it instead of returning success for everything.

    switch (Privilege)
    {
    case EUserPrivileges::Type::CanPlay:
    case EUserPrivileges::Type::CanPlayOnline:
    case EUserPrivileges::Type::CanCommunicateOnline:
    case EUserPrivileges::Type::CanUseUserGeneratedContent:
    case EUserPrivileges::Type::CanUserCrossPlay:
        Delegate.ExecuteIfBound(LocalUserId, Privilege, (uint32)IOnlineIdentity::EPrivilegeResults::NoFailures);
        break;
    default:
        // Unknown privilege.
        Delegate.ExecuteIfBound(LocalUserId, Privilege, (uint32)IOnlineIdentity::EPrivilegeResults::GenericFailure);
        break;
    }
}

FPlatformUserId FOnlineIdentityInterfaceEOSImpl::GetPlatformUserIdFromUniqueNetId(const FUniqueNetId &UniqueNetId) const
{
    auto Row = this->LocalAccounts.FindFirstByValue(CLocalAccountsUniqueNetId.Create(UniqueNetId.AsShared()));
    if (Row.IsValid())
    {
        return CLocalAccountsIdentityUser.Get(Row.ToSharedRef())->GetProcessGlobalUserHandle();
    }
    return PLATFORMUSERID_NONE;
}

FPlatformUserId FOnlineIdentityInterfaceEOSImpl::GetPlatformUserIdFromLocalUserNum(int32 LocalUserNum) const
{
    auto Row = this->LocalAccounts.FindFirstByValue(CLocalAccountsLocalUserNum.Create(LocalUserNum));
    if (Row.IsValid())
    {
        return CLocalAccountsIdentityUser.Get(Row.ToSharedRef())->GetProcessGlobalUserHandle();
    }
    return PLATFORMUSERID_NONE;
}

int32 FOnlineIdentityInterfaceEOSImpl::GetLocalUserNumFromPlatformUserId(FPlatformUserId PlatformUserId) const
{
    auto Row = this->LocalAccounts.FindFirstByValue(CLocalAccountsPlatformUserId.Create(PlatformUserId));
    if (Row.IsValid())
    {
        return CLocalAccountsLocalUserNum.Get(Row.ToSharedRef());
    }
    return -1;
}

FString FOnlineIdentityInterfaceEOSImpl::GetAuthType() const
{
    return TEXT("");
}

bool FOnlineIdentityInterfaceEOSImpl::GetLocalUserNum(const FUniqueNetId &UniqueNetId, int32 &OutLocalUserNum) const
{
    auto Row = this->LocalAccounts.FindFirstByValue(CLocalAccountsUniqueNetId.Create(UniqueNetId.AsShared()));
    if (Row.IsValid())
    {
        OutLocalUserNum = CLocalAccountsLocalUserNum.Get(Row.ToSharedRef());
        return true;
    }
    OutLocalUserNum = 0;
    return false;
}

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

TSharedPtr<const FCrossPlatformAccountId> FOnlineIdentityInterfaceEOSImpl::GetCrossPlatformAccountId(
    const FUniqueNetId &UniqueNetId) const
{
    auto Row = this->LocalAccounts.FindFirstByValue(CLocalAccountsUniqueNetId.Create(UniqueNetId.AsShared()));
    if (Row.IsValid())
    {
        return CLocalAccountsIdentityUser.Get(Row.ToSharedRef())->GetCrossPlatformAccountId();
    }
    return nullptr;
}

bool FOnlineIdentityInterfaceEOSImpl::IsCrossPlatformAccountProviderAvailable() const
{
    return !this->Config->GetCrossPlatformAccountProvider().IsEqual(NAME_None);
}

TSharedPtr<FOnlineSubsystemRedpointEAS> FOnlineIdentityInterfaceEOSImpl::GetEpicGamesOnlineSubsystem() const
{
    return this->EpicGamesOnlineSubsystem;
}

#endif

FName FOnlineIdentityInterfaceEOSImpl::GetInstanceName() const
{
    return this->InstanceName;
}

bool FOnlineIdentityInterfaceEOSImpl::GetNativeSubsystemName(int32 LocalUserNum, FName &OutNativeSubsystemName) const
{
    auto Row = this->LocalAccounts.FindFirstByValue(CLocalAccountsLocalUserNum.Create(LocalUserNum));
    if (Row.IsValid())
    {
        OutNativeSubsystemName = CLocalAccountsIdentityUser.Get(Row.ToSharedRef())->GetNativeSubsystemName();
        return !OutNativeSubsystemName.IsNone();
    }
    return false;
}

}

REDPOINT_EOS_CODE_GUARD_END()