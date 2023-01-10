// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectKGameMode.h"
#include "ProjectKCharacter.h"
#include "UObject/ConstructorHelpers.h"

AProjectKGameMode::AProjectKGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/PC/BP_Pododaejang"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
