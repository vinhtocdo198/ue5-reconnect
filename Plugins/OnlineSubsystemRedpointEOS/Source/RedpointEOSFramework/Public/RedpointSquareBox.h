// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Components/ContentWidget.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSFramework/SSquareBox.h"
#include "RedpointSquareBoxAxis.h"

#include "RedpointSquareBox.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

DECLARE_DYNAMIC_DELEGATE_RetVal(ERedpointSquareBoxAxis, FGetRedpointSquareBoxAxis);

UCLASS(BlueprintType, Category = "Redpoint EOS Online Framework")
class REDPOINTEOSFRAMEWORK_API URedpointSquareBox : public UContentWidget
{
    GENERATED_BODY()

protected:
    TSharedPtr<::Redpoint::EOS::Framework::SSquareBox> MyBox;

public:
    URedpointSquareBox(const FObjectInitializer &ObjectInitializer);

    /**
     * The axis which defines the size of the square box. This should typically be the axis that is set to "Fill" by the
     * parent. The other axis will always match the size of the axis selected here.
     */
    UPROPERTY(EditAnywhere, Category = "Redpoint EOS Online Framework")
    ERedpointSquareBoxAxis Axis;

    UPROPERTY()
    FGetRedpointSquareBoxAxis AxisDelegate;

    virtual void SynchronizeProperties() override;
    virtual void ReleaseSlateResources(bool bReleaseChildren) override;

#if WITH_EDITOR
    virtual const FText GetPaletteCategory() override;
#endif

protected:
    virtual UClass *GetSlotClass() const override;
    virtual void OnSlotAdded(UPanelSlot *InSlot) override;
    virtual void OnSlotRemoved(UPanelSlot *InSlot) override;

    virtual TSharedRef<SWidget> RebuildWidget() override;

    PROPERTY_BINDING_IMPLEMENTATION(ERedpointSquareBoxAxis, Axis);
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()