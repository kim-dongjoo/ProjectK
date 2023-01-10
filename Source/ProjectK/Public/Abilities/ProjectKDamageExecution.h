// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "ProjectKDamageExecution.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTK_API UProjectKDamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:

	//Constructor. Sets up the attribute captures needed to run the calculation.
	UProjectKDamageExecution();

	//Execution implementation. The bulk of this operation is in this function.
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
	
};
