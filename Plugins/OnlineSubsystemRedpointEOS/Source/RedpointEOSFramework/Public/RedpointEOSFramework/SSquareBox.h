// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointSquareBoxAxis.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Layout/SBox.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2129440658, Redpoint::EOS::Framework)
{

class REDPOINTEOSFRAMEWORK_API SSquareBox : public SPanel
{
    SLATE_DECLARE_WIDGET(SSquareBox, SPanel)

public:
    SLATE_BEGIN_ARGS(SSquareBox)
        : _Content()
        , _Axis(ERedpointSquareBoxAxis::Horizontal)
    {
    }

    /**
     * The slot that contains the child content for this box.
     */
    SLATE_DEFAULT_SLOT(FArguments, Content)

    /**
     * The axis which defines the size of the square box. This should typically be the axis that is set to "Fill" by the
     * parent. The other axis will always match the size of the axis selected here.
     */
    SLATE_ATTRIBUTE(ERedpointSquareBoxAxis, Axis)

    SLATE_END_ARGS()

    SSquareBox();
    virtual ~SSquareBox() override;

    void Construct(const FArguments &InArgs);

    /**
     * Set the child content for this box.
     */
    void SetContent(const TSharedRef<SWidget> &InContent);

    /**
     * Set the axis which defines the size of the square box. This should typically be the axis that is set to "Fill" by
     * the parent. The other axis will always match the size of the axis selected here.
     */
    void SetAxis(const TAttribute<ERedpointSquareBoxAxis> &InAxis);

    virtual void OnArrangeChildren(const FGeometry &AllottedGeometry, FArrangedChildren &ArrangedChildren)
        const override;
    virtual FChildren *GetChildren() override;
    virtual int32 OnPaint(
        const FPaintArgs &Args,
        const FGeometry &AllottedGeometry,
        const FSlateRect &MyCullingRect,
        FSlateWindowElementList &OutDrawElements,
        int32 LayerId,
        const FWidgetStyle &InWidgetStyle,
        bool bParentEnabled) const override;

protected:
    virtual FVector2D ComputeDesiredSize(float) const override;

    struct FBoxOneChildSlot : ::TSingleWidgetChildrenWithBasicLayoutSlot<EInvalidateWidgetReason::None>
    {
        friend SSquareBox;
        using ::TSingleWidgetChildrenWithBasicLayoutSlot<
            EInvalidateWidgetReason::None>::TSingleWidgetChildrenWithBasicLayoutSlot;
    };
    FBoxOneChildSlot ChildSlot;

private:
    TAttribute<ERedpointSquareBoxAxis> Axis;
    FGeometry LastAllottedGeometry;
};

}

namespace Redpoint::EOS::Framework
{
REDPOINT_EOS_FILE_NS_EXPORT(2129440658, Redpoint::EOS::Framework, SSquareBox)
}

REDPOINT_EOS_CODE_GUARD_END()