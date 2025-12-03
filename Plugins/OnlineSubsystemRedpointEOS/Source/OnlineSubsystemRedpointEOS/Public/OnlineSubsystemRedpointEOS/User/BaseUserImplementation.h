// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "OnlineSubsystemRedpointEOS/User/EOSUser.h"
#include "OnlineSubsystemRedpointEOS/User/EOSUserFactory.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSCore/Id/UserIdMap.h"
#include "RedpointEOSCore/Module.h"
#include "RedpointEOSUserCache/UserCacheSystem.h"
#include "RedpointEOSUserCache/UserInfo.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

template <typename TDerivedClass, typename TBaseClass>
class TBaseUserImplementation : public TBaseClass, public IEOSUser
{
private:
    FName InstanceName;
    TSharedRef<const FUniqueNetIdEOS> UserId;
    Redpoint::EOS::UserCache::FUserInfoPtr UserInfo;

protected:
    TBaseUserImplementation(const FName &InInstanceName, const TSharedRef<const FUniqueNetIdEOS> &InUserId)
        : InstanceName(InInstanceName)
        , UserId(InUserId)
        , UserInfo()
    {
    }

    TBaseUserImplementation(const FName &InInstanceName, const Redpoint::EOS::UserCache::FUserInfoPtr &InUserInfo)
        : InstanceName(InInstanceName)
        , UserId(Redpoint::EOS::Core::Id::GetUniqueNetId(InUserInfo->GetUserId()).ToSharedRef())
        , UserInfo(InUserInfo)
    {
    }

public:
    UE_NONCOPYABLE(TBaseUserImplementation);
    virtual ~TBaseUserImplementation() override = default;

    virtual FUniqueNetIdRef GetUserId() const override
    {
        return this->UserId;
    }

    virtual FString GetRealName() const override
    {
        return TEXT("");
    }

    virtual FString GetDisplayName(const FString &Platform) const override
    {
        if (this->UserInfo.IsValid())
        {
            return this->UserInfo->GetAttributes().FindRef(TEXT("displayName"));
        }
        return TEXT("");
    }

    virtual bool GetUserAttribute(const FString &AttrName, FString &OutAttrValue) const override
    {
        if (this->UserInfo.IsValid())
        {
            auto *Value = UserInfo->GetAttributes().Find(AttrName);
            if (Value != nullptr)
            {
                OutAttrValue = *Value;
                return true;
            }
        }
        OutAttrValue = TEXT("");
        return false;
    }

    bool IsValid() const
    {
        return this->UserInfo.IsValid();
    }

    TSharedRef<const FUniqueNetIdEOS> GetUserIdEOS() const
    {
        return this->UserId;
    }

    Redpoint::EOS::UserCache::FUserInfoRef GetUserInfo() const
    {
        checkf(this->IsValid(), TEXT("FOnlineUserEOS is not valid to call GetUserInfo on."));
        return this->UserInfo.ToSharedRef();
    }

public:
    /**
     * Get a bunch of users of this type, without waiting for their online information to be ready.
     */
    static TUserIdMap<TSharedPtr<TDerivedClass>> GetUnresolved(
        const TSharedRef<FEOSUserFactory> &InUserFactory,
        const TSharedRef<const FUniqueNetIdEOS> &InQueryingUserId,
        const TArray<TSharedRef<const FUniqueNetIdEOS>> &InUserIds)
    {
        using namespace ::Redpoint::EOS::UserCache;
        using namespace ::Redpoint::EOS::Core::Id;
        using namespace ::Redpoint::EOS::API;

        auto UserCache = InUserFactory->PlatformHandle->GetSystem<IUserCacheSystem>();
        TUserIdMap<TSharedPtr<TDerivedClass>> NewMap;
        for (const auto &UserId : InUserIds)
        {
            NewMap.Add(
                *UserId,
                MakeShareable(new TDerivedClass(
                    InUserFactory->InstanceName,
                    UserCache->GetUnresolvedUser(GetAccountId(InQueryingUserId), GetAccountId(UserId)))));
        }
        return NewMap;
    }

    /**
     * Get a bunch of users of this type, waiting for their online information to be ready.
     */
    static void Get(
        const TSharedRef<FEOSUserFactory> &InUserFactory,
        const TSharedRef<const FUniqueNetIdEOS> &InQueryingUserId,
        const TArray<TSharedRef<const FUniqueNetIdEOS>> &InUserIds,
        TFunction<void(const TUserIdMap<TSharedPtr<TDerivedClass>> &)> OnResolved)
    {
        using namespace ::Redpoint::EOS::UserCache;
        using namespace ::Redpoint::EOS::Core::Id;
        using namespace ::Redpoint::EOS::API;

        TArray<UE::Online::FAccountId> TargetUserIds;
        for (const auto &UserId : InUserIds)
        {
            TargetUserIds.Add(GetAccountId(UserId));
        }

        auto UserCache = InUserFactory->PlatformHandle->GetSystem<IUserCacheSystem>();
        UserCache->GetUsers(
            GetAccountId(InQueryingUserId),
            TargetUserIds,
            IUserCacheSystem::FOnGetUsersComplete::CreateLambda(
                [OnResolved, CapturedInstanceName = InUserFactory->InstanceName](
                    FError ErrorCode,
                    TMap<UE::Online::FAccountId, FErrorableUserInfo> UserInfos) {
                    TUserIdMap<TSharedPtr<TDerivedClass>> NewMap;
                    for (const auto &KV : UserInfos)
                    {
                        if (!KV.Value.IsErrored())
                        {
                            NewMap.Add(
                                *GetUniqueNetId(KV.Key).ToSharedRef(),
                                MakeShareable(new TDerivedClass(CapturedInstanceName, KV.Value.GetUserInfo())));
                        }
                        else
                        {
                            NewMap.Add(*GetUniqueNetId(KV.Key).ToSharedRef(), nullptr);
                        }
                    }
                    OnResolved(NewMap);
                }));
    }

    static TSharedPtr<TDerivedClass> NewInvalid(const TSharedRef<const FUniqueNetIdEOS> &InUserId)
    {
        return MakeShareable(new TDerivedClass(NAME_None, InUserId));
    }
};

REDPOINT_EOS_CODE_GUARD_END()