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
		UserSettings->ApplySettings();
	}
}

void ASeaCraftPlayerController::SetGraphicsQuality(int32 InGraphicsQuality)
{
	if (PlayerInput != NULL)
	{
		USeaCraftUserSettings *UserSettings = CastChecked<USeaCraftUserSettings>(GEngine->GetGameUserSettings());
		UserSettings->SetGraphicsQuality(InGraphicsQuality);
		UserSettings->ApplySettings();
	}
}

bool ASeaCraftPlayerController::IsGameInputAllowed() const
{
	return bAllowGameActions && !bCinematicMode;
}

