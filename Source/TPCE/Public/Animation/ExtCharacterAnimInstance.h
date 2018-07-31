// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UObject/ObjectMacros.h"
#include "Animation/AnimInstance.h"
#include "Math/Bounds.h"
#include "ExtraTypes.h"

#include "ExtCharacterAnimInstance.generated.h"

class USkeletalMeshComponent;
class UAnimSequence;
class UCurveFloat;
class AExtCharacter;
class UExtCharacterMovementComponent;

USTRUCT(BlueprintType)
struct FootIKOffset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Height;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Roll;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Pitch;

	FootIKOffset():
		Height(0.0f),
		Roll(0.0f),
		Pitch(0.0f)
	{

	}

	FootIKOffset(const float InHeight, const float InRoll, const float InPitch):
		Height(InHeight),
		Roll(InRoll),
		Pitch(InPitch)
	{}
};

/**
 * AnimInstance base class to be used specifically with an ExtCharacter.
 * It has been designed to work with the following restrictions:
 *
 * Average wall height is 300cm (9.8ft):
 * Wall height of 400cm (13.1ft) will be slightly larger but may work as well.
 * Wall Depth (thickness): 10-20cm
 *
 * Door Height: 210-230cm
 * Door Width: 110-140cm
 * Add 10-20cm for a door frame that you may include for aesthetics.
 *
 * Average stair height and depth:
 * Step Height: 10-20cm
 * Step Length/Depth: 30-40cm
 *
 * A step height of 15cm is the best looking but can cause problems with certain levels.
 * For instance if you have a wall that is 300cm high then you would need 20 continuous steps to reach that height.
 * But if you have a wall that is 400cm high then you would need 26.6 steps to reach that height leaving us with a weirdly short last step.
 * If you use a step height of 10cm or 20cm and depth of 30cm, these will work for most walls without any decimals but may look too small
 * and slightly larger than one would expect.
 *
 * Stairs should have a simple ramp collider for the capsule and a complex collider for Foot IK.
 *
 * Slopes should have no more than 30 deg for good Foot IK. In Slopes prefer to use a left/right foot forward pose as it appears more natural
 * than having both feet parallel to each other. Avoid crouching. It almost never works unless the slope is very short.
 *
 */
UCLASS(abstract)
class TPCE_API UExtCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	static const float AngleTolerance;

	UExtCharacterAnimInstance();

private:

	uint32 bHasMovementModeChanged : 1;
	uint32 bHasCrouchedChanged : 1;
	uint32 bHasGaitChanged : 1;
	uint32 bHasPerformingGenericActionChanged : 1;

	/** Used to adjust the root bone rotation when in ragdoll */
	FQuat RootBoneRotation;

	/** */
	float TurnInPlaceTargetYaw;

	/** */
	FVector LastCharacterMeshLocation;

	/** */
	UPROPERTY(BlueprintReadOnly, Transient, DuplicateTransient, Category = "References", meta = (AllowPrivateAccess="true"))
	AExtCharacter* CharacterOwner;

	/** */
	UPROPERTY(BlueprintReadOnly, Transient, DuplicateTransient, Category = "References", meta = (AllowPrivateAccess = "true"))
	UExtCharacterMovementComponent* CharacterOwnerMovement;

	/** */
	UPROPERTY(BlueprintReadOnly, Transient, DuplicateTransient, Category = "References", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* CharacterOwnerMesh;

	/** Current movement mode of the character as indicated by its character movement component. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<EMovementMode> MovementMode;

	/** Current custom movement mode of the character as indicated by its character movement component. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	uint8 CustomMovementMode;

	/**  */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	ECharacterGait Gait;

	/**  */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	ECharacterRotationMode RotationMode;

	/**
	 * Indicates the character was moving (speed > 0) in any direction in the last frame. No reason can be implied for the movement. If you want to know if the
	 * character is moving in the current frame use bIsMoving or bIsMoving2D.
	 *
	 * @see bIsMoving
	 * @see bIsMoving2D
	 */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	uint32 bWasMoving : 1;

	/**
	* Indicates the character was moving in the XY plane (ground speed > 0). No reason can be implied for the movement. If you want to know if the
	* character is moving in the current frame use bIsMoving or bIsMoving2D.
	*
	* @see bIsMoving
	* @see bIsMoving2D
	*/
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	uint32 bWasMoving2D : 1;

	/**
	 * Indicates the character is moving (speed > 0) in any direction. No reason can be implied for the movement. If you want to know if the
	 * character is moving due to controller input check bIsAccelerating. If you only want to know if the character is moving in the XY
	 * plane check bIsMoving2D.
	 *
	 * @see bIsMoving2D
	 * @see bIsAccelerating
	 */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	uint32 bIsMoving: 1;

	/**
	* Indicates the character is moving in the XY plane (ground speed > 0). No reason can be implied for the movement. If you want to know if the
	* character is moving due to controller input check bIsAccelerating. If you want to know if the character is moving in any direction
	* check bIsMoving.
	*
	* @see bIsMoving
	* @see bIsAccelerating
	*/
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	uint32 bIsMoving2D : 1;

	/**
	 * Indicates the character is being driven by a player or AI to move, regardless of whether or not it is actually moving. If you want to determine if
	 * the character is actually moving check bIsMoving or bIsMoving2D.
	 *
	 * @see bIsMoving
     * @see bIsMoving2D
     */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	uint32 bIsAccelerating: 1;

	/** Indicates the character is crouched. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	uint32 bIsCrouched : 1;

	/** Indicates the character is performing a jump action. Used to distinguish from the cases where a fall is accidental. Only possible when walking. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	uint32 bIsJumping : 1;

	/** Indicates the character is performing action. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	uint32 bIsPerformingGenericAction : 1;

	/** If true character is in ragdoll mode. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character|Ragdoll", meta = (AllowPrivateAccess = "true"))
	uint32 bIsRagdoll : 1;

	/** If true character was in ragdoll mode last frame. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character|Ragdoll", meta = (AllowPrivateAccess = "true"))
	uint32 bWasRagdoll : 1;

	/** If true character is in ragdoll mode. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character|Ragdoll", meta = (AllowPrivateAccess = "true"))
	uint32 bIsRagdollFacingDown : 1;

	/** If true character is getting up from ragdoll. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	uint32 bIsGettingUp: 1;

	/** If true character was getting up from ragdoll last frame. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	uint32 bWasGettingUp : 1;

	/** If true character is turning in place. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character|TurnInPlace", meta = (AllowPrivateAccess = "true"))
	uint32 bIsTurningInPlace : 1;

	/** If true character is turning in place to the right. Only valid if bIsTurningInPlace is true. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character|TurnInPlace", meta = (AllowPrivateAccess = "true"))
	uint32 bIsTurningInPlaceRight : 1;

	/** If true character was turning in place in the last frame. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character|TurnInPlace", meta = (AllowPrivateAccess = "true"))
	uint32 bWasTurningInPlace : 1;

	/** If true character was turning in place to the right in the last frame. Only valid if bWasTurningInPlace is true. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character|TurnInPlace", meta = (AllowPrivateAccess = "true"))
	uint32 bWasTurningInPlaceRight : 1;

	/** If true turn in place is long (180deg) otherwise short (90deg). */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character|TurnInPlace", meta = (AllowPrivateAccess = "true"))
	uint32 bIsTurnInPlaceLong : 1;

	/** If true the last 90deg of the turn should still use the long animation otherwise change to the short animation. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character|TurnInPlace", meta = (AllowPrivateAccess = "true"))
	uint32 bShouldTurnInPlaceFinishLong : 1;

	/** If true character was pivot turning last frame. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character|PivotTurn", meta = (AllowPrivateAccess = "true"))
	uint32 bWasPivotTurning : 1;

	/** If true character is pivot turning. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character|PivotTurn", meta = (AllowPrivateAccess = "true"))
	uint32 bIsPivotTurning : 1;

	/** If true foot IK should be applied. This is for in game control. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character|Foot IK", meta = (AllowPrivateAccess = "true"))
	uint32 bEnableFootIK : 1;

	/** 
	 * If direction the character is pivot turning if bIsPivotTurning is true.
	 * @see bIsPivotTurning
	 */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character|PivotTurn", meta = (AllowPrivateAccess = "true"))
	ECardinalDirection PivotTurnDirection;

	/**  */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	FVector LastCharacterLocation;

	/**  */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	FRotator LastCharacterRotation;

	/**  */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	FVector CharacterLocation;

	/**  */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	FRotator CharacterRotation;

	/**  */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	FRotator LookRotation;

	/** The yaw delta between look rotation and character rotation. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	FRotator LookDelta;

	/** Actor the character should be trying to look at instead of using the normal rules for aim offset. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	AActor* LookAtActor;

	/** Cardinal direction of the movement vector (acceleration or velocity) in relation to the look rotation. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	ECardinalDirection LookCardinalDirection;

	/** Current Velocity of the character. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	FVector Velocity;

	/** Current acceleration imposed to the character. This value is always a result of input from a Controller. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	FVector Acceleration;

	/** Current speed of the character. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	float Speed;

	/** Current ground speed of the character. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	float GroundSpeed;

	/** Current speed of the character. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	float LastSpeed;

	/** Current ground speed of the character. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	float LastGroundSpeed;

	/** Last Non-Zero Acceleration imposed to the character. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	FRotator LastMovementAccelerationRotation;

	/** Last Acceleration Vector with a Non-Zero projection in the XY plane. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	FVector LastMovementAcceleration;

	/** Last Velocity Rotation with a Non-Zero projection in the XY plane. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	FRotator LastMovementVelocityRotation;

	/** Last Velocity Vector with a Non-Zero projection in the XY plane. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	FVector LastMovementVelocity;

	/** Any difference between course (velocity direction) and heading (characcter forward) in the XY plane. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	float MovementDrift;

	/** How long should it take for the character to get up. This value is obtained from the character and determines the playrate of the get up animation. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Character", meta = (AllowPrivateAccess = "true"))
	float GetUpDelay;

	/** How fast Aim Offset should reach the desired look rotation. Use 0 for immediate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Skeleton", meta = (AllowPrivateAccess = "true", ClampMin = "0", UIMin = "0"))
	float AimOffsetInterpSpeed;

	/** How fast the aim offset should reset. Use 0 for immediate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Skeleton", meta = (AllowPrivateAccess = "true", ClampMin = "0", UIMin = "0"))
	float AimOffsetResetInterpSpeed;

	/** How fast the root bone offset should reset. Use 0 for immediate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Skeleton", meta = (AllowPrivateAccess = "true", ClampMin = "0", UIMin = "0"))
	float RootBoneOffsetResetInterpSpeed;

	/** Curve used to determine the correct animation position for turn in place given an angular distance to the target rotation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Walking|Idle|TurnInPlace", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* TurnInPlaceLeftLongCurveNormal;

	/** Curve used to determine the correct animation position for turn in place given an angular distance to the target rotation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Walking|Idle|TurnInPlace", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* TurnInPlaceRightLongCurveNormal;

	/** Curve used to determine the correct animation position for turn in place given an angular distance to the target rotation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Walking|Idle|TurnInPlace", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* TurnInPlaceLeftShortCurveNormal;

	/** Curve used to determine the correct animation position for turn in place given an angular distance to the target rotation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Walking|Idle|TurnInPlace", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* TurnInPlaceRightShortCurveNormal;

	/** Curve used to determine the correct animation position for turn in place given an angular distance to the target rotation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Walking|Idle|TurnInPlace", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* TurnInPlaceLeftCurveLeftFootFwd;

	/** Curve used to determine the correct animation position for turn in place given an angular distance to the target rotation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Walking|Idle|TurnInPlace", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* TurnInPlaceRightCurveLeftFootFwd;

	/** Curve used to determine the correct animation position for turn in place given an angular distance to the target rotation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Walking|Idle|TurnInPlace", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* TurnInPlaceLeftCurveCrouched;

	/** Curve used to determine the correct animation position for turn in place given an angular distance to the target rotation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Walking|Idle|TurnInPlace", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* TurnInPlaceRightCurveCrouched;

	/**
	 * Walking speed used to calculate the GaitScale. Only the walking animation is played below this speed.
	 * Above this speed, the running animation will start to blend in.
	 * @see GaitScale
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Walking", meta = (AllowPrivateAccess = "true"))
	float WalkSpeed;

	/**
	 * Running speed used to calculate the GaitScale. The sprinting animation will start to blend in above this speed.
	 * @see GaitScale
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Walking", meta = (AllowPrivateAccess = "true"))
	float RunSpeed;

	/**
	 * Sprinting speed used to calculate the GaitScale. Only the sprinting animation is played above this speed.
	 * @see GaitScale
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Walking", meta = (AllowPrivateAccess = "true"))
	float SprintSpeed;

	/**
	 * Walking speed used to calculate the GaitScaleCrouched. Only the walking animation is played below this speed.
	 * Above this speed, the running animation will start to blend in.
	 * @see GaitScaleCrouched
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Walking", meta = (AllowPrivateAccess = "true"))
	float WalkSpeedCrouched;

	/**
	 * Running speed used to calculate the GaitScaleCrouched. Only the running animation is played above this speed.
	 * @see GaitScaleCrouched
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Walking", meta = (AllowPrivateAccess = "true"))
	float RunSpeedCrouched;

	/**
	 * Intended movement speed of the walking animation. Used to calculate WalkPlayRate and SpeedWarpScale
	 * @see WalkPlayRate, SpeedWarpScale 
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Walking", meta = (AllowPrivateAccess = "true"))
	float AnimWalkSpeed;

	/**
	 * Intended movement speed of the running animation. Used to calculate WalkPlayRate and SpeedWarpScale
	 * @see WalkPlayRate, SpeedWarpScale
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Walking", meta = (AllowPrivateAccess = "true"))
	float AnimRunSpeed;

	/**
	 * Intended movement speed of the sprinting animation. Used to calculate WalkPlayRate and SpeedWarpScale
	 * @see WalkPlayRate, SpeedWarpScale
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Walking", meta = (AllowPrivateAccess = "true"))
	float AnimSprintSpeed;

	/**
	 * Intended movement speed of the walking crouched animation. Used to calculate WalkPlayRateCrouched. SpeedWarpScale is not affected since it's not used for crouching.
	 * @see WalkPlayRateCrouched
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Walking", meta = (AllowPrivateAccess = "true"))
	float AnimWalkSpeedCrouched;

	/**
	 * Intended movement speed of the running crouched animation. Used to calculate WalkPlayRateCrouched. SpeedWarpScale is not affected since it's not used for crouching.
	 * @see WalkPlayRateCrouched
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Walking", meta = (AllowPrivateAccess = "true"))
	float AnimRunSpeedCrouched;

protected:

	/** Numeric representation of the current gait (walk/run/sprint) in the range [0, 3] according to the configured speeds. **/
	UPROPERTY(BlueprintReadWrite, Transient, Category = "Animation|Walking", meta = (AllowPrivateAccess = "true"))
	float GaitScale;

	/** Numeric representation of the current gait (walk/run) in the range [0, 2] for the crouched stance according to the configured speeds. Character cannot sprint while crouched. **/
	UPROPERTY(BlueprintReadWrite, Transient, Category = "Animation|Walking", meta = (AllowPrivateAccess = "true"))
	float GaitScaleCrouched;

	/** Play rate for walking animations. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "Animation|Walking", meta = (AllowPrivateAccess = "true"))
	float PlayRateWalk;

	/** Play rate for walking crouched animations. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "Animation|Walking", meta = (AllowPrivateAccess = "true"))
	float PlayRateWalkCrouched;

	/** Speed warping scale used for walking animations. **/
	UPROPERTY(BlueprintReadWrite, Transient, Category = "Animation|Walking|Speed Warping", meta = (AllowPrivateAccess = "true"))
	float SpeedWarpScale;

	/** Anim position for the turn in place animation calculated after the corresponding turn in place curve. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "Animation|Walking|Idle|TurnInPlace", meta = (AllowPrivateAccess = "true"))
	float TurnInPlaceLeftLongAnimPositionNormal;

	/** Anim position for the turn in place animation calculated after the corresponding turn in place curve. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "Animation|Walking|Idle|TurnInPlace", meta = (AllowPrivateAccess = "true"))
	float TurnInPlaceRightLongAnimPositionNormal;

	/** Anim position for the turn in place animation calculated after the corresponding turn in place curve. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "Animation|Walking|Idle|TurnInPlace", meta = (AllowPrivateAccess = "true"))
	float TurnInPlaceLeftShortAnimPositionNormal;

	/** Anim position for the turn in place animation calculated after the corresponding turn in place curve. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "Animation|Walking|Idle|TurnInPlace", meta = (AllowPrivateAccess = "true"))
	float TurnInPlaceRightShortAnimPositionNormal;

	/** Anim position for the turn in place animation calculated after the corresponding turn in place curve. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "Animation|Walking|Idle|TurnInPlace", meta = (AllowPrivateAccess = "true"))
	float TurnInPlaceLeftAnimPositionLeftFootFwd;

	/** Anim position for the turn in place animation calculated after the corresponding turn in place curve. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "Animation|Walking|Idle|TurnInPlace", meta = (AllowPrivateAccess = "true"))
	float TurnInPlaceRightAnimPositionLeftFootFwd;

	/** Anim position for the turn in place animation calculated after the corresponding turn in place curve. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "Animation|Walking|Idle|TurnInPlace", meta = (AllowPrivateAccess = "true"))
	float TurnInPlaceLeftAnimPositionCrouched;

	/** Anim position for the turn in place animation calculated after the corresponding turn in place curve. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "Animation|Walking|Idle|TurnInPlace", meta = (AllowPrivateAccess = "true"))
	float TurnInPlaceRightAnimPositionCrouched;

	/** Cached location of the foot bone used to adjust the corresponding IK foot bone for better blending when the character comes out of ragdoll. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "Animation|Foot IK", meta = (AllowPrivateAccess = "true"))
	FVector RagdollLeftFootLocation;

	/** Cached rotation of the foot bone used to adjust the corresponding IK foot bone for better blending when the character comes out of ragdoll. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "Animation|Foot IK", meta = (AllowPrivateAccess = "true"))
	FVector RagdollRightFootLocation;

	/** Cached location of the foot bone used to adjust the corresponding IK foot bone for better blending when the character comes out of ragdoll. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "Animation|Foot IK", meta = (AllowPrivateAccess = "true"))
	FRotator RagdollLeftFootRotation;

	/** Cached rotation of the foot bone used to adjust the corresponding IK foot bone for better blending when the character comes out of ragdoll. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "Animation|Foot IK", meta = (AllowPrivateAccess = "true"))
	FRotator RagdollRightFootRotation;

	/** Angular offset from character rotation to look rotation. X is Yaw, Y is Pitch. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "Animation|Skeleton")
	FVector2D AimOffset;

	/** Angular offset of the root bone in relation to the mesh component rotation. X is Yaw, Y is Pitch. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Animation|Skeleton")
	FVector2D RootBoneOffset;

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	virtual void NativeUpdateGaitScale(float DeltaSeconds); 
	virtual void NativeUpdatePivotTurn(const FVector& InLastVelocity, float DeltaSeconds);
	virtual void NativeUpdateTurnInPlace(float DeltaSeconds);
	virtual void NativeUpdateAimOffset(float DeltaSeconds);

	virtual void RaiseEvents();

	void SetMovementMode(const EMovementMode Value, const uint8 CustomValue); 
	void SetCrouched(const bool Value);
	void SetGait(const ECharacterGait Value);
	void SetPerformingGenericAction(const bool Value);

	UFUNCTION()
	void HandleRagdollChanged(AExtCharacter* Sender);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void OnMovementModeChanged();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void OnCrouchedChanged();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void OnGaitChanged();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void OnPerformingGenericActionChanged();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void OnRagdollEnded();


public:

	/**
	* Retrieves the Time (Horizontal-Axis value) from a curve within the provided animation sequence given a Value (vertical-axis value).
	* Can be used for distance matching using curves embedded in the animation sequence.
	* The curve must comply with a few restrictions:
	*   - Keys must have unique values, so for a given value, it maps to a unique position in the timeline of the animation.
	*   - Key values must be sorted in increasing order.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Animation)
	float FindCurveTimeFromValue(UAnimSequence* InAnimSequence, const FName CurveName, const float Value) const;

	FORCEINLINE AExtCharacter* GetCharacterOwner() const { return CharacterOwner; }

	FORCEINLINE UExtCharacterMovementComponent* GetCharacterOwnerMovement() const { return CharacterOwnerMovement; }

	FORCEINLINE USkeletalMeshComponent* GetCharacterOwnerMesh() const { return CharacterOwnerMesh; }

	FORCEINLINE TEnumAsByte<EMovementMode> GetMovementMode() const { return MovementMode; }

	FORCEINLINE uint8 GetCustomMovementMode() const { return CustomMovementMode; }

	FORCEINLINE ECharacterGait GetGait() const { return Gait; }

	FORCEINLINE ECharacterRotationMode GetRotationMode() const { return RotationMode; } 

	FORCEINLINE bool WasMoving() const { return bWasMoving; }

	FORCEINLINE bool WasMoving2D() const { return bWasMoving2D;  }

	FORCEINLINE bool IsMoving() const { return bIsMoving; }

	FORCEINLINE bool IsMoving2D() const { return bIsMoving2D; }

	FORCEINLINE bool IsAccelerating() const { return bIsAccelerating;  }

	FORCEINLINE bool IsCrouched() const { return bIsCrouched; }

	FORCEINLINE bool IsJumping() const { return bIsJumping; }

	FORCEINLINE bool IsPerformingGenericAction() const { return bIsPerformingGenericAction; }

	FORCEINLINE bool IsRagdoll() const { return bIsRagdoll; }

	FORCEINLINE bool WasRagdoll() const { return bWasRagdoll; }

	FORCEINLINE bool IsRagdollFacingDown() const { return bIsRagdollFacingDown; }

	FORCEINLINE bool IsGettingUp() const { return bIsGettingUp; }

	FORCEINLINE bool WasGettingUp() const { return bWasGettingUp; }

	FORCEINLINE bool IsTurningInPlace() const { return bIsTurningInPlace; }

	FORCEINLINE bool IsTurningInPlaceRight() const { return bIsTurningInPlaceRight; }

	FORCEINLINE bool WasTurningInPlace() const { return bWasTurningInPlace; }

	FORCEINLINE bool WasTurningInPlaceRight() const { return bWasTurningInPlaceRight; }

	FORCEINLINE bool IsTurnInPlaceLong() const { return bIsTurnInPlaceLong; }

	FORCEINLINE bool ShouldTurnInPlaceFinishLong() const { return bShouldTurnInPlaceFinishLong; }

	FORCEINLINE bool WasPivotTurning() const { return bWasPivotTurning; }

	FORCEINLINE bool IsPivotTurning() const { return bIsPivotTurning; }

	FORCEINLINE bool IsFootIKEnabled() const { return bEnableFootIK; }

	FORCEINLINE ECardinalDirection GetPivotTurnDirection() const { return PivotTurnDirection; }

	FORCEINLINE FVector GetLastCharacterLocation() const { return LastCharacterLocation; }

	FORCEINLINE FRotator GetLastCharacterRotation() const { return LastCharacterRotation; }

	FORCEINLINE FVector GetCharacterLocation() const { return CharacterLocation; }

	FORCEINLINE FRotator GetCharacterRotation() const { return CharacterRotation; }

	FORCEINLINE FRotator GetLookRotation() const { return LookRotation; }

	FORCEINLINE FRotator GetLookDelta() const { return LookDelta; }

	FORCEINLINE AActor* GetLookAtActor() const { return LookAtActor; }

	FORCEINLINE ECardinalDirection GetLookCardinalDirection() const { return LookCardinalDirection; }

	FORCEINLINE FVector GetVelocity() const { return Velocity; }

	FORCEINLINE FVector GetAcceleration() const { return Acceleration; }

	FORCEINLINE float GetSpeed() const { return Speed; }

	FORCEINLINE float GetGroundSpeed() const { return GroundSpeed; }

	FORCEINLINE float GetLastSpeed() const { return LastSpeed; }

	FORCEINLINE float GetLastGroundSpeed() const { return LastGroundSpeed; }

	FORCEINLINE FRotator GetLastMovementAccelerationRotation() const { return LastMovementAccelerationRotation; }

	FORCEINLINE FVector GetLastMovementAcceleration() const { return LastMovementAcceleration; }

	FORCEINLINE FRotator GetLastMovementVelocityRotation() const { return LastMovementVelocityRotation; }

	FORCEINLINE FVector GetLastMovementVelocity() const { return LastMovementVelocity; }

	FORCEINLINE float GetMovementDrift() const { return MovementDrift; }

	FORCEINLINE float GetGetUpDelay() const { return GetUpDelay; }

	FORCEINLINE float GetAimOffsetInterpSpeed() const { return AimOffsetInterpSpeed; }

	FORCEINLINE float GetAimOffsetResetInterpSpeed() const { return AimOffsetResetInterpSpeed; }

	FORCEINLINE float GetRootBoneOffsetResetInterpSpeed() const { return RootBoneOffsetResetInterpSpeed; }

	FORCEINLINE UCurveFloat* GetTurnInPlaceLeftLongCurveNormal() const { return TurnInPlaceLeftLongCurveNormal; }

	FORCEINLINE UCurveFloat* GetTurnInPlaceRightLongCurveNormal() const { return TurnInPlaceRightLongCurveNormal; }

	FORCEINLINE UCurveFloat* GetTurnInPlaceLeftShortCurveNormal() const { return TurnInPlaceLeftShortCurveNormal; }

	FORCEINLINE UCurveFloat* GetTurnInPlaceRightShortCurveNormal() const { return TurnInPlaceRightShortCurveNormal; }

	FORCEINLINE UCurveFloat* GetTurnInPlaceLeftCurveLeftFootFwd() const { return TurnInPlaceLeftCurveLeftFootFwd; }

	FORCEINLINE UCurveFloat* GetTurnInPlaceRightCurveLeftFootFwd() const { return TurnInPlaceRightCurveLeftFootFwd; }

	FORCEINLINE UCurveFloat* GetTurnInPlaceLeftCurveCrouched() const { return TurnInPlaceLeftCurveCrouched; }

	FORCEINLINE UCurveFloat* GetTurnInPlaceRightCurveCrouched() const { return TurnInPlaceRightCurveCrouched; }

	FORCEINLINE float GetWalkSpeed() const { return WalkSpeed; }

	FORCEINLINE float GetRunSpeed() const { return RunSpeed; }

	FORCEINLINE float GetSprintSpeed() const { return SprintSpeed; }

	FORCEINLINE float GetAnimWalkSpeed() const { return AnimWalkSpeed; }

	FORCEINLINE float GetAnimRunSpeed() const { return AnimRunSpeed; }

	FORCEINLINE float GetAnimSprintSpeed() const { return AnimSprintSpeed; }

	FORCEINLINE float GetWalkSpeedCrouched() const { return WalkSpeedCrouched; }

	FORCEINLINE float GetRunSpeedCrouched() const { return RunSpeedCrouched; }

	FORCEINLINE float GetAnimWalkSpeedCrouched() const { return AnimWalkSpeedCrouched; }

	FORCEINLINE float GetAnimRunSpeedCrouched() const { return AnimRunSpeedCrouched; }
};
