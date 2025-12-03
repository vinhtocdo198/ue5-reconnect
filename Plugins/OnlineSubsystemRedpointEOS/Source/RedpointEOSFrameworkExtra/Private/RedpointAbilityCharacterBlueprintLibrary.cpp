// Copyright June Rhodes. All Rights Reserved.

#include "RedpointAbilityCharacterBlueprintLibrary.h"

#include "AbilitySystemComponent.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

void URedpointAbilityCharacterBlueprintLibrary::GiveAbilityFromClass(
    ARedpointAbilityBaseCharacter *Character,
    TSubclassOf<UGameplayAbility> AbilityClass,
    int32 Level,
    int32 InputIndex,
    FGameplayAbilitySpecHandle &Handle,
    bool &Success)
{
    if (IsValid(Character) && Character->HasAuthority() && AbilityClass)
    {
        Handle = Character->GetAbilitySystemComponent()->GiveAbility(
            FGameplayAbilitySpec(AbilityClass.GetDefaultObject(), Level, (int32)InputIndex));
        Success = true;
    }
    else
    {
        Handle = FGameplayAbilitySpecHandle();
        Success = false;
    }
}

void URedpointAbilityCharacterBlueprintLibrary::ClearAbilityFromClass(
    ARedpointAbilityBaseCharacter *Character,
    const FGameplayAbilitySpecHandle &Handle,
    bool &Success)
{
    if (IsValid(Character) && Character->HasAuthority())
    {
        Character->GetAbilitySystemComponent()->ClearAbility(Handle);
        Success = true;
    }
    else
    {
        Success = false;
    }
}

void URedpointAbilityCharacterBlueprintLibrary::ClearAllAbilities(ARedpointAbilityBaseCharacter *Character)
{
    if (IsValid(Character) && Character->HasAuthority())
    {
        Character->GetAbilitySystemComponent()->ClearAllAbilities();
    }
}

UGameplayAbility *URedpointAbilityCharacterBlueprintLibrary::GetPrimaryAbilityInstanceFromHandle(
    UAbilitySystemComponent *AbilitySystemComponent,
    FGameplayAbilitySpecHandle Handle)
{
    if (AbilitySystemComponent)
    {
        FGameplayAbilitySpec *AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
        if (AbilitySpec)
        {
            return AbilitySpec->GetPrimaryInstance();
        }
    }

    return nullptr;
}

UGameplayAbility *URedpointAbilityCharacterBlueprintLibrary::GetPrimaryAbilityInstanceFromClass(
    UAbilitySystemComponent *AbilitySystemComponent,
    TSubclassOf<UGameplayAbility> InAbilityClass)
{
    if (AbilitySystemComponent)
    {
        FGameplayAbilitySpec *AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromClass(InAbilityClass);
        if (AbilitySpec)
        {
            return AbilitySpec->GetPrimaryInstance();
        }
    }

    return nullptr;
}

bool URedpointAbilityCharacterBlueprintLibrary::IsPrimaryAbilityInstanceActive(
    UAbilitySystemComponent *AbilitySystemComponent,
    FGameplayAbilitySpecHandle Handle)
{
    if (AbilitySystemComponent)
    {
        FGameplayAbilitySpec *AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
        if (AbilitySpec)
        {
            return AbilitySpec->GetPrimaryInstance()->IsActive();
        }
    }

    return false;
}

bool URedpointAbilityCharacterBlueprintLibrary::IsAbilitySpecHandleValid(FGameplayAbilitySpecHandle Handle)
{
    return Handle.IsValid();
}

FGameplayAbilitySpecHandle URedpointAbilityCharacterBlueprintLibrary::FindAbilitySpecHandleForClass(
    UAbilitySystemComponent *AbilitySystem,
    TSubclassOf<UGameplayAbility> AbilityClass,
    UObject *OptionalSourceObject)
{
    FScopedAbilityListLock ActiveScopeLock(*AbilitySystem);

    for (FGameplayAbilitySpec &Spec : AbilitySystem->GetActivatableAbilities())
    {
        TSubclassOf<UGameplayAbility> SpecAbilityClass = Spec.Ability->GetClass();
        if (SpecAbilityClass == AbilityClass)
        {
            if (!OptionalSourceObject || (OptionalSourceObject && Spec.SourceObject == OptionalSourceObject))
            {
                return Spec.Handle;
            }
        }
    }

    return FGameplayAbilitySpecHandle();
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()