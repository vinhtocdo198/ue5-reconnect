// Copyright June Rhodes. All Rights Reserved.

#include "RedpointStarterInputMappingContext.h"

REDPOINT_EOS_FLOAT_CONVERSION_PERMIT_BEGIN()

#include "InputModifiers.h"
#include "InputTriggers.h"
#include "RedpointStarterJumpAction.h"
#include "RedpointStarterMoveAction.h"

URedpointStarterInputMappingContext::URedpointStarterInputMappingContext()
{
    {
        FEnhancedActionKeyMapping Mapping;
        Mapping.Action = GetDefault<URedpointStarterJumpAction>();
        Mapping.Key = EKeys::SpaceBar;
        Mapping.Triggers.Add(GetMutableDefault<UInputTriggerPressed>());
        this->Mappings.Add(Mapping);
    }
    {
        FEnhancedActionKeyMapping Mapping;
        Mapping.Action = GetDefault<URedpointStarterMoveAction>();
        Mapping.Key = EKeys::W;
        Mapping.Triggers.Add(GetMutableDefault<UInputTriggerDown>());
        this->Mappings.Add(Mapping);
    }
    {
        FEnhancedActionKeyMapping Mapping;
        Mapping.Action = GetDefault<URedpointStarterMoveAction>();
        Mapping.Key = EKeys::S;
        Mapping.Triggers.Add(GetMutableDefault<UInputTriggerDown>());
        Mapping.Modifiers.Add(GetMutableDefault<UInputModifierNegate>());
        this->Mappings.Add(Mapping);
    }
    {
        FEnhancedActionKeyMapping Mapping;
        Mapping.Action = GetDefault<URedpointStarterMoveAction>();
        Mapping.Key = EKeys::A;
        Mapping.Triggers.Add(GetMutableDefault<UInputTriggerDown>());
        Mapping.Modifiers.Add(GetMutableDefault<UInputModifierSwizzleAxis>());
        Mapping.Modifiers.Add(GetMutableDefault<UInputModifierNegate>());
        this->Mappings.Add(Mapping);
    }
    {
        FEnhancedActionKeyMapping Mapping;
        Mapping.Action = GetDefault<URedpointStarterMoveAction>();
        Mapping.Key = EKeys::D;
        Mapping.Triggers.Add(GetMutableDefault<UInputTriggerDown>());
        Mapping.Modifiers.Add(GetMutableDefault<UInputModifierSwizzleAxis>());
        this->Mappings.Add(Mapping);
    }
}

REDPOINT_EOS_FLOAT_CONVERSION_PERMIT_END()