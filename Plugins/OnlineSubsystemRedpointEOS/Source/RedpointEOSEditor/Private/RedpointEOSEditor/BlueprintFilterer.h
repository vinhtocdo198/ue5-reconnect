// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Utils/HasEventRegistration.h"
#include "UObject/UnrealType.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3119426896, Redpoint::EOS::Editor)
{
using namespace ::Redpoint::EOS::Core::Utils;

class FBlueprintFilterer : public IHasEventRegistration, public TSharedFromThis<FBlueprintFilterer>
{
private:
    FDelegateHandle OnSettingChangedDelegateHandle;

public:
    FBlueprintFilterer() = default;
    virtual void RegisterEvents() override;
    UE_NONCOPYABLE(FBlueprintFilterer);
    virtual ~FBlueprintFilterer() override;

private:
    void OnSettingChanged(UObject *, FPropertyChangedEvent &Property);
    void SyncSubsystemNodeVisibility();
};

}

namespace Redpoint::EOS::Editor
{
REDPOINT_EOS_FILE_NS_EXPORT(3119426896, Redpoint::EOS::Editor, FBlueprintFilterer);
}

REDPOINT_EOS_CODE_GUARD_END()