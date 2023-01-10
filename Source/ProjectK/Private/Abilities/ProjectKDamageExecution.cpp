// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/ProjectKDamageExecution.h"
#include "Abilities/ProjectKAttributeSet.h"
#include "AbilitySystemComponent.h"

//Helper struct for fetching the stats necessary for the melee calculation.
struct FMeleeDamageStatics
{
	//Capturedef declarations for attributes.
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Damage);
	DECLARE_ATTRIBUTE_CAPTUREDEF(DefensePower);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Health);

	//Default constructor.
	FMeleeDamageStatics()
	{
		//Capturedef definitions for attributes. 

		//AttackPower and Damage from the Source of the Gameplay Effect running this calculation. Snapshotted at the moment the calculation is made.
		DEFINE_ATTRIBUTE_CAPTUREDEF(UProjectKAttributeSet, AttackPower, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UProjectKAttributeSet, Damage, Source, true);

		//DefensePower and Health from the target of the Gameplay Effect using this calculation.
		DEFINE_ATTRIBUTE_CAPTUREDEF(UProjectKAttributeSet, DefensePower, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UProjectKAttributeSet, Health, Target, false);
	}

};

//Static helper function to quickly fetch the melee damage capture attributes.
static const FMeleeDamageStatics& MeleeDamageStatics()
{
	static FMeleeDamageStatics DmgStatics;
	return DmgStatics;
}

//Constructor for the Melee Damage Execution. We capture only the attributes used to make the calculation, not the health attribute itself. We apply that at the end of the execution.
UProjectKDamageExecution::UProjectKDamageExecution()
{
	RelevantAttributesToCapture.Add(MeleeDamageStatics().DamageDef);
	RelevantAttributesToCapture.Add(MeleeDamageStatics().DefensePowerDef);
	RelevantAttributesToCapture.Add(MeleeDamageStatics().AttackPowerDef);
}

//Main implementation for the execution.
void UProjectKDamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{

	//Step 1: Obtain references to the target and source Ability System Component. Each copy of a Gameplay Effect will have one of each.
	UAbilitySystemComponent* TargetABSC = ExecutionParams.GetTargetAbilitySystemComponent();
	AActor* TargetActor = TargetABSC ? TargetABSC->GetAvatarActor() : nullptr;

	UAbilitySystemComponent* SourceABSC = ExecutionParams.GetSourceAbilitySystemComponent();
	AActor* SourceActor = SourceABSC ? SourceABSC->GetAvatarActor() : nullptr;

	//Get the owning GameplayEffect Spec so that you can use its variables and tags.
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	//Aggregator Evaluate Parameters used during the attribute capture.
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	//Capturing damage.
	float Damage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(MeleeDamageStatics().DamageDef, EvaluationParameters, Damage);

	//Capturing DefensePower
	float DefensePower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(MeleeDamageStatics().DefensePowerDef, EvaluationParameters, DefensePower);

	//Defense will at minimum be 1.
	if (DefensePower <= 1.0f)
	{
		DefensePower = 1.0f;
	}

	//Capturing AttackPower.
	float AttackPower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(MeleeDamageStatics().AttackPowerDef, EvaluationParameters, AttackPower);

	//Performing the actual damage calculation. Fianl damage done = Damage * AttackPower / Endurance
	float DamageDone = Damage + (AttackPower / DefensePower);

	//We floor damage at 1.
	if (DamageDone <= 0.f)
	{
		DamageDone = 1.0f;
	}

	//Final execution output. We can add more than one AddOutputModifier to change multiple parameters at a time based on more complicated calculations. Here we apply -DamageDone to to Health. Health itself is clamped in the AttributeSet.
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(MeleeDamageStatics().HealthProperty, EGameplayModOp::Additive, -DamageDone));
}
