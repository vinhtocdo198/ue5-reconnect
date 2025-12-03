// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Internationalization/ILocalizedTextSource.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1012675700, Redpoint::EOS::Tests::Presence)
{

class FPresenceTestLocalizedTextSource : public ILocalizedTextSource
{
public:
    FPresenceTestLocalizedTextSource() = default;
    UE_NONCOPYABLE(FPresenceTestLocalizedTextSource);
    virtual ~FPresenceTestLocalizedTextSource() override = default;

    static bool bRegistered;

    virtual int32 GetPriority() const override;
    virtual bool GetNativeCultureName(const ELocalizedTextSourceCategory InCategory, FString &OutNativeCultureName)
        override;
    virtual void GetLocalizedCultureNames(
        const ELocalizationLoadFlags InLoadFlags,
        TSet<FString> &OutLocalizedCultureNames) override;
    virtual void LoadLocalizedResources(
        const ELocalizationLoadFlags InLoadFlags,
        TArrayView<const FString> InPrioritizedCultures,
        FTextLocalizationResource &InOutNativeResource,
        FTextLocalizationResource &InOutLocalizedResource) override;
    virtual EQueryLocalizedResourceResult QueryLocalizedResource(
        const ELocalizationLoadFlags InLoadFlags,
        TArrayView<const FString> InPrioritizedCultures,
        const FTextId InTextId,
        FTextLocalizationResource &InOutNativeResource,
        FTextLocalizationResource &InOutLocalizedResource) override;
};

}

namespace Redpoint::EOS::Tests::Presence
{
REDPOINT_EOS_FILE_NS_EXPORT(1012675700, Redpoint::EOS::Tests::Presence, FPresenceTestLocalizedTextSource)
}

REDPOINT_EOS_CODE_GUARD_END()