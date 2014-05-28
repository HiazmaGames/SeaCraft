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

	/** Cached current weapon group */
	UPROPERTY(Category = Vehicle, VisibleAnywhere, BlueprintReadOnly)
	TArray<USeaCraftVehicleWeaponComponent*> CurrentWeapons;


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
	// Damage & death

public:
	/** Identifies if pawn is in its dying state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Health)
	uint32 bIsDying : 1;

	/** Current health of the Pawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Health)
	float Health;

	/** Take damage, handle death */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) OVERRIDE;

	/** Pawn suicide */
	virtual void Suicide();

	/** Kill this pawn */
	virtual void KilledBy(class APawn* EventInstigator);

	/** Returns True if the pawn can die in the current state */
	virtual bool CanDie(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser) const;

	/**
	 * Kills pawn.  Server/authority only.
	 * @param KillingDamage - Damage amount of the killing blow
	 * @param DamageEvent - Damage event of the killing blow
	 * @param Killer - Who killed this pawn
	 * @param DamageCauser - the Actor that directly caused the damage (i.e. the Projectile that exploded, the Weapon that fired, etc)
	 * @returns true if allowed
	 */
	virtual bool Die(float KillingDamage, struct FDamageEvent const& DamageEvent, class AController* Killer, class AActor* DamageCauser);

	/** Die when we fall out of the world */
	virtual void FellOutOfWorld(const class UDamageType& dmgType) OVERRIDE;

	/** Called on the actor right before replication occurs */
	virtual void PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker) OVERRIDE;
protected:
	/** Notification when killed, for both the server and client. */
	virtual void OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser);

	/** Play effects on hit */
	virtual void PlayHit(float DamageTaken, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser);

	/** Sets up the replication for taking a hit */
	void ReplicateHit(float Damage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser, bool bKilled);

	/** Play hit or death on client */
	UFUNCTION()
	void OnRep_LastTakeHitInfo();

	/** Responsible for cleaning up bodies on clients. */
	virtual void TornOff();


	//////////////////////////////////////////////////////////////////////////
	// Reading data

public:
	/** Get current health */
	UFUNCTION(BlueprintCallable, Category = "Player|Vehicle|SeaCraftVehicle")
	float GetHealth() const;

	/** Get max health */
	UFUNCTION(BlueprintCallable, Category = "Player|Vehicle|SeaCraftVehicle")
	int32 GetMaxHealth() const;

	/** Check if pawn is still alive */
	UFUNCTION(BlueprintCallable, Category = "Player|Vehicle|SeaCraftVehicle")
	bool IsAlive() const;

protected:
	/** Replicate where this pawn was last hit and damaged */
	UPROPERTY(Transient, ReplicatedUsing=OnRep_LastTakeHitInfo)
	struct FTakeHitInfo LastTakeHitInfo;

	/** Time at which point the last take hit info for the actor times out and won't be replicated; Used to stop join-in-progress effects all over the screen */
	float LastTakeHitTimeTimeout;

	/** Current firing state */
	uint8 bWantsToFire : 1;

};
