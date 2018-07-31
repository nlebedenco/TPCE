// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UObject/ObjectMacros.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Math/Bounds.h"
#include "ExtraTypes.h"
#include "ExtraMacros.h"

#include "ExtCharacterMovementComponent.generated.h"

class ACharacter;
class AExtCharacter;
class FNetworkPredictionData_Client_Character;

/**
 *
 */
USTRUCT(BlueprintType)
struct FAdaptiveRotationSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FBounds Speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FBounds RotationRateFactor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FBounds RotationRateLimit;
};

/**
 *
 */
USTRUCT(BlueprintType)
struct FPivotTurnSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FBounds AccelerationFactor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FBounds FrictionFactor;
};


/** Extra Movement capabilities, determining available movement options for VSICharacters. */
USTRUCT(BlueprintType)
struct FMovementPropertiesEx
{
	GENERATED_BODY()

	FMovementPropertiesEx();

	/** If true, this Pawn is capable of walking (as opposed to running). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MovementProperties)
	uint32 bCanWalkInsteadOfRun : 1;

	/** If true, this Pawn is capable of sprinting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MovementProperties)
	uint32 bCanSprint : 1;

	/** If true, this Pawn is capable of performing action. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MovementProperties)
	uint32 bCanPerformGenericAction : 1;
};


/**
 * Extended Character Movement component that supports 3 extra movement actions replicated as compressed flags:
 *
 *     - Walk (as opposed to run)
 *     - Sprint
 *     - Generic Action (any action that affects movement such as aiming, skeaning, etc...)
 *
 * Note that MaxAcceleration and GroundFriction are treated as temporaries and potentially overwriten every frame.
 * Users can set a desired max acceleration using the member corresponding to the specific movement mode of choice. ie. MaxFlyAcceleration.
 * WalkFriction can be used to set the desired friction for Walking. Other movement modes work with the friction defined by the physics volume.
 *
 * Note that walk can be active simultaneously to other actions such as jump, crouch, generic action and even sprint.
 *
 * Movement rotation can be interpolated. Turn in place can be delayed but can only emply a constant rotation rate. This is to
 * reduce the impact of network corrections.
 *
 * Also note that MaxWalkSpeedCrouched is not used so any assigned value will be ignored. Set up the appropriate movement setting
 * in the character class instead.
 */
UCLASS()
class TPCE_API UExtCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:

#if WITH_EDITOR
	static const FName NAME_TurnInPlaceTargetYaw_None;
	static const FName NAME_TurnInPlaceTargetYaw_Suspended;
#endif

	static const float AngleTolerance;

	UExtCharacterMovementComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:  // Bitfields

	/** */
	UPROPERTY(BlueprintReadOnly, Category = "Character Movement: PivotTurn", meta = (AllowPrivateAccess = "true"))
	uint32 bIsPivotTurning : 1;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character Movement: PivotTurn")
	uint32 bCanPivotTurn : 1;

	/**
	 * If true Turn in Place Max Distance can be enfored.
	 * Internally used to prevent snapping when the character comes from a different rotation mode or from ragdoll
	 * facing the wrong direction. Once the character is back inside the look threshold this flag becomes true again.
	 */
	uint32 bCanEnforceTurnInPlaceRotationMaxDistance : 1;

	/**
	 * If true Control Rotation Max Distance can be enfored.
	 * Internally used to prevent snapping when the character comes from a different rotation mode or from ragdoll
	 * facing the wrong direction. Once the character is back inside the look threshold this flag becomes true again.
	 */
	uint32 bCanEnforceControlRotationMaxDistance : 1;

public: // Bitfields

	/** If true, Character can walk off a ledge when walking. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Walking")
	uint32 bCanWalkOffLedgesWhenWalking : 1;

	/** If true, Character can walk off a ledge when running. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Walking")
	uint32 bCanWalkOffLedgesWhenRunning : 1;

	/** If true, Character can walk off a ledge when sprinting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Walking")
	uint32 bCanWalkOffLedgesWhenSprinting : 1;

	/** If true, Character can walk off a ledge when performing action. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Walking")
	uint32 bCanWalkOffLedgesWhenPerformingGenericAction : 1;

	/** If true, try to walk (or keep walking) on next update. If false, try to stop on next update. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Character Movement (General Settings)")
	uint32 bWantsToWalkInsteadOfRun : 1;

	/** If true, try to sprint (or keep sprinting) on next update. If false, try to stop on next update. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Character Movement (General Settings)")
	uint32 bWantsToSprint : 1;

	/** If true, try to perform the generic action (or keep performing it) on next update. If false, try to stop on next update. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Character Movement (General Settings)")
	uint32 bWantsToPerformGenericAction : 1;

	/**
	 * If true use velocity as the movement vector instead of acceleration. Only used when OrientRotationToMovement is true.
	 * @see OrientRotationToMovement
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement (Rotation Settings)")
	uint32 bUseVelocityAsMovementVector : 1;

	/**
	 * If true rotation is interpolated using rotation rate as interpolation speed. Scaled by distance to Target, so it has a strong start speed and ease out.
	 * Does not apply to turn in place which has its own rate.
	 * @see RotationRate
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement (Rotation Settings)")
	uint32 bInterpolateToTargetRotation : 1;

	/** If true the character can rotate while jumping. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Jumping / Falling")
	uint32 bCanRotateWhileJumping : 1;

	/** If true, when jumping and not a ragdoll force the temporary adjust of BrakingFrictionFactor in the character movement component to produce some slide on landing, rather than reduce. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Jumping / Falling")
	uint32 bPreserveMovementOnLanding : 1;

	/*
	 * If true character will rotate with different rates depending on its ground speed in meters/second. This should normally produce faster rotations as
	 * the character moves faster. Helps simulating angular momentum similarly to how pivot turning works for linear movement.
	 * @see AdaptiveRotationSettings
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement (Rotation Settings)", AdvancedDisplay)
	uint32 bEnableAdaptiveRotationRate : 1;

	/**
	 * If true MaxAcceleration and GroundFriction will be dynamically adjusted when velocity and acceleration have opposing directions giving the character more "weight".
	 * This provides time for the pivot turn animation to play before movement starts in the opposite direction. 
	 * @see PivotTurnSettings
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: PivotTurn")
	uint32 bEnablePivotTurn : 1;

	/**
	 * If true and RotationMode is OrientToController character will turn in place. This variable is only evaluated once before the action and has no
	 * effect if already turning, in other words, it won't interrupt a turn in progress.
	 * @see RotationMode
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: TurnInPlace")
	uint32 bEnableTurnInPlace : 1;

	/**
	 * If true turn in place will only start after a delay specified by TurnInPlaceDelay.
	 * @see TurnInPlaceDelay
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: TurnInPlace", meta = (editcondition = "bEnableTurnInPlace"))
	uint32 bUseTurnInPlaceDelay : 1;

private: // Variables

#if WITH_EDITOR

	/** Current Speed */
	UPROPERTY(VisibleInstanceOnly, Transient, DuplicateTransient, Category = Velocity, meta=(DisplayName="Speed"))
	float InEditorSpeed;

	/** Current speed in the XY plane */
	UPROPERTY(VisibleInstanceOnly, Transient, DuplicateTransient, Category = Velocity, meta = (DisplayName = "Ground Speed"))
	float InEditorGroundSpeed;

	UPROPERTY(VisibleInstanceOnly, Transient, DuplicateTransient, Category = "Character Movement: TurnInPlace", meta = (DisplayName = "Turn In Place Target Yaw"))
	FString TurnInPlaceTargetYawDisplayText;

#endif

protected:  // Variables

	/** Pawn that owns this component. */
	UPROPERTY(BlueprintReadOnly, Transient, DuplicateTransient, meta = (AllowPrivateAccess = "true"))
	AExtCharacter* ExtCharacterOwner;

	/**
	 * A finite value indicates the direction the character is turning to.
	 * INIFINITY means the character has completed the turn (target yaw has been reached).
	 * -INFINITY means that turn in place has been suspended possibly due to some other action taking place.
	 * Use IsTurningInPlace() if you only want to know whether the character is turning or not and do not care about
	 * the reasons.
	 * @see IsTurningInPlace()
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Character Movement: TurnInPlace", meta=(AllowPrivateAccess="true"))
	float TurnInPlaceTargetYaw;

	/** Smoothly updated rotation offset used when rotating to desired contol rotation. */
	float RotationOffset;

	/** Time counter used for turn in place delay. */
	float TurnInPlaceTimeCounter;

	/** Cardinal direction of the movement vector (acceleration or velocity) in relation to the look rotation. */
	ECardinalDirection LookCardinalDirection;

	/** Used to cut the rotation rate (by setting to zero) and slowly ramp it up again every subsequent update. */
	float RotationRateFactor;

	/** The rotation we want to keep during a fall. */
	FRotator FallRotation;

	/** Store a copy of the ground speed of when we started to fall/jump this is going to be our max speed to avoid accelerating in the air. */
	float MaxFallingGroundSpeed;

	/** Difference in degrees between course (velocity direction) and heading (character forward) in the XY plane. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadonly, Transient, Category = Velocity, meta = (AllowPrivateAccess = "true"))
	float MovementDrift;

	/** */
	UPROPERTY()
	FVector SimulatedAcceleration;

public: // Variables

	/**
	 * Properties that define how the component can move. For temporary changes during runtime use ExtraMovementState instead.
	 * @see ExtraMovementState
	 * @see ResetExtraMoveState()
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = NavMovement, meta = (DisplayName = "Extra Movement Capabilities", Keywords = "Extra Movement"))
	FMovementPropertiesEx ExtraMovementProps;

	/** Expresses runtime state of character's extra movement. Put all temporal changes to movement properties here */
	UPROPERTY(Transient)
	FMovementPropertiesEx ExtraMovementState;

	/**
	 * Speed corresponds to ground speed in degrees/second. Limits are in degrees. Note that limits should normally be much lower when interpolating as opposed to using a constant rotation rate.
	 * @see EnableAdaptiveRotationRate
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement (Rotation Settings)", meta = (editcondition = "bEnableAdaptiveRotationRate"), AdvancedDisplay)
	FAdaptiveRotationSettings AdaptiveRotationSettings; 

	/**
	 * Options used for pivot turning. 
	 * @see EnablePivotTurn
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: PivotTurn", meta = (editcondition = "bEnablePivotTurn", ClampMin = "0", UIMin = "0"))
	FPivotTurnSettings PivotTurnSettings;

	/**
	 * Minimum speed for pivot turning.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: PivotTurn", meta = (editcondition = "bEnablePivotTurn"))
	float PivotTurnMinSpeed;

	/** Maximum absolute angle the character can look before being force to rotate. Only used if UseControllerDesiredRotation is true and OrientRotationToMovement is false. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement (Rotation Settings)", meta = (ClampMin = "45", UIMin = "45", ClampMax = "90", UIMax = "90"))
	float LookAngleThreshold;

	/**
	 * Maximum angular distance in deg the character is allowed to lag behind the control rotation when moving and UseControllerDesiredRotation is true.
	 * Ignored if OrientRotationToMovement is true.
	 * @see OrientRotationToMovement
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement (Rotation Settings)", meta = (editcondition = "bUseControllerDesiredRotation && !bOrientRotationToMovement", ClampMin = "0", UIMin = "0", ClampMax = "180", UIMax = "180"))
	float ControlRotationMaxDistance;

	/**   */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: TurnInPlace", meta = (editcondition = "bUseTurnInPlaceDelay", ClampMin = "0", UIMin = "0"))
	float TurnInPlaceDelay;

	/**   */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: TurnInPlace", meta = (editcondition = "bEnableTurnInPlace", ClampMin = "0", UIMin = "0"))
	FRotator TurnInPlaceRotationRate;

	/**   */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: TurnInPlace", meta = (editcondition = "bEnableTurnInPlace && !bUseTurnInPlaceDelay", ClampMin = "0", UIMin = "0", ClampMax = "180", UIMax = "180"))
	float TurnInPlaceMaxDistance;

	/** Input acceleration scale. Can be used to increase/decrease the character's ability to change direction without having to modify ground/fluid friction values.	  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement (General Settings)", meta = (ClampMin = "0", UIMin = "0"))
	float InputAccelerationScale;

	/** Last velocity vector with a non-zero projection in the XY plane */
	UPROPERTY(BlueprintReadOnly, Transient, DuplicateTransient, Category = Velocity, meta = (AllowPrivateAccess = "true"))
	FVector LastMovementVelocity;

	/** Last velocity vector when acceleration was non-zero */
	UPROPERTY(BlueprintReadOnly, Transient, DuplicateTransient, Category = Velocity, meta = (AllowPrivateAccess = "true"))
	FVector LastAcceleratedVelocity;

	/** Last non-zero acceleration vector. */
	UPROPERTY(BlueprintReadOnly, Transient, DuplicateTransient, Category = Velocity, meta = (AllowPrivateAccess = "true"))
	FVector LastMovementAcceleration;

	/** During a brake velocity is clamped to zero if its magnitude is below this value. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement (General Settings)", meta = (ClampMin = "0", UIMin = "0"), AdvancedDisplay)
	float BrakingSpeedTolerance;

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Walking", meta = (ClampMin = "0", UIMin = "0"))
	float MaxWalkAcceleration;

	 /**
	 * Setting that affects movement control. Higher values allow faster changes in direction.
	 * If bUseSeparateBrakingFriction is false, also affects the ability to stop more quickly when braking (whenever Acceleration is zero), where it is multiplied by BrakingFrictionFactor.
	 * When braking, this property allows you to control how much friction is applied when moving across the ground, applying an opposing force that scales with current velocity.
	 * This can be used to simulate slippery surfaces such as ice or oil by changing the value (possibly based on the material pawn is standing on).
	 * @see BrakingDecelerationWalking, BrakingFriction, bUseSeparateBrakingFriction, BrakingFrictionFactor
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Walking", meta = (ClampMin = "0", UIMin = "0"))
	float WalkFriction;

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Jumping / Falling", meta = (ClampMin = "0", UIMin = "0"))
	float MaxFallingAcceleration;

	/**
	 * Factor used in place of BrakingFrictionFactor to multiply actual value of friction used when braking in Walking/NavWalking after landing.
	 * Only used if bPreserveMovementOnLand is true.
	 * @see BrakingFrictionFactor
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Jumping / Falling", meta = (ClampMin = "0", UIMin = "0"))
	float BrakingFrictionFactorLanding;

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Ragdoll", meta = (ClampMin = "0", UIMin = "0"))
	float BrakingDecelerationLanding;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Swimming", meta = (ClampMin = "0", UIMin = "0"))
	float MaxSwimAcceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Flying", meta = (ClampMin = "0", UIMin = "0"))
	float MaxFlyAcceleration;

	/**
	 * Factor used in place of BrakingFrictionFactor to multiply actual value of friction used when braking in Ragdoll.
	 * @see BrakingFrictionFactor
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Ragdoll", meta = (ClampMin = "0", UIMin = "0"))
	float BrakingFrictionFactorRagdoll;

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Ragdoll", meta = (ClampMin = "0", UIMin = "0"))
	float BrakingDecelerationRagdoll;

protected: // Methods

	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual FVector ScaleInputAcceleration(const FVector& InputAcceleration) const override;

	virtual void ApplyVelocityBraking(float DeltaTime, float Friction, float BrakingDeceleration) override;
	virtual void SimulateMovement(float DeltaSeconds) override;

	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity);

	/** Called after MovementMode has changed. It does special handling for starting certain modes then calls OnAfterMovementModeChanged and notifies the CharacterOwner. */
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

public: // Methods

#if WITH_EDITOR
	virtual bool CanEditChange(const UProperty* InProperty) const override;
#endif

	virtual void PostLoad() override;
	virtual void SetUpdatedComponent(USceneComponent* NewUpdatedComponent) override;

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	virtual float GetMaxSpeed() const override;
	virtual float GetMaxBrakingDeceleration() const override;
	virtual float GetBrakingFrictionFactor() const;

	virtual FVector GetSimulatedAcceleration() const; 

	virtual void SetReplicatedAcceleration(const FVector& Value);
	virtual void SetReplicatedPivotTurn(bool bInIsPivotTurning);
	virtual void SetReplicatedTurnInPlace(float InTurnInPlaceTargetYaw);

	virtual FRotator GetDeltaRotation(float DeltaSeconds) const final;
	virtual FRotator ComputeOrientToMovementRotation(const FRotator& CurrentRotation, float DeltaSeconds, FRotator& DeltaRotation) const final;

	virtual FRotator GetRotationInterpSpeed(const FRotator& InterpSpeed, const FBounds& SpeedRange, const FBounds& FactorRange, const FBounds& Limits) const;
	virtual FRotator GetDeltaRotation(const FRotator& CurrentRotation, const FRotator& DesiredRotation, float DeltaSeconds) const;
	virtual FRotator ComputeOrientToMovementRotation(const FRotator& CurrentRotation, float DeltaSeconds);
	virtual FRotator ComputeOrientToLookRotation(const FRotator& LookRotation, const float NorthSegmentHalfWidth, float Buffer, float DeltaSeconds);

	virtual bool ShouldRemainVertical() const override;

	/** Perform rotation over deltaTime. This is an override to support interpolation, turn in place and adaptive rotation. */
	virtual void PhysicsRotation(float DeltaSeconds) override;

	/**
	 * @return true if the character is allowed to turn in place in the current state. Does not apply if ground speed is not zero, if falling
	 * or OrientRotationToMovement is true. By default, character is only allowed to turn in place if bEnableTurnInPlace is true and it
	 * is neither a ragdoll nor getting up. Note that physics rotation is temporarily disabled while a root motion animation is playing unless
	 * AllowPhysicsRotationDuringAnimRootMotion is true.
	 * @see OrientRotationToMovement
	 * @see AllowPhysicsRotationDuringAnimRootMotion
	 */
	virtual bool CanTurnInPlaceInCurrentState() const;

	virtual void ResetTurnInPlaceState();
	virtual void ResetControllerDesireRotationState(); 

	virtual bool CanCrouchInCurrentState() const override;
	virtual bool CanWalkOffLedges() const override;

	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void UpdateCharacterStateAfterMovement(float DeltaSeconds) override;

	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	/** Resets rotation rate factor to zero. */
	void ResetRotationRateFactor();

	/** Resets runtime extra movement state to character's movement capabilities */
	void ResetExtraMoveState() { ExtraMovementState = ExtraMovementProps; }

	/** @return true if currently walking */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
	virtual bool IsWalkingInsteadOfRunning() const;

	/**
	 * Checks if object is valid and walk is allowed (as opposed to running) in the current state and calls CharacterOwner->OnStartWalk() if successful.
	 * In general you should set bWantsToWalkInsteadOfRun instead to have the walk persist during movement, or just use the walk functions on the owning Character.
	 * @param	bClientSimulation	true when called when bIsWalkingInsteadOfRunning is replicated to non owned clients.
	 */
	virtual void Walk(bool bClientSimulation = false);

	/**
	 * Checks if object is valid and calls OnEndWalk() if successful.
	 * @param	bClientSimulation	true when called when bIsWalkingInsteadOfRunning is replicated to non owned clients.
	 */
	virtual void UnWalk(bool bClientSimulation = false);

	/** @return true if the character is allowed to walk in the current state (as opposed to running). By default it is allowed when moving on ground and not falling, if CanEverWalkInsteadOfRun() is true. */
	virtual bool CanWalkInCurrentState() const;

	/** @return true if component can walk */
	FORCEINLINE bool CanEverWalkInsteadOfRun() const { return ExtraMovementState.bCanWalkInsteadOfRun; }

	/** @return true if currently sprinting */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
	virtual bool IsSprinting() const;

	/**
	 * Checks if object is valid and sprint is allowed in the current state and calls CharacterOwner->OnStartSprint() if successful.
	 * In general you should set bWantsToSprint instead to have the sprint persist during movement, or just use the sprint functions on the owning Character.
	 * @param	bClientSimulation	true when called when bIsSprinting is replicated to non owned clients.
	 */
	virtual void Sprint(bool bClientSimulation = false);

	/**
	 * Checks if object is valid and calls OnEndSprint() if successful.
	 * @param	bClientSimulation	true when called when bIsSprinting is replicated to non owned clients.
	 */
	virtual void UnSprint(bool bClientSimulation = false);

	/** @return true if the character is allowed to sprint in the current state. By default it is allowed when walking and not falling, if CanEverSprint() is true. */
	virtual bool CanSprintInCurrentState() const;

	/** @return true if component can sprint */
	FORCEINLINE bool CanEverSprint() const { return ExtraMovementState.bCanSprint; }

	/** @return true if currently is performing the generic action */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
	virtual bool IsPerformingGenericAction() const;

	/**
	 * Checks if object is valid and allowed to perform the generic action in the current state and calls CharacterOwner->OnStartGenericAction() if successful.
	 * In general you should set bWantsToPerformGenericAction instead to have the action persist during movement, or just use the action functions on the owning Character.
	 * @param	bClientSimulation	true when called when bIsPerformingGenericAction is replicated to non owned clients.
	 */
	virtual void PerformGenericAction(bool bClientSimulation = false);

	/**
	 * Checks if object is valid and calls OnEndGenericAction() if successful.
	 * @param	bClientSimulation	true when called when bIsPerformingGenericAction is replicated to non owned clients.
	 */
	virtual void UnPerformGenericAction(bool bClientSimulation = false);

	/** @return true if the character is allowed to perform action in the current state. By default it is allowed when walking and not falling, if CanEverPerformGenericAction() is true. */
	virtual bool CanPerformGenericActionInCurrentState() const;

	/** @return true if component can perform action */
	FORCEINLINE bool CanEverPerformGenericAction() const { return ExtraMovementState.bCanPerformGenericAction; }

	/** @return true if currently jumping  */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
	bool IsJumping() const;

	/** @return true if currently jump landing  */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
	virtual bool IsLanding() const;

	/** */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
	virtual bool PredictStopLocation(FVector& OutStopLocation, const float TimeLimit = 2.0f, const float TimeStep = 0.01666667f);

	/** */
	FORCEINLINE AExtCharacter* GetExtCharacterOwner() const { return ExtCharacterOwner; }

	/** @return difference in degrees between course (velocity direction) and heading (character forward) in the XY plane. */
	FORCEINLINE float GetMovementDrift() const { return MovementDrift; }

	/** */
	FORCEINLINE bool IsPivotTurning() const { return bIsPivotTurning; }

	/** */
	FORCEINLINE ETurnInPlaceState GetTurnInPlaceState() const
	{
		return FMath::IsFinite(TurnInPlaceTargetYaw) ? ETurnInPlaceState::InProgress
			: (TurnInPlaceTargetYaw > 0.f) ? ETurnInPlaceState::Done
			: ETurnInPlaceState::Suspended;
	}

	/** @return Target yaw for turn in place. */
	FORCEINLINE float GetTurnInPlaceTargetYaw() const { return TurnInPlaceTargetYaw; }
};

class TPCE_API FSavedMove_ExtCharacter: public FSavedMove_Character
{
public:

	typedef FSavedMove_Character Super;
	
	enum
	{
		FLAG_WantsToWalkInsteadOfRun = Super::FLAG_Custom_0,
		FLAG_WantsToSprint = Super::FLAG_Custom_1,
		FLAG_WantsToPerformGenericAction = Super::FLAG_Custom_2,
	};

	bool bWantsToWalkInsteadOfRun;
	bool bWantsToSprint;
	bool bWantsToPerformGenericAction;

public:

	virtual void Clear() override;
	virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;
	// virtual bool IsImportantMove(const FSavedMovePtr& LastAckedMove) const override;
	// virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const override;
	virtual void PrepMoveFor(ACharacter* Character) override;
	virtual uint8 GetCompressedFlags() const override;
};

class FNetworkPredictionData_Client_ExtCharacter: public FNetworkPredictionData_Client_Character
{
public:

	typedef FNetworkPredictionData_Client_Character Super;

	FNetworkPredictionData_Client_ExtCharacter(const UCharacterMovementComponent& ClientMovement);

	virtual FSavedMovePtr AllocateNewMove() override;
};
