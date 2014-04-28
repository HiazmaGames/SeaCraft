// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

#include "SeaCraft.h"

FName ASeaCraftVehicle::VehicleMeshComponentName(TEXT("SeaCraftVehicleMesh"));

ASeaCraftVehicle::ASeaCraftVehicle(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	VehicleMesh = PCIP.CreateDefaultSubobject<USkeletalMeshComponent>(this, VehicleMeshComponentName);
	static FName CollisionProfileName(TEXT("Vehicle"));
	VehicleMesh->SetCollisionProfileName(CollisionProfileName);
	VehicleMesh->BodyInstance.bSimulatePhysics = true;
	VehicleMesh->BodyInstance.bNotifyRigidBodyCollision = true;
	VehicleMesh->BodyInstance.bUseCCD = true;
	VehicleMesh->bBlendPhysics = true;
	VehicleMesh->bUseSingleBodyPhysics = true;
	VehicleMesh->bGenerateOverlapEvents = true;
	RootComponent = VehicleMesh;

	bWantsToFire = false;
}

void ASeaCraftVehicle::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Activate first weapon group
	TArray<USeaCraftVehicleWeaponComponent*> Components;
	this->GetComponents<USeaCraftVehicleWeaponComponent>(Components);

	for (int i = 0; i < Components.Num(); i++)
	{
		SetWeaponGroup(Components[i]->GetGroupID());
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
// Weapon control

void ASeaCraftVehicle::SetWeaponGroup(FName WeaponGroup)
{
	// Stop current weapons before changing group
	StopWeaponFire();

	CurrentWeaponGroup = WeaponGroup;
	CurrentWeapons.Empty();

	TArray<USeaCraftVehicleWeaponComponent*> Components;
	this->GetComponents<USeaCraftVehicleWeaponComponent>(Components);

	for (int i = 0; i < Components.Num(); i++)
	{
		if (Components[i]->GetGroupID() == CurrentWeaponGroup)
		{
			CurrentWeapons.Add(Components[i]);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// Weapon usage

void ASeaCraftVehicle::StartWeaponFire()
{
	if (!bWantsToFire)
	{
		bWantsToFire = true;
		
		for (USeaCraftVehicleWeaponComponent* Weapon : CurrentWeapons)
		{
			Weapon->StartFire();
		}
	}
}

void ASeaCraftVehicle::StopWeaponFire()
{
	if (bWantsToFire)
	{
		bWantsToFire = false;
		
		for (USeaCraftVehicleWeaponComponent* Weapon : CurrentWeapons)
		{
			Weapon->StopFire();
		}
	}
}

bool ASeaCraftVehicle::CanFire() const
{
	// @TODO Check vehicle health, weapon distance and state, obstacles, etc.
	return true;
}


//////////////////////////////////////////////////////////////////////////
// Input

void ASeaCraftVehicle::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	check(InputComponent);

	InputComponent->BindAction("StartFire", IE_Pressed, this, &ASeaCraftVehicle::OnStartFire);
	InputComponent->BindAction("StopFire", IE_Released, this, &ASeaCraftVehicle::OnStopFire);

	InputComponent->BindAction("NextWeapon", IE_Pressed, this, &ASeaCraftVehicle::OnNextWeapon);
	InputComponent->BindAction("PrevWeapon", IE_Pressed, this, &ASeaCraftVehicle::OnPrevWeapon);

	UE_LOG(LogWeapon, Warning, TEXT("WEAPON: Input action binded"));
}

void ASeaCraftVehicle::OnStartFire()
{
	ASeaCraftPlayerController* MyPC = Cast<ASeaCraftPlayerController>(Controller);
	if (MyPC && MyPC->IsGameInputAllowed())
	{
		StartWeaponFire();
	}
}

void ASeaCraftVehicle::OnStopFire()
{
	StopWeaponFire();
}

void ASeaCraftVehicle::OnNextWeapon()
{
	// @TODO OnNextWeapon
}

void ASeaCraftVehicle::OnPrevWeapon()
{
	// @TODO OnPrevWeapon
}
