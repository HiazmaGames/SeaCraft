// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

#include "SeaCraft.h"

ASeaCraftPlayerController::ASeaCraftPlayerController(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	bAllowGameActions = true;
}

void ASeaCraftPlayerController::InitInputSystem()
{
	Super::InitInputSystem();

	if (PlayerInput != NULL)
	{
		USeaCraftUserSettings *UserSettings = CastChecked<USeaCraftUserSettings>(GEngine->GetGameUserSettings());
		UserSettings->ApplySettings(false);
	}
}

void ASeaCraftPlayerController::SetGraphicsQuality(int32 InGraphicsQuality)
{
	if (PlayerInput != NULL)
	{
		USeaCraftUserSettings *UserSettings = CastChecked<USeaCraftUserSettings>(GEngine->GetGameUserSettings());
		UserSettings->SetGraphicsQuality(InGraphicsQuality);
		UserSettings->ApplySettings(false);
	}
}

void ASeaCraftPlayerController::Quit()
{
	ConsoleCommand("quit");
}

bool ASeaCraftPlayerController::IsGameInputAllowed() const
{
	return bAllowGameActions && !bCinematicMode;
}

