// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#pragma once

#include "OceanSpectrumComponent.generated.h"

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
