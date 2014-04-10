// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

#pragma once

#include "OceanStateActor.generated.h"

/**
 * Calculates wave height based on SK_Ocean shader approach
 */
UCLASS(ClassGroup = Ocean, Blueprintable, BlueprintType)
class AOceanStateActor : public AActor
{
	GENERATED_UCLASS_BODY()

	/** A UBillboardComponent to hold Icon sprite */
	UPROPERTY()
	TSubobjectPtr<UBillboardComponent> SpriteComponent;

	/** Icon sprite */
	UTexture2D* SpriteTexture;

	/** Set global zero ocean level (ocean mesh Z location) */
	UFUNCTION(BlueprintCallable, Category = "World|Ocean")
	void SetGlobalOceanLevel(float OceanLevel);

	/** Set time for waves movement calculation */
	UFUNCTION(BlueprintCallable, Category = "World|Ocean")
	void SetWaveHeightPannerTime(float Time);

	/** Global zero ocean level (ocean mesh Z location) */
	UFUNCTION(BlueprintCallable, Category = "World|Ocean")
	float GetGlobalOceanLevel() const;

	/** SKOcean: WorldPositionDivider param */
	UFUNCTION(BlueprintCallable, Category = "World|Ocean")
	float GetWorldPositionDivider() const;

	/** SKOcean: WaveUVDivider param */
	UFUNCTION(BlueprintCallable, Category = "World|Ocean")
	float GetWaveUVDivider() const;

	/** SKOcean: WaveHeightPannerX param */
	UFUNCTION(BlueprintCallable, Category = "World|Ocean")
	float GetWaveHeightPannerX() const;

	/** SKOcean: WaveHeightPannerY param */
	UFUNCTION(BlueprintCallable, Category = "World|Ocean")
	float GetWaveHeightPannerY() const;

	/** SKOcean: WaveHeight param */
	UFUNCTION(BlueprintCallable, Category = "World|Ocean")
	float GetWaveHeight() const;

	/** SKOcean: WaterHeight param */
	UFUNCTION(BlueprintCallable, Category = "World|Ocean")
	float GetWaterHeight() const;

	/** Normalmap which will be used to determite the wave height */
	UPROPERTY(EditAnywhere, Category = OceanSetup)
	class UTexture2D* OceanHeightMap;

	/** Get ocean level at desired position */
	float GetOceanLevelAtLocation(FVector& Location) const;

	/** Get ocean surface normal at desired location */
	FLinearColor GetOceanSurfaceNormal(FVector& Location) const;

	/** Wave velocity is determined by UV change rate */
	FVector GetOceanWaveVelocity(FVector& Location) const;

	// Begin AActor interface
	virtual void PostInitializeComponents() OVERRIDE;
	// End AActor interface

protected:
	//
	// Cached data to prevent large memory operations each frame
	//
	
	/** Flat to keep "uncompression" state */
	bool bRawDataReady;

	/** Decompressed PNG image */
	TArray<uint8> HeightMapRawData;

	/** Return pixel color from loaded raw data */
	FColor GetHeighMapPixelColor(float U, float V) const;

protected:
	//
	// Wave height calculation parameters
	//
	UPROPERTY(EditAnywhere, Category = OceanSetup)
	float GlobalOceanLevel;

	UPROPERTY(EditAnywhere, Category = OceanSetup)
	float WorldPositionDivider;

	UPROPERTY(EditAnywhere, Category = OceanSetup)
	float WaveUVDivider;

	UPROPERTY(EditAnywhere, Category = OceanSetup)
	float WaveHeightPannerX;

	UPROPERTY(EditAnywhere, Category = OceanSetup)
	float WaveHeightPannerY;

	UPROPERTY(EditAnywhere, Category = OceanSetup)
	float WaveHeightPannerTime;

	UPROPERTY(EditAnywhere, Category = OceanSetup)
	float WaveHeight;

	UPROPERTY(EditAnywhere, Category = OceanSetup)
	float WaterHeight;
};
