// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ProjectK.h"
#include "GameFramework/SaveGame.h"
#include "ProjectKSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTK_API UProjectKSaveGame : public USaveGame
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, Category = Basic)
	FString SaveSlotName;

	UProjectKSaveGame();

};
