// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Components/SceneComponent.h"

#include "ArmComponent.generated.h"

/**
 * This component maintains an endpoint at a fixed distance from the parent.
 */
UCLASS(ClassGroup=Camera, meta=(BlueprintSpawnableComponent), hideCategories = (Mobility))
class TPCE_API UArmComponent : public USceneComponent
{
	GENERATED_BODY()

public:

	/** The name of the socket at the end of the spring arm (looking back towards the spring arm origin) */
	static const FName SocketName;

	UArmComponent();

private:

	/** Previous Target Location */
	FVector PreviousTargetLocation;

	/** Previous smoothed location between the current and the target location */
	FVector PreviousSmoothedLocation;

	/** Previous smoothed rotation between the current and the target rotation */
	FRotator PreviousSmoothedRotation;

	/** Cached component-space socket location */
	FVector RelativeSocketLocation;

	/** Cached component-space socket rotation */
	FQuat RelativeSocketRotation;

	/**
	 * Draws markers at the target (in green) and the lagged position (in yellow).
	 * A line is drawn between the two locations, in green normally but in red if the distance to the lag target has been clamped (by TargetLagMaxDistance).
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Camera Boom|Lag", AdvancedDisplay)
	uint32 bDrawDebugMarkers : 1;

	void UpdateRelativeSocketCoordinates(const FRotator& DesiredRot, const FVector& DesiredLoc);

protected:

	virtual void OnRegister() override;

	/** Return an adjusted arm location to be used as the final location. Implementations may take several things into consideration, such as collision, for example. */
	virtual FVector CalcTargetArm(const FVector& Origin, const FRotator& Rotation, float DeltaTime);

	/** Interpolation method used for target lag */
	virtual FVector VInterpTo(const FVector& Current, const FVector& Target, float DeltaTime, float InterpSpeed);

	/** Interpolation method used for target rotation lag */
	virtual FRotator RInterpTo(const FRotator& Current, const FRotator& Target, float DeltaTime, float InterpSpeed);

public:

	/** Normal length of the arm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Boom")
	float TargetArmLength;

	/** Offset at start of the arm, applied in world space. Use this if you want a world-space offset from the parent component instead of the usual relative-space offset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Boom")
	FVector TargetOffset;

	/**
	 * If this component is placed on a pawn, should it use the view/control rotation of the pawn where possible?
	 * When disabled, the component will revert to using the stored RelativeRotation of the component.
	 * Note that this component itself does not rotate, but instead maintains its relative rotation to its parent as normal,
	 * and just repositions and rotates its children as desired by the inherited rotation settings. Use GetTargetRotation()
	 * if you want the rotation target based on all the settings (UsePawnControlRotation, InheritPitch, etc).
	 *
	 * @see GetTargetRotation(), APawn::GetViewRotation()
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Boom")
	bool bUsePawnControlRotation;

	/** Should we inherit pitch from parent component. Does nothing if using Absolute Rotation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Boom")
	bool bInheritPitch;

	/** Should we inherit yaw from parent component. Does nothing if using Absolute Rotation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Boom")
	bool bInheritYaw;

	/** Should we inherit roll from parent component. Does nothing if using Absolute Rotation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Boom")
	bool bInheritRoll;

	/**
	 * If true, arm lags behind target position to smooth its movement.
	 * @see TargetLagSpeed
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Lag")
	bool bEnableTargetLag;

	/**
	 * If true, Target lags behind target rotation to smooth its movement.
	 * @see TargetRotationLagSpeed
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Lag")
	bool bEnableTargetRotationLag;

	/**
	 * If bUseTargetLagSubstepping is true, sub-step Target damping so that it handles fluctuating frame rates well (though this comes at a cost).
	 * @see TargetLagMaxTimeStep
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Lag", AdvancedDisplay)
	bool bUseTargetLagSubstepping;

	/** If bEnableTargetLag is true, controls how quickly Target reaches target position. Low values are slower (more lag), high values are faster (less lag), while zero is instant (no lag). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Lag", meta = (editcondition = "bEnableTargetLag", ClampMin = "0.0", ClampMax = "1000.0", UIMin = "0.0", UIMax = "1000.0"))
	float TargetLagSpeed;

	/** If bEnableTargetRotationLag is true, controls how quickly Target reaches target position. Low values are slower (more lag), high values are faster (less lag), while zero is instant (no lag). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Lag", meta = (editcondition = "bEnableTargetRotationLag", ClampMin = "0.0", ClampMax = "1000.0", UIMin = "0.0", UIMax = "1000.0"))
	float TargetRotationLagSpeed;

	/** Max time step used when sub-stepping Target lag. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Lag", AdvancedDisplay, meta = (editcondition = "bUseTargetLagSubstepping", ClampMin = "0.005", ClampMax = "0.5", UIMin = "0.005", UIMax = "0.5"))
	float TargetLagMaxTimeStep;

	/** Max distance the Target target may lag behind the current location. If set to zero, no max distance is enforced. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Lag", meta = (editcondition = "bEnableTargetLag", ClampMin = "0.0", UIMin = "0.0"))
	float TargetLagMaxDistance;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void ApplyWorldOffset(const FVector& InOffset, bool bWorldShift) override;
	virtual bool HasAnySockets() const override;
	virtual FTransform GetSocketTransform(FName InSocketName, ERelativeTransformSpace TransformSpace = RTS_World) const override;
	virtual void QuerySupportedSockets(TArray<FComponentSocketDescription>& OutSockets) const override;

	/**
	 * Get the target rotation we inherit, used as the base target for the boom rotation.
	 * This is derived from attachment to our parent and considering the UsePawnControlRotation and absolute rotation flags.
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera Boom|Target")
	FRotator GetTargetRotation() const;

	/** Get the target location considering offset and plane constraints. */
	UFUNCTION(BlueprintCallable, Category = "Camera Boom|Target")
	FVector GetTargetLocation() const;
};
