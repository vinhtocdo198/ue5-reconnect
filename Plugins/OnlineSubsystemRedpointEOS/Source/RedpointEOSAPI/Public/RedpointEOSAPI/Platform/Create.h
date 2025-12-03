// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSAPI/Templates/Util/ApiCallNativeAllocator.h"
#include "RedpointEOSAPI/Templates/Util/ApiCallNativeConverter.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Platform
{

class REDPOINTEOSAPI_API FCreate
{
private:
    typedef EOS_Platform_Options NativeOptions;
    typedef ::Redpoint::EOS::API::Templates::Util::FApiCallNativeAllocator NativeAllocator;
    typedef ::Redpoint::EOS::API::Templates::Util::FApiCallNativeConverter NativeConverter;
    static FORCEINLINE int NativeOptionsVersion()
    {
        return EOS_PLATFORM_OPTIONS_API_LATEST;
    }
    static FORCEINLINE const TCHAR *StatCallName()
    {
        return TEXT("RedpointEOS/API/EOS_") TEXT(PREPROCESSOR_TO_STRING(Platform)) TEXT("_")
            TEXT(PREPROCESSOR_TO_STRING(Create));
    }
    static FORCEINLINE const TCHAR *LogCallName()
    {
        return TEXT(PREPROCESSOR_TO_STRING(EOS_Platform_Create));
    }

public:
    class Options
    {
    public:
        ParamHelpers::TRequired<bool> bIsServer;
        ParamHelpers::TRequired<FString> EncryptionKey;
        ParamHelpers::TRequired<FString> CacheDirectory;
        ParamHelpers::TRequired<FString> OverrideCountryCode;
        ParamHelpers::TRequired<FString> OverrideLocaleCode;
        ParamHelpers::TRequired<uint64_t> Flags;
        ParamHelpers::TRequired<FString> ProductId;
        ParamHelpers::TRequired<FString> SandboxId;
        ParamHelpers::TRequired<FString> DeploymentId;
        ParamHelpers::TRequired<FString> ClientId;
        ParamHelpers::TRequired<FString> ClientSecret;
        ParamHelpers::TRequired<EOS_Platform_RTCOptions *> RTCOptions;
        ParamHelpers::TRequired<uint32> TickBudgetInMilliseconds;
        ParamHelpers::TRequired<void *> ReservedOptions;
    };

    typedef EOS_HPlatform Result;

public:
    static Result Execute(const Options &InOptions);

private:
    static void MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator);
};

}

REDPOINT_EOS_CODE_GUARD_END()