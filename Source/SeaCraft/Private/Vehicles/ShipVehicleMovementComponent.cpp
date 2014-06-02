// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

#include "SeaCraft.h"

UShipVehicleMovementComponent::UShipVehicleMovementComponent(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	Mass = 1500.0f;
	// InertiaTensorScale = FVector(1.0f, 0.80f, 1.0f);
	AngErrorAccumulator = 0.0f;

	IdleBrakeInput = 0.05f;
	StopThreshold = 10.0f;
	ThrottleInputRate.RiseRate = 6.0f;
	ThrottleInputRate.FallRate = 10.0f;
	BrakeInputRate.RiseRate = 6.0f;
	BrakeInputRate.FallRate = 10.0f;
	HandbrakeInputRate.RiseRate = 12.0f;
	HandbrakeInputRate.FallRate = 12.0f;
	SteeringInputRate.RiseRate = 2.5f;
	SteeringInputRate.FallRate = 5.0f;

	ThrustForceFactor = 40000.0f;
	ReverseForceFactor = 10000.0f;
	
	TurnTorqueFactor = 40.0f;
	MaxTurnAngle = 10.0f;

	MaxGearForward = 4;
	MaxGearBackward = -2;
	CurrentGearCustom = 0;
}

FVector UShipVehicleMovementComponent::GetCOMOffset()
{
	return COMOffset;
}

void UShipVehicleMovementComponent::UpdateState(float DeltaTime)
{
	// update input values
	APawn* MyOwner = UpdatedComponent ? Cast<APawn>(UpdatedComponent->GetOwner()) : NULL;
	if (MyOwner && MyOwner->IsLocallyControlled())
	{
		SteeringInput = SteeringInputRate.InterpInputValue(DeltaTime, SteeringInput, CalcSteeringInput());
		ThrottleInput = ThrottleInputRate.InterpInputValue(DeltaTime, ThrottleInput, CalcThrottleInput());
		BrakeInput = BrakeInputRate.InterpInputValue(DeltaTime, BrakeInput, CalcBrakeInput());
		HandbrakeInput = HandbrakeInputRate.InterpInputValue(DeltaTime, HandbrakeInput, CalcHandbrakeInput());

		// and send to server
		ServerUpdateState(SteeringInput, ThrottleInput, BrakeInput, HandbrakeInput, GetCurrentGear(), TargetTurnAngle);
	}
	else
	{
		// use replicated values for remote pawns
		SteeringInput = ReplicatedState.SteeringInput;
		ThrottleInput = ReplicatedState.ThrottleInput;
		BrakeInput = ReplicatedState.BrakeInput;
		HandbrakeInput = ReplicatedState.HandbrakeInput;
		TargetTurnAngle = ReplicatedState.TargetTurnAngle;
		SetTargetGear(ReplicatedState.CurrentGear, true);
	}
}

bool UShipVehicleMovementComponent::ServerUpdateState_Validate(float InSteeringInput, float InThrottleInput, float InBrakeInput, float InHandbrakeInput, int32 InCurrentGear, float InTargetAngle)
{
	return true;
}

void UShipVehicleMovementComponent::ServerUpdateState_Implementation(float InSteeringInput, float InThrottleInput, float InBrakeInput, float InHandbrakeInput, int32 InCurrentGear, float InTargetAngle)
{
	SteeringInput = InSteeringInput;
	ThrottleInput = InThrottleInput;
	BrakeInput = InBrakeInput;
	HandbrakeInput = InHandbrakeInput;
	TargetTurnAngle = InTargetAngle;

	if (!GetUseAutoGears())
	{
		SetTargetGear(InCurrentGear, true);
	}

	// update state of inputs
	ReplicatedState.SteeringInput = InSteeringInput;
	ReplicatedState.ThrottleInput = InThrottleInput;
	ReplicatedState.BrakeInput = InBrakeInput;
	ReplicatedState.HandbrakeInput = InHandbrakeInput;
	ReplicatedState.CurrentGear = InCurrentGear;
	ReplicatedState.TargetTurnAngle = InTargetAngle;
}

float UShipVehicleMovementComponent::CalcSteeringInput()
{
	return /*RawSteeringInput*/ (float)TargetTurnAngle / (float)MaxTurnAngle;
}

float UShipVehicleMovementComponent::CalcBrakeInput()
{
	const float ForwardSpeed = GetForwardSpeed();

	float NewBrakeInput = 0.0f;

	// @TODO Anchor down even calculation here
	return NewBrakeInput;

	// if player wants to move forwards...
	if (RawThrottleInput > 0.1f)  // MSS expanded hardcoded dead zone from 0.01f to filter weird throttle noise.
	{
		// if vehicle is moving backwards, then press brake
		if (ForwardSpeed < -StopThreshold)
		{
			NewBrakeInput = 1.0f;
			SetTargetGear(1, true);
		}
	}

	// if player wants to move backwards...
	else if (RawThrottleInput < -0.1f)
	{
		// if vehicle is moving forwards, then press brake
		if (ForwardSpeed > StopThreshold)
		{
			NewBrakeInput = 1.0f;			// Seems a bit severe to have 0 or 1 braking. Better control can be had by allowing continuous brake input values
			// set to reverse
			SetTargetGear(-1, true);
		}
	}

	// if player isn't pressing forward or backwards...
	else
	{
		// If almost stationary, stick brakes on
		if (FMath::Abs(ForwardSpeed) <  StopThreshold)
		{
			NewBrakeInput = 0.1f + 0.1f * FMath::Abs(ForwardSpeed);
		}
		else
		{
			NewBrakeInput = IdleBrakeInput;
		}
	}

	return FMath::Clamp<float>(NewBrakeInput, 0.0, 1.0);
}

float UShipVehicleMovementComponent::CalcHandbrakeInput()
{
	return (bRawHandbrakeInput == true) ? 1.0f : 0.0f;
}

float UShipVehicleMovementComponent::CalcThrottleInput()
{
	if (CurrentGearCustom < 0)
	{
		return -1.0f * (float)CurrentGearCustom / (float)MaxGearBackward;
	}

	return /*RawThrottleInput*/ (float)CurrentGearCustom / (float)MaxGearForward;
}

void UShipVehicleMovementComponent::ClearInput()
{
	SteeringInput = 0.0f;
	ThrottleInput = 0.0f;
	BrakeInput = 0.0f;
	HandbrakeInput = 0.0f;
}

void UShipVehicleMovementComponent::SetThrottleInput(float Throttle)
{
	RawThrottleInput = FMath::Clamp(Throttle, -1.0f, 1.0f);
}

void UShipVehicleMovementComponent::SetSteeringInput(float Steering)
{
	RawSteeringInput = FMath::Clamp(Steering, -1.0f, 1.0f);
}

void UShipVehicleMovementComponent::SetHandbrakeInput(bool bNewHandbrake)
{
	bRawHandbrakeInput = bNewHandbrake;
}

void UShipVehicleMovementComponent::SetGearUp(bool bNewGearUp)
{
	bRawGearUpInput = bNewGearUp;
}

void UShipVehicleMovementComponent::SetGearDown(bool bNewGearDown)
{
	bRawGearDownInput = bNewGearDown;
}

void UShipVehicleMovementComponent::SetTargetGear(int32 GearNum, bool bImmediate)
{
#if WITH_PHYSX
	//
	// PHYSX GEAR
	//
#endif

	if (GearNum > 0)
	{
		CurrentGearCustom = FMath::Min(GearNum, MaxGearForward);
	}
	else
	{
		CurrentGearCustom = FMath::Max(GearNum, MaxGearBackward);
	}
}

void UShipVehicleMovementComponent::SetUseAutoGears(bool bUseAuto)
{
#if WITH_PHYSX
	//
	// PHYSX GEAR
	//
#endif
}

void UShipVehicleMovementComponent::SetTargetTurnAngle(float TurnAngle)
{
	if (TurnAngle > 0)
	{
		TargetTurnAngle = FMath::Min(TurnAngle, MaxTurnAngle);
	}
	else
	{
		TargetTurnAngle = FMath::Max(TurnAngle, -MaxTurnAngle);
	}
}

float UShipVehicleMovementComponent::GetForwardSpeed() const
{
#if WITH_PHYSX
	//
	// COMPUTE FORWARD SPEED
	//
#endif

	AActor* Vehicle = GetOwner();
	if (Vehicle != NULL)
	{
		return Vehicle->GetVelocity().Size();
	}

	return 0.0f;
}

float UShipVehicleMovementComponent::GetEngineRotationSpeed() const
{
#if WITH_PHYSX
	//
	// PHYSX RPM
	//
#endif

	return 0.0f;
}

int32 UShipVehicleMovementComponent::GetCurrentGear() const
{
#if WITH_PHYSX
	//
	// PHYSX
	//
#endif

	return CurrentGearCustom;
}

int32 UShipVehicleMovementComponent::GetTargetGear() const
{
#if WITH_PHYSX
	//
	// PHYSX
	//
#endif

	return 0;
}

int32 UShipVehicleMovementComponent::GetMaxGear(bool bForward) const
{
#if WITH_PHYSX
	//
	// PHYSX
	//
#endif

	if (bForward)
	{
		return MaxGearForward;
	}
	else
	{
		return MaxGearBackward;
	}

	return 0;
}

bool UShipVehicleMovementComponent::GetUseAutoGears() const
{
#if WITH_PHYSX
	//
	// PHYSX
	//
#endif

	return false;
}

float UShipVehicleMovementComponent::GetTargetTurnAngle() const
{
	return TargetTurnAngle;
}

float UShipVehicleMovementComponent::GetMaxTurnAngle() const
{
	return MaxTurnAngle;
}

void UShipVehicleMovementComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UShipVehicleMovementComponent, ReplicatedState);
}

void UShipVehicleMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(!UpdatedComponent)
	{
		return;
	}

	// Update player input and replicate it
	UpdateState(DeltaTime);

	// React on world and input
	PerformMovement(DeltaTime);
}

void UShipVehicleMovementComponent::PerformMovement(float DeltaTime)
{
	if(!UpdatedComponent)
	{
		return;
	}

	const FVector OldLocation = PawnOwner->GetActorLocation();
	const FRotator OldRotation = PawnOwner->GetActorRotation();

	// @TODO - We calc input only when player is in
	if (true)
	{
		// XYZ === Throttle, Steering, Rise == Forwards, Sidewards, Upwards
		FVector X, Y, Z;
		GetAxes(OldRotation, X, Y, Z);

		// Translate motor location to world coordinates
		FVector MotorLocationWorld = OldLocation + OldRotation.RotateVector(MotorLocation);

		// Minor rotation affects Yaw only
		FVector ForceRotation = Z * TurnTorqueFactor * SteeringInput * DeltaTime;
		UpdatedComponent->SetPhysicsAngularVelocity(ForceRotation, true);

		// The magic happens here
		FVector ForceApplication = FVector::ZeroVector;

		if (ThrottleInput > 0)
		{
			ForceApplication = (ThrustForceFactor * Mass) * ThrottleInput * X * DeltaTime;
		}
		else
		{
			ForceApplication = (ReverseForceFactor * Mass) * ThrottleInput * X * DeltaTime;
		}

		ForceApplication = ForceApplication.RotateAngleAxis(MaxTurnAngle * -SteeringInput, FVector(0.0f, 0.0f, 1.0f));

		// Add forces and rotations (use velocity instead torque/force)
		UpdatedComponent->AddForceAtLocation(ForceApplication, MotorLocationWorld);
	}
}

void UShipVehicleMovementComponent::GetAxes(FRotator A, FVector& X, FVector& Y, FVector& Z)
{
	FRotationMatrix R(A);
	R.GetScaledAxes(X, Y, Z);
}
