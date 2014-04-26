// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

#pragma once

#include "GameFramework/Pawn.h"
#include "SeaCraftVehicle.generated.h"

/**
 * Basic class for all vehicles that controls weapons, seats, effects and animations
 */
UCLASS(abstract, HeaderGroup=Pawn, config = Game, dependson = AController, BlueprintType, Blueprintable, hidecategories = (Navigation, "AI|Navigation"))
class ASeaCraftVehicle : public APawn
{
	GENERATED_UCLASS_BODY()

	/** Configure weapons and setup initial variables */
	virtual void PostInitializeComponents() OVERRIDE;


	//////////////////////////////////////////////////////////////////////////
	// Vehicle mesh

	/** The main skeletal mesh associated with this Vehicle */
	UPROPERTY(Category = Vehicle, VisibleDefaultsOnly, BlueprintReadOnly)
	TSubobjectPtr<class USkeletalMeshComponent> VehicleMesh;

	/** Name of the MeshComponent. Use this name if you want to prevent creation of the component (with PCIP.DoNotCreateDefaultSubobject). */
	static FName VehicleMeshComponentName;


	//////////////////////////////////////////////////////////////////////////
	// Weapon control

	/** Current controlled weapon group */
	FName CurrentWeaponGroup;

	/** Activates weapon group and makes it controlled by player */
	UFUNCTION(BlueprintCallable, Category = "Player|Vehicle|SeaCraftVehicle")
	void SetWeaponGroup(FName WeaponGroup);


	//////////////////////////////////////////////////////////////////////////
	// Weapon usage

	/** [local] starts weapon fire */
	void StartWeaponFire();

	/** [local] stops weapon fire */
	void StopWeaponFire();

	/** Check if vehicle can fire weapon */
	bool CanFire() const;


	//////////////////////////////////////////////////////////////////////////
	// Input handlers

	/** Setup vehicle specific input handlers */
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) OVERRIDE;

	/** Player pressed start fire action */
	void OnStartFire();

	/** Player released start fire action */
	void OnStopFire();

	/** Player pressed next weapon action */
	void OnNextWeapon();

	/** Player pressed prev weapon action */
	void OnPrevWeapon();


	//////////////////////////////////////////////////////////////////////////
	// Weapon usage helpers

	/** Get the aim of the weapon, allowing for adjustments to be made by the weapon */
	//virtual FVector GetAdjustedAim() const;

	/** Get the aim of the camera */
	//FVector GetCameraAim() const;

	/** Get the originating location for camera damage */
	//FVector GetCameraDamageStartLocation(const FVector& AimDir) const;

	/** Find hit */
	//FHitResult WeaponTrace(const FVector& TraceFrom, const FVector& TraceTo) const;


protected:
	/** Current firing state */
	uint8 bWantsToFire : 1;
	
};
