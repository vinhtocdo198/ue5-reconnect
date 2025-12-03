// Copyright June Rhodes. All Rights Reserved.

#include "RedpointSquareBox.h"

#include "RedpointSquareBoxSlot.h"
#include "Widgets/SNullWidget.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

#if WITH_EDITOR
#define LOCTEXT_NAMESPACE "UMG"
#endif

URedpointSquareBox::URedpointSquareBox(const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer)
    , MyBox()
    , Axis(ERedpointSquareBoxAxis::Horizontal)
    , AxisDelegate()
{
    this->bIsVariable = false;
    this->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void URedpointSquareBox::SynchronizeProperties()
{
    Super::SynchronizeProperties();

    TAttribute<ERedpointSquareBoxAxis> AxisBinding = PROPERTY_BINDING(ERedpointSquareBoxAxis, Axis);

    this->MyBox->SetAxis(AxisBinding);
}

void URedpointSquareBox::ReleaseSlateResources(bool bReleaseChildren)
{
    Super::ReleaseSlateResources(bReleaseChildren);

    this->MyBox.Reset();
}

#if WITH_EDITOR

const FText URedpointSquareBox::GetPaletteCategory()
{
    return LOCTEXT("Panel", "Panel");
}

#endif

UClass *URedpointSquareBox::GetSlotClass() const
{
    return URedpointSquareBoxSlot::StaticClass();
}

void URedpointSquareBox::OnSlotAdded(UPanelSlot *InSlot)
{
    if (this->MyBox.IsValid())
    {
        CastChecked<URedpointSquareBoxSlot>(InSlot)->BuildSlot(this->MyBox.ToSharedRef());
    }
}

void URedpointSquareBox::OnSlotRemoved(UPanelSlot *InSlot)
{
    if (this->MyBox.IsValid())
    {
        this->MyBox->SetContent(SNullWidget::NullWidget);
    }
}

TSharedRef<SWidget> URedpointSquareBox::RebuildWidget()
{
    this->MyBox = SNew(::Redpoint::EOS::Framework::SSquareBox);

    if (this->GetChildrenCount() > 0)
    {
        Cast<URedpointSquareBoxSlot>(this->GetContentSlot())->BuildSlot(this->MyBox.ToSharedRef());
    }

    return this->MyBox.ToSharedRef();
}

#if WITH_EDITOR
#undef LOCTEXT_NAMESPACE
#endif

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()