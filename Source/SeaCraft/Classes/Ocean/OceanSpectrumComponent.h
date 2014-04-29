// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#pragma once

#include "OceanSpectrumComponent.generated.h"

#define PAD16(n) (((n)+15)/16*16)

USTRUCT()
struct FOceanData
{
	GENERATED_USTRUCT_BODY()

	/** The size of displacement map. Must be power of 2. */
	UPROPERTY(EditDefaultsOnly, Category = Ocean)
	int32 DispMapDimension;

	/** The side length (world space) of square patch. Typical value is 1000 ~ 2000. */
	UPROPERTY(EditDefaultsOnly, Category = Ocean)
	float PatchLength;

	/** Adjust the time interval for simulation (controls the simulation speed) */
	UPROPERTY(EditDefaultsOnly, Category = Ocean)
	float TimeScale;

	/** Amplitude for transverse wave. Around 1.0 (not the world space height). */
	UPROPERTY(EditDefaultsOnly, Category = Ocean)
	float WaveAmplitude;

	/** Wind direction. Normalization not required */
	UPROPERTY(EditDefaultsOnly, Category = Ocean)
	FVector2D WindDirection;

	/** The bigger the wind speed, the larger scale of wave crest. But the wave scale can be no larger than PatchLength. 
		Around 100 ~ 1000 */
	UPROPERTY(EditDefaultsOnly, Category = Ocean)
	float WindSpeed;

	/** This value damps out the waves against the wind direction. Smaller value means higher wind dependency. */
	UPROPERTY(EditDefaultsOnly, Category = Ocean)
	float WindDependency;

	/** The amplitude for longitudinal wave. Higher value creates pointy crests. Must be positive. */
	UPROPERTY(EditDefaultsOnly, Category = Ocean)
	float ChoppyScale;

	/** defaults */
	FOceanData()
	{
		DispMapDimension = 512;
		PatchLength = 2000.0f;
		TimeScale = 0.8f;
		WaveAmplitude = 0.35f;
		WindDirection = FVector2D(0.8f, 0.6f);
		WindSpeed = 600.0f;
		WindDependency = 0.07f;
		ChoppyScale = 1.3f;
	}
};

/**
 * Renders normals and heightmap from Phillips spectrum
 */
UCLASS(HeaderGroup = Component, ClassGroup = Ocean, editinlinenew, meta = (BlueprintSpawnableComponent))
class UOceanSpectrumComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

	/** Render target for normal map that can be used by the editor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OceanSpectrum)
	class UTextureRenderTarget2D* NormalsTarget;

	/** Render target for height map that can be used by the editor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OceanSpectrum)
	class UTextureRenderTarget2D* HeightTarget;

protected:
	/** Ocean spectrum data */
	UPROPERTY(EditDefaultsOnly, Category = Config)
	FOceanData OceanConfig;

	/** Initialize the vector field */
	void InitHeightMap(FOceanData& Params, FVector2D* out_h0, float* out_omega);

public:
	// Begin UActorComponent Interface
	virtual void SendRenderTransform_Concurrent() OVERRIDE;
	virtual bool RequiresGameThreadEndOfFrameUpdates() const
	{
		// this method could probably be removed allowing them to run on any thread, but it isn't worth the trouble
		return true;
	}
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) OVERRIDE;
	// End UActorComponent Interface

	// Begin UObject Interface
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) OVERRIDE;
#endif // WITH_EDITOR
	// End UObject Interface

	/** Render the ocean spectrum to the texture */
	void UpdateContent();

	/*static*/ void UpdateDeferredSpectrums(/*FSceneInterface* Scene*/);

	/**
	* Updates the contents of the given ocean spectrum by processing FFT calculations on spectrum.
	* This must be called on the game thread.
	*/
	void UpdateOceanSpectrumContents(class UOceanSpectrumComponent* OceanSpectrumComponent);

};
