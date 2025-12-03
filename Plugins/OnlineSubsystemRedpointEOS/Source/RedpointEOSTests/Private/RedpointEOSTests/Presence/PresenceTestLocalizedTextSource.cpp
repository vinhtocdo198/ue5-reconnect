// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/Presence/PresenceTestLocalizedTextSource.h"

#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/TextLocalizationResource.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1012675700, Redpoint::EOS::Tests::Presence)
{

bool FPresenceTestLocalizedTextSource::bRegistered = false;

int32 FPresenceTestLocalizedTextSource::GetPriority() const
{
    return ELocalizedTextSourcePriority::Lowest;
}

bool FPresenceTestLocalizedTextSource::GetNativeCultureName(
    const ELocalizedTextSourceCategory InCategory,
    FString &OutNativeCultureName)
{
    OutNativeCultureName = FInternationalization::Get().GetCurrentCulture()->GetName();
    return true;
}

void FPresenceTestLocalizedTextSource::GetLocalizedCultureNames(
    const ELocalizationLoadFlags InLoadFlags,
    TSet<FString> &OutLocalizedCultureNames)
{
    auto CultureName = FInternationalization::Get().GetCurrentCulture()->GetName();
    OutLocalizedCultureNames.Add(CultureName);
}

void FPresenceTestLocalizedTextSource::LoadLocalizedResources(
    const ELocalizationLoadFlags InLoadFlags,
    TArrayView<const FString> InPrioritizedCultures,
    FTextLocalizationResource &InOutNativeResource,
    FTextLocalizationResource &InOutLocalizedResource)
{
    FTextLocalizationResource Resource;
    Resource.AddEntry(
        FTextKey("RedpointEOSTests"),
        FTextKey("StatusTextTest"),
        TEXT("Status number is {Number}"),
        TEXT("Status number is {Number}"),
        0);
    InOutNativeResource = Resource;
    InOutLocalizedResource = Resource;
}

EQueryLocalizedResourceResult FPresenceTestLocalizedTextSource::QueryLocalizedResource(
    const ELocalizationLoadFlags InLoadFlags,
    TArrayView<const FString> InPrioritizedCultures,
    const FTextId InTextId,
    FTextLocalizationResource &InOutNativeResource,
    FTextLocalizationResource &InOutLocalizedResource)
{
    return EQueryLocalizedResourceResult::NotImplemented;
}

}

REDPOINT_EOS_CODE_GUARD_END()