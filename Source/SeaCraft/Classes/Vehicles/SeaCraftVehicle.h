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

	/** List of unique weapon groups */
	TArray<FName> WeaponGroups;

	/** Current controlled weapon group */
	FName CurrentWeaponGroup;

	/** Returns index of desired weapon group, -1 if group is not found */
	int32 FindWeaponGroup(const FName& WeaponGroup);

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


protected:
	/** Current firing state */
	uint8 bWantsToFire : 1;

	/** Cached current weapon group */
	TArray<USeaCraftVehicleWeaponComponent*> CurrentWeapons;

};
