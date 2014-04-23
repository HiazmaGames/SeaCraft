// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

#pragma once

#include "SeaCraftVWeapon_Projectile.h"
#include "SeaCraftProjectile.generated.h"

/**
 * Main class for projectiles
 */
UCLASS(Abstract, Blueprintable, DependsOn = USeaCraftVWeapon_Projectile)
class ASeaCraftProjectile : public AActor
{
	GENERATED_UCLASS_BODY()

	/** Initial setup */
	virtual void PostInitializeComponents() OVERRIDE;

	/** Setup velocity */
	void InitVelocity(FVector& ShootDirection);

	/** Handle hit */
	UFUNCTION()
	void OnImpact(const FHitResult& HitResult);

protected:
	/** Movement component */
	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
	TSubobjectPtr<UProjectileMovementComponent> MovementComp;

	/** Collisions */
	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
	TSubobjectPtr<USphereComponent> CollisionComp;

	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
	TSubobjectPtr<UParticleSystemComponent> ParticleComp;

	/** Effects for explosion */
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	TSubclassOf<class ASeaCraftExplosionEffect> ExplosionTemplate;

	/** Controller that fired me (cache for damage calculations) */
	TWeakObjectPtr<AController> MyController;

	/** Projectile data */
	struct FProjectileWeaponData WeaponConfig;

	/** Did it explode? */
	UPROPERTY(Transient, ReplicatedUsing=OnRep_Exploded)
	bool bExploded;

	/** [client] explosion happened */
	UFUNCTION()
	void OnRep_Exploded();

	/** Trigger explosion */
	void Explode(const FHitResult& Impact);

	/** Shutdown projectile and prepare for destruction */
	void DisableAndDestroy();

	/** Update velocity on client */
	virtual void PostNetReceiveVelocity(const FVector& NewVelocity) OVERRIDE;
};
