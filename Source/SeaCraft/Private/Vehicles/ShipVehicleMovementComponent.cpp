// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

#include "SeaCraft.h"

UShipVehicleMovementComponent::UShipVehicleMovementComponent(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	Mass = 1500.0f;
	OceanLevel = 0.0f;
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

	// Wave reaction defaults
	ThrustForceFactor = 40000.0f;
	ReverseForceFactor = 10000.0f;
	
	TurnTorqueFactor = 40.0f;
	MaxTurnAngle = 10.0f;

	TensionTorqueFactor = 0.01f;
	TensionTorqueRollFactor = 10000000.0f;
	TensionTorquePitchFactor = 30.0f;	// Pitch is mostly controlled by tesntion dots
	TensionDepthFactor = 10.0f;

	AltitudeFactor = 10.0f;
	VelocityFactor = 1.0f;

	MaxAltitudeForce = 600.0f;
	MinimumAltituteToReact = 30.0f;

	bDebugTensionDots = false;
	bUseMetacentricForces = false;

	LongitudinalMetacenter = FVector(0.0f, 0.0f, 150.0f);
	TransverseMetacenter = FVector(0.0, 0.0, 50.0);
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
		ServerUpdateState(SteeringInput, ThrottleInput, BrakeInput, HandbrakeInput, GetCurrentGear());
	}
	else
	{
		// use replicated values for remote pawns
		SteeringInput = ReplicatedState.SteeringInput;
		ThrottleInput = ReplicatedState.ThrottleInput;
		BrakeInput = ReplicatedState.BrakeInput;
		HandbrakeInput = ReplicatedState.HandbrakeInput;
		SetTargetGear(ReplicatedState.CurrentGear, true);
	}
}

bool UShipVehicleMovementComponent::ServerUpdateState_Validate(float InSteeringInput, float InThrottleInput, float InBrakeInput, float InHandbrakeInput, int32 InCurrentGear)
{
	return true;
}

void UShipVehicleMovementComponent::ServerUpdateState_Implementation(float InSteeringInput, float InThrottleInput, float InBrakeInput, float InHandbrakeInput, int32 InCurrentGear)
{
	SteeringInput = InSteeringInput;
	ThrottleInput = InThrottleInput;
	BrakeInput = InBrakeInput;
	HandbrakeInput = InHandbrakeInput;

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
}

float UShipVehicleMovementComponent::CalcSteeringInput()
{
	return RawSteeringInput;
}

float UShipVehicleMovementComponent::CalcBrakeInput()
{
	const float ForwardSpeed = GetForwardSpeed();

	float NewBrakeInput = 0.0f;

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
	return RawThrottleInput;
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
}

void UShipVehicleMovementComponent::SetUseAutoGears(bool bUseAuto)
{
#if WITH_PHYSX
	//
	// PHYSX GEAR
	//
#endif
}

float UShipVehicleMovementComponent::GetForwardSpeed() const
{
#if WITH_PHYSX
	//
	// COMPUTE FORWARD SPEED
	//
#endif

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

	return 0;
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

bool UShipVehicleMovementComponent::GetUseAutoGears() const
{
#if WITH_PHYSX
	//
	// PHYSX
	//
#endif

	return false;
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
	PerformWaveReaction(DeltaTime);
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
		FVector ForceApplication = (ThrustForceFactor * Mass) * ThrottleInput * X * DeltaTime;
		ForceApplication = ForceApplication.RotateAngleAxis(MaxTurnAngle * -SteeringInput, FVector(0.0f, 0.0f, 1.0f));

		// Add forces and rotations (use velocity instead torque/force)
		UpdatedComponent->AddForceAtLocation(ForceApplication, MotorLocationWorld);
	}
}

void UShipVehicleMovementComponent::PerformWaveReaction(float DeltaTime)
{
	if (!UpdatedComponent)
	{
		return;
	}

	const FVector OldLocation = PawnOwner->GetActorLocation();
	const FRotator OldRotation = PawnOwner->GetActorRotation();
	const FVector OldLinearVelocity = UpdatedComponent->GetPhysicsLinearVelocity();
	const FVector OldAngularVelocity = UpdatedComponent->GetPhysicsAngularVelocity();
	const FVector OldCenterOfMassWorld = OldLocation + OldRotation.RotateVector(COMOffset);

	// XYZ === Throttle, Steering, Rise == Forwards, Sidewards, Upwards
	FVector X, Y, Z;
	GetAxes(OldRotation, X, Y, Z);

	// Process tension dots and get torque from wind/waves
	for (FVector TensionDot : TensionDots)
	{
		// Translate point to world coordinates
		FVector TensionDotDisplaced = OldRotation.RotateVector(TensionDot + COMOffset);
		FVector TensionDotWorld = OldLocation + TensionDotDisplaced;

		// Get point depth
		float DotAltitude = GetAltitude(TensionDotWorld);
		//TensionedAltitude += DotAltitude;

		// Don't process dots above water
		if (DotAltitude > 0)
		{
			continue;
		}
		
		// Surface normal (not modified!)
		FVector DotSurfaceNormal = GetSurfaceNormal(TensionDotWorld) * GetSurfaceWavesNum();
		// Modify normal with real Z value and normalize it
		DotSurfaceNormal.Z = GetOceanLevel(TensionDotWorld);
		DotSurfaceNormal.Normalize();

		// Point dynamic pressure [http://en.wikipedia.org/wiki/Dynamic_pressure]
		// rho = 1.03f for ocean water
		FVector WaveVelocity = GetWaveVelocity(TensionDotWorld);
		float DotQ = 0.515f * FMath::Square(WaveVelocity.Size());
		FVector WaveForce = FVector(0.0,0.0,1.0) * DotQ /* DotSurfaceNormal*/ * (-DotAltitude) * TensionDepthFactor;
		
		// We don't want Z to be affected by DotQ
		WaveForce.Z /= DotQ;

		// Scale to DeltaTime to break FPS addiction
		WaveForce *= DeltaTime;

		UpdatedComponent->AddForceAtLocation(WaveForce * Mass, TensionDotWorld);
	}

	// Static metacentric forces (can be useful on small waves)
	if (bUseMetacentricForces)
	{
		FVector TensionTorqueResult = FVector(0.0f, 0.0f, 0.0f);

		// Calc recovering torque (transverce)
		FRotator RollRot = FRotator(0.0f, 0.0f, 0.0f);
		RollRot.Roll = OldRotation.Roll;
		FVector MetacenterDisplaced = RollRot.RotateVector(TransverseMetacenter + COMOffset);
		TensionTorqueResult += X * FVector::DotProduct((TransverseMetacenter - MetacenterDisplaced), 
			FVector(0.0f, -1.0f, 0.0f)) * TensionTorqueRollFactor;

		// Calc recovering torque (longitude)
		FRotator PitchRot = FRotator(0.0f, 0.0f, 0.0f);
		PitchRot.Pitch = OldRotation.Pitch;
		MetacenterDisplaced = PitchRot.RotateVector(LongitudinalMetacenter + COMOffset);
		TensionTorqueResult += Y * FVector::DotProduct((LongitudinalMetacenter - MetacenterDisplaced), 
			FVector(1.0f, 0.0f, 0.0f)) * TensionTorquePitchFactor;

		// Apply torque
		TensionTorqueResult *= DeltaTime;
		UpdatedComponent->AddTorque(TensionTorqueResult);
	}
}

void UShipVehicleMovementComponent::GetAxes(FRotator A, FVector& X, FVector& Y, FVector& Z)
{
	FRotationMatrix R(A);
	R.GetScaledAxes(X, Y, Z);
}

float UShipVehicleMovementComponent::GetOceanLevel(FVector& WorldLocation) const
{
	ASeaCraftGameState* const MyGameState = Cast<ASeaCraftGameState>(GetWorld()->GameState);
	if (MyGameState)
	{
		return MyGameState->GetOceanLevelAtLocation(WorldLocation);
	}

	return OceanLevel;
}

float UShipVehicleMovementComponent::GetAltitude(FVector& WorldLocation) const
{
	return WorldLocation.Z - GetOceanLevel(WorldLocation) /*+ COMOffset.Z*/;
}

FLinearColor UShipVehicleMovementComponent::GetSurfaceNormal(FVector& WorldLocation) const
{
	ASeaCraftGameState* const MyGameState = Cast<ASeaCraftGameState>(GetWorld()->GameState);
	if (MyGameState)
	{
		return MyGameState->GetOceanSurfaceNormal(WorldLocation);
	}

	return FVector();
}

int32 UShipVehicleMovementComponent::GetSurfaceWavesNum() const
{
	ASeaCraftGameState* const MyGameState = Cast<ASeaCraftGameState>(GetWorld()->GameState);
	if (MyGameState)
	{
		return MyGameState->GetOceanWavesNum();
	}

	return 0;
}

FVector UShipVehicleMovementComponent::GetWaveVelocity(FVector& WorldLocation) const
{
	ASeaCraftGameState* const MyGameState = Cast<ASeaCraftGameState>(GetWorld()->GameState);
	if (MyGameState)
	{
		return MyGameState->GetOceanWaveVelocity(WorldLocation);
	}

	return FVector();
}
