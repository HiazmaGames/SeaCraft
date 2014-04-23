// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

#pragma once

#include "SeaCraftTypes.h"
#include "SeaCraftImpactEffect.generated.h"

/**
 * Spawnable effect for weapon hit impact - NOT replicated to clients
 * Each impact type should be defined as separate blueprint
 */
UCLASS(Abstract, Blueprintable)
class ASeaCraftImpactEffect : public AActor
{
	GENERATED_UCLASS_BODY()

	/** Default impact FX used when material specific override doesn't exist */
	UPROPERTY(EditDefaultsOnly, Category=Defaults)
	UParticleSystem* DefaultFX;

	/** Default impact sound used when material specific override doesn't exist */
	UPROPERTY(EditDefaultsOnly, Category=Defaults)
	USoundCue* DefaultSound;

	/** Default decal when material specific override doesn't exist */
	UPROPERTY(EditDefaultsOnly, Category=Defaults)
	struct FDecalData DefaultDecal;

	/** Surface data for spawning */
	UPROPERTY(BlueprintReadOnly, Category=Surface)
	FHitResult SurfaceHit;

	/** Spawn effect */
	virtual void PostInitializeComponents() OVERRIDE;

protected:

	/** Get FX for material type */
	UParticleSystem* GetImpactFX(TEnumAsByte<EPhysicalSurface> SurfaceType) const;

	/** Get sound for material type */
	USoundCue* GetImpactSound(TEnumAsByte<EPhysicalSurface> SurfaceType) const;
};
