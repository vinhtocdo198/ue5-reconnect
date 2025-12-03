// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSAPI/Templates/Util/ApiCallNativeAllocator.h"
#include "RedpointEOSAPI/Templates/Util/ApiCallNativeConverter.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::LobbySessionsShared
{

class REDPOINTEOSAPI_API FAttributeData
{
public:
    enum class EType : uint8
    {
        Int64,
        Double,
        Bool,
        String,
    };

private:
    FString _Key;
    union {
        int64 AsInt64;
        double AsDouble;
        bool AsBool;
    } _Value;
    FString _ValueAsString;
    EType _ValueType;

public:
    FAttributeData();
    FAttributeData(const FString &Key, int64 Value);
    FAttributeData(const FString &Key, double Value);
    FAttributeData(const FString &Key, bool Value);
    FAttributeData(const FString &Key, const FString &Value);
    FAttributeData(
        const EOS_Lobby_AttributeData &InLobbyData,
        const ::Redpoint::EOS::API::Templates::Util::FApiCallNativeConverter &Converter);
    FAttributeData(
        const EOS_Sessions_AttributeData &InSessionData,
        const ::Redpoint::EOS::API::Templates::Util::FApiCallNativeConverter &Converter);

    EOS_Lobby_AttributeData *ToLobbyViaAllocator(
        ::Redpoint::EOS::API::Templates::Util::FApiCallNativeAllocator &Allocator) const;
    EOS_Sessions_AttributeData *ToSessionViaAllocator(
        ::Redpoint::EOS::API::Templates::Util::FApiCallNativeAllocator &Allocator) const;

    const FString &Key() const;
    const EType &Type() const;

    bool TryGetInt64(int64 &OutValue) const;
    bool TryGetDouble(double &OutValue) const;
    bool TryGetBool(bool &OutValue) const;
    bool TryGetString(FString &OutValue) const;

    int64 GetInt64() const;
    double GetDouble() const;
    bool GetBool() const;
    FString GetString() const;

    bool operator==(const FAttributeData &) const;
    bool operator!=(const FAttributeData &) const;

    REDPOINTEOSAPI_API friend uint32 GetTypeHash(const FAttributeData &);
};

}

REDPOINT_EOS_CODE_GUARD_END()