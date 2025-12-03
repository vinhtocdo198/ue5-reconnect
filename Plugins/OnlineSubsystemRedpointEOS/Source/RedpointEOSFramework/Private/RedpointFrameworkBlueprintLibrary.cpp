// Copyright June Rhodes. All Rights Reserved.

#include "RedpointFrameworkBlueprintLibrary.h"

#include "LogRedpointEOSFramework.h"
#include "RedpointEOSCore/Utils/AccessPrivate.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

struct FRedpointListViewHorizontalEntrySpacingAccessor
{
    using Member = float UListView::*;
    friend Member GetPrivate(FRedpointListViewHorizontalEntrySpacingAccessor);
};
struct FRedpointListViewVerticalEntrySpacingAccessor
{
    using Member = float UListView::*;
    friend Member GetPrivate(FRedpointListViewVerticalEntrySpacingAccessor);
};
struct FRedpointListViewMyListViewAccessor
{
    using Member = TSharedPtr<SListView<UObject *>> UListView::*;
    friend Member GetPrivate(FRedpointListViewMyListViewAccessor);
};
template struct TRedpointAccessPrivate<
    FRedpointListViewHorizontalEntrySpacingAccessor,
    &UListView::HorizontalEntrySpacing>;
template struct TRedpointAccessPrivate<FRedpointListViewVerticalEntrySpacingAccessor, &UListView::VerticalEntrySpacing>;
template struct TRedpointAccessPrivate<FRedpointListViewMyListViewAccessor, &UListView::MyListView>;

void URedpointFrameworkBlueprintLibrary::SetListViewEntrySpacing(UListView *InListView, float InEntrySpacing)
{
    if (!IsValid(InListView))
    {
        UE_LOG(
            LogRedpointEOSFramework,
            Warning,
            TEXT("Spacing can not be set for list view, as the list view is not valid."));
        return;
    }

    ((*InListView).*GetPrivate(FRedpointListViewHorizontalEntrySpacingAccessor())) = InEntrySpacing;
    ((*InListView).*GetPrivate(FRedpointListViewVerticalEntrySpacingAccessor())) = InEntrySpacing;
}

bool URedpointFrameworkBlueprintLibrary::IsRedpointFrameworkFreeEdition()
{
#if REDPOINT_EOS_IS_FREE_EDITION
    return true;
#else
    return false;
#endif
}

bool URedpointFrameworkBlueprintLibrary::IsPlayerControllerSpectating(APlayerController *PlayerController)
{
    if (!IsValid(PlayerController))
    {
        UE_LOG(
            LogRedpointEOSFramework,
            Error,
            TEXT("Player controller passed to 'Is Player Controller Spectating?' was not valid."));
        return false;
    }

    return PlayerController->GetStateName().IsEqual(NAME_Spectating);
}

void URedpointFrameworkBlueprintLibrary::SetPlayerControllerSpectating(
    APlayerController *PlayerController,
    bool bIsSpectating)
{
    if (!IsValid(PlayerController))
    {
        UE_LOG(
            LogRedpointEOSFramework,
            Error,
            TEXT("Player controller passed to 'Set Player Controller Spectating' was not valid."));
        return;
    }

    return PlayerController->ChangeState(bIsSpectating ? NAME_Spectating : NAME_Playing);
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()