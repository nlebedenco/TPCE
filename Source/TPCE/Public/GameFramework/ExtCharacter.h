// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Interface.h"
#include "GameFramework/Character.h"
#include "Animation/AnimTypes.h"
#include "Math/Bounds.h"
#include "TimerManager.h"
#include "ExtraTypes.h"

#include "ExtCharacter.generated.h"

class UPrimitiveComponent;
class UWidgetManagerComponent;
class UArrowComponent;
class UReboundSpringArmComponent;
class UCameraComponent;
class UInputComponent;
class UExtCharacterMovementComponent;
class FLifetimeProperty;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGenericActionChangedSignature, AExtCharacter*, Sender);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCrouchChangedSignature, AExtCharacter*, Sender);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGaitChangedSignature, AExtCharacter*, Sender);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRotationModeChangedSignature, AExtCharacter*, Sender);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRagdollChangedSignature, AExtCharacter*, Sender);

// UP NEXT
// ---------------------------------
// TODO: Comment all methods with All/Local/Server to indicate where they are expected to be called
// TODO: Add missing comments/tooltips to members
// TODO: Try to improve the movement blend with a walk-in-place animation for gait scale 0. This should reduce leg crossing. 
// TODO: Idle Breaks
// TODO: Female animations
// TODO: Better crouch animations
// TODO: Animations for walk 45 and 135 deg w/ LeftFootFwd
// TODO: Carry Weapon
// TODO: Fire Action
// TODO: Interact with scene objects
// TODO: Additive Hit Reactions
// TODO: Stun
// TODO: Knockback
// TODO: Jump cooldown (on landing)
// TODO: Add SlopeWarping to Foot Placement AnimNode: rotate IKFootRoot to the slope angle then pull pelvis down to avoid overstretching the legs. Slope angle can be obtained from the character's ground check.
// TODO: Maybe use Start Movement transitions instead of hardcoded anim start positions for a better visual
// TODO: Maybe use Stop Movement transitions based on foot sync instead of FootPosition and FootAngle curves to eliminate leg glicthes/tremors?
// TODO: Better solution for foot placement in staircases
// TODO: Better solution for pelvis adjustment in speed warping
// TODO: Handle walking off ledges more gracefully
// TODO: Hand IK on getting up
// TODO: Create Physics Asset for shadow casting
// TODO: Profile of performance cost of the animated character in the game.
// TODO: Disable certain features according to LOD (leaning, breathing, speed warping, foot IK, ...)

USTRUCT(BlueprintType)
struct FCharacterGaitSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float MaxSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float MaxAcceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float Friction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float BrakingDeceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float BrakingFrictionFactor;
};

USTRUCT(BlueprintType)
struct FCharacterStanceSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCharacterGaitSettings Walk;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCharacterGaitSettings Run;
};

USTRUCT(BlueprintType)
struct FCharacterAttitudeSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCharacterStanceSettings Standing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCharacterStanceSettings Crouched;
};


USTRUCT(BlueprintType)
struct FCharacterMovementSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCharacterAttitudeSettings Primary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCharacterAttitudeSettings Secondary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCharacterGaitSettings Sprint;
};

