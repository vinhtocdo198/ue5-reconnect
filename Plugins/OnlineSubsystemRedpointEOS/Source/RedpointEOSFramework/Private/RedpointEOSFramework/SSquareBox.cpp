// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSFramework/SSquareBox.h"

#include "Layout/ArrangedChildren.h"
#include "Layout/LayoutUtils.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2129440658, Redpoint::EOS::Framework)
{

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-variable-declarations"
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#endif

SLATE_IMPLEMENT_WIDGET(SSquareBox)

void SSquareBox::PrivateRegisterAttributes(FSlateAttributeInitializer &AttributeInitializer)
{
    SLATE_ADD_MEMBER_ATTRIBUTE_DEFINITION_WITH_NAME(
        AttributeInitializer,
        "SlotPadding",
        ChildSlot.SlotPaddingAttribute,
        EInvalidateWidgetReason::Layout);
}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

SSquareBox::SSquareBox()
    : ChildSlot(this)
{
    SetCanTick(false);
    bCanSupportFocus = false;
}

SSquareBox::~SSquareBox() = default;

void SSquareBox::Construct(const FArguments &InArgs)
{
    SetAxis(InArgs._Axis);

    ChildSlot.HAlign(EHorizontalAlignment::HAlign_Center)
        .VAlign(EVerticalAlignment::VAlign_Center)[InArgs._Content.Widget];
}

void SSquareBox::SetContent(const TSharedRef<SWidget> &InContent)
{
    ChildSlot[InContent];
}

void SSquareBox::SetAxis(const TAttribute<ERedpointSquareBoxAxis> &InAxis)
{
    SetAttribute(Axis, InAxis, EInvalidateWidgetReason::Layout);
}

void SSquareBox::OnArrangeChildren(const FGeometry &AllottedGeometry, FArrangedChildren &ArrangedChildren) const
{
    const EVisibility ChildVisibility = ChildSlot.GetWidget()->GetVisibility();
    if (ArrangedChildren.Accepts(ChildVisibility))
    {
        const FMargin SlotPadding(ChildSlot.GetPadding());

        AlignmentArrangeResult XAlignmentResult = AlignChild<Orient_Horizontal>(
            static_cast<float>(AllottedGeometry.GetLocalSize().X),
            ChildSlot,
            SlotPadding);
        AlignmentArrangeResult YAlignmentResult =
            AlignChild<Orient_Vertical>(static_cast<float>(AllottedGeometry.GetLocalSize().Y), ChildSlot, SlotPadding);

        ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(
            ChildSlot.GetWidget(),
            FVector2f(XAlignmentResult.Offset, YAlignmentResult.Offset),
            FVector2f(XAlignmentResult.Size, YAlignmentResult.Size)));
    }
}

FChildren *SSquareBox::GetChildren()
{
    return &ChildSlot;
}

int32 SSquareBox::OnPaint(
    const FPaintArgs &Args,
    const FGeometry &AllottedGeometry,
    const FSlateRect &MyCullingRect,
    FSlateWindowElementList &OutDrawElements,
    int32 LayerId,
    const FWidgetStyle &InWidgetStyle,
    bool bParentEnabled) const
{
    auto &LastAllottedGeometryRef = const_cast<SSquareBox *>(this)->LastAllottedGeometry;
    if (LastAllottedGeometryRef.GetLocalSize() != AllottedGeometry.GetLocalSize())
    {
        LastAllottedGeometryRef = AllottedGeometry;
        const_cast<SSquareBox *>(this)->Invalidate(EInvalidateWidgetReason::Prepass);
    }

    FArrangedChildren ArrangedChildren(EVisibility::Visible);
    this->ArrangeChildren(AllottedGeometry, ArrangedChildren);

    if (ArrangedChildren.Num() > 0)
    {
        check(ArrangedChildren.Num() == 1);
        FArrangedWidget &TheChild = ArrangedChildren[0];

        return TheChild.Widget->Paint(
            Args.WithNewParent(this),
            TheChild.Geometry,
            MyCullingRect,
            OutDrawElements,
            LayerId,
            InWidgetStyle,
            ShouldBeEnabled(bParentEnabled));
    }

    return LayerId;
}

FVector2D SSquareBox::ComputeDesiredSize(float) const
{
    EVisibility ChildVisibility = ChildSlot.GetWidget()->GetVisibility();

    if (ChildVisibility != EVisibility::Collapsed)
    {
        auto Vector = ChildSlot.GetWidget()->GetDesiredSize() + ChildSlot.GetPadding().GetDesiredSize();
        if (this->Axis.Get() == ERedpointSquareBoxAxis::Horizontal)
        {
            return FVector2D(
                static_cast<double>(Vector.X),
                static_cast<double>(this->LastAllottedGeometry.GetLocalSize().X));
        }
        else
        {
            return FVector2D(
                static_cast<double>(this->LastAllottedGeometry.GetLocalSize().Y),
                static_cast<double>(Vector.Y));
        }
    }

    return FVector2D::ZeroVector;
}

}

REDPOINT_EOS_CODE_GUARD_END()