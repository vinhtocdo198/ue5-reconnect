// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Components/PanelSlot.h"
#include "Layout/Margin.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointSquareBox.h"

#include "RedpointSquareBoxSlot.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS(BlueprintType, Category = "Redpoint EOS Online Framework")
class REDPOINTEOSFRAMEWORK_API URedpointSquareBoxSlot : public UPanelSlot
{
    GENERATED_BODY()

public:
    URedpointSquareBoxSlot(const FObjectInitializer &ObjectInitializer);

private:
    /**
     * A pointer to the button to allow us to adjust the size, padding...etc at runtime.
     */
    TWeakPtr<::Redpoint::EOS::Framework::SSquareBox> SizeBox;

public:
    virtual void SynchronizeProperties() override;

    /** Builds the underlying slot for the slate SizeBox. */
    void BuildSlot(const TSharedRef<::Redpoint::EOS::Framework::SSquareBox> &InSizeBox);

    virtual void ReleaseSlateResources(bool bReleaseChildren) override;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()