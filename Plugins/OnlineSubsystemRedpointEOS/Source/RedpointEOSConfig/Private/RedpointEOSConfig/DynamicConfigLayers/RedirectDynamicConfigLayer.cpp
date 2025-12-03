// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSConfig/DynamicConfigLayers/RedirectDynamicConfigLayer.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(1724827442, Redpoint::EOS::Config::DynamicConfigLayers)
{

EDynamicConfigLayerApplyTo FRedirectDynamicConfigLayer::GetLayerAppliesTo() const
{
    return EDynamicConfigLayerApplyTo::Engine;
}

FString FRedirectDynamicConfigLayer::GetLayerName() const
{
    return TEXT("Redirect");
}

FString FRedirectDynamicConfigLayer::GetLayerIniContent() const
{
    return TEXT("[CoreRedirects]\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointEOSStarterPlayerController\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointStarterPlayerController\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointEOSStarterMoveAction\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointStarterMoveAction\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointEOSStarterJumpAction\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointStarterJumpAction\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointEOSStarterInputMappingContext\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointStarterInputMappingContext\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointEOSStarterGameMode\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointStarterGameMode\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointEOSStarterCharacter\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointStarterCharacter\")\n\
+StructRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointEOSPartyMemberInfo\",NewName=\"/Script/RedpointEOSFramework.RedpointPartyMemberInfo\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointEOSPartyMember\",NewName=\"/Script/RedpointEOSFramework.RedpointPartyMember\")\n\
+StructRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointEOSPartyAttributeValue\",NewName=\"/Script/RedpointEOSFramework.RedpointPartyAttributeValue\")\n\
+EnumRedirects=(OldName=\"/Script/RedpointEOSFramework.ERedpointEOSPartyAttributeType\",NewName=\"/Script/RedpointEOSFramework.ERedpointPartyAttributeType\")\n\
\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointStarterPlayerController\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointStarterPlayerController\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointStarterMoveAction\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointStarterMoveAction\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointStarterJumpAction\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointStarterJumpAction\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointStarterInputMappingContext\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointStarterInputMappingContext\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointStarterGameMode\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointStarterGameMode\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointStarterCharacter\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointStarterCharacter\")\n\
\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointAbilityBaseCharacter\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointAbilityBaseCharacter\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointAbilityBaseGameplayAbility\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointAbilityBaseGameplayAbility\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointAbilityBasePlayerState\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointAbilityBasePlayerState\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointAbilityCharacterBlueprintLibrary\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointAbilityCharacterBlueprintLibrary\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointAbilityCharacterMovementComponent\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointAbilityCharacterMovementComponent\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointAbilityPlayerCharacter\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointAbilityPlayerCharacter\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointAbilityPlayerState\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointAbilityPlayerState\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointAbilityTask_WaitLanded\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointAbilityTask_WaitLanded\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointAbilityWithInputIdInterface\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointAbilityWithInputIdInterface\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointAbilityWithLevelInterface\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointAbilityWithLevelInterface\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointAttributeSetBase\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointAttributeSetBase\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointAttributeSetDash\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointAttributeSetDash\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointAttributeSetDestructablePoints\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointAttributeSetDestructablePoints\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointAttributeSetHealth\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointAttributeSetHealth\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointAttributeSetInteract\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointAttributeSetInteract\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointAttributeSetJetpack\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointAttributeSetJetpack\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointAttributeSetMovement\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointAttributeSetMovement\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointAttributeSetReserveAmmo\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointAttributeSetReserveAmmo\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointAttributeSetStun\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointAttributeSetStun\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointBackpackComponent\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointBackpackComponent\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointBackpackItem\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointBackpackItem\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointBackpackItemAttachmentInterface\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointBackpackItemAttachmentInterface\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointBackpackItemNotificationInterface\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointBackpackItemNotificationInterface\")\n\
+StructRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointBackpackLoadoutEntry\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointBackpackLoadoutEntry\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointDamageNotificationInterface\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointDamageNotificationInterface\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointHasBackpackInterface\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointHasBackpackInterface\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointIsAliveInterface\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointIsAliveInterface\")\n\
+ClassRedirects=(OldName=\"/Script/RedpointEOSFramework.RedpointOverridesMaxSpeedInterface\",NewName=\"/Script/RedpointEOSFrameworkExtra.RedpointOverridesMaxSpeedInterface\")\n\
");
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()