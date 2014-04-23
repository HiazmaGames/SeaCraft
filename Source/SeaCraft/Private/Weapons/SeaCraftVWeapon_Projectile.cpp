// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

#include "SeaCraft.h"

USeaCraftVWeapon_Projectile::USeaCraftVWeapon_Projectile(const class FPostConstructInitializeProperties& PCIP) : Super(PCIP)
{

}

//////////////////////////////////////////////////////////////////////////
// Weapon usage

void USeaCraftVWeapon_Projectile::FireWeapon()
{
	// @TODO Add projectile shoot dir and origin calculations
	FVector ShootDir; //= GetAdjustedAim();
	FVector Origin; //= GetMuzzleLocation();
	
	ServerFireProjectile(Origin, ShootDir);
}

bool USeaCraftVWeapon_Projectile::ServerFireProjectile_Validate(FVector Origin, FVector_NetQuantizeNormal ShootDir)
{
	return true;
}

void USeaCraftVWeapon_Projectile::ServerFireProjectile_Implementation(FVector Origin, FVector_NetQuantizeNormal ShootDir)
{
	FTransform SpawnTM(ShootDir.Rotation(), Origin);
	ASeaCraftProjectile* Projectile = Cast<ASeaCraftProjectile>(UGameplayStatics::BeginSpawningActorFromClass(this, ProjectileConfig.ProjectileClass, SpawnTM));
	if (Projectile)
	{
		// @TODO Connect component owner and projectile
		// Projectile->Instigator = Instigator;
		// Projectile->SetOwner(this);
		Projectile->InitVelocity(ShootDir);

		UGameplayStatics::FinishSpawningActor(Projectile, SpawnTM);
	}
}

void USeaCraftVWeapon_Projectile::ApplyWeaponConfig(FProjectileWeaponData& Data)
{
	Data = ProjectileConfig;
}
