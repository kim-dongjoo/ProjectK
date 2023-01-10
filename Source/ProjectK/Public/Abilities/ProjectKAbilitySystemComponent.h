// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ProjectK.h"
#include "AbilitySystemComponent.h"
#include "Abilities/ProjectKAbilityTypes.h"
#include "ProjectKAbilitySystemComponent.generated.h"

class UProjectKGameplayAbility;

/**
 * Subclass of ability system component with game-specific data
 * Most games will need to make a game-specific subclass to provide utility functions
 */
UCLASS()
class PROJECTK_API UProjectKAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	// Constructors and overrides
	UProjectKAbilitySystemComponent();

	/** Returns a list of currently active ability instances that match the tags */
	void GetActiveAbilitiesWithTags(const FGameplayTagContainer& GameplayTagContainer, TArray<UProjectKGameplayAbility*>& ActiveAbilities);

	/** Version of function in AbilitySystemGlobals that returns correct type */
	static UProjectKAbilitySystemComponent* GetAbilitySystemComponentFromActor(const AActor* Actor, bool LookForComponent = false);
	
};
