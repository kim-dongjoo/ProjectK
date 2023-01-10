// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/ProjectKAbilitySystemComponent.h"
#include "ProjectKCharacter.h"
#include "Abilities/ProjectKGameplayAbility.h"
#include "AbilitySystemGlobals.h"

UProjectKAbilitySystemComponent::UProjectKAbilitySystemComponent() {}

void UProjectKAbilitySystemComponent::GetActiveAbilitiesWithTags(const FGameplayTagContainer& GameplayTagContainer, TArray<UProjectKGameplayAbility*>& ActiveAbilities)
{
	TArray<FGameplayAbilitySpec*> AbilitiesToActivate;
	GetActivatableGameplayAbilitySpecsByAllMatchingTags(GameplayTagContainer, AbilitiesToActivate, false);

	// Iterate the list of all ability specs
	for (FGameplayAbilitySpec* Spec : AbilitiesToActivate)
	{
		// Iterate all instances on this ability spec
		TArray<UGameplayAbility*> AbilityInstances = Spec->GetAbilityInstances();

		for (UGameplayAbility* ActiveAbility : AbilityInstances)
		{
			ActiveAbilities.Add(Cast<UProjectKGameplayAbility>(ActiveAbility));
		}
	}
}

UProjectKAbilitySystemComponent* UProjectKAbilitySystemComponent::GetAbilitySystemComponentFromActor(const AActor* Actor, bool LookForComponent)
{
	return Cast<UProjectKAbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor, LookForComponent));
}
