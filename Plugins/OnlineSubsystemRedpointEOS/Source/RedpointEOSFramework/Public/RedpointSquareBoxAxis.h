// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointSquareBoxAxis.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UENUM(BlueprintType, Category = "Redpoint EOS Online Framework")
enum class ERedpointSquareBoxAxis : uint8
{
    /**
     * The width and height of the widget will be based on the width that the parent is providing for this widget.
     */
    Horizontal,

    /**
     * The width and height of the widget will be based on the height that the parent is providing for this widget.
     */
    Vertical,
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()