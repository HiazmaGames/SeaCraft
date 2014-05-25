// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

#pragma once

#include "GameFramework/PawnMovementComponent.h"
#include "ShipVehicleMovementComponent.generated.h"

USTRUCT()
struct FReplicatedShipState
{
	GENERATED_USTRUCT_BODY()

	// input replication: steering
	UPROPERTY()
	float SteeringInput;

	// input replication: throttle
	UPROPERTY()
	float ThrottleInput;

	// input replication: brake
	UPROPERTY()
	float BrakeInput;

	// input replication: handbrake
	UPROPERTY()
	float HandbrakeInput;

	// state replication: current gear
	UPROPERTY()
	int32 CurrentGear;
};

/** We use our own struct to be sure that nothing will be changed externally */
USTRUCT()
struct FShipVehicleInputRate
{
	GENERATED_USTRUCT_BODY()

	// Rate at which the input value rises
	UPROPERTY(EditAnywhere, Category = ShipVehicleInputRate)
	float RiseRate;

	// Rate at which the input value falls
	UPROPERTY(EditAnywhere, Category = ShipVehicleInputRate)
	float FallRate;

	FShipVehicleInputRate() : RiseRate(5.0f), FallRate(5.0f) { }

	/** Change an output value using max rise and fall rates */
	float InterpInputValue(float DeltaTime, float CurrentValue, float NewValue) const
	{
		const float DeltaValue = NewValue - CurrentValue;
		const bool bRising = (DeltaValue > 0.0f) == (CurrentValue > 0.0f);
		const float MaxDeltaValue = DeltaTime * (bRising ? RiseRate : FallRate);
		const float ClampedDeltaValue = FMath::Clamp(DeltaValue, -MaxDeltaValue, MaxDeltaValue);
		return CurrentValue + ClampedDeltaValue;
	}
};

/**
 * 
 */
UCLASS(HeaderGroup=Component)
class UShipVehicleMovementComponent : public UPawnMovementComponent
{
	GENERATED_UCLASS_BODY()

	/** Mass to set the vehicle chassis to. It's much easier to tweak vehicle settings when
	 * the mass doesn't change due to tweaks with the physics asset. [kg] */
	UPROPERTY(EditAnywhere, Category = VehicleSetup)
	float Mass;

	/** Where motor is situated on ship */
	UPROPERTY(EditAnywhere, Category = VehicleSetup)
	FVector MotorLocation;

	/** Override center of mass offset, makes tweaking easier [uu] */
	UPROPERTY(EditAnywhere, Category = VehicleSetup, AdvancedDisplay)
	FVector COMOffset;
	
	/** Set the user input for the vehicle throttle */
	UFUNCTION(BlueprintCallable, Category = "Game|Components|ShipVehicleMovement")
	void SetThrottleInput(float Throttle);

	/** Set the user input for the vehicle steering */
	UFUNCTION(BlueprintCallable, Category = "Game|Components|ShipVehicleMovement")
	void SetSteeringInput(float Steering);

	/** Set the user input for handbrake */
	UFUNCTION(BlueprintCallable, Category = "Game|Components|ShipVehicleMovement")
	void SetHandbrakeInput(bool bNewHandbrake);

	/** Set the user input for gear up */
	UFUNCTION(BlueprintCallable, Category = "Game|Components|ShipVehicleMovement")
	void SetGearUp(bool bNewGearUp);

	/** Set the user input for gear down */
	UFUNCTION(BlueprintCallable, Category = "Game|Components|ShipVehicleMovement")
	void SetGearDown(bool bNewGearDown);

	/** Set the user input for gear (-1 reverse, 0 neutral, 1+ forward)*/
	UFUNCTION(BlueprintCallable, Category = "Game|Components|ShipVehicleMovement")
	void SetTargetGear(int32 GearNum, bool bImmediate);

	/** Set the flag that will be used to select auto-gears */
	UFUNCTION(BlueprintCallable, Category = "Game|Components|ShipVehicleMovement")
	void SetUseAutoGears(bool bUseAuto);

	/** How fast the vehicle is moving forward */
	UFUNCTION(BlueprintCallable, Category = "Game|Components|ShipVehicleMovement")
	float GetForwardSpeed() const;

	/** Get current engine's rotation speed */
	UFUNCTION(BlueprintCallable, Category = "Game|Components|ShipVehicleMovement")
	float GetEngineRotationSpeed() const;

	/** Get current gear */
	UFUNCTION(BlueprintCallable, Category = "Game|Components|ShipVehicleMovement")
	int32 GetCurrentGear() const;

	/** Get target gear */
	UFUNCTION(BlueprintCallable, Category = "Game|Components|ShipVehicleMovement")
	int32 GetTargetGear() const;

	/** Get gears numbrt */
	UFUNCTION(BlueprintCallable, Category = "Game|Components|ShipVehicleMovement")
	int32 GetMaxGear(bool bForward = true) const;

	/** Are gears being changed automatically? */
	UFUNCTION(BlueprintCallable, Category = "Game|Components|ShipVehicleMovement")
	bool GetUseAutoGears() const;

	//Begin UActorComponent Interface
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) OVERRIDE;
	//End UActorComponent Interface

protected:
	/** Perform movement on an autonomous client */
	virtual void PerformMovement(float DeltaTime);

	/** Additional math */
	static void GetAxes(FRotator A, FVector& X, FVector& Y, FVector& Z);

protected:
	/** Maximum gear to move forward */
	UPROPERTY(EditAnywhere, Category = VehicleGears)
	int32 MaxGearForward;

	/** Maximum gear to move backward (negative!) */
	UPROPERTY(EditAnywhere, Category = VehicleGears)
	int32 MaxGearBackward;

	/** Current vehicle gear */
	UPROPERTY(Transient)
	int32 CurrentGearCustom;

	// Replicated state of vehicle 
	UPROPERTY(Transient, Replicated)
	FReplicatedShipState ReplicatedState;

	// accumulator for RB replication errors 
	float AngErrorAccumulator;

	// What the player has the steering set to. Range -1...1
	UPROPERTY(Transient)
	float RawSteeringInput;

	// What the player has the accelerator set to. Range -1...1
	UPROPERTY(Transient)
	float RawThrottleInput;

	// True if the player is holding the handbrake
	UPROPERTY(Transient)
	uint32 bRawHandbrakeInput : 1;

	// True if the player is holding gear up
	UPROPERTY(Transient)
	uint32 bRawGearUpInput : 1;

	// True if the player is holding gear down
	UPROPERTY(Transient)
	uint32 bRawGearDownInput : 1;

	// Steering output to physics system. Range -1...1
	UPROPERTY(Transient)
	float SteeringInput;

	// Accelerator output to physics system. Range 0...1
	UPROPERTY(Transient)
	float ThrottleInput;

	// Brake output to physics system. Range 0...1
	UPROPERTY(Transient)
	float BrakeInput;

	// Handbrake output to physics system. Range 0...1
	UPROPERTY(Transient)
	float HandbrakeInput;

	// How much to press the brake when the player has release throttle
	UPROPERTY(EditAnywhere, Category = VehicleInput)
	float IdleBrakeInput;

	// Auto-brake when absolute vehicle forward speed is less than this
	UPROPERTY(EditAnywhere, Category = VehicleInput)
	float StopThreshold;

	// Rate at which input throttle can rise and fall
	UPROPERTY(EditAnywhere, Category = VehicleInput, AdvancedDisplay)
	FShipVehicleInputRate ThrottleInputRate;

	// Rate at which input brake can rise and fall
	UPROPERTY(EditAnywhere, Category = VehicleInput, AdvancedDisplay)
	FShipVehicleInputRate BrakeInputRate;

	// Rate at which input handbrake can rise and fall
	UPROPERTY(EditAnywhere, Category = VehicleInput, AdvancedDisplay)
	FShipVehicleInputRate HandbrakeInputRate;

	// Rate at which input steering can rise and fall
	UPROPERTY(EditAnywhere, Category = VehicleInput, AdvancedDisplay)
	FShipVehicleInputRate SteeringInputRate;

	/** Compute steering input */
	float CalcSteeringInput();

	/** Compute brake input */
	float CalcBrakeInput();

	/** Compute handbrake input */
	float CalcHandbrakeInput();

	/** Compute throttle input */
	virtual float CalcThrottleInput();

	/** Clear all interpolated inputs to default values */
	virtual void ClearInput();

	/** Read current state for simulation */
	void UpdateState(float DeltaTime);

	/** Pass current state to server */
	UFUNCTION(reliable, server, WithValidation)
	void ServerUpdateState(float InSteeringInput, float InThrottleInput, float InBrakeInput, float InHandbrakeInput, int32 CurrentGear);

	/** Get the local COM offset */
	virtual FVector GetCOMOffset();

protected:

	/** Forwards acceleration */
	UPROPERTY(EditAnywhere, Category = WaveReaction)
	float ThrustForceFactor;

	/** Backwards acceleration */
	UPROPERTY(EditAnywhere, Category = WaveReaction)
	float ReverseForceFactor;

	/** Turn acceleration */
	UPROPERTY(EditAnywhere, Category = WaveReaction)
	float TurnTorqueFactor;

	/** Max angle of motor? */
	UPROPERTY(EditAnywhere, Category = WaveReaction)
	float MaxTurnAngle;

};
