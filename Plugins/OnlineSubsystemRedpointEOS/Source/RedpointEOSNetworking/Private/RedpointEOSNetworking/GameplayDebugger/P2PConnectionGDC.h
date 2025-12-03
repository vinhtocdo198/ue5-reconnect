// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if WITH_GAMEPLAY_DEBUGGER && !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "Containers/Ticker.h"
#include "GameplayDebuggerCategory.h"
#include "RedpointEOSAPI/SDK.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_GAMEPLAY_DEBUGGER && !REDPOINT_EOS_IS_DEDICATED_SERVER

class APlayerController;
class AActor;

namespace REDPOINT_EOS_FILE_NS_ID(2834297394, Redpoint::EOS::Networking::GameplayDebugger)
{

class FP2PConnectionGameplayDebuggerCategory : public FGameplayDebuggerCategory
{
private:
    bool bHasRegisteredPeriodicNATCheck = false;
    bool bIsQueryingNAT = false;
    FTSTicker::FDelegateHandle NextNATCheck;
    EOS_ENATType QueriedNATType = EOS_ENATType::EOS_NAT_Unknown;

    bool PerformNATQuery(float DeltaSeconds, TWeakObjectPtr<UWorld> World);

public:
    FP2PConnectionGameplayDebuggerCategory();
    UE_NONCOPYABLE(FP2PConnectionGameplayDebuggerCategory);
    virtual ~FP2PConnectionGameplayDebuggerCategory() override = default;

    void CollectData(APlayerController *OwnerPC, AActor *DebugActor) override;
    void DrawData(APlayerController *OwnerPC, FGameplayDebuggerCanvasContext &CanvasContext) override;

    static TSharedRef<FGameplayDebuggerCategory> MakeInstance();

protected:
    struct FRepData
    {
        void Serialize(FArchive &Ar);
    };

    FRepData DataPack;
};

}

namespace Redpoint::EOS::Networking::GameplayDebugger
{
REDPOINT_EOS_FILE_NS_EXPORT(
    2834297394,
    Redpoint::EOS::Networking::GameplayDebugger,
    FP2PConnectionGameplayDebuggerCategory)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()