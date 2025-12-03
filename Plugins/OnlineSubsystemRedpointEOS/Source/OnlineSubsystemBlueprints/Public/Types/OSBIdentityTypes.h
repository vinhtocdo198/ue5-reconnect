// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineIdentityInterface.h"

#include "OSBIdentityTypes.generated.h"

/** Whether to show UI to resolve the privilege if there is no access when GetUserPrivilege */
UENUM(BlueprintType, Meta = (ScriptName = "EShowPrivilegeResolveUI"))
enum class EShowPrivilegeResolveUI_ : uint8
{
    /** Use default behavior, which preserved the behaviour prior to the introduction of this. */
    Default,
    /** Show UI to resolve privilege */
    Show,
    /** Not show UI to resolve privilege */
    NotShow
};
