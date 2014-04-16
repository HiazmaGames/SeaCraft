// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

#include "SeaCraft.h"

AOceanStateActor::AOceanStateActor(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		// A helper class object we use to find target UTexture2D object in resource package
		ConstructorHelpers::FObjectFinderOptional<UTexture2D> NoteTextureObject;

		// Icon sprite category name
		FName ID_Notes;

		// Icon sprite display name
		FText NAME_Notes;

		FConstructorStatics()
			// Use helper class object to find the texture
			// "/Engine/EditorResources/S_Note" is resource path
			: NoteTextureObject(TEXT("/Engine/EditorResources/S_Note"))
			, ID_Notes(TEXT("Notes"))
			, NAME_Notes(NSLOCTEXT("SpriteCategory", "Notes", "Notes"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	// We need a scene component to attach Icon sprite
	TSubobjectPtr<USceneComponent> SceneComponent = PCIP.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneComp"));
	RootComponent = SceneComponent;
	RootComponent->Mobility = EComponentMobility::Static;

#if WITH_EDITORONLY_DATA
	SpriteComponent = PCIP.CreateEditorOnlyDefaultSubobject<UBillboardComponent>(this, TEXT("Sprite"));
	if (SpriteComponent)
	{
		SpriteComponent->Sprite = ConstructorStatics.NoteTextureObject.Get();		// Get the sprite texture from helper class object
		SpriteComponent->SpriteInfo.Category = ConstructorStatics.ID_Notes;			// Assign sprite category name
		SpriteComponent->SpriteInfo.DisplayName = ConstructorStatics.NAME_Notes;	// Assign sprite display name
		SpriteComponent->AttachParent = RootComponent;								// Attach sprite to scene component
		SpriteComponent->Mobility = EComponentMobility::Static;
	}
#endif // WITH_EDITORONLY_DATA

	// Set defaults
	HeightMapWaves = 1;

	WorldPositionDivider = 0.5f;
	WaveUVDivider = 11000.0f;

	WaveHeightPannerX = 0.015f;
	WaveHeightPannerY = 0.01f;

	WaveHeight = 100.0f;
	WaterHeight = 100.0f;

	WaveHeightPannerTime = 0.0f;
}

void AOceanStateActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Load raw data with first function call
	if (OceanHeightMap)
	{
		bRawDataReady = OceanHeightMap->Source.GetMipData(HeightMapRawData, 0);
		UE_LOG(LogOcean, Log, TEXT("Ocean heighmap load status: %d"), (int)bRawDataReady);
	}
	else
	{
		UE_LOG(LogOcean, Warning, TEXT("Heightmap is not set for ocean state!"));
		bRawDataReady = false;
	}
}

float AOceanStateActor::GetOceanLevelAtLocation(FVector& Location) const
{
	// Check that we've set a texture
	if (!OceanHeightMap)
	{
		return 0.0f;
	}

	check(OceanHeightMap->Source.IsValid());

	// Check we have a raw data loaded
	if (!bRawDataReady)
	{
		UE_LOG(LogOcean, Warning, TEXT("Can't load raw data of ocean heightmap!"));
		return 0.0f;
	}

	//
	// SKOcean shader algorythm
	//

	// World UV location
	float WorldUVx = Location.X / WorldPositionDivider / WaveUVDivider;
	float WorldUVy = Location.Y / WorldPositionDivider / WaveUVDivider;

	// Apply panner
	WorldUVx += WaveHeightPannerX * WaveHeightPannerTime;
	WorldUVy += WaveHeightPannerY * WaveHeightPannerTime;

	// Get heightmap color
	const FLinearColor PixelColor = FLinearColor(GetHeighMapPixelColor(WorldUVx, WorldUVy));

	// Calculate wave height
	float OceanLevel = PixelColor.A * WaveHeight - WaterHeight;

	return OceanLevel + GlobalOceanLevel;
}

FLinearColor AOceanStateActor::GetOceanSurfaceNormal(FVector& Location) const
{
	// Check that we've set a texture
	if (!OceanHeightMap)
	{
		return FLinearColor();
	}

	check(OceanHeightMap->Source.IsValid());

	// Check we have a raw data loaded
	if (!bRawDataReady)
	{
		UE_LOG(LogOcean, Warning, TEXT("Can't load raw data of ocean heightmap!"));
		return FLinearColor();
	}

	//
	// SKOcean shader algorythm
	//

	// World UV location
	float WorldUVx = Location.X / WorldPositionDivider / WaveUVDivider;
	float WorldUVy = Location.Y / WorldPositionDivider / WaveUVDivider;

	// Apply panner
	WorldUVx += WaveHeightPannerX * WaveHeightPannerTime;
	WorldUVy += WaveHeightPannerY * WaveHeightPannerTime;

	// Get heightmap color
	return FLinearColor(GetHeighMapPixelColor(WorldUVx, WorldUVy));
}

FVector AOceanStateActor::GetOceanWaveVelocity(FVector& Location) const
{
	FVector WaveVelocity = FVector(WaveHeightPannerX, WaveHeightPannerY, 0.0f);

	// Scale to the world size (in m/sec!)
	WaveVelocity *= WorldPositionDivider * WaveUVDivider / 100.0f;

	return WaveVelocity;
}

int32 AOceanStateActor::GetOceanWavesNum() const
{
	return HeightMapWaves;
}

FColor AOceanStateActor::GetHeighMapPixelColor(float U, float V) const
{
	// Check we have a raw data loaded
	if (!bRawDataReady)
	{
		UE_LOG(LogOcean, Warning, TEXT("Ocean heightmap raw data is not loaded! Pixel is empty."));
		return FColor(0,0,0,0);
	}

	// We are using the source art so grab the original width/height
	const int32 Width = OceanHeightMap->Source.GetSizeX();
	const int32 Height = OceanHeightMap->Source.GetSizeY();
	const bool bUseSRGB = OceanHeightMap->SRGB;

	check(Width > 0 && Height > 0 && HeightMapRawData.Num() > 0);

	// Normalize UV first
	const float NormalizedU = U > 0 ? FMath::Fractional(U) : 1.0 + FMath::Fractional(U);
	const float NormalizedV = V > 0 ? FMath::Fractional(V) : 1.0 + FMath::Fractional(V);

	const int PixelX = NormalizedU * (Width-1) + 1;
	const int PixelY = NormalizedV * (Height-1) + 1;

	// Get color from 
	const FColor* SrcPtr = &((FColor*)(HeightMapRawData.GetTypedData()))[(PixelY - 1) * Width + PixelX - 1];

	return *SrcPtr;
}


void AOceanStateActor::SetGlobalOceanLevel(float OceanLevel)
{
	GlobalOceanLevel = OceanLevel;
}

void AOceanStateActor::SetWaveHeightPannerTime(float Time)
{
	WaveHeightPannerTime = Time;
}


float AOceanStateActor::GetGlobalOceanLevel() const
{
	return GlobalOceanLevel;
}

float AOceanStateActor::GetWorldPositionDivider() const
{
	return WorldPositionDivider;
}

float AOceanStateActor::GetWaveUVDivider() const
{
	return WaveUVDivider;
}

float AOceanStateActor::GetWaveHeightPannerX() const
{
	return WaveHeightPannerX;
}

float AOceanStateActor::GetWaveHeightPannerY() const
{
	return WaveHeightPannerY;
}

float AOceanStateActor::GetWaveHeight() const
{
	return WaveHeight;
}

float AOceanStateActor::GetWaterHeight() const
{
	return WaterHeight;
}