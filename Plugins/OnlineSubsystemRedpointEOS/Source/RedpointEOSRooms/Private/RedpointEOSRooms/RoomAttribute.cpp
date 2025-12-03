// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/RoomAttribute.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(113096794, Redpoint::EOS::Rooms)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::API::LobbySessionsShared;

FRoomAttribute::FRoomAttribute()
    : _ValueAsString()
    , _ValueType(EType::Int64)
    , _Visibility(EVisibility::Public)
{
    _Value.AsInt64 = 0;
}

FRoomAttribute::FRoomAttribute(int64 InValue, EVisibility InVisibility)
    : _ValueAsString()
    , _ValueType(EType::Int64)
    , _Visibility(InVisibility)
{
    _Value.AsInt64 = InValue;
}

FRoomAttribute::FRoomAttribute(double InValue, EVisibility InVisibility)
    : _ValueAsString()
    , _ValueType(EType::Double)
    , _Visibility(InVisibility)
{
    _Value.AsDouble = InValue;
}

FRoomAttribute::FRoomAttribute(bool InValue, EVisibility InVisibility)
    : _ValueAsString()
    , _ValueType(EType::Bool)
    , _Visibility(InVisibility)
{
    _Value.AsBool = InValue;
}

FRoomAttribute::FRoomAttribute(const FString &InValue, EVisibility InVisibility)
    : _ValueAsString(InValue)
    , _ValueType(EType::String)
    , _Visibility(InVisibility)
{
    _Value.AsInt64 = 0;
}

FRoomAttribute::FRoomAttribute(const TCHAR *InValue, EVisibility InVisibility)
    : _ValueAsString(InValue)
    , _ValueType(EType::String)
    , _Visibility(InVisibility)
{
    _Value.AsInt64 = 0;
}

FRoomAttribute::FRoomAttribute(const TArray<FString> &InValues, EVisibility InVisibility)
    : _ValueAsString(FString::Join(InValues, TEXT(";")))
    , _ValueType(EType::String)
    , _Visibility(InVisibility)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);
#if DO_CHECK
    for (const auto &Value : InValues)
    {
        checkf(
            !Value.Contains(TEXT(";")),
            TEXT("Entry in string array value for FRoomAttribute contains ';' character, which can not be preserved."));
    }
#endif
    _Value.AsInt64 = 0;
}

FRoomAttribute::FRoomAttribute(const FRoomAttribute &InValue, EVisibility InVisibility)
    : _Value(InValue._Value)
    , _ValueAsString(InValue._ValueAsString)
    , _ValueType(InValue._ValueType)
    , _Visibility(InVisibility)
{
}

FRoomAttribute::FRoomAttribute(const FAttributeData &InValue, EVisibility InVisibility)
    : _ValueType(InValue.Type())
    , _Visibility(InVisibility)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);
    switch (InValue.Type())
    {
    case EType::Int64:
        _Value.AsInt64 = InValue.GetInt64();
        break;
    case EType::Double:
        _Value.AsDouble = InValue.GetDouble();
        break;
    case EType::Bool:
        _Value.AsBool = InValue.GetBool();
        break;
    case EType::String:
        _ValueAsString = InValue.GetString();
        break;
    default:
        checkf(false, TEXT("Unhandled FRoomAttribute::EType value!"));
        break;
    }
}

FAttributeData FRoomAttribute::ToAttributeData(const FString &AssociatedKey) const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);
    switch (this->_ValueType)
    {
    case EType::Int64:
        return FAttributeData(AssociatedKey, this->_Value.AsInt64);
    case EType::Double:
        return FAttributeData(AssociatedKey, this->_Value.AsDouble);
    case EType::Bool:
        return FAttributeData(AssociatedKey, this->_Value.AsBool);
    case EType::String:
        return FAttributeData(AssociatedKey, this->_ValueAsString);
    default:
        return FAttributeData(AssociatedKey, (int64)0);
    }
}

EOS_ELobbyAttributeVisibility FRoomAttribute::GetLobbyAttributeVisibility() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);
    if (this->_Visibility == EVisibility::Public)
    {
        return EOS_ELobbyAttributeVisibility::EOS_LAT_PUBLIC;
    }
    else
    {
        return EOS_ELobbyAttributeVisibility::EOS_LAT_PRIVATE;
    }
}

EOS_ESessionAttributeAdvertisementType FRoomAttribute::GetSessionAttributeAdvertisementType() const
{
    if (this->_Visibility == EVisibility::Public)
    {
        return EOS_ESessionAttributeAdvertisementType::EOS_SAAT_Advertise;
    }
    else
    {
        return EOS_ESessionAttributeAdvertisementType::EOS_SAAT_DontAdvertise;
    }
}

const FRoomAttribute::EType &FRoomAttribute::Type() const
{
    return this->_ValueType;
}

const FRoomAttribute::EVisibility &FRoomAttribute::Visibility() const
{
    return this->_Visibility;
}

bool FRoomAttribute::TryGetInt64(int64 &OutValue) const
{
    if (this->_ValueType == EType::Int64)
    {
        OutValue = this->_Value.AsInt64;
        return true;
    }
    return false;
}

bool FRoomAttribute::TryGetDouble(double &OutValue) const
{
    if (this->_ValueType == EType::Double)
    {
        OutValue = this->_Value.AsDouble;
        return true;
    }
    return false;
}

