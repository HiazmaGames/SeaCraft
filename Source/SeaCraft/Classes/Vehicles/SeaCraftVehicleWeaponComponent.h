// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

#pragma once

#include "GameFramework/PawnMovementComponent.h"
#include "SeaCraftVehicleWeaponComponent.generated.h"

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
 * 
 */
UCLASS(HeaderGroup=Component)
class USeaCraftVehicleWeaponComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

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
	// Reading data

	/** Get current weapon state */
	EVWeaponState::Type GetCurrentState() const;


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

	/** Is weapon fire active? */
	uint32 bWantsToFire : 1;

	//////////////////////////////////////////////////////////////////////////
	// Input - server side

	UFUNCTION(reliable, server, WithValidation)
	void ServerStartFire();

	UFUNCTION(reliable, server, WithValidation)
	void ServerStopFire();

	//////////////////////////////////////////////////////////////////////////
	// Weapon usage

	/** [local] weapon specific fire implementation */
	virtual void FireWeapon() PURE_VIRTUAL(USeaCraftVehicleWeaponComponent::FireWeapon, );

	/** [server] fire & update ammo */
	UFUNCTION(reliable, server, WithValidation)
	void ServerHandleFiring();

	/** [local + server] handle weapon fire */
	void HandleFiring();

	/** Update weapon state */
	void SetWeaponState(EVWeaponState::Type NewState);

	/** Determine current weapon state */
	void DetermineWeaponState();
};
