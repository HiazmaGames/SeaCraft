// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

#pragma once

#include "SeaCraftGameState.generated.h"

/**
 * 
 */
UCLASS()
class ASeaCraftGameState : public AGameState
{
	GENERATED_UCLASS_BODY()

	/** Get link to actor that keeps ocean config */
	UFUNCTION(BlueprintCallable, Category = "World|Ocean")
	class AOceanStateActor* GetOceanStateActor() const;

	/** Get ocean level at desired position */
	UFUNCTION(BlueprintCallable, Category = "World|Ocean")
	float GetOceanLevelAtLocation(FVector& Location) const;

	/** Get ocean surface normal at desired location */
	UFUNCTION(BlueprintCallable, Category = "World|Ocean")
	FLinearColor GetOceanSurfaceNormal(FVector& Location) const;

	/** Get ocean wave horizontal velocity at desired location */
	UFUNCTION(BlueprintCallable, Category = "World|Ocean")
	FVector GetOceanWaveVelocity(FVector& Location) const;

	/** How much waves are defined by normal map */
	int32 GetOceanWavesNum() const;

	// Begin AActor interface
	virtual void BeginPlay() OVERRIDE;
	// End AActor interface

protected:
	/** Cached ocean state actor to avoid seatch each frame with ship */
	TWeakObjectPtr<AOceanStateActor> MainOceanStateActor;
};