bool FRoomAttribute::TryGetBool(bool &OutValue) const
{
    if (this->_ValueType == EType::Bool)
    {
        OutValue = this->_Value.AsBool;
        return true;
    }
    return false;
}

bool FRoomAttribute::TryGetString(FString &OutValue) const
{
    if (this->_ValueType == EType::String)
    {
        OutValue = this->_ValueAsString;
        return true;
    }
    return false;
}

bool FRoomAttribute::TryGetStringArray(TArray<FString> &OutValues) const
{
    FString Buffer;
    bool bSuccess = TryGetString(Buffer);
    if (!bSuccess)
    {
        return false;
    }
    Buffer.ParseIntoArray(OutValues, TEXT(";"), false);
    return true;
}

int64 FRoomAttribute::GetInt64() const
{
    check(this->_ValueType == EType::Int64);
    return this->_Value.AsInt64;
}

double FRoomAttribute::GetDouble() const
{
    check(this->_ValueType == EType::Double);
    return this->_Value.AsDouble;
}

bool FRoomAttribute::GetBool() const
{
    check(this->_ValueType == EType::Bool);
    return this->_Value.AsBool;
}

FString FRoomAttribute::GetString() const
{
    check(this->_ValueType == EType::String);
    return this->_ValueAsString;
}

TArray<FString> FRoomAttribute::GetStringArray() const
{
    check(this->_ValueType == EType::String);
    TArray<FString> Result;
    this->_ValueAsString.ParseIntoArray(Result, TEXT(";"), false);
    return Result;
}

bool FRoomAttribute::operator==(const FRoomAttribute &Other) const
{
    if (this->_ValueType != Other._ValueType)
    {
        return false;
    }
    switch (this->_ValueType)
    {
    case EType::Int64:
        return this->_Value.AsInt64 == Other._Value.AsInt64;
    case EType::Double:
        return FMath::IsNearlyEqual(this->_Value.AsDouble, Other._Value.AsDouble);
    case EType::Bool:
        return this->_Value.AsBool == Other._Value.AsBool;
    case EType::String:
        return this->_ValueAsString == Other._ValueAsString;
    default:
        return false;
    }
}

bool FRoomAttribute::operator!=(const FRoomAttribute &Other) const
{
    return !(*this == Other);
}

bool FRoomAttribute::operator<(const FRoomAttribute &Other) const
{
    if (this->_ValueType != Other._ValueType)
    {
        return false;
    }
    switch (this->_ValueType)
    {
    case EType::Int64:
        return this->_Value.AsInt64 < Other._Value.AsInt64;
    case EType::Double:
        return this->_Value.AsDouble < Other._Value.AsDouble;
    case EType::Bool:
        return (int)this->_Value.AsBool < (int)Other._Value.AsBool;
    case EType::String:
    default:
        return false;
    }
}

bool FRoomAttribute::operator>(const FRoomAttribute &Other) const
{
    if (this->_ValueType != Other._ValueType)
    {
        return false;
    }
    switch (this->_ValueType)
    {
    case EType::Int64:
        return this->_Value.AsInt64 > Other._Value.AsInt64;
    case EType::Double:
        return this->_Value.AsDouble > Other._Value.AsDouble;
    case EType::Bool:
        return (int)this->_Value.AsBool > (int)Other._Value.AsBool;
    case EType::String:
    default:
        return false;
    }
}

bool FRoomAttribute::operator<=(const FRoomAttribute &Other) const
{
    if (this->_ValueType != Other._ValueType)
    {
        return false;
    }
    switch (this->_ValueType)
    {
    case EType::Int64:
        return this->_Value.AsInt64 <= Other._Value.AsInt64;
    case EType::Double:
        return this->_Value.AsDouble <= Other._Value.AsDouble;
    case EType::Bool:
        return (int)this->_Value.AsBool <= (int)Other._Value.AsBool;
    case EType::String:
    default:
        return false;
    }
}

bool FRoomAttribute::operator>=(const FRoomAttribute &Other) const
{
    if (this->_ValueType != Other._ValueType)
    {
        return false;
    }
    switch (this->_ValueType)
    {
    case EType::Int64:
        return this->_Value.AsInt64 >= Other._Value.AsInt64;
    case EType::Double:
        return this->_Value.AsDouble >= Other._Value.AsDouble;
    case EType::Bool:
        return (int)this->_Value.AsBool >= (int)Other._Value.AsBool;
    case EType::String:
    default:
        return false;
    }
}

uint32 GetTypeHash(const FRoomAttribute &Attr)
{
    auto H = GetTypeHashHelper(Attr._ValueType);
    switch (Attr._ValueType)
    {
    case FRoomAttribute::EType::Int64:
        H = HashCombine(H, GetTypeHashHelper(Attr._Value.AsInt64));
        break;
    case FRoomAttribute::EType::Double:
        H = HashCombine(H, GetTypeHashHelper(Attr._Value.AsDouble));
        break;
    case FRoomAttribute::EType::Bool:
        H = HashCombine(H, GetTypeHashHelper(Attr._Value.AsBool));
        break;
    case FRoomAttribute::EType::String:
        H = HashCombine(H, GetTypeHashHelper(Attr._ValueAsString));
        break;
    default:
        break;
    }
    return H;
}

}

REDPOINT_EOS_CODE_GUARD_END()