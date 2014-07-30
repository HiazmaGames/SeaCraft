// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

#pragma once

#include "SeaCraftVehicleWeaponComponent.generated.h"

UENUM(BlueprintType)
namespace EVWeaponState
{
	enum Type
	{
		Idle,
		Firing,
		Reloading,
		PreFire,
		PostFire
	};
}

/**
 * Basic class for weapon "inside" vehicle mesh
 */
UCLASS(Abstract/*, HeaderGroup = Component*/)
class USeaCraftVehicleWeaponComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

	// Begin UActorComponent Interface
	virtual void InitializeComponent() override;
	// End UActorComponent Interface

	/** Time between two consecutive shots */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	float TimeBetweenShots;

	/** Inifite ammo for reloads */
	UPROPERTY(EditDefaultsOnly, Category = Ammo)
	bool bInfiniteAmmo;


	//////////////////////////////////////////////////////////////////////////
	// Input

	/** [local + server] start weapon fire */
	virtual void StartFire();

	/** [local + server] stop weapon fire */
	virtual void StopFire();


	//////////////////////////////////////////////////////////////////////////
	// Control

	/** Check if weapon can fire */
	bool CanFire() const;


	//////////////////////////////////////////////////////////////////////////
	// Ammo

	/** [server] add ammo */
	UFUNCTION(BlueprintCallable, Category = "Game|VehicleWeapon")
	void GiveAmmo(int32 AddAmount);

	/** Consume a bullet */
	void UseAmmo();


	//////////////////////////////////////////////////////////////////////////
	// Reading data

	/** Get current weapon state */
	UFUNCTION(BlueprintCallable, Category = "Game|VehicleWeapon")
	EVWeaponState::Type GetCurrentState() const;

	/** Get vehicle-unique weapon id */
	UFUNCTION(BlueprintCallable, Category = "Game|VehicleWeapon")
	FName GetWeaponID() const;

	/** Get vehicle-unique group id */
	UFUNCTION(BlueprintCallable, Category = "Game|VehicleWeapon")
	FName GetGroupID() const;

	/** Get current ammo amount (total) */
	UFUNCTION(BlueprintCallable, Category = "Game|VehicleWeapon")
	int32 GetCurrentAmmo() const;

	/** Get maximum ammo amount */
	UFUNCTION(BlueprintCallable, Category = "Game|VehicleWeapon")
	int32 GetMaxAmmo() const;

	/** Check if weapon has infinite ammo (include owner's cheats) */
	UFUNCTION(BlueprintCallable, Category = "Game|VehicleWeapon")
	bool HasInfiniteAmmo() const;

	/** Get last active turret socket */
	UFUNCTION(BlueprintCallable, Category = "Game|VehicleWeapon")
	FName GetLastActiveTurretBarrel() const;


protected:
	/** Vehicle-unique ID to get and/or reference weapon in blueprints */
	UPROPERTY(EditDefaultsOnly, Category = WeaponInfo)
	FName WeaponID;

	/** Vehicle-unique group ID to control weapons in group simultaneously */
	UPROPERTY(EditDefaultsOnly, Category = WeaponInfo)
	FName GroupID;

	/** Firing sockets names */
	UPROPERTY(EditDefaultsOnly, Category = Turrets)
	TArray<FName> TurretSockets;

	/** Current weapon state */
	EVWeaponState::Type CurrentState;

	/** Last active turret barrel */
	UPROPERTY(Transient, Replicated)
	int32 LastActiveTurretBarrel;

	/** Is weapon fire active? */
	uint32 bWantsToFire : 1;

	/** Weapon is refiring */
	uint32 bRefiring;

	/** Time of last successful weapon fire */
	float LastFireTime;

	/** Current total ammo */
	UPROPERTY(Transient, Replicated)
	int32 CurrentAmmo;

	/** Maximum ammo */
	UPROPERTY(EditDefaultsOnly, Category = Ammo)
	int32 MaxAmmo;


	//////////////////////////////////////////////////////////////////////////
	// Input - server side

	UFUNCTION(reliable, server, WithValidation)
	void ServerStartFire();

	UFUNCTION(reliable, server, WithValidation)
	void ServerStopFire();


	//////////////////////////////////////////////////////////////////////////
	// Weapon usage

	/** Update weapon state */
	void SetWeaponState(EVWeaponState::Type NewState);

	/** Determine current weapon state */
	void DetermineWeaponState();

	/** [local] weapon specific fire implementation */
	virtual void FireWeapon() PURE_VIRTUAL(USeaCraftVehicleWeaponComponent::FireWeapon, );

	/** [server] fire & update ammo */
	UFUNCTION(reliable, server, WithValidation)
	void ServerHandleFiring();

	/** [local + server] handle weapon fire */
	void HandleFiring();

	/** [local + server] firing started */
	virtual void OnBurstStarted();

	/** [local + server] firing finished */
	virtual void OnBurstFinished();


	//////////////////////////////////////////////////////////////////////////
	// Weapon usage helpers

public:
	/** Get the aim of the weapon, allowing for adjustments to be made by the weapon */
	UFUNCTION(BlueprintCallable, Category = "Game|VehicleWeapon")
	virtual FVector GetAdjustedAim() const;

	/** Get the muzzle location of the weapon */
	UFUNCTION(BlueprintCallable, Category = "Game|VehicleWeapon")
	FVector GetMuzzleLocation() const;

	/** Get direction of weapon's muzzle */
	UFUNCTION(BlueprintCallable, Category = "Game|VehicleWeapon")
	FVector GetMuzzleDirection() const;

protected:
	/** Get the aim of the camera */
	FVector GetCameraAim() const;

	/** Get the originating location for camera damage */
	FVector GetCameraDamageStartLocation(const FVector& AimDir) const;

	/** Find hit */
	FHitResult WeaponTrace(const FVector& TraceFrom, const FVector& TraceTo) const;

};
