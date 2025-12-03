// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(518246272, Redpoint::EOS::Rooms)
{

class REDPOINTEOSROOMS_API FRoomId
{
private:
    FName ProviderName;
    FString RoomId;

public:
    FRoomId(const FName &InProviderName, const FString &InRoomId);
    UE_NONCOPYABLE(FRoomId);
    ~FRoomId() = default;

    const FName &GetProviderName() const;
    const FString &GetRoomId() const;

    FString ToString() const;
    static TSharedPtr<const FRoomId> FromString(const FString &InRoomId);

    bool IsEqual(const FRoomId &OtherRoomId) const;
    bool IsEqual(const TSharedRef<const FRoomId> &OtherRoomId) const;

    friend inline uint32 GetTypeHash(const FRoomId &Value)
    {
        return GetTypeHash(Value.ToString());
    }

    void SplitToInt32(int32 &Out1, int32 &Out2, int32 &Out3, int32 &Out4, int32 &Out5) const;
    static TSharedPtr<const FRoomId> JoinFromInt32(int32 In1, int32 In2, int32 In3, int32 In4, int32 In5);
};

typedef TSharedRef<const FRoomId> FRoomIdRef;
typedef TSharedPtr<const FRoomId> FRoomIdPtr;

}

namespace Redpoint::EOS::Rooms
{
REDPOINT_EOS_FILE_NS_EXPORT(518246272, Redpoint::EOS::Rooms, FRoomId)
REDPOINT_EOS_FILE_NS_EXPORT(518246272, Redpoint::EOS::Rooms, FRoomIdRef)
REDPOINT_EOS_FILE_NS_EXPORT(518246272, Redpoint::EOS::Rooms, FRoomIdPtr)
}

REDPOINT_EOS_CODE_GUARD_END()