// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSAPI/Templates/Util/ApiCallNativeAllocator.h"
#include "RedpointEOSAPI/Templates/Util/ApiCallNativeConverter.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Auth
{

class REDPOINTEOSAPI_API FCopyUserAuthToken
{
private:
    typedef EOS_HAuth NativeHandle;
    typedef EOS_Auth_CopyUserAuthTokenOptions NativeOptions;
    typedef EOS_Auth_Token NativeResult;
    typedef ::Redpoint::EOS::API::Templates::Util::FApiCallNativeAllocator NativeAllocator;
    typedef ::Redpoint::EOS::API::Templates::Util::FApiCallNativeConverter NativeConverter;
    static FORCEINLINE int NativeOptionsVersion()
    {
        return 1;
    }
    static FORCEINLINE EOS_EResult NativeFunction(
        NativeHandle Handle,
        const NativeOptions *Options,
        EOS_EpicAccountId LocalUserId,
        NativeResult **Result)
    {
        return EOS_Auth_CopyUserAuthToken(Handle, Options, LocalUserId, Result);
    }
    static FORCEINLINE void NativeReleaseFunction(NativeResult *Result)
    {
        EOS_Auth_Token_Release(Result);
    }
    static FORCEINLINE const TCHAR *StatCallName()
    {
        return TEXT("RedpointEOS/API/EOS_Auth_CopyUserAuthToken");
    }
    static FORCEINLINE const TCHAR *LogCallName()
    {
        return TEXT("EOS_Auth_CopyUserAuthToken");
    }

public:
    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_EpicAccountId> LocalUserId;
    };

    class Result
    {
    public:
        FString App;
        FString ClientId;
        EOS_EpicAccountId AccountId;
        FString AccessToken;
        double ExpiresIn;
        FString ExpiresAt;
        EOS_EAuthTokenType AuthType;
        FString RefreshToken;
        double RefreshExpiresIn;
        FString RefreshExpiresAt;
    };

public:
    static void Execute(
        const FPlatformHandle &InHandle,
        const Options &InOptions,
        EOS_EResult &OutResultCode,
        Result &OutResult);
    static void Execute(NativeHandle InHandle, const Options &InOptions, EOS_EResult &OutResultCode, Result &OutResult);

private:
    static void MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator);
    static void MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter);
};

}

REDPOINT_EOS_CODE_GUARD_END()