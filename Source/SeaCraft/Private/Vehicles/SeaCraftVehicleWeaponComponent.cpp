// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

#include "SeaCraft.h"

USeaCraftVehicleWeaponComponent::USeaCraftVehicleWeaponComponent(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	bWantsToFire = false;
	CurrentState = EVWeaponState::Idle;

	LastActiveTurretBarrel = 0;

	CurrentAmmo = 0;
	TimeBetweenShots = 0.2f;
	LastFireTime = 0.0f;
}

//////////////////////////////////////////////////////////////////////////
// Input

void USeaCraftVehicleWeaponComponent::StartFire()
{
	AActor* Owner = GetOwner();
	if (Owner == NULL)
	{
		return;
	}

	if (Owner->Role < ROLE_Authority)
	{
		ServerStartFire();
	}

	if (!bWantsToFire)
	{
		bWantsToFire = true;
		DetermineWeaponState();
	}
}

void USeaCraftVehicleWeaponComponent::StopFire()
{
	AActor* Owner = GetOwner();
	if (Owner == NULL)
	{
		return;
	}

	if (Owner->Role < ROLE_Authority)
	{
		ServerStopFire();
	}

	if (bWantsToFire)
	{
		bWantsToFire = false;
		DetermineWeaponState();
	}
}

bool USeaCraftVehicleWeaponComponent::ServerStartFire_Validate()
{
	return true;
}

void USeaCraftVehicleWeaponComponent::ServerStartFire_Implementation()
{
	StartFire();
}

bool USeaCraftVehicleWeaponComponent::ServerStopFire_Validate()
{
	return true;
}

void USeaCraftVehicleWeaponComponent::ServerStopFire_Implementation()
{
	StopFire();
}

//////////////////////////////////////////////////////////////////////////
// Control

bool USeaCraftVehicleWeaponComponent::CanFire() const
{
	// @TODO Check fire conditions
	// 1. check we have alive vehicle
	// 2. check we're idle or firing (else we can't fire)
	// 3. check we have no pending reload

	return true;
}


//////////////////////////////////////////////////////////////////////////
// Ammo

void USeaCraftVehicleWeaponComponent::GiveAmmo(int AddAmount)
{
	// @TODO Check max ammo capacity reached
	CurrentAmmo += AddAmount;
}

void USeaCraftVehicleWeaponComponent::UseAmmo()
{
	if (!HasInfiniteAmmo())
	{
		CurrentAmmo--;
	}
}


//////////////////////////////////////////////////////////////////////////
// Weapon usage

void USeaCraftVehicleWeaponComponent::HandleFiring()
{
	APawn* MyPawn = Cast<APawn>(GetOwner());

	if ((CurrentAmmo > 0 || HasInfiniteAmmo()) && CanFire())
	{
		if (MyPawn && MyPawn->IsLocallyControlled())
		{
			FireWeapon();

			UseAmmo();

			// Change barrel
			LastActiveTurretBarrel++;
			if (LastActiveTurretBarrel >= TurretSockets.Num())
			{
				LastActiveTurretBarrel = 0;
			}

			// @TODO Send vehicle event to trigger effects
		}
	}
	else if (MyPawn && MyPawn->IsLocallyControlled())
	{
		if (GetCurrentAmmo() == 0 && !bRefiring)
		{
			// TODO Notify out of ammo
		}
	}

	if (MyPawn && MyPawn->IsLocallyControlled())
	{
		// Local client will notify server
		if (MyPawn->Role < ROLE_Authority)
		{
			ServerHandleFiring();
		}

		// Setup refire timer
		bRefiring = (CurrentState == EVWeaponState::Firing && TimeBetweenShots > 0.0f);
		if (bRefiring)
		{
			MyPawn->GetWorldTimerManager().SetTimer(this, &USeaCraftVehicleWeaponComponent::HandleFiring, TimeBetweenShots, false);
		}
	}

	LastFireTime = GetWorld()->GetTimeSeconds();
}

bool USeaCraftVehicleWeaponComponent::ServerHandleFiring_Validate()
{
	return true;
}

void USeaCraftVehicleWeaponComponent::ServerHandleFiring_Implementation()
{
	HandleFiring();

	// @TODO Check we should update ammo
}

void USeaCraftVehicleWeaponComponent::SetWeaponState(EVWeaponState::Type NewState)
{
	const EVWeaponState::Type PrevState = CurrentState;

	if (PrevState == EVWeaponState::Firing && NewState != EVWeaponState::Firing)
	{
		OnBurstFinished();
	}

	CurrentState = NewState;

	if (PrevState != EVWeaponState::Firing && NewState == EVWeaponState::Firing)
	{
		OnBurstStarted();
	}
}

void USeaCraftVehicleWeaponComponent::DetermineWeaponState()
{
	EVWeaponState::Type NewState = EVWeaponState::Idle;

	if ((bWantsToFire == true) && (CanFire() == true))
	{
		NewState = EVWeaponState::Firing;
	}

	SetWeaponState(NewState);
}

