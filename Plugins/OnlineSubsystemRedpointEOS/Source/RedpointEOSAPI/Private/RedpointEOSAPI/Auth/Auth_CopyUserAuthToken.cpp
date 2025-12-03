// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Auth/CopyUserAuthToken.h"

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Logging.h"
#include "RedpointEOSAPI/Stats.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Auth
{

void FCopyUserAuthToken::Execute(
    const FPlatformHandle &InHandle,
    const Options &InOptions,
    EOS_EResult &OutResultCode,
    Result &OutResult)
{
    Execute(InHandle->Get<NativeHandle>(), InOptions, OutResultCode, OutResult);
}

void FCopyUserAuthToken::Execute(
    NativeHandle InHandle,
    const Options &InOptions,
    EOS_EResult &OutResultCode,
    Result &OutResult)
{
    checkf(IsInGameThread(), TEXT("Calls to the EOS SDK can only be made on the game thread!"));
    
    OutResultCode = EOS_EResult::EOS_UnexpectedError;
    OutResult = {};

    DECLARE_CYCLE_STAT(StatCallName(), STAT_Call, STATGROUP_RedpointEOS);
    SCOPE_CYCLE_COUNTER(STAT_Call);

    UE_LOG(LogRedpointEOSAPI, VeryVerbose, TEXT("[starting] %s"), LogCallName());

    if (InHandle == nullptr)
    {
        UE_LOG(
            LogRedpointEOSAPI,
            VeryVerbose,
            TEXT("[complete] %s"),
            *ConvertError(
                 LogCallName(),
                 TEXT("The platform instance has been shutdown, so SDK calls can not be made."),
                 EOS_EResult::EOS_NoConnection)
                 .ToLogString());
        OutResultCode = EOS_EResult::EOS_NoConnection;
        OutResult = {};
        return;
    }
    TSharedRef<NativeAllocator> Allocator = MakeShared<NativeAllocator>();
    TSharedRef<NativeConverter> Converter = MakeShared<NativeConverter>();
    NativeOptions _NativeOptions = {};
    _NativeOptions.ApiVersion = NativeOptionsVersion();
    MapOptions(_NativeOptions, InOptions, *Allocator);
    NativeResult *_NativeResult = nullptr;
    OutResultCode = NativeFunction(InHandle, &_NativeOptions, *InOptions.LocalUserId, &_NativeResult);
    if (_NativeResult != nullptr)
    {
        MapResult(OutResult, *_NativeResult, *Converter);
    }
    UE_LOG(
        LogRedpointEOSAPI,
        VeryVerbose,
        TEXT("[complete] %s"),
        *ConvertError(LogCallName(), TEXT("Sync copy call completed."), OutResultCode).ToLogString());
    return;
};

void FCopyUserAuthToken::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
}

void FCopyUserAuthToken::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.App = Converter.FromUtf8(NativeResult.App);
    Result.ClientId = Converter.FromUtf8(NativeResult.ClientId);
    Result.AccountId = NativeResult.AccountId;
    Result.AccessToken = Converter.FromUtf8(NativeResult.AccessToken);
    Result.ExpiresIn = NativeResult.ExpiresIn;
    Result.ExpiresAt = Converter.FromUtf8(NativeResult.ExpiresAt);
    Result.AuthType = NativeResult.AuthType;
    Result.RefreshToken = Converter.FromUtf8(NativeResult.RefreshToken);
    Result.RefreshExpiresIn = NativeResult.RefreshExpiresIn;
    Result.RefreshExpiresAt = Converter.FromUtf8(NativeResult.RefreshExpiresAt);
}

}

REDPOINT_EOS_CODE_GUARD_END()