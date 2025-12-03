// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSFriends/Database/SerializedCachedFriend.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSAPI/Common/ExternalAccountType.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3744226451, Redpoint::EOS::Friends::Database)
{
using namespace ::Redpoint::EOS::API::Common;

static FArchive &operator<<(FArchive &Ar, EOS_EExternalAccountType &Obj)
{
    if (Ar.IsSaving())
    {
        auto Value = ExternalAccountTypeMap.Get(Obj);
        Ar << Value;
    }
    else
    {
        FString Value;
        Ar << Value;
        Obj = ExternalAccountTypeMap.Get(Value);
    }
    return Ar;
}

FArchive &operator<<(FArchive &Ar, FSerializedCachedFriend::FSerializedCachedFriendLinkedAccount &Obj)
{
    int8 Version = -1;
    if (Ar.IsSaving())
    {
        Version = 1;
    }
    Ar << Version;
    Ar << Obj.ExternalAccountType;
    Ar << Obj.DisplayName;
    Ar << Obj.AccountId;
    Ar << Obj.LastLoginTime;
    return Ar;
}

FArchive &operator<<(FArchive &Ar, FSerializedCachedFriend &Obj)
{
    int8 Version = -1;
    if (Ar.IsSaving())
    {
        Version = 1;
    }
    Ar << Version;
    if (Ar.IsLoading())
    {
        Obj.LinkedAccounts.Empty();
        int32 Count;
        Ar << Count;
        for (int32 i = 0; i < Count; i++)
        {
            EOS_EExternalAccountType Key;
            FSerializedCachedFriend::FSerializedCachedFriendLinkedAccount Value;
            Ar << Key;
            Ar << Value;
            Obj.LinkedAccounts.Add(Key, Value);
        }
    }
    else
    {
        int32 Count = Obj.LinkedAccounts.Num();
        Ar << Count;
        for (const auto &KV : Obj.LinkedAccounts)
        {
            auto Key = KV.Key;
            auto Value = KV.Value;
            Ar << Key;
            Ar << Value;
        }
    }
    Ar << Obj.AccountAvatarUrl;
    Ar << Obj.AccountAvatarUrlLastFetched;
    return Ar;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()