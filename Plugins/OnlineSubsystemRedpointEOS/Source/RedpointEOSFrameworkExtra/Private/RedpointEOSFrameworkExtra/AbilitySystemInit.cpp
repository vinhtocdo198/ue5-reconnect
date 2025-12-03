// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSFrameworkExtra/AbilitySystemInit.h"

#include "LogRedpointEOSFrameworkExtra.h"
#include "RedpointAbilityWithInputIdInterface.h"
#include "RedpointAbilityWithLevelInterface.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3297874782, Redpoint::EOS::Framework::Extra)
{

void AbilitySystemInit(
    AActor *Owner,
    bool &bInOutAbilitySystemInitialized,
    UAbilitySystemComponent *AbilitySystemComponent,
    TArray<TSubclassOf<UGameplayAbility>> InitialCharacterAbilities,
    TSubclassOf<UGameplayEffect> InitialDefaultAttributes,
    FActiveGameplayEffectHandle &OutDefaultGameplayEffectHandle)
{
    // If we're already initialized, do nothing.
    if (bInOutAbilitySystemInitialized)
    {
        return;
    }

    // Go through our UPROPERTYs and construct attribute sets from the ability system.
    for (TFieldIterator<FObjectProperty> PropertyIt(Owner->GetClass(), EFieldIteratorFlags::IncludeSuper); PropertyIt;
         ++PropertyIt)
    {
        FObjectProperty const *ChildProperty = *PropertyIt;
        if (ChildProperty->PropertyClass->IsChildOf(UAttributeSet::StaticClass()))
        {
            UAttributeSet *ExistingAttributeSet =
                Cast<UAttributeSet>(ChildProperty->GetObjectPropertyValue_InContainer(Owner));
            if (!ExistingAttributeSet)
            {
                UE_LOG(
                    LogRedpointEOSFrameworkExtra,
                    Verbose,
                    TEXT("Automatically initializing attribute set of class '%s' for property '%s' on '%s'."),
                    *ChildProperty->PropertyClass->GetClassPathName().ToString(),
                    *ChildProperty->GetFullName(),
                    *Owner->GetClass()->GetClassPathName().ToString());
                ChildProperty->SetObjectPropertyValue_InContainer(
                    Owner,
                    const_cast<UAttributeSet *>(AbilitySystemComponent->GetAttributeSet(ChildProperty->PropertyClass)));
            }
        }
    }

    // Grant startup abilities on server.
    if (Owner->GetLocalRole() == ROLE_Authority)
    {
        for (auto StartupAbility : InitialCharacterAbilities)
        {
            UGameplayAbility *AbilityCDO = StartupAbility.GetDefaultObject();
            if (!IsValid(AbilityCDO))
            {
                continue;
            }

            int32 AbilityLevel = 0;
            int32 AbilityInputId = INDEX_NONE;
            if (AbilityCDO->Implements<URedpointAbilityWithLevelInterface>())
            {
                AbilityLevel = IRedpointAbilityWithLevelInterface::Execute_GetAbilityLevel(AbilityCDO, nullptr);
            }
            if (AbilityCDO->Implements<URedpointAbilityWithInputIdInterface>())
            {
                AbilityInputId = IRedpointAbilityWithInputIdInterface::Execute_GetAbilityInputId(AbilityCDO, nullptr);
            }

            AbilitySystemComponent->GiveAbility(
                FGameplayAbilitySpec(StartupAbility, AbilityLevel, AbilityInputId, Owner));
        }
    }

    // Apply default attributes at startup. Can run on server and client.
    if (InitialDefaultAttributes)
    {
        FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
        EffectContext.AddSourceObject(Owner);

        FGameplayEffectSpecHandle NewHandle =
            AbilitySystemComponent->MakeOutgoingSpec(InitialDefaultAttributes, 1, EffectContext);
        if (NewHandle.IsValid())
        {
            OutDefaultGameplayEffectHandle =
                AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*NewHandle.Data.Get());
        }
    }

    // We've done our one time initialization now.
    bInOutAbilitySystemInitialized = true;
}

}

REDPOINT_EOS_CODE_GUARD_END()