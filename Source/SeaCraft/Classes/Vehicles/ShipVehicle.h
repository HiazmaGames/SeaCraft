// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

#pragma once

#include "GameFramework/Pawn.h"
#include "ShipVehicle.generated.h"

/**
 * 
 */
UCLASS(abstract/*, HeaderGroup=Pawn*/, config = Game, dependson = (AController), BlueprintType)
class AShipVehicle : public ASeaCraftVehicle
{
	GENERATED_UCLASS_BODY()

	//////////////////////////////////////////////////////////////////////////
	// Ship physics (movement)

	/** Vehicle simulation component */
	UPROPERTY(Category = Vehicle, VisibleDefaultsOnly, BlueprintReadOnly)
	TSubobjectPtr<class UShipVehicleMovementComponent> VehicleMovement;

	/** Name of the VehicleMovement. Use this name if you want to use a different class (with PCIP.SetDefaultSubobjectClass). */
	static FName VehicleMovementComponentName;


	//////////////////////////////////////////////////////////////////////////
	// Camera

	/** Arm to hold the camera */
	UPROPERTY(Category = Vehicle, VisibleDefaultsOnly, BlueprintReadOnly)
	TSubobjectPtr<class USpringArmComponent> CameraSpringArm;

	/** Main vehicle camera */
	UPROPERTY(Category = Vehicle, VisibleDefaultsOnly, BlueprintReadOnly)
	TSubobjectPtr<class UCameraComponent> VehicleCamera;


	//////////////////////////////////////////////////////////////////////////
	// Reading data

public:
	/** Get current floatage (total) */
	UFUNCTION(BlueprintCallable, Category = "Player|Vehicle|SeaCraftVehicle")
	float GetFloatage() const;

	/** Get max floatage (total) */
	UFUNCTION(BlueprintCallable, Category = "Player|Vehicle|SeaCraftVehicle")
	int32 GetMaxFloatage() const;

};
