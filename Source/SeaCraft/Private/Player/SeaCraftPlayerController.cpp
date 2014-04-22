// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

#include "SeaCraft.h"

ASeaCraftPlayerController::ASeaCraftPlayerController(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	bAllowGameActions = true;
}

bool ASeaCraftPlayerController::IsGameInputAllowed() const
{
	return bAllowGameActions && !bCinematicMode;
}