/**
 * An extended Character that can Walk, Run, Sprint, Crouch, Jump, Perform a generic action and turn into a Ragdoll.
 * The default gait is Run. 
 *
 * Characters of this class can assume one of two attitudes with distinct movement settings. Normally the character will be in its Primary attitude.
 * When the generic action is activated it then assumes the Secondary attitude. 
 *
 * Neither Crouch, Jump nor the generic action can be performed while the character is Sprinting.
 *
 * Ragdoll is considered a form of animation so it is not replicated. Characters should turn into ragdoll as a consequence of their replicated state but conditions may vary
 * a lot from game to game.
 *
 * The ragdoll implementation makes the following assumptions:
 *
 * 1. The character mesh has an associated physics asset
 * 2. The physics asset has a root bone that is the parent of the pelvis bone.
 * 3. The root bone has an associated shape (usually a sphere) that has:
 *     MassInKg = 0.001;
 *     PhysicsType = Kinematic;
 *     CollisionResponse = false;
 *     Skip Scale from Animation = true;
 *     Center = (0, 0, 1);
 *     Radius = 1.0;
 *     Consider for Bounds = true;
 *
 * 4. In the constraint profile (or default constraint settings) used for ragdoll there must be a pelvis:root constraint setup as follows:
 *     Parent Dominates = true;
 *     Linear Limits: X Motion = Free;
 *     Linear Limits: Y Motion = Limited;
 *     Linear Limits: Z Motion = Limited;
 *     Linear Limits: Limit = 5.0;
 *     Linear Limits: Scale Linear Limits = true;
 *     Linear Limits: Soft Constraint = false;
 *     Linear Limits: Restitution = 0;
 *     Linear Limits: Contact Distance = 5.0;
 *     Linear Limits: Linear Break = false;
 *     Angular Limits: all free
 *
 * NOTE: (for developers fo the future) this class is meant to be camera agnostic (with exception of the relay to controller camera feature).
 * This means no camera component or camera dependent properties. Anything camera related should be implemented by derived classes.
 *
 * NOTE: We don't cache the "last hit" in CharacterMovement, and there maybe multiple hits per simulation step, but there are a variety of ways
 * to get notifications for such events (called in this order). 
 *     - virtual AActor::NotifyHit()
 *     - (Actor Blueprint Event) EventHit (called by NotifyHit())
 *     - virtual UCharacterMovementComponent::HandleImpact()
 *     - virtual ACharacter::MoveBlockedBy() (called by HandleImpact())
 *
 * @see Character
 * @see CharacterMovementComponent
 * @see ExtCharacterMovementComponent
 */
UCLASS(abstract, meta = (ShortTooltip = "An extended character that can Walk, Run, Sprint, Crouch, Jump, perform a generic action and turn into a Ragdoll with many other advanced movement features."))
class TPCE_API AExtCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	
	AExtCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:	// Bitfields

	/** */
	UPROPERTY(EditAnywhere, Category = Debug)
	uint32 bEnableDebugDraw : 1;

	/** If true character is in ragdoll mode. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	uint32 bIsRagdoll : 1;

	/**
	 * If true all camera functions will be relayed to the Controller.
	 * @see CalcCamera()
	 * @see HasActiveCameraComponent()
	 * @see HasActivePawnControlCameraComponent()
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), AdvancedDisplay)
	uint32 bRelayCameraFunctionsToController : 1;

public:		// Bitfields

	/** If true foot IK is used. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Animation)
	uint32 bEnableFootIK : 1;

	/** If true character is female. Can be used to play animations based on gender. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character)
	uint32 bIsFemale : 1;

	/**
	 * Ignore move input when in ragdoll state.
	 * Avoid changing this value in ragdoll state. AController:IgnoreMoveInput is a counter, not a boolean flag
	 * so it has to be released the same number of times it was acquired to be cleared properly. If necessary one can
	 * use AController::ResetIgnoreMoveInput() to reset the ignore state regardless of how many times it was acquired.
	 * @see AController::IgnoreMoveInput
	 * @see AController::ResetIgnoreMoveInput().
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ragdoll)
	uint32 bIgnoreMoveInputWhenRagdoll : 1;

	/**
	 * Ignore look input when in ragdoll state.
	 * Avoid changing this value in ragdoll state. AController:IgnoreLookInput is a counter, not a boolean flag
	 * so it has to be released the same number of times it was acquired to be cleared properly. If necessary one can
	 * use AController::ResetIgnoreLookInput() to reset the ignore state regardless of how many times it was acquired.
	 * @see Controller::IgnoreLookInput
	 * @see Controller::ResetIgnoreLookInput().
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ragdoll)
	uint32 bIgnoreLookInputWhenRagdoll : 1;

	/** Stop movement immediately when unpossessed. This has no effect on a ragdoll.  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character)
	uint32 bStopWhenUnpossessed : 1;

	/** Set by character movement to specify that this Character is currently walking. */
	UPROPERTY(BlueprintReadOnly, Transient, ReplicatedUsing = OnRep_IsWalkingInsteadOfRunning, Category = Character)
	uint32 bIsWalkingInsteadOfRunning : 1;

	/** Set by character movement to specify that this Character is currently sprinting. */
	UPROPERTY(BlueprintReadOnly, Transient, ReplicatedUsing = OnRep_IsSprinting, Category = Character)
	uint32 bIsSprinting : 1;

	/** Set by character movement to specify that this Character is currently performing the generic action. */
	UPROPERTY(BlueprintReadOnly, Transient, ReplicatedUsing = OnRep_IsPerformingGenericAction, Category = Character)
	uint32 bIsPerformingGenericAction: 1;

	/**
	 * If true character is jumping. Can be used to distinguish from when the character is unwinllingly falling.
	 * Always false when MovementMode is not MOVE_Falling. In this case you can refer to bHasLandedSafely to determine if the character
	 * has landed safely.
	 */
	UPROPERTY(BlueprintReadOnly, Transient, Category = Character)
	uint32 bIsJumping : 1;

