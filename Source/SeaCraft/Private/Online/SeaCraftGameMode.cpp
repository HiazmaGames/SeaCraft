// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

#include "SeaCraft.h"

ASeaCraftGameMode::ASeaCraftGameMode(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	PlayerControllerClass = ASeaCraftPlayerController::StaticClass();
	GameStateClass = ASeaCraftGameState::StaticClass();
}


