// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ArmComponent.h"
#include "UObject/ObjectMacros.h"
#include "Engine/EngineTypes.h"
#include "Components/SceneComponent.h"

#include "TelescopicArmComponent.generated.h"

/**
 * This component tries to maintain its children at a fixed distance from the parent,
 * and can smoothly move the endpoint when the target arm length is modified.
 *
 */
UCLASS(ClassGroup = Camera, meta = (BlueprintSpawnableComponent))
class TPCE_API UTelescopicArmComponent : public UArmComponent
{
	GENERATED_BODY()

public:

	UTelescopicArmComponent();

private:

	float PreviousTargetArmLength;

	/** If true, the telescopic arm will start at full length. */
	UPROPERTY(EditDefaultsOnly, Category = "Camera Boom")
	bool bStartAtFullLength;

protected:

	FORCEINLINE bool CanStartAtFullLength() const { return bStartAtFullLength; }

	UFUNCTION(BlueprintCallable, Category = "Camera Boom", meta = (DisplayName = "StartAtFullLength"))
	bool K2_CanStartAtFullLength() const { return bStartAtFullLength; }

	virtual FVector CalcTargetArm(const FVector& Origin, const FRotator& Rotation, float DeltaTime) override;

public:

	/**
	 * If true, the telescopic arm will lag to its designated length.
	 * @see ResizeLagSpeed
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Lag")
	uint32 bEnableResizeLag : 1;

	/** Controls how quickly the telescopic arm should reach its designated length. Low values are slower (more lag), high values are faster (less lag), while zero is instant (no lag). 
	 * @see bEnableResizeLag
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Lag", meta = (editcondition = "bEnableResizeLag", ClampMin = "0.0", ClampMax = "1000.0", UIMin = "0.0", UIMax = "1000.0"))
	float ResizeLagSpeed;

	virtual void OnRegister() override;
	virtual void InitializeComponent() override;
};