protected:

	/**
	 * If true the character has landed safely from the last Falling state. Optimistically this flag becomes true when starting to fall but can be
	 * later modified in Landed() or OnLanded().
	 */
	UPROPERTY(BlueprintReadWrite, Transient, Category = Character)
	uint32 bHasLandedSafely : 1;

private:	// Variables

	/* Handle for the timer triggered after landing. */
	FTimerHandle LandingTimerHandle;

	/* Handle for the timer triggered when getting up from ragdoll. */
	FTimerHandle GettingUpTimerHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"), AdvancedDisplay)
	FName MoveForwardInputName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"), AdvancedDisplay)
	FName MoveRightInputName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"), AdvancedDisplay)
	FName LookUpInputName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"), AdvancedDisplay)
	FName LookRightInputName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"), AdvancedDisplay)
	FName CrouchInputName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"), AdvancedDisplay)
	FName JumpInputName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"), AdvancedDisplay)
	FName WalkInputName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"), AdvancedDisplay)
	FName SprintInputName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"), AdvancedDisplay)
	FName GenericActionInputName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"), AdvancedDisplay)
	FName FireInputName;

	/** Name of the collision profile to be used by the capsule component when in ragdoll mode. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Ragdoll, meta = (AllowPrivateAccess = "true"), AdvancedDisplay)
	FName RagdollCapsuleCollisionProfileName;

	/** Name of the collision profile to be used by the mesh component when in ragdoll mode. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Ragdoll, meta = (AllowPrivateAccess = "true"), AdvancedDisplay)
	FName RagdollMeshCollisionProfileName;

	/** Name of the constraint profile name to be used when in ragdoll mode. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Ragdoll, meta = (AllowPrivateAccess = "true"), AdvancedDisplay)
	FName RagdollMeshConstraintProfileName;;

	/** Name of the bone that is considered pelvis of the character. All bones below and including the pelvis are set to simulate physics when in ragdoll mode. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"), AdvancedDisplay)
	FName PelvisBoneName;

	/** Name of the bone that is considered left foot of the character. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"), AdvancedDisplay)
	FName LeftFootBoneName;

	/** Name of the bone that is considered right foot of the character. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"), AdvancedDisplay)
	FName RightFootBoneName;

#if WITH_EDITORONLY_DATA

	/** Component shown in the editor only to indicate Look Rotation */
	UPROPERTY()
	UArrowComponent* LookRotationArrow;

	/** Component shown in the editor only to indicate Look Rotation */
	UPROPERTY()
	UArrowComponent* LookRotationYawArrow;

	/** Component shown in the editor only to indicate Velocity */
	UPROPERTY()
	UArrowComponent* VelocityArrow;

	/** Component shown in the editor only to indicate Last Non-Zero Velocity */
	UPROPERTY()
	UArrowComponent* LastVelocityArrow;

	/** Component shown in the editor only to indicate Acceleration */
	UPROPERTY()
	UArrowComponent* AccelerationArrow;

	/** Component shown in the editor only to indicate Last non-zero Acceleration */
	UPROPERTY()
	UArrowComponent* LastAccelerationArrow;

#endif

	/** Current character gait determined by the latest movement actions */
	UPROPERTY(BlueprintReadOnly, Transient, Category = Character, meta = (AllowPrivateAccess = "true"))
	ECharacterGait Gait;

	/** Current character rotation mode. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = Character, meta = (AllowPrivateAccess = "true"))
	ECharacterRotationMode RotationMode;

protected:

	/**
	 * Replicated movement mode plus jump state. This is necessary because ACharacter::ReplicatedMovementMode does not have a virtual rep notify.
	 * TODO: remove when ACharacter::ReplicatedMovementMode gets a rep notify.
	 */
	UPROPERTY(Transient, ReplicatedUsing=OnRep_ReplicatedExtMovementMode)
	uint8 ReplicatedExtMovementMode;

public:		// Variables

	/** Custom replicated movement. */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_ReplicatedExtMovement)
	FRepExtMovement ReplicatedExtMovement;

	/** */
	UPROPERTY(Transient, ReplicatedUsing=OnRep_ReplicatedLook)
	FRepLook ReplicatedLook;

	/** */
	UPROPERTY(BlueprintReadOnly, Transient, Replicated, Category = Character, meta = (AllowPrivateAccess = "true", DisplayName = "LookAtActor"))
	class AActor* ReplicatedLookAtActor;

	/** Speed in cm/s to look up/down after player input input. Use 0 for instant. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	float LookUpInputSpeed;

	/** Speed in cm/s to look left/right after player input. Use 0 for instant. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	float LookRightInputSpeed;

	/**
	 * Amount of time needed for the character to get up from ragdoll. Tipically this should match the length of the get up animation used.
	 * @see OnGettingUpComplete()
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ragdoll, meta = (ClampMin = "0", UIMin = "0"))
	float GetUpDelay;

	/**
	 * Amount of delay after landing from a jump to consider it complete an call OnLandingComplete()
	 * @see OnLandingComplete()
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character, meta = (ClampMin = "0", UIMin = "0"))
	float LandingDelay;

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character)
	FCharacterMovementSettings MovementSettings;

public: // Dynamic Multicast Delegates

	UPROPERTY(BlueprintAssignable, Category = Character)
	FCrouchChangedSignature CrouchChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = Character)
	FGenericActionChangedSignature GenericActionChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = Character)
	FGaitChangedSignature GaitChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = Character)
	FRotationModeChangedSignature RotationModeChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = Character)
	FRagdollChangedSignature RagdollChangedDelegate;

private:	// Methods

	/**
	 * Callback for the LandingTimer. Calls OnLandingComplete() without the risk of being overriden by derived classes.
	 * @see OnLandingComplete()
	 */
	void LandingTimer_OnTime();
	

	/**
	* Callback for the GettingUpTimer. Calls OnGettingUpComplete() without the risk of being overriden by derived classes.
	* @see OnGettingUpComplete()
	*/
	void GettingUpTimer_OnTime();

	/** Update character rotation settings. */
	void OnRotationModeChangedInternal();

protected:	// Methods

#if WITH_EDITOR
	virtual void UpdateDebugComponentsVisibility();
#endif

	/** [all] Update internal state. */
	void SetGait(ECharacterGait NewGait);

	/** [all] Cancel the process of getting up. */
	void CancelGettingUp();

	/** [server + local] Cancel the process of jumping landing. */
	void CancelLanding();

	/** */
	UFUNCTION(Server, Reliable, WithValidation)
	virtual void ServerSetLookAtActor(AActor* InActor);

	/** Handle Extended Movement Mode replicated from server */
	UFUNCTION()
	virtual void OnRep_ReplicatedExtMovementMode();

	/** Handle Extended Movement replicated from server */
	UFUNCTION()
	virtual void OnRep_ReplicatedExtMovement();

	/** Handle Look replicated from server */
	UFUNCTION()
	virtual void OnRep_ReplicatedLook();

	/** Handle Walking replicated from server */
	UFUNCTION()
	virtual void OnRep_IsWalkingInsteadOfRunning();

	/** Handle Sprinting replicated from server */
	UFUNCTION()
	virtual void OnRep_IsSprinting();

	/** Handle Generic Action replicated from server */
	UFUNCTION()
	virtual void OnRep_IsPerformingGenericAction();

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** [local] Handle player input to move forwards/backward */
	virtual void PlayerInputMoveForward(float Value);

	/** [local] Handle player input to move side to side */
	virtual void PlayerInputMoveRight(float Value);

	/** [local] Handle player input to look up/down */
	virtual void PlayerInputLookUp(float Value);

	/** [local] Handle player input to look left/right */
	virtual void PlayerInputLookRight(float Value);

	/** [local] Handle player input to start crouching */
	virtual void PlayerInputStartCrouch();

	/** [local] Handle player input to stop crouching */
	virtual void PlayerInputStopCrouch();

	/** [local] Handle player input to start jumping */
	virtual void PlayerInputStartJump();

	/** [local] Handle player input to stop jumping */
	virtual void PlayerInputStopJump();

	/** [local] Handle player input to start walking */
	virtual void PlayerInputStartWalk();

	/** [local] Handle player input to stop walking */
	virtual void PlayerInputStopWalk();

	/** [local] Handle player input to start sprinting */
	virtual void PlayerInputStartSprint();

	/** [local] Handle player input to stop sprinting */
	virtual void PlayerInputStopSprint();

	/** [local] Handle player input to start generic action */
	virtual void PlayerInputStartGenericAction();

	/** [local] Handle player input to stop generic action*/
	virtual void PlayerInputStopGenericAction();

	/** [local] Handle player input to start firing */
	virtual void PlayerInputStartFire();

	/** [local] Handle player input to stop firing */
	virtual void PlayerInputStopFire();

	/** Temporary Function for Ragdoll Testing. */
	void ToggleRagdoll();

	/** Temporary Function for Ragdoll Testing. */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerToggleRagdoll();

	/** Temporary Function for Ragdoll Testing. */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetRagdoll(bool Value);


	/** Update movement component settings every time crouched, gait or generic action changes. */
	virtual void UpdateMovementComponentSettings();

	void UpdateMovementComponentSettings(const FCharacterGaitSettings& Settings);

	/** Called when the character is restarted. */
	virtual void OnRestart();

	virtual bool CanJumpInternal_Implementation() const override;

	/**
	 * Customizable event to check if the player can request to crouch in the current state.
	 * Actual action is still subject to validation by the movement component.
	 *
	 * @return Whether the character can crouch in the current state.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Pawn|Character", meta = (DisplayName = "CanCrouch"))
	bool CanCrouchInternal() const;
	virtual bool CanCrouchInternal_Implementation() const;

	/**
	 * Customizable event to check if the player can request to walk in the current state.
	 * Actual action is still subject to validation by the movement component.
	 *
	 * @return Whether the character can walk in the current state.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Pawn|Character", meta = (DisplayName = "CanWalk"))
	bool CanWalkInternal() const;
	virtual bool CanWalkInternal_Implementation() const;

	/**
	 * Customizable event to check if the player can request to sprint in the current state.
	 * Actual action is still subject to validation by the movement component.
	 *
	 * @return Whether the character can sprint in the current state.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Pawn|Character", meta = (DisplayName = "CanSprint"))
	bool CanSprintInternal() const;
	virtual bool CanSprintInternal_Implementation() const;

	/**
	 * Customizable event to check if the player can request to perform the generic action in the current state.
	 * Actual action is still subject to validation by the movement component.
	 *
	 * @return Whether the character can sprint in the current state.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Pawn|Character", meta = (DisplayName = "CanPerformGenericAction"))
	bool CanPerformGenericActionInternal() const;
	virtual bool CanPerformGenericActionInternal_Implementation() const;


	/** [all] Called when Character stops ragdolling. Called on non-owned Characters through bIsRagdoll replication. */
	virtual void OnEndRagdoll();

	/** [all] Called when Character stops ragdolling. Called on non-owned Characters through bIsRagdoll replication. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnEndRagdoll"))
	void K2_OnEndRagdoll();

	/** [all] Called when Character ragdolls. Called on non-owned Characters through bIsRagdoll replication. */
	virtual void OnStartRagdoll();

	/** [all] Called when Character ragdolls. Called on non-owned Characters through bIsRagdoll replication. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnStartRagdoll"))
	void K2_OnStartRagdoll();

	/**
	 * [all] Called from CharacterMovementComponent to notify the character that the movement mode has changed. This method is guaranteed to be called before
	 * the blueprint implementable event even in derived classes.
	 * @param	PrevMovementMode                Movement mode before the change
	 * @param	CurrentMovementMode             Current movement mode
	 * @param	PrevCustomMode                  Custom mode before the change (applicable if PrevMovementMode is Custom)
	 * @param	CurrentCustomMovementMode       Current custom mode (applicable if NewMovementMode is Custom)
	 */
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, EMovementMode CurrentMovementMode, uint8 PrevCustomMovementMode, uint8 CurrentCustomMovementMode);

	/** [all] Called whenever bIsCrouched is changed. */
	virtual void OnCrouchedChanged();

	/**
	 * [all] Called whenever Gait is changed.
	 * Note that Sprint takes precedence over Walk/Run so while bIsSprinting is true any changes in the Walk flag are ignored.
	 */
	virtual void OnGaitChanged();

	/** [all] Called whenever IsPerformingGenericAction is changed. */
	virtual void OnPerformingGenericActionChanged();

	/** [all] Called whenever Rotation Mode is changed. */
	virtual void OnRotationModeChanged();

	/** [all] Called whenever ragdoll flag is changed. */
	virtual void OnRagdollChanged();

	/**
	 * [server + local] Called with a delay after landing.
	 * @see LandingDelay
	 */
	virtual void OnLandingComplete();

	/** [server + local] Called when landing is canceled before completion. */
	virtual void OnLandingCanceled();

	/**
	 * [all] Called with a certain delay after recovering from ragdoll.
	 * @see GetUpDelay
	 */
	virtual void OnGettingUpComplete();

	/**
	 * [all] Called when getting up from ragdoll is canceled before completion.
	 * @see GetUpDelay
	 */
	virtual void OnGettingUpCanceled();

