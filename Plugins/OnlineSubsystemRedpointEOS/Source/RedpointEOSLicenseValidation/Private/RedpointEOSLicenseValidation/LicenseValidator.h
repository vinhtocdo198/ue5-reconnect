// Copyright June Rhodes. All Rights Reserved.

#pragma once

#if REDPOINT_EOS_IS_FREE_EDITION

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Utils/ExclusiveAsyncOperationQueue.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(213993558, Redpoint::EOS::LicenseValidation)
{
using namespace ::Redpoint::EOS::Core::Utils;

typedef TDelegate<void(bool bIsLicenseValid)> FOnLicenseValidatorComplete;

class FLicenseValidator
{
private:
    static TSharedRef<FExclusiveAsyncOperationQueue> OperationQueue;
    static bool bHasChecked;
    static bool bHasValidLicense;
    static FString LastCheckedLicenseKey;

    static void OnExecuteOperation(
        const FExclusiveAsyncOperationQueue::FOnDone &InOnDone,
        FString InLicenseKey,
        bool bInPerformEditorAgreementCheck,
        FOnLicenseValidatorComplete InOnLicenseValidatorComplete);

public:
    static void ValidateLicenseKey(
        const FString &InLicenseKey,
        bool bInPerformEditorAgreementCheck,
        const FOnLicenseValidatorComplete &InOnLicenseValidatorComplete);
};

}

namespace Redpoint::EOS::LicenseValidation
{
REDPOINT_EOS_FILE_NS_EXPORT(213993558, Redpoint::EOS::LicenseValidation, FOnLicenseValidatorComplete)
REDPOINT_EOS_FILE_NS_EXPORT(213993558, Redpoint::EOS::LicenseValidation, FLicenseValidator)
}

REDPOINT_EOS_CODE_GUARD_END()

#endif