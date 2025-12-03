// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSCore/Id/UniqueNetIdEOS.h"

#include "RedpointEOSCore/Id/EOSString_OpaqueId.h"
#include "RedpointEOSCore/Logging.h"

FUniqueNetIdEOS::FUniqueNetIdEOS(EOS_ProductUserId InInternalProductUserId, bool bInIsDedicatedServer)
    : ProductUserId(InInternalProductUserId)
    , DataBytes(nullptr)
    , DataBytesSize(0)
    , bIsDedicatedServer(bInIsDedicatedServer)
{
    auto Str = StringCast<ANSICHAR>(*this->ToString());
    this->DataBytesSize = Str.Length() + 1;
    this->DataBytes = (uint8 *)FMemory::MallocZeroed(this->DataBytesSize);
    FMemory::Memcpy(this->DataBytes, Str.Get(), Str.Length());
}

FName FUniqueNetIdEOS::GetTypeStatic()
{
    return FName(TEXT("RedpointEOS"));
}

TSharedRef<const FUniqueNetIdEOS> FUniqueNetIdEOS::MakeInvalidId()
{
    return MakeShareable(new FUniqueNetIdEOS(nullptr, false));
}

TSharedRef<const FUniqueNetIdEOS> FUniqueNetIdEOS::MakeDedicatedServerId()
{
    return MakeShareable(new FUniqueNetIdEOS(EOS_ProductUserId_FromString(""), true));
}

FUniqueNetIdEOS::FUniqueNetIdEOS(EOS_ProductUserId InProductUserId)
    : FUniqueNetIdEOS(InProductUserId, false)
{
    check(Redpoint::EOS::Core::Id::EOSString_ProductUserId::IsValid(InProductUserId));
}

bool FUniqueNetIdEOS::Compare(const FUniqueNetId &Other) const
{
    if (Other.GetType() != GetType())
    {
        return false;
    }

    if (Other.GetType() == GetTypeStatic())
    {
        const FUniqueNetIdEOS &OtherEOS = (const FUniqueNetIdEOS &)Other;
        if (Redpoint::EOS::Core::Id::EOSString_ProductUserId::IsValid(OtherEOS.GetProductUserId()) &&
            Redpoint::EOS::Core::Id::EOSString_ProductUserId::IsValid(this->GetProductUserId()))
        {
            // We only want to compare product user IDs, since most APIs won't be able
            // to provide the Epic account ID that might be present in the IDs returned by
            // the identity interface.
            return OtherEOS.GetProductUserIdString() == this->GetProductUserIdString();
        }
    }

    return (GetSize() == Other.GetSize()) && (FMemory::Memcmp(GetBytes(), Other.GetBytes(), GetSize()) == 0);
}

FUniqueNetIdEOS::~FUniqueNetIdEOS()
{
    FMemory::Free(this->DataBytes);
}

/** Get the product user ID. Do not compare this with null for validation; instead call HasValidProductUserId. */
EOS_ProductUserId FUniqueNetIdEOS::GetProductUserId() const
{
    return this->ProductUserId;
}

bool FUniqueNetIdEOS::HasValidProductUserId() const
{
    if (this->bIsDedicatedServer)
    {
        return true;
    }

    return Redpoint::EOS::Core::Id::EOSString_ProductUserId::IsValid(this->ProductUserId);
}

FName FUniqueNetIdEOS::GetType() const
{
    return GetTypeStatic();
}

const uint8 *FUniqueNetIdEOS::GetBytes() const
{
    return this->DataBytes;
}

int32 FUniqueNetIdEOS::GetSize() const
{
    return this->DataBytesSize;
}

bool FUniqueNetIdEOS::IsValid() const
{
    return this->bIsDedicatedServer || Redpoint::EOS::Core::Id::EOSString_ProductUserId::IsValid(this->ProductUserId);
}

FString FUniqueNetIdEOS::ToString() const
{
    if (this->bIsDedicatedServer)
    {
        return EOS_DEDICATED_SERVER_ID;
    }

    return this->GetProductUserIdString(false);
}

FString FUniqueNetIdEOS::ToDebugString() const
{
    if (this->bIsDedicatedServer)
    {
        return EOS_DEDICATED_SERVER_ID;
    }

    return this->GetProductUserIdString(true);
}

TSharedPtr<const FUniqueNetIdEOS> FUniqueNetIdEOS::ParseFromString(const FString &ProductUserIdStr)
{
    using namespace ::Redpoint::EOS::Core;
    using namespace ::Redpoint::EOS::Core::Id;

    if (ProductUserIdStr == FString(EOS_DEDICATED_SERVER_ID))
    {
        return FUniqueNetIdEOS::MakeDedicatedServerId();
    }

    EOS_ProductUserId ProductUserId = nullptr;
    if (!ProductUserIdStr.IsEmpty())
    {
        if (EOSString_ProductUserId::FromString(ProductUserIdStr, ProductUserId) != EOS_EResult::EOS_Success)
        {
            UE_LOG(LogRedpointEOSCore, Error, TEXT("Malformed product user ID: %s"), *ProductUserIdStr);
            return nullptr;
        }
    }

    if (!EOSString_ProductUserId::IsValid(ProductUserId))
    {
        UE_LOG(LogRedpointEOSCore, Error, TEXT("Malformed product user ID: %s"), *ProductUserIdStr);
        return nullptr;
    }

    return MakeShared<FUniqueNetIdEOS>(ProductUserId);
}

uint32 GetTypeHash(const FUniqueNetIdEOS &A)
{
    return GetTypeHash(A.ToString());
}

const TSharedRef<const FUniqueNetIdEOS> &FUniqueNetIdEOS::InvalidId()
{
    static const TSharedRef<const FUniqueNetIdEOS> InvalidId = FUniqueNetIdEOS::MakeInvalidId();
    return InvalidId;
}

const TSharedRef<const FUniqueNetIdEOS> &FUniqueNetIdEOS::DedicatedServerId()
{
    static const TSharedRef<const FUniqueNetIdEOS> DedicatedServerId = FUniqueNetIdEOS::MakeDedicatedServerId();
    return DedicatedServerId;
}

FArchive &operator<<(FArchive &Ar, FUniqueNetIdEOS &OtherId)
{
    auto IdSer = OtherId.ToString();
    return Ar << IdSer;
}

FString FUniqueNetIdEOS::GetProductUserIdString(bool Debug) const
{
    if (Redpoint::EOS::Core::Id::EOSString_ProductUserId::IsNone(this->ProductUserId))
    {
        return Debug ? TEXT("<no product user>") : TEXT("");
    }

    FString Str;
    if (Redpoint::EOS::Core::Id::EOSString_ProductUserId::ToString(this->ProductUserId, Str) ==
        EOS_EResult::EOS_Success)
    {
        return Str;
    }

    return Debug ? TEXT("<unknown product user>") : TEXT("");
}

bool FUniqueNetIdEOS::IsDedicatedServer() const
{
    return this->bIsDedicatedServer;
}