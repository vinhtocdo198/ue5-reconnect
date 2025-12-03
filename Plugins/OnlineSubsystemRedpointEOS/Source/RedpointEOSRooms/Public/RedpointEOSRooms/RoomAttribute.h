// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/LobbySessionsShared/AttributeData.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(113096794, Redpoint::EOS::Rooms)
{
using namespace ::Redpoint::EOS::API::LobbySessionsShared;

class REDPOINTEOSROOMS_API FRoomAttribute
{
public:
    enum class EVisibility : uint8
    {
        Public,
        Private,
    };
    typedef FAttributeData::EType EType;

private:
    union {
        int64 AsInt64;
        double AsDouble;
        bool AsBool;
    } _Value;
    FString _ValueAsString;
    EType _ValueType;
    EVisibility _Visibility;

public:
    FRoomAttribute();
    FRoomAttribute(int64 Value, EVisibility Visibility = EVisibility::Public);
    FRoomAttribute(double Value, EVisibility Visibility = EVisibility::Public);
    FRoomAttribute(bool Value, EVisibility Visibility = EVisibility::Public);
    FRoomAttribute(const FString &Value, EVisibility Visibility = EVisibility::Public);
    FRoomAttribute(const TCHAR *Value, EVisibility Visibility = EVisibility::Public);
    FRoomAttribute(const TArray<FString> &Values, EVisibility Visibility = EVisibility::Public);
    FRoomAttribute(const FRoomAttribute &Value, EVisibility Visibility);
    FRoomAttribute(const FAttributeData &Value, EVisibility Visibility);

    FAttributeData ToAttributeData(const FString &AssociatedKey) const;
    EOS_ELobbyAttributeVisibility GetLobbyAttributeVisibility() const;
    EOS_ESessionAttributeAdvertisementType GetSessionAttributeAdvertisementType() const;

    const EType &Type() const;
    const EVisibility &Visibility() const;

    bool TryGetInt64(int64 &OutValue) const;
    bool TryGetDouble(double &OutValue) const;
    bool TryGetBool(bool &OutValue) const;
    bool TryGetString(FString &OutValue) const;
    bool TryGetStringArray(TArray<FString> &OutValues) const;

    int64 GetInt64() const;
    double GetDouble() const;
    bool GetBool() const;
    FString GetString() const;
    TArray<FString> GetStringArray() const;

    bool operator==(const FRoomAttribute &) const;
    bool operator!=(const FRoomAttribute &) const;
    bool operator<(const FRoomAttribute &) const;
    bool operator>(const FRoomAttribute &) const;
    bool operator<=(const FRoomAttribute &) const;
    bool operator>=(const FRoomAttribute &) const;

    REDPOINTEOSROOMS_API friend uint32 GetTypeHash(const FRoomAttribute &);
};

}

namespace Redpoint::EOS::Rooms
{
REDPOINT_EOS_FILE_NS_EXPORT(113096794, Redpoint::EOS::Rooms, FRoomAttribute)
}

REDPOINT_EOS_CODE_GUARD_END()