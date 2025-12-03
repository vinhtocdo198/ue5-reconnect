// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/LobbySessionsShared/AttributeData.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::LobbySessionsShared
{

FAttributeData::FAttributeData()
    : _Key()
    , _ValueAsString()
    , _ValueType(EType::Int64)
{
    _Value.AsInt64 = 0;
}

FAttributeData::FAttributeData(const FString &InKey, int64 InValue)
    : _Key(InKey)
    , _ValueAsString()
    , _ValueType(EType::Int64)
{
    _Value.AsInt64 = InValue;
}

FAttributeData::FAttributeData(const FString &InKey, double InValue)
    : _Key(InKey)
    , _ValueAsString()
    , _ValueType(EType::Double)
{
    _Value.AsDouble = InValue;
}

FAttributeData::FAttributeData(const FString &InKey, bool InValue)
    : _Key(InKey)
    , _ValueAsString()
    , _ValueType(EType::Bool)
{
    _Value.AsBool = InValue;
}

FAttributeData::FAttributeData(const FString &InKey, const FString &InValue)
    : _Key(InKey)
    , _ValueAsString(InValue)
    , _ValueType(EType::String)
{
    _Value.AsInt64 = 0;
}

FAttributeData::FAttributeData(
    const EOS_Lobby_AttributeData &InLobbyData,
    const ::Redpoint::EOS::API::Templates::Util::FApiCallNativeConverter &Converter)
    : _Key(Converter.FromUtf8(InLobbyData.Key))
    , _ValueAsString()
    , _ValueType(EType::Int64)
{
    switch (InLobbyData.ValueType)
    {
    case EOS_ELobbyAttributeType::EOS_AT_INT64:
        this->_Value.AsInt64 = InLobbyData.Value.AsInt64;
        this->_ValueType = EType::Int64;
        break;
    case EOS_ELobbyAttributeType::EOS_AT_DOUBLE:
        this->_Value.AsDouble = InLobbyData.Value.AsDouble;
        this->_ValueType = EType::Double;
        break;
    case EOS_ELobbyAttributeType::EOS_AT_BOOLEAN:
        this->_Value.AsBool = InLobbyData.Value.AsBool == EOS_TRUE;
        this->_ValueType = EType::Bool;
        break;
    case EOS_ELobbyAttributeType::EOS_AT_STRING:
        this->_Value.AsInt64 = 0;
        this->_ValueAsString = Converter.FromUtf8(InLobbyData.Value.AsUtf8);
        this->_ValueType = EType::String;
        break;
    default:
        this->_Value.AsInt64 = 0;
        this->_ValueType = EType::Int64;
        break;
    }
}

FAttributeData::FAttributeData(
    const EOS_Sessions_AttributeData &InSessionData,
    const ::Redpoint::EOS::API::Templates::Util::FApiCallNativeConverter &Converter)
    : _Key(Converter.FromUtf8(InSessionData.Key))
    , _ValueAsString()
    , _ValueType(EType::Int64)
{
    switch (InSessionData.ValueType)
    {
    case EOS_ESessionAttributeType::EOS_AT_INT64:
        this->_Value.AsInt64 = InSessionData.Value.AsInt64;
        this->_ValueType = EType::Int64;
        break;
    case EOS_ESessionAttributeType::EOS_AT_DOUBLE:
        this->_Value.AsDouble = InSessionData.Value.AsDouble;
        this->_ValueType = EType::Double;
        break;
    case EOS_ESessionAttributeType::EOS_AT_BOOLEAN:
        this->_Value.AsBool = InSessionData.Value.AsBool == EOS_TRUE;
        this->_ValueType = EType::Bool;
        break;
    case EOS_ESessionAttributeType::EOS_AT_STRING:
        this->_Value.AsInt64 = 0;
        this->_ValueAsString = Converter.FromUtf8(InSessionData.Value.AsUtf8);
        this->_ValueType = EType::String;
        break;
    default:
        this->_Value.AsInt64 = 0;
        this->_ValueType = EType::Int64;
        break;
    }
}

EOS_Lobby_AttributeData *FAttributeData::ToLobbyViaAllocator(
    ::Redpoint::EOS::API::Templates::Util::FApiCallNativeAllocator &Allocator) const
{
    auto &Data = Allocator.Allocate<EOS_Lobby_AttributeData>();
    Data.ApiVersion = EOS_LOBBY_ATTRIBUTEDATA_API_LATEST;
    Data.Key = Allocator.AsUtf8(this->_Key);
    switch (this->_ValueType)
    {
    case EType::Int64:
        Data.Value.AsInt64 = this->_Value.AsInt64;
        Data.ValueType = EOS_ELobbyAttributeType::EOS_AT_INT64;
        break;
    case EType::Double:
        Data.Value.AsDouble = this->_Value.AsDouble;
        Data.ValueType = EOS_ELobbyAttributeType::EOS_AT_DOUBLE;
        break;
    case EType::Bool:
        Data.Value.AsBool = this->_Value.AsBool;
        Data.ValueType = EOS_ELobbyAttributeType::EOS_AT_BOOLEAN;
        break;
    case EType::String:
        Data.Value.AsUtf8 = Allocator.AsUtf8(this->_ValueAsString);
        Data.ValueType = EOS_ELobbyAttributeType::EOS_AT_STRING;
        break;
    default:
        Data.Value.AsInt64 = 0;
        Data.ValueType = EOS_ELobbyAttributeType::EOS_AT_INT64;
        break;
    }
    return &Data;
}

EOS_Sessions_AttributeData *FAttributeData::ToSessionViaAllocator(
    ::Redpoint::EOS::API::Templates::Util::FApiCallNativeAllocator &Allocator) const
{
    auto &Data = Allocator.Allocate<EOS_Sessions_AttributeData>();
    Data.ApiVersion = EOS_SESSIONS_ATTRIBUTEDATA_API_LATEST;
    Data.Key = Allocator.AsUtf8(this->_Key);
    switch (this->_ValueType)
    {
    case EType::Int64:
        Data.Value.AsInt64 = this->_Value.AsInt64;
        Data.ValueType = EOS_ESessionAttributeType::EOS_AT_INT64;
        break;
    case EType::Double:
        Data.Value.AsDouble = this->_Value.AsDouble;
        Data.ValueType = EOS_ESessionAttributeType::EOS_AT_DOUBLE;
        break;
    case EType::Bool:
        Data.Value.AsBool = this->_Value.AsBool;
        Data.ValueType = EOS_ESessionAttributeType::EOS_AT_BOOLEAN;
        break;
    case EType::String:
        Data.Value.AsUtf8 = Allocator.AsUtf8(this->_ValueAsString);
        Data.ValueType = EOS_ESessionAttributeType::EOS_AT_STRING;
        break;
    default:
        Data.Value.AsInt64 = 0;
        Data.ValueType = EOS_ESessionAttributeType::EOS_AT_INT64;
        break;
    }
    return &Data;
}

const FString &FAttributeData::Key() const
{
    return this->_Key;
}

const FAttributeData::EType &FAttributeData::Type() const
{
    return this->_ValueType;
}

bool FAttributeData::TryGetInt64(int64 &OutValue) const
{
    if (this->_ValueType == EType::Int64)
    {
        OutValue = this->_Value.AsInt64;
        return true;
    }
    return false;
}

bool FAttributeData::TryGetDouble(double &OutValue) const
{
    if (this->_ValueType == EType::Double)
    {
        OutValue = this->_Value.AsDouble;
        return true;
    }
    return false;
}

bool FAttributeData::TryGetBool(bool &OutValue) const
{
    if (this->_ValueType == EType::Bool)
    {
        OutValue = this->_Value.AsBool;
        return true;
    }
    return false;
}

bool FAttributeData::TryGetString(FString &OutValue) const
{
    if (this->_ValueType == EType::String)
    {
        OutValue = this->_ValueAsString;
        return true;
    }
    return false;
}

int64 FAttributeData::GetInt64() const
{
    check(this->_ValueType == EType::Int64);
    return this->_Value.AsInt64;
}

double FAttributeData::GetDouble() const
{
    check(this->_ValueType == EType::Double);
    return this->_Value.AsDouble;
}

bool FAttributeData::GetBool() const
{
    check(this->_ValueType == EType::Bool);
    return this->_Value.AsBool;
}

FString FAttributeData::GetString() const
{
    check(this->_ValueType == EType::String);
    return this->_ValueAsString;
}

bool FAttributeData::operator==(const FAttributeData &Other) const
{
    if (this->_Key != Other._Key || this->_ValueType != Other._ValueType)
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

bool FAttributeData::operator!=(const FAttributeData &Other) const
{
    return !(*this == Other);
}

uint32 GetTypeHash(const FAttributeData &A)
{
    uint32 H = GetTypeHashHelper(A._Key);
    H = HashCombineFast(H, GetTypeHashHelper(A._ValueType));
    switch (A._ValueType)
    {
    case FAttributeData::EType::Int64:
        H = HashCombineFast(H, GetTypeHashHelper(A._Value.AsInt64));
        break;
    case FAttributeData::EType::Double:
        H = HashCombineFast(H, GetTypeHashHelper(A._Value.AsDouble));
        break;
    case FAttributeData::EType::Bool:
        H = HashCombineFast(H, GetTypeHashHelper(A._Value.AsBool));
        break;
    case FAttributeData::EType::String:
        H = HashCombineFast(H, GetTypeHashHelper(A._ValueAsString));
        break;
    default:
        checkf(false, TEXT("Unsupported FAttributeData::EType in GetTypeHash!"));
        break;
    }
    return H;
}

}

REDPOINT_EOS_CODE_GUARD_END()