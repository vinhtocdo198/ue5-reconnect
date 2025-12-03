// Copyright June Rhodes. All Rights Reserved.

#include "RedpointSquareBoxSlot.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

URedpointSquareBoxSlot::URedpointSquareBoxSlot(const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void URedpointSquareBoxSlot::ReleaseSlateResources(bool bReleaseChildren)
{
    Super::ReleaseSlateResources(bReleaseChildren);

    SizeBox.Reset();
}

void URedpointSquareBoxSlot::BuildSlot(const TSharedRef<::Redpoint::EOS::Framework::SSquareBox> &InSizeBox)
{
    SizeBox = InSizeBox;

    SynchronizeProperties();

    SizeBox.Pin()->SetContent(Content ? Content->TakeWidget() : SNullWidget::NullWidget);
}

void URedpointSquareBoxSlot::SynchronizeProperties()
{
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()