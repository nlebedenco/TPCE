// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ArmComponent.h"
#include "UObject/ObjectMacros.h"
#include "Engine/EngineTypes.h"
#include "Components/SceneComponent.h"

#include "ReboundSpringArmComponent.generated.h"

/**
 * This component tries to maintain its children at a fixed distance from the parent,
 * but will retract the children if there is a collision, and spring back when there is no collision.
 *
 * Example: Use as a 'camera boom' to keep the follow camera for a player from passing through the world.
 */
UCLASS(ClassGroup = Camera, meta = (BlueprintSpawnableComponent))
class TPCE_API UReboundSpringArmComponent : public UArmComponent
{
	GENERATED_BODY()

public:

	UReboundSpringArmComponent();

private:

	/** If true, the spring arm will start at full length. */
	UPROPERTY(EditDefaultsOnly, Category = "Camera Boom")
	bool bStartAtFullLength;

	float PreviousTargetArmLength;

protected:

	/** Temporary variables when applying Collision Test displacement to notify if its being applied and by how much */
	bool bIsCameraFixed;
	FVector UnfixedCameraPosition;

	virtual FVector CalcTargetArm(const FVector& Origin, const FRotator& Rotation, float DeltaTime) override;

	FORCEINLINE bool CanStartAtFullLength() const { return bStartAtFullLength; }

	UFUNCTION(BlueprintCallable, Category = "Camera Boom|Lag", meta = (DisplayName = "StartAtFullLength"))
	bool K2_CanStartAtFullLength() const { return bStartAtFullLength; }

public:

	/** Offset at end of the arm; use this instead of the relative offset of the attached component to ensure the line trace works as desired */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	FVector SocketOffset;

	/** Radius of the the query probe sphere in unreal units */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CameraCollision, meta = (editcondition = "bDoCollisionTest"))
	float ProbeSize;

	/** Collision channel of the query probe (defaults to ECC_Camera) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CameraCollision, meta = (editcondition = "bDoCollisionTest"))
	TEnumAsByte<ECollisionChannel> ProbeChannel;

	/** If true, do a collision test using ProbeChannel and ProbeSize to prevent camera clipping into level.  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CameraCollision)
	bool bDoCollisionTest;

	/**
	 * If true, the spring arm will lag to restore its full length.
	 * @see ReboundLagSpeed
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Lag")
	bool bEnableReboundLag;

	/** 
	 * Controls how quickly the spring arm should extend to its final length. Low values are slower (more lag), high values are faster (less lag), while zero is instant (no lag).
	 * @see bEnableReboundLag
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Lag", meta = (editcondition = "bEnableReboundLag", ClampMin = "0.0", ClampMax = "1000.0", UIMin = "0.0", UIMax = "1000.0"))
	float ReboundLagSpeed;

	virtual void OnRegister() override;
	virtual void InitializeComponent() override;

	/** Get the position where the camera should be without applying the Collision Test displacement */
	UFUNCTION(BlueprintCallable, Category = CameraCollision)
	FVector GetUnfixedCameraPosition() const;

	/** Is the Collision Test displacement being applied? */
	UFUNCTION(BlueprintCallable, Category = CameraCollision)
	bool IsCollisionFixApplied() const;
};