void USeaCraftVehicleWeaponComponent::OnBurstStarted()
{
	APawn* MyPawn = Cast<APawn>(GetOwner());
	if (MyPawn == NULL)
	{
		return;
	}

	// Start firing, can be delayed to satisfy TimeBetweenShots
	const float GameTime = GetWorld()->GetTimeSeconds();
	if (LastFireTime > 0 && TimeBetweenShots > 0.0f &&
		LastFireTime + TimeBetweenShots > GameTime)
	{
		MyPawn->GetWorldTimerManager().SetTimer(this, &USeaCraftVehicleWeaponComponent::HandleFiring, LastFireTime + TimeBetweenShots - GameTime, false);
	}
	else
	{
		HandleFiring();
	}
}

void USeaCraftVehicleWeaponComponent::OnBurstFinished()
{
	APawn* MyPawn = Cast<APawn>(GetOwner());
	if (MyPawn == NULL)
	{
		return;
	}

	MyPawn->GetWorldTimerManager().ClearTimer(this, &USeaCraftVehicleWeaponComponent::HandleFiring);
	bRefiring = false;
}


//////////////////////////////////////////////////////////////////////////
// Weapon usage helpers

FVector USeaCraftVehicleWeaponComponent::GetCameraAim() const
{
	APawn* MyPawn = Cast<APawn>(GetOwner());
	ASeaCraftPlayerController* const PlayerController = MyPawn ? Cast<ASeaCraftPlayerController>(MyPawn->Controller) : NULL;
	FVector FinalAim = FVector::ZeroVector;

	if (PlayerController)
	{
		FVector CamLoc;
		FRotator CamRot;
		PlayerController->GetPlayerViewPoint(CamLoc, CamRot);
		FinalAim = CamRot.Vector();
	}
	else if (MyPawn)
	{
		FinalAim = MyPawn->GetBaseAimRotation().Vector();
	}

	return FinalAim;
}

FVector USeaCraftVehicleWeaponComponent::GetAdjustedAim() const
{
	FVector FinalAim = FVector::ZeroVector;

	// We're using socket rotation as targeting vector
	FinalAim = GetMuzzleDirection();

	return FinalAim;
}

FVector USeaCraftVehicleWeaponComponent::GetCameraDamageStartLocation(const FVector& AimDir) const
{
	APawn* MyPawn = Cast<APawn>(GetOwner());
	ASeaCraftPlayerController* PC = MyPawn ? Cast<ASeaCraftPlayerController>(MyPawn->Controller) : NULL;
	FVector OutStartTrace = FVector::ZeroVector;

	if (PC)
	{
		// use player's camera
		FRotator UnusedRot;
		PC->GetPlayerViewPoint(OutStartTrace, UnusedRot);

		// Adjust trace so there is nothing blocking the ray between the camera and the pawn, and calculate distance from adjusted start
		OutStartTrace = OutStartTrace + AimDir * ((MyPawn->GetActorLocation() - OutStartTrace) | AimDir);
	}

	return OutStartTrace;
}

FVector USeaCraftVehicleWeaponComponent::GetMuzzleLocation() const
{
	ASeaCraftVehicle* MyVehicle = Cast<ASeaCraftVehicle>(GetOwner());
	if (MyVehicle)
	{
		return MyVehicle->VehicleMesh->GetSocketLocation(GetLastActiveTurretBarrel());
	}
	
	return FVector::ZeroVector;
}

FVector USeaCraftVehicleWeaponComponent::GetMuzzleDirection() const
{
	ASeaCraftVehicle* MyVehicle = Cast<ASeaCraftVehicle>(GetOwner());
	if (MyVehicle)
	{
		return MyVehicle->VehicleMesh->GetSocketRotation(GetLastActiveTurretBarrel()).Vector();
	}
	
	return FVector::UpVector;
}

FHitResult USeaCraftVehicleWeaponComponent::WeaponTrace(const FVector& StartTrace, const FVector& EndTrace) const
{
	static FName WeaponFireTag = FName(TEXT("WeaponTrace"));

	// Perform trace to retrieve hit info
	FCollisionQueryParams TraceParams(WeaponFireTag, true, GetOwner());
	TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = true;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingle(Hit, StartTrace, EndTrace, COLLISION_WEAPON, TraceParams);

	return Hit;
}


//////////////////////////////////////////////////////////////////////////
// Reading data

EVWeaponState::Type USeaCraftVehicleWeaponComponent::GetCurrentState() const
{
	return CurrentState;
}

FName USeaCraftVehicleWeaponComponent::GetWeaponID() const
{
	return WeaponID;
}

FName USeaCraftVehicleWeaponComponent::GetGroupID() const
{
	return GroupID;
}

int32 USeaCraftVehicleWeaponComponent::GetCurrentAmmo() const
{
	return CurrentAmmo;
}

bool USeaCraftVehicleWeaponComponent::HasInfiniteAmmo() const
{
	return bInfiniteAmmo;
}

FName USeaCraftVehicleWeaponComponent::GetLastActiveTurretBarrel() const
{
	if (LastActiveTurretBarrel >= TurretSockets.Num())
	{
		return TEXT("None");
	}

	return TurretSockets[LastActiveTurretBarrel];
}

void USeaCraftVehicleWeaponComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(USeaCraftVehicleWeaponComponent, CurrentAmmo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(USeaCraftVehicleWeaponComponent, LastActiveTurretBarrel, COND_SimulatedOnly);
}