public:		// Methods

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker) override;
	virtual bool GatherExtMovement();
	virtual void PreNetReceive() override;
	virtual void PostNetReceive() override;

#if WITH_EDITOR
	virtual bool CanEditChange(const UProperty* InProperty) const override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& e) override;
#endif

	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual void Restart() override final;
	virtual void UnPossessed() override;

	virtual void PawnStartFire(uint8 FireModeNum) final;

	virtual void CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult) override;
	virtual bool HasActiveCameraComponent() const override;
	virtual bool HasActivePawnControlCameraComponent() const override;

	virtual void Landed(const FHitResult& Hit) override;

	// virtual FVector GetAcceleration() const;

	/** [server + local] Called from Character Movement Component before an update. */
	virtual void OnUpdateBeforeMovement(float DeltaSeconds);

	/** [server + local] Called from Character Movement Component after an update. At this point, rotation is still going to be calculated and applied. */
	virtual void OnUpdateAfterMovement(float DeltaSeconds);

	/** [all] Called from Character Movement Component after an update. */
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity);

	/**
	 * [all] Called from Character Movement Component to notify the character that the movement mode has changed. This method is now final.
	 * Derived classes can override the overloaded version that is protected instead.
	 * @param	PrevMovementMode	Movement mode before the change
	 * @param	PrevCustomMode		Custom mode before the change (applicable if PrevMovementMode is Custom)
	 */
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) final;

	/** [server + local] Called after OnJumped() every time the character effectivelly jumps and changes its movement mode to MOVE_Falling. */
	virtual void OnJumped_Implementation();


	/** 
	 * @return true if this character is currently able to crouch (and is not currently crouched). This method was made final for consistency.
	 * @see CanCrouchInternal()
	 */
	virtual bool CanCrouch() const final;

	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;


	/**
	 * Request the character to start walking. The request is processed on the next update of the CharacterMovementComponent.
	 * @see OnStartWalk
	 * @see IsWalkingInsteadOfRunning
	 * @see CharacterMovement->WantsToWalk
	 */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Character", meta = (HidePin = "bClientSimulation"))
	virtual void Walk(bool bClientSimulation = false);

	/**
	 * Request the character to stop walking. The request is processed on the next update of the CharacterMovementComponent.
	 * @see OnEndWalk
	 * @see IsWalkingInsteadOfRunning
	 * @see CharacterMovement->WantsToWalk
	 */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Character", meta = (HidePin = "bClientSimulation"))
	virtual void UnWalk(bool bClientSimulation = false);

	/** @return true if this character is currently able to walk (and is not currently walking) */
	UFUNCTION(BlueprintCallable, Category = Character)
	bool CanWalk() const;

	/** [all] Called when Character stops walking and returns to run. Called on non-owned Characters through bIsWalkingInsteadOfRunning replication. */
	virtual void OnEndWalk();

	/** Event when Character stops walking and returns to run. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnEndWalk"))
	void K2_OnEndWalk();

	/** [all] Called when Character starts walking (as opposed to running). Called on non-owned Characters through bIsWalkingInsteadOfRunning replication. */
	virtual void OnStartWalk();

	/** Event when Character starts walking (as opposed to running). */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnStartWalk"))
	void K2_OnStartWalk();

	/**
	 * Request the character to start sprinting. The request is processed on the next update of the CharacterMovementComponent.
	 * @see OnStartSprint
	 * @see IsSprinting
	 * @see CharacterMovement->WantsToSprint
	 */
	UFUNCTION(BlueprintCallable, Category="Pawn|Character", meta=(HidePin="bClientSimulation"))
	virtual void Sprint(bool bClientSimulation = false);

	/**
	 * Request the character to stop sprinting. The request is processed on the next update of the CharacterMovementComponent.
	 * @see OnEndSprint
	 * @see IsSprinting
	 * @see CharacterMovement->WantsToSprint
	 */
	UFUNCTION(BlueprintCallable, Category="Pawn|Character", meta=(HidePin="bClientSimulation"))
	virtual void UnSprint(bool bClientSimulation = false);

	/** @return true if this character is currently able to sprint (and is not currently sprinting) */
	UFUNCTION(BlueprintCallable, Category = Character)
	bool CanSprint() const;

	/** [all] Called when Character stops sprinting. Called on non-owned Characters through bIsSprinting replication. */
	virtual void OnEndSprint();

	/** Event when Character stops sprinting. */
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName = "OnEndSprint"))
	void K2_OnEndSprint();

	/** [all] Called when Character sprints. Called on non-owned Characters through bIsSprinting replication. */
	virtual void OnStartSprint();

	/** Event when Character sprints. */
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName = "OnStartSprint"))
	void K2_OnStartSprint();

	/**
	 * Request the character to start performing the generic action. The request is processed on the next update of the CharacterMovementComponent.
	 * @see OnStartGenericAction
	 * @see IsPerformingGenericAction
	 * @see CharacterMovement->WantsToPerformGenericAction
	 */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Character", meta = (HidePin = "bClientSimulation"))
	virtual void PerformGenericAction(bool bClientSimulation = false);

	/**
	 * Request the character to stop performing the generic action. The request is processed on the next update of the CharacterMovementComponent.
	 * @see OnEndGenericAction
	 * @see IsPerformingGenericAction
	 * @see CharacterMovement->WantsToPerformGenericAction
	 */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Character", meta = (HidePin = "bClientSimulation"))
	virtual void UnPerformGenericAction(bool bClientSimulation = false);

	/** @return true if this character is currently able to perform the generic action (and is not currently performing it) */
	UFUNCTION(BlueprintCallable, Category = Character)
	bool CanPerformGenericAction() const;

	/** [all] Called when Character stops performing the generic action. Called on non-owned Characters through bIsPerformingGenericAction replication. */
	virtual void OnEndGenericAction();

	/** Event when Character stops performing the generic action. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnEndGenericAction"))
	void K2_OnEndGenericAction();

	/** [all] Called when Character starts performing the generic action. Called on non-owned Characters through bIsPerformingGenericAction replication. */
	virtual void OnStartGenericAction();

	/** Event when Character starts performing the generic action. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnStartGenericAction"))
	void K2_OnStartGenericAction();

	virtual UPawnMovementComponent* GetMovementComponent() const override;

	/** */
	FORCEINLINE UExtCharacterMovementComponent* GetExtCharacterMovement() const { return (UExtCharacterMovementComponent*)(GetCharacterMovement()); }

	/** */
	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(DisplayName="ExtCharacterMovementComponent"))
	UExtCharacterMovementComponent* K2_GetExtCharacterMovement() const { return (UExtCharacterMovementComponent*)(GetCharacterMovement()); }

	/** @return	Look rotation of the character. */
	FORCEINLINE FRotator GetLookRotation() const { return ReplicatedLook.Rotation; }

	/** @return	Look rotation of the character. */
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName="LookRotation"))
	FRotator K2_GetLookRotation() const { return GetLookRotation(); }

	/** [server+local] Set actor for the character to look at. */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Character")
	void SetLookAtActor(AActor* InActor);

	/** @return	Actor this character should be looking at. */
	FORCEINLINE AActor* GetLookAtActor() const { return ReplicatedLookAtActor; }

	/** */
	FORCEINLINE bool IsRagdoll() const { return bIsRagdoll; }

	/** */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Character")
	void SetRagdoll(bool Value);

	/** */
	FORCEINLINE bool IsGettingUp() const { return GettingUpTimerHandle.IsValid(); }

	/** */
	FORCEINLINE bool IsLanding() const { return LandingTimerHandle.IsValid(); }

	/** */
	FORCEINLINE ECharacterGait GetGait() const { return Gait; }

	/** */
	FORCEINLINE ECharacterRotationMode GetRotationMode() const { return RotationMode; }

	/** */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Character")
	void SetRotationMode(ECharacterRotationMode Value);

	/** */
	FORCEINLINE FName GetPelvisBoneName() const { return PelvisBoneName; }

	/** */
	FORCEINLINE FName GetLeftFootBoneName() const { return LeftFootBoneName; }

	/** */
	FORCEINLINE FName GetRightFootBoneName() const { return RightFootBoneName; }

#if WITH_EDITOR

	UArrowComponent* GetLookRotationArrow() const { return LookRotationArrow; }
	UArrowComponent* GetLookRotationYawArrow() const { return LookRotationYawArrow; }
	UArrowComponent* GetVelocityArrow() const { return VelocityArrow; }
	UArrowComponent* GetLastVelocityArrow() const { return LastVelocityArrow; }
	UArrowComponent* GetAccelerationArrow() const { return AccelerationArrow; }
	UArrowComponent* GetLastAccelerationArrow() const { return LastAccelerationArrow; }

#endif
};

