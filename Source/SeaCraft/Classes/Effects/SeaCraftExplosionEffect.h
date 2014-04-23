// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

#pragma once

#include "SeaCraftTypes.h"
#include "SeaCraftExplosionEffect.generated.h"

/**
 * Spawnable effect for explosion - NOT replicated to clients
 * Each explosion type should be defined as separate blueprint
 */
UCLASS(Abstract, Blueprintable)
class ASeaCraftExplosionEffect : public AActor
{
	GENERATED_UCLASS_BODY()

	/** Explosion FX */
	UPROPERTY(EditDefaultsOnly, Category=Effect)
	UParticleSystem* ExplosionFX;

	/** Explosion light */
	UPROPERTY(VisibleDefaultsOnly, Category=Effect)
	TSubobjectPtr<UPointLightComponent> ExplosionLight;

	/** How long keep explosion light on? */
	UPROPERTY(EditDefaultsOnly, Category=Effect)
	float ExplosionLightFadeOut;

	/** Explosion sound */
	UPROPERTY(EditDefaultsOnly, Category=Effect)
	USoundCue* ExplosionSound;
	
	/** Explosion decals */
	UPROPERTY(EditDefaultsOnly, Category=Effect)
	struct FDecalData Decal;

	/** Surface data for spawning */
	UPROPERTY(BlueprintReadOnly, Category=Surface)
	FHitResult SurfaceHit;

	/** Spawn explosion */
	virtual void BeginPlay() OVERRIDE;

	/** Update fading light */
	virtual void Tick(float DeltaSeconds) OVERRIDE;

private:
	/** Point light component name */
	FName ExplosionLightComponentName;
};
