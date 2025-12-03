// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Interfaces/OnlinePurchaseInterface.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Purchase
{

#if PLATFORM_ANDROID

/**
 * @note: Do not use this interface in game code. It only exists to support certain automation
 * tests that need to know when FinalizePurchase completes asynchronously. There is absolutely
 * no support for using this interface.
 */
class ONLINESUBSYSTEMREDPOINTEOS_API IOnlinePurchaseWithFinalizePurchaseCallback : public IOnlinePurchase
{
public:
    DECLARE_DELEGATE_OneParam(FOnFinalizeReceiptComplete, const FOnlineError &);

    virtual void AcknowledgePurchaseWithCallback(
        const FUniqueNetId &UserId,
        const FString &ReceiptId,
        const IOnlinePurchaseWithFinalizePurchaseCallback::FOnFinalizeReceiptComplete &Delegate) = 0;
    virtual void ConsumePurchaseWithCallback(
        const FUniqueNetId &UserId,
        const FString &ReceiptId,
        const IOnlinePurchaseWithFinalizePurchaseCallback::FOnFinalizeReceiptComplete &Delegate) = 0;
};

#endif // #if PLATFORM_ANDROID

}

REDPOINT_EOS_CODE_GUARD_END()
