// Copyright Epic Games, Inc. All Rights Reserved.

#include "astarGameMode.h"
#include "astarPlayerController.h"
#include "astarCharacter.h"
#include "UObject/ConstructorHelpers.h"

AastarGameMode::AastarGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = AastarPlayerController::StaticClass();

	DefaultPawnClass = nullptr;
	// set default controller to our Blueprinted controller
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/TopDown/Blueprints/BP_TopDownPlayerController"));
	if(PlayerControllerBPClass.Class != NULL)
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}
}