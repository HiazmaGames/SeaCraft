// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

#pragma once

#include "./Vehicles/SeaCraftVehicleWeaponComponent.h"
#include "SeaCraftVWeapon_Projectile.generated.h"

USTRUCT()
struct FProjectileWeaponData
{
	GENERATED_USTRUCT_BODY()

	/** Projectile class */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class ASeaCraftProjectile> ProjectileClass;

	/** Life time */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	float ProjectileLife;

	/** Damage at impact point */
	UPROPERTY(EditDefaultsOnly, Category=WeaponStat)
	int32 ExplosionDamage;

	/** Radius of damage */
	UPROPERTY(EditDefaultsOnly, Category=WeaponStat)
	float ExplosionRadius;

	/** Type of damage */
	UPROPERTY(EditDefaultsOnly, Category=WeaponStat)
	TSubclassOf<UDamageType> DamageType;

	/** Defaults */
	FProjectileWeaponData()
	{
		ProjectileClass = NULL;
		ProjectileLife = 10.0f;
		ExplosionDamage = 100;
		ExplosionRadius = 300.0f;
		DamageType = UDamageType::StaticClass();
	}
};

/**
 * A weapon that fires a visible projectile
 */
UCLASS(/*HeaderGroup = Component,*/ ClassGroup = Vehicle, editinlinenew, meta = (BlueprintSpawnableComponent))
class USeaCraftVWeapon_Projectile : public USeaCraftVehicleWeaponComponent
{
	GENERATED_UCLASS_BODY()

	/** Apply config on projectile */
	void ApplyWeaponConfig(FProjectileWeaponData& Data);

protected:

	/** Weapon config */
	UPROPERTY(EditDefaultsOnly, Category=Config)
	FProjectileWeaponData ProjectileConfig;

	//////////////////////////////////////////////////////////////////////////
	// Weapon usage

	/** [local] weapon specific fire implementation */
	virtual void FireWeapon() OVERRIDE;

	/** spawn projectile on server */
	UFUNCTION(reliable, server, WithValidation)
	void ServerFireProjectile(FVector Origin, FVector_NetQuantizeNormal ShootDir);

};
