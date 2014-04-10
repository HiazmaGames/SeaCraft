// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

#include "SeaCraft.h"

ASeaCraftGameState::ASeaCraftGameState(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	MainOceanStateActor = NULL;
}

void ASeaCraftGameState::BeginPlay()
{
	Super::BeginPlay();

	// Find ocean state actor on scene
	TActorIterator< AOceanStateActor > ActorItr = TActorIterator< AOceanStateActor >(GetWorld());
	while (ActorItr)
	{
		MainOceanStateActor = *ActorItr;

		// Don't look for next actor, we need just one!
		break;
		//++ActorItr;
	}

	if (MainOceanStateActor.IsValid())
	{
		UE_LOG(LogShipPhysics, Log, TEXT("Ocean state successfully found by GameState"));
	}
	else
	{
		UE_LOG(LogShipPhysics, Warning, TEXT("Can't find ocean state actor! Default ocean level will be used."));
	}
}

AOceanStateActor* ASeaCraftGameState::GetOceanStateActor() const
{
	if (MainOceanStateActor.IsValid())
	{
		return MainOceanStateActor.Get();
	}

	return NULL;
}

float ASeaCraftGameState::GetOceanLevelAtLocation(FVector& Location) const
{
	if (MainOceanStateActor.IsValid())
	{
		return MainOceanStateActor->GetOceanLevelAtLocation(Location);
	}

	return 0.0f;
}

FLinearColor ASeaCraftGameState::GetOceanSurfaceNormal(FVector& Location) const
{
	if (MainOceanStateActor.IsValid())
	{
		return MainOceanStateActor->GetOceanSurfaceNormal(Location);
	}

	return FLinearColor();
}

FVector ASeaCraftGameState::GetOceanWaveVelocity(FVector& Location) const
{
	if (MainOceanStateActor.IsValid())
	{
		return MainOceanStateActor->GetOceanWaveVelocity(Location);
	}

	return FVector();
}
