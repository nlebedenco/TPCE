// Fill out your copyright notice in the Description page of Project Settings.

#include "GameFramework/ExtCharacterMovementComponent.h"
#include "GameFramework/ExtCharacter.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PhysicsVolume.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Navigation/PathFollowingComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/NetworkObjectList.h"
#include "PhysicsEngine/ConstraintInstance.h"
#include "Curves/CurveFloat.h"

#include "Math/MathExtensions.h"
#include "Kismet/Kismet.h"

#include "DrawDebugHelpers.h"
#include "ExtraMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogExtCharacterMovement, Log, All);

FORCEINLINE static int32 GetCVarNetEnableSkipProxyPredictionOnNetUpdate()
{
	static const auto CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("p.NetEnableSkipProxyPredictionOnNetUpdate"));
	check(CVar);
	return CVar->GetInt();
}

FMovementPropertiesEx::FMovementPropertiesEx() :
	bCanWalkInsteadOfRun(false),
	bCanSprint(false),
	bCanPerformGenericAction(false)
{
}

#if WITH_EDITOR
const FName UExtCharacterMovementComponent::NAME_TurnInPlaceTargetYaw_None(TEXT("None"));
const FName UExtCharacterMovementComponent::NAME_TurnInPlaceTargetYaw_Suspended(TEXT("Suspended"));
#endif

const float UExtCharacterMovementComponent::AngleTolerance = 1e-3f;

UExtCharacterMovementComponent::UExtCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Default character can crouch
	NavAgentProps.bCanCrouch = true;
	ResetMoveState();

	// Default character can do everything
	ExtraMovementProps.bCanWalkInsteadOfRun = true;
	ExtraMovementProps.bCanSprint = true;
	ExtraMovementProps.bCanPerformGenericAction = true;
	ResetExtraMoveState();

	// Use acceleration for path following
	bUseAccelerationForPaths = true;

	// Do not change velocity on slopes
	bMaintainHorizontalGroundVelocity = false;

	InputAccelerationScale = 1.0f;
	
	JumpZVelocity = 350.f;
	AirControl = 0.1f;
	bPreserveMovementOnLanding = true;

	// Max acceleration should be treated as a temporary and only used in Authority or Autonomous proxy
	MaxAcceleration = 0.f;
	MaxWalkAcceleration = 800.f;
	MaxSwimAcceleration = 100.f;
	MaxFlyAcceleration = 500.f;
	// Character shouldn't normally accelerate when falling (contribution of gravity is not affected by this property)
	MaxFallingAcceleration = 0.f;

	// Max Speed
	MaxWalkSpeed = 400.f;

	// MaxWalkSpeedCrouched is not used.
	MaxWalkSpeedCrouched = 0.f;

	// Ground Friction should be treated as a temporary and only used in Authority or Autonomous proxy
	GroundFriction = 0.f;
	// Friction
	WalkFriction = 6.0f;

	// Braking Deceleration
	BrakingDecelerationWalking = 400.0f;
	BrakingDecelerationSwimming = 100.f;
	BrakingDecelerationFlying = 500.f;

	// Braking Deceleration Factor
	BrakingDecelerationRagdoll = 0.0f;
	BrakingDecelerationLanding = 50.f;

	// Braking Friction Factor
	BrakingFrictionFactor = 1.0f;
	BrakingFrictionFactorRagdoll = 0.3f;
	BrakingFrictionFactorLanding = 0.5f;

	// This should be adjusted according to view point distance and character scale but a value
	// of one should be reasonable and stable for most cases.
	BrakingSpeedTolerance = 1.f; 

	// Rotation Settings
	RotationRateFactor = 1.0f;
	bInterpolateToTargetRotation = false;
	LookAngleThreshold = 60.0f;
	ControlRotationMaxDistance = 0.0f;

	// Adaptive Rotation Settings (Simulate Angular Momentum)
	bEnableAdaptiveRotationRate = true;
	AdaptiveRotationSettings.Speed = FBounds(165.0f, 375.0f);
	AdaptiveRotationSettings.RotationRateFactor = FBounds(0.5f, 1.f);
	AdaptiveRotationSettings.RotationRateLimit = FBounds(120.f, 480.0f);

	// Pivot Turn
	bEnablePivotTurn = true;
	PivotTurnMinSpeed = 250.f;
	PivotTurnSettings.AccelerationFactor = FBounds(0.2f, 1.0f);
	PivotTurnSettings.FrictionFactor = FBounds(0.4f, 1.0f);

	// Turn In Place
	bEnableTurnInPlace = true;
	bUseTurnInPlaceDelay = false;
	TurnInPlaceDelay = 0.5f; 
	TurnInPlaceRotationRate = FRotator(0.0f, 180.f, 0.f);
	TurnInPlaceMaxDistance = 90.0f;
	TurnInPlaceTargetYaw = INFINITY;

	// Walk of Ledges
	bCanWalkOffLedgesWhenCrouching = true;
	bCanWalkOffLedgesWhenWalking = true;
	bCanWalkOffLedgesWhenRunning = true;
	bCanWalkOffLedgesWhenSprinting = true;
	bCanWalkOffLedgesWhenPerformingGenericAction = true;
}

#if WITH_EDITOR

bool UExtCharacterMovementComponent::CanEditChange(const UProperty* InProperty) const
{
	bool bCanChange = Super::CanEditChange(InProperty);

	if (bCanChange)
	{
		const FName PropertyName = (InProperty != NULL) ? InProperty->GetFName() : NAME_None;
		if (PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, MaxAcceleration)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, GroundFriction)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, bUseControllerDesiredRotation)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, bOrientRotationToMovement)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, bUseAccelerationForPaths)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, MaxWalkSpeedCrouched))
		{
			bCanChange = false;
		}
	}

	return bCanChange;
}

#endif

void UExtCharacterMovementComponent::PostLoad()
{
	Super::PostLoad();

	ExtCharacterOwner = Cast<AExtCharacter>(CharacterOwner);
}

void UExtCharacterMovementComponent::SetUpdatedComponent(USceneComponent* NewUpdatedComponent)
{
	Super::SetUpdatedComponent(NewUpdatedComponent);

	ExtCharacterOwner = Cast<AExtCharacter>(CharacterOwner);
}

void UExtCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	ResetMoveState();
	ResetExtraMoveState();
}


void UExtCharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

#if WITH_EDITOR
	TurnInPlaceTargetYawDisplayText = FMath::IsFinite(TurnInPlaceTargetYaw) ? FString::SanitizeFloat(TurnInPlaceTargetYaw) :
		(TurnInPlaceTargetYaw > 0.f) ? NAME_TurnInPlaceTargetYaw_None.ToString() : NAME_TurnInPlaceTargetYaw_Suspended.ToString();
#endif
}

/// Replication


void UExtCharacterMovementComponent::SetReplicatedAcceleration(const FVector& Value)
{
	checkActorRoleExactly(ROLE_SimulatedProxy);

	SimulatedAcceleration = Value;
}

void UExtCharacterMovementComponent::SetReplicatedPivotTurn(bool bInIsPivotTurning)
{
	checkActorRoleExactly(ROLE_SimulatedProxy);

	bIsPivotTurning = bInIsPivotTurning;
}

void UExtCharacterMovementComponent::SetReplicatedTurnInPlace(float InTurnInPlaceTargetYaw)
{
	checkActorRoleExactly(ROLE_SimulatedProxy);

	TurnInPlaceTargetYaw = InTurnInPlaceTargetYaw;
}


/// Movement Update

void UExtCharacterMovementComponent::ApplyVelocityBraking(float DeltaTime, float Friction, float BrakingDeceleration)
{
	// Full override to let speed tolerance be configurable possibly to a higher value than the originally hardcoded 0.1mm/s.
	// After all perception of movement depends on several things including environment scale, camera distance, etc.
	FULL_OVERRIDE();

	if (Velocity.IsZero() || !HasValidData() || HasAnimRootMotion() || DeltaTime < MIN_TICK_TIME)
	{
		return;
	}

	const float FrictionFactor = FMath::Max(0.f, GetBrakingFrictionFactor());
	Friction = FMath::Max(0.f, Friction * FrictionFactor);
	BrakingDeceleration = FMath::Max(0.f, BrakingDeceleration);
	const bool bZeroFriction = (Friction == 0.f);
	const bool bZeroBraking = (BrakingDeceleration == 0.f);

	if (bZeroFriction && bZeroBraking)
	{
		return;
	}

	const FVector OldVel = Velocity;

	// subdivide braking to get reasonably consistent results at lower frame rates
	// (important for packet loss situations w/ networking)
	float RemainingTime = DeltaTime;
	const float MaxTimeStep = (1.0f / 33.0f);

	// Decelerate to brake to a stop
	const FVector RevAccel = (bZeroBraking ? FVector::ZeroVector : (BrakingDeceleration * Velocity.GetSafeNormal()));
	while (RemainingTime >= MIN_TICK_TIME)
	{
		// Zero friction uses constant deceleration, so no need for iteration.
		const float dt = ((RemainingTime > MaxTimeStep && !bZeroFriction) ? FMath::Min(MaxTimeStep, RemainingTime * 0.5f) : RemainingTime);
		RemainingTime -= dt;

		// apply friction and braking
		Velocity -= (Friction * Velocity + RevAccel) * dt;

		// Don't reverse direction
		if ((Velocity | OldVel) <= 0.f)
		{
			Velocity = FVector::ZeroVector;
			return;
		}
	}

	// Clamp to zero if nearly zero, or if below min threshold and braking.
	const float VSizeSq = Velocity.SizeSquared();
	if (VSizeSq <= FMath::Square(BrakingSpeedTolerance) || (!bZeroBraking && VSizeSq <= (BRAKE_TO_STOP_VELOCITY * BRAKE_TO_STOP_VELOCITY)))
	{
		Velocity = FVector::ZeroVector;
	}
}

void UExtCharacterMovementComponent::SimulateMovement(float DeltaSeconds)
{
	// Full override needed because original implementation sets Acceleration to Velocity normal but we want to use the SimulatedAcceleration
	// from movement replication. Another option would be to just let Acceleration be assigned by original implementation an overwrite it
	// in OnMovementUpdated(...) but then we would have had normalized the velocity to waste.

	FULL_OVERRIDE();

	if (!HasValidData() || UpdatedComponent->Mobility != EComponentMobility::Movable || UpdatedComponent->IsSimulatingPhysics())
	{
		return;
	}

	const bool bIsSimulatedProxy = (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy);

	// Workaround for replication not being updated initially
	if (bIsSimulatedProxy &&
		CharacterOwner->GetReplicatedMovement().Location.IsZero() &&
		CharacterOwner->GetReplicatedMovement().Rotation.IsZero() &&
		CharacterOwner->GetReplicatedMovement().LinearVelocity.IsZero())
	{
		return;
	}

	// If base is not resolved on the client, we should not try to simulate at all
	if (CharacterOwner->GetReplicatedBasedMovement().IsBaseUnresolved())
	{
		UE_LOG(LogExtCharacterMovement, Verbose, TEXT("Base for simulated character '%s' is not resolved on client, skipping SimulateMovement"), *CharacterOwner->GetName());
		return;
	}

	FVector OldVelocity;
	FVector OldLocation;

	// Scoped updates can improve performance of multiple MoveComponent calls.
	{
		FScopedMovementUpdate ScopedMovementUpdate(UpdatedComponent, bEnableScopedMovementUpdates ? EScopedUpdate::DeferredUpdates : EScopedUpdate::ImmediateUpdates);

		bool bHandledNetUpdate = false;
		if (bIsSimulatedProxy)
		{
			// Handle network changes
			if (bNetworkUpdateReceived)
			{
				bNetworkUpdateReceived = false;
				bHandledNetUpdate = true;
				UE_LOG(LogExtCharacterMovement, Verbose, TEXT("Proxy %s received net update"), *GetNameSafe(CharacterOwner));
				if (bNetworkMovementModeChanged)
				{
					ApplyNetworkMovementMode(CharacterOwner->GetReplicatedMovementMode());
					bNetworkMovementModeChanged = false;
				}
				else if (bJustTeleported || bForceNextFloorCheck)
				{
					// Make sure floor is current. We will continue using the replicated base, if there was one.
					bJustTeleported = false;
					UpdateFloorFromAdjustment();
				}
			}
			else if (bForceNextFloorCheck)
			{
				UpdateFloorFromAdjustment();
			}
		}

		if (MovementMode == MOVE_None)
		{
			ClearAccumulatedForces();
			return;
		}

		//TODO: Also ApplyAccumulatedForces()?
		HandlePendingLaunch();
		ClearAccumulatedForces();

		Acceleration = GetSimulatedAcceleration();
		AnalogInputModifier = 1.0f;				// Not currently used for simulated movement

		MaybeUpdateBasedMovement(DeltaSeconds);

		// simulated pawns predict location
		OldVelocity = Velocity;
		OldLocation = UpdatedComponent->GetComponentLocation();

		// May only need to simulate forward on frames where we haven't just received a new position update.
		if (!bHandledNetUpdate || !bNetworkSkipProxyPredictionOnNetUpdate || !GetCVarNetEnableSkipProxyPredictionOnNetUpdate())
		{
			UE_LOG(LogExtCharacterMovement, Verbose, TEXT("Proxy %s simulating movement"), *GetNameSafe(CharacterOwner));
			FStepDownResult StepDownResult;
			MoveSmooth(Velocity, DeltaSeconds, &StepDownResult);

			// find floor and check if falling
			if (IsMovingOnGround() || MovementMode == MOVE_Falling)
			{
				const bool bSimGravityDisabled = (CharacterOwner->bSimGravityDisabled && bIsSimulatedProxy);
				if (StepDownResult.bComputedFloor)
				{
					CurrentFloor = StepDownResult.FloorResult;
				}
				else if (Velocity.Z <= 0.f)
				{
					FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, Velocity.IsZero(), NULL);
				}
				else
				{
					CurrentFloor.Clear();
				}

				if (!CurrentFloor.IsWalkableFloor())
				{
					if (!bSimGravityDisabled)
					{
						// No floor, must fall.
						if (Velocity.Z <= 0.f || bApplyGravityWhileJumping || !CharacterOwner->IsJumpProvidingForce())
						{
							Velocity = NewFallVelocity(Velocity, FVector(0.f, 0.f, GetGravityZ()), DeltaSeconds);
						}
					}
					SetMovementMode(MOVE_Falling);
				}
				else
				{
					// Walkable floor
					if (IsMovingOnGround())
					{
						AdjustFloorHeight();
						SetBase(CurrentFloor.HitResult.Component.Get(), CurrentFloor.HitResult.BoneName);
					}
					else if (MovementMode == MOVE_Falling)
					{
						if (CurrentFloor.FloorDist <= MIN_FLOOR_DIST || (bSimGravityDisabled && CurrentFloor.FloorDist <= MAX_FLOOR_DIST))
						{
							// Landed
							SetPostLandedPhysics(CurrentFloor.HitResult);
						}
						else
						{
							if (!bSimGravityDisabled)
							{
								// Continue falling.
								Velocity = NewFallVelocity(Velocity, FVector(0.f, 0.f, GetGravityZ()), DeltaSeconds);
							}
							CurrentFloor.Clear();
						}
					}
				}
			}
		}
		else
		{
			UE_LOG(LogExtCharacterMovement, Verbose, TEXT("Proxy %s SKIPPING simulate movement"), *GetNameSafe(CharacterOwner));
		}

		// consume path following requested velocity
		bHasRequestedVelocity = false;

		OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
	} // End scoped movement update

	  // Call custom post-movement events. These happen after the scoped movement completes in case the events want to use the current state of overlaps etc.
	CallMovementUpdateDelegate(DeltaSeconds, OldLocation, OldVelocity);

	MaybeSaveBaseLocation();
	UpdateComponentVelocity();
	bJustTeleported = false;

	LastUpdateLocation = UpdatedComponent ? UpdatedComponent->GetComponentLocation() : FVector::ZeroVector;
	LastUpdateRotation = UpdatedComponent ? UpdatedComponent->GetComponentQuat() : FQuat::Identity;
	LastUpdateVelocity = Velocity;
}

void UExtCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	// Mind that this method is called for every net role but it's also the last step in the movement update process. By the time we get here,
	// movement has been performed already for the frame. Nothing saved here can be used by the character movement component itself unless it's
	// ok to be one frame late.

	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

	check(ExtCharacterOwner);

#if WITH_EDITOR
	// Square roots for PIE
	InEditorSpeed = Velocity.Size();
	InEditorGroundSpeed = Velocity.Size2D();
#endif

	// Ramp up RotationRateFactor.
	RotationRateFactor = FMath::Clamp(RotationRateFactor + DeltaSeconds, 0.f, 1.f);

	const bool bIsMoving2D = Velocity.SizeSquared2D() > KINDA_SMALL_NUMBER;
	if (bIsMoving2D)
		LastMovementVelocity = Velocity;

	const bool bIsAccelerating = Acceleration.SizeSquared() > KINDA_SMALL_NUMBER;
	if (bIsAccelerating)
	{
		LastMovementAcceleration = Acceleration;
		LastAcceleratedVelocity = Velocity;
	}

	// Calculate Drift
	if (USkeletalMeshComponent* Mesh = ExtCharacterOwner->GetMesh())
	{
		const FRotator MeshOrientation = (Mesh->GetComponentQuat() * ExtCharacterOwner->GetBaseRotationOffset().Inverse()).Rotator();
		MovementDrift = FMath::FindDeltaAngleDegrees(MeshOrientation.Yaw, LastMovementVelocity.Rotation().Yaw);
	}
	else
	{
		MovementDrift = 0.f;
	}

	ExtCharacterOwner->OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
}

void UExtCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	if (!HasValidData())
	{
		return;
	}

	// Reset pivot turn state if not moving on ground
	if (!(MovementMode == MOVE_Walking && PreviousMovementMode == MOVE_NavWalking)
		&& !(MovementMode == MOVE_NavWalking && PreviousMovementMode == MOVE_Walking))
	{
		bIsPivotTurning = false;
	}

	switch (MovementMode)
	{
	case MOVE_Walking:
	case MOVE_NavWalking:
		RotationOffset = 0.0f;
		break;
	case MOVE_Falling:
		// Set Max Acceleration
		MaxAcceleration = MaxFallingAcceleration;
		// Save last ground speed as max falling speed to prevent accelerating in mid air.
		MaxFallingGroundSpeed = Velocity.Size2D();
		// Set fall rotation to be the movement direction or default to the character's rotation
		FallRotation = CharacterOwner->GetActorRotation();
		// Reset LookCardinalDirection
		LookCardinalDirection = ECardinalDirection::North;
		break;
	}

	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
};

void UExtCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	checkComponentRoleAtLeast(ROLE_AutonomousProxy);

	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);

	check(ExtCharacterOwner);

	// Check for a change in walk state. Players toggle walk by changing bWantsToWalkInsteadOfRun.
	const bool bAllowedToWalk = CanWalkInCurrentState();
	if ((!bAllowedToWalk || !bWantsToWalkInsteadOfRun) && ExtCharacterOwner->bIsWalkingInsteadOfRunning)
	{
		UnWalk(false);
	}
	else if (bAllowedToWalk && bWantsToWalkInsteadOfRun && !ExtCharacterOwner->bIsWalkingInsteadOfRunning)
	{
		Walk(false);
	}

	// Check for a change in perform action state. Players toggle perform action by changing bWantsToPerformGenericAction.
	const bool bAllowedToPerformGenericAction = CanPerformGenericActionInCurrentState();
	if ((!bAllowedToPerformGenericAction || !bWantsToPerformGenericAction) && ExtCharacterOwner->bIsPerformingGenericAction)
	{
		UnPerformGenericAction(false);
	}
	else if (bAllowedToPerformGenericAction && bWantsToPerformGenericAction && !ExtCharacterOwner->bIsPerformingGenericAction)
	{
		PerformGenericAction(false);
	}

	// Check for a change in sprint state. Players toggle sprint by changing bWantsToSprint.
	const bool bAllowedToSprint = CanSprintInCurrentState();
	if ((!bAllowedToSprint || !bWantsToSprint) && ExtCharacterOwner->bIsSprinting)
	{
		UnSprint(false);
	}
	else if (bAllowedToSprint && bWantsToSprint && !ExtCharacterOwner->bIsSprinting)
	{
		Sprint(false);
	}

	switch (MovementMode)
	{
	case MOVE_Walking:
	case MOVE_NavWalking:
		{
			MaxAcceleration = MaxWalkAcceleration;
			GroundFriction = WalkFriction;

			// Calculate the cosine of the shortest angle between Velocity and Acceleration. It indicates how aligned the vectors are in the range [+1, -1]
			// where +1 is perfectly aligned and -1 is in the exact opposite direction. A common mistake is to assume the cosine to be a linear function.
			// Mind that a value of 0 corresponds to 90deg but 0.5 does not correspond to 45deg yet to 60deg. In fact cos(45deg) is aprox 0.70710678.
			const float MovementDeflection = FVector::DotProduct(LastMovementAcceleration.GetSafeNormal2D(), LastMovementVelocity.GetSafeNormal2D());

			if (bIsPivotTurning)
			{
				if (MovementDeflection > 0.7071068f
					|| (!(Acceleration.SizeSquared2D() > KINDA_SMALL_NUMBER) && !(Velocity.SizeSquared2D() > KINDA_SMALL_NUMBER)))
				{
					bIsPivotTurning = false;
					goto SkipPivotTurnAdjusts;
				}
			}
			else if (bEnablePivotTurn
				&& MovementDeflection < -0.173648f 		// Movement deflection > 100deg
				&& LastAcceleratedVelocity.SizeSquared2D() > FMath::Square(PivotTurnMinSpeed)
				&& !ExtCharacterOwner->IsLanding()
				&& !ExtCharacterOwner->IsGettingUp()
				&& !ExtCharacterOwner->IsRagdoll())
			{
				bIsPivotTurning = true;
			}
			else
			{
				goto SkipPivotTurnAdjusts;
			}

			// Dynamically change MaxAcceleration and GroundFriction when pushing to change direction giving the character more "weight".
			// This is essential to the pivot system, as it allows time for the pivot to play before accelerating in the opposite direction.
			// It must be GroundFriction and not BrakingFrictionFactor to be modified because when acceleration is not zero
			// BrakingFrictionFactor is not used. In the end we're not interested in reducing friction to delay braking but
			// to reduce the rate of change of the character's velocity, or in other reduce the contribution of acceleration making it more
			// resistant to direction changes.
			// Check the arc to 45 deg to 130 deg. Any difference below 90 deg will be the upper bound (Alpha = 1) anything above 130 deg will be the lower bound (Alpha = 0).
			// It's ok to use the MovementDeflection from last frame here cause we haven't computed the new velocity yet.
			const float Alpha = FMath::InterpEaseIn(0.f, 1.0f, FMath::Clamp<float>(FMath::GetRangePct(-0.6427870f, 0.f, MovementDeflection), 0.f, 1.f), 3.f);

			MaxAcceleration *= FMath::GetRangeValue(FVector2D(PivotTurnSettings.AccelerationFactor), Alpha);
			GroundFriction *= FMath::GetRangeValue(FVector2D(PivotTurnSettings.FrictionFactor), Alpha);

			SkipPivotTurnAdjusts: void(0);
		}
		break;
	case MOVE_Falling:
		MaxAcceleration = MaxFallingAcceleration;
		break;
	case MOVE_Swimming:
		MaxAcceleration = MaxSwimAcceleration;
		// TODO: Implement buoyancy if swimming in ragdoll (increased linear/angular damp and counter gravity accel)
		break;
	case MOVE_Flying:
		MaxAcceleration = MaxFlyAcceleration;
		// TODO: Implement zero gravity if flying in ragdoll (counter the gravity accel)
		break;
	default:
		MaxAcceleration = 0.f;
	}

	// Call character
	ExtCharacterOwner->OnUpdateBeforeMovement(DeltaSeconds);
}

void UExtCharacterMovementComponent::UpdateCharacterStateAfterMovement(float DeltaSeconds)
{
	checkComponentRoleAtLeast(ROLE_AutonomousProxy);

	Super::UpdateCharacterStateAfterMovement(DeltaSeconds);

	check(ExtCharacterOwner);

	// Un-walk if no longer allowed to be walking
	if (ExtCharacterOwner->bIsWalkingInsteadOfRunning && !CanWalkInCurrentState())
	{
		UnWalk(false);
	}

	// UnPerformGenericAction if no longer allowed to be perform.
	if (ExtCharacterOwner->bIsPerformingGenericAction && !CanPerformGenericActionInCurrentState())
	{
		UnPerformGenericAction(false);
	}

	// Un-sprint if no longer allowed to be sprinting
	if (ExtCharacterOwner->bIsSprinting && !CanSprintInCurrentState())
	{
		UnSprint(false);
	}

	ExtCharacterOwner->OnUpdateAfterMovement(DeltaSeconds);
}



/// Speed, Acceleration and Deceleration

float UExtCharacterMovementComponent::GetMaxSpeed() const
{
	// Full override to support different max speeds for each movement mode including a ground speed limit for falling.
	FULL_OVERRIDE();

	switch (MovementMode)
	{
	case MOVE_Walking:
	case MOVE_NavWalking:
		return MaxWalkSpeed;
	case MOVE_Falling:
		return MaxFallingGroundSpeed;
	case MOVE_Swimming:
		return MaxSwimSpeed;
	case MOVE_Flying:
		return MaxFlySpeed;
	case MOVE_Custom:
		return MaxCustomMovementSpeed;
	default:
		return 0.f;
	}
}

FVector UExtCharacterMovementComponent::ScaleInputAcceleration(const FVector& InputAcceleration) const
{
	return Super::ScaleInputAcceleration(InputAcceleration) * InputAccelerationScale;
}

float UExtCharacterMovementComponent::GetMaxBrakingDeceleration() const
{
	FULL_OVERRIDE();

	check(ExtCharacterOwner);
	if (ExtCharacterOwner->IsRagdoll())
		return BrakingDecelerationRagdoll;

	if (ExtCharacterOwner->IsLanding())
		return BrakingDecelerationLanding;

	switch (MovementMode)
	{
	case MOVE_Walking:
	case MOVE_NavWalking:
		return BrakingDecelerationWalking;
	case MOVE_Falling:
		return BrakingDecelerationFalling;
	case MOVE_Swimming:
		return BrakingDecelerationSwimming;
	case MOVE_Flying:
		return BrakingDecelerationFlying;
	default:
		return 0.f;
	}
}

float UExtCharacterMovementComponent::GetBrakingFrictionFactor() const
{
	if (ExtCharacterOwner->IsRagdoll())
		return BrakingFrictionFactorRagdoll;

	if (ExtCharacterOwner->IsLanding())
		return BrakingFrictionFactorLanding;

	return BrakingFrictionFactor;
}

FVector UExtCharacterMovementComponent::GetSimulatedAcceleration() const
{
	return SimulatedAcceleration;
}


/// Rotations

bool UExtCharacterMovementComponent::ShouldRemainVertical() const
{
	// Full override to avoid unecessary checks.
	FULL_OVERRIDE();

	return true;
}

FORCEINLINE static float CalculateAdaptiveRotationRateFactor(const FBounds& SpeedInterval, const FBounds& RotationRateFactorInterval, float Speed)
{
	// Calculate a rotation rate factor based on how the provided speed compares to a speed interval.All interval values must be greater than or equal to 0.0f.
	// If the provided speed is below SpeedInterval.LowerBound, the resultant rate factor will be within the range[0, RotationRateFactorInterval.LowerBound]
	// following the same proportion.Otherwise it will be the unclampped mapping of speed in SpeedInterval to RotationRateFactorInterval.

	return (Speed > SpeedInterval.LowerBound) ? FMath::GetMappedRangeValueUnclamped(FVector2D(SpeedInterval), FVector2D(RotationRateFactorInterval), Speed)
		: FMath::GetMappedRangeValueClamped(FVector2D(0.f, SpeedInterval.LowerBound), FVector2D(0.f, RotationRateFactorInterval.LowerBound), Speed);
}

FORCEINLINE static float CalculateConstantDeltaRotationAxis(const float Current, const float Target, float DeltaTime, float InterpSpeed)
{
	// if DeltaSeconds is 0, do not perform any interpolation (Location was already calculated for that frame)
	if (InterpSpeed == 0.f || DeltaTime == 0.f || Current == Target)
	{
		return 0.f;
	}

	// Distance to reach
	const float Delta = FMath::FindDeltaAngleDegrees(Current, Target);

	// If no interp speed, jump to target value
	if (InterpSpeed < 0.f)
	{
		return Delta;
	}

	// If step is too small, jump to target value
	if (FMath::IsNearlyZero(Delta, 1e-3f))
	{
		return Delta;
	}

	const float DeltaInterpSpeed = DeltaTime * InterpSpeed;

	// Delta Move, Clamp so we do not over shoot.
	return FMath::Clamp<float>(Delta, -DeltaInterpSpeed, DeltaInterpSpeed);
}

FORCEINLINE static float CalculateInterpDeltaRotationAxis(const float Current, const float Target, float DeltaTime, float InterpSpeed)
{
	// if DeltaSeconds is 0, do not perform any interpolation (Location was already calculated for that frame)
	if (InterpSpeed == 0.f || DeltaTime == 0.f || Current == Target)
	{
		return 0.f;
	}

	// Distance to reach
	const float Delta = FMath::FindDeltaAngleDegrees(Current, Target);

	// If no interp speed, jump to target value
	if (InterpSpeed < 0.f)
	{
		return Delta;
	}

	// If step is too small, jump to target value
	if (FMath::IsNearlyZero(Delta, 1e-3f))
	{
		return Delta;
	}

	const float DeltaInterpSpeed = DeltaTime * InterpSpeed;

	// Delta Move, Clamp so we do not over shoot. Resort to a constant rotation if delta < 1.
	return (Delta < -1.0f || Delta > 1.0f) ? Delta * FMath::Clamp<float>(DeltaInterpSpeed, 0.f, 1.f)
		: FMath::Clamp<float>(Delta, -DeltaInterpSpeed, DeltaInterpSpeed);
}

FRotator UExtCharacterMovementComponent::GetRotationInterpSpeed(const FRotator& InterpSpeed, const FBounds& SpeedRange, const FBounds& FactorRange, const FBounds& Limits) const
{
	// Dynamic Rotation: adjust rotation rate according to ground speed
	float AdaptiveRotationFactor = 1.0f;
	if (bEnableAdaptiveRotationRate)
	{
		const float GroundSpeedSquared = Velocity.SizeSquared2D();
		if (MovementMode == MOVE_Walking || MovementMode == MOVE_NavWalking || MovementMode == MOVE_Flying)
		{
			AdaptiveRotationFactor = CalculateAdaptiveRotationRateFactor(FMath::Square(SpeedRange), FactorRange, GroundSpeedSquared);
		}

		return FRotator(
			FMath::Clamp(InterpSpeed.Pitch * AdaptiveRotationFactor, Limits.LowerBound, Limits.UpperBound),
			FMath::Clamp(InterpSpeed.Yaw * AdaptiveRotationFactor, Limits.LowerBound, Limits.UpperBound),
			FMath::Clamp(InterpSpeed.Roll * AdaptiveRotationFactor, Limits.LowerBound, Limits.UpperBound)
		);
	}

	return InterpSpeed;
}

FRotator UExtCharacterMovementComponent::GetDeltaRotation(float DeltaSeconds) const
{
	return Super::GetDeltaRotation(DeltaSeconds);
}

FRotator UExtCharacterMovementComponent::GetDeltaRotation(const FRotator& CurrentRotation, const FRotator& DesiredRotation, float DeltaSeconds) const
{
	const FRotator InterpSpeed = GetRotationInterpSpeed(RotationRate, AdaptiveRotationSettings.Speed, AdaptiveRotationSettings.RotationRateFactor, AdaptiveRotationSettings.RotationRateLimit);

	if (bInterpolateToTargetRotation)
		return FRotator(
			CalculateInterpDeltaRotationAxis(CurrentRotation.Pitch, DesiredRotation.Pitch, DeltaSeconds, InterpSpeed.Pitch),
			CalculateInterpDeltaRotationAxis(CurrentRotation.Yaw, DesiredRotation.Yaw, DeltaSeconds, InterpSpeed.Yaw),
			CalculateInterpDeltaRotationAxis(CurrentRotation.Roll, DesiredRotation.Roll, DeltaSeconds, InterpSpeed.Roll)
		);
	else 
		return FRotator(
			CalculateConstantDeltaRotationAxis(CurrentRotation.Pitch, DesiredRotation.Pitch, DeltaSeconds, InterpSpeed.Pitch),
			CalculateConstantDeltaRotationAxis(CurrentRotation.Yaw, DesiredRotation.Yaw, DeltaSeconds, InterpSpeed.Yaw),
			CalculateConstantDeltaRotationAxis(CurrentRotation.Roll, DesiredRotation.Roll, DeltaSeconds, InterpSpeed.Roll)
		);
}

FRotator UExtCharacterMovementComponent::ComputeOrientToMovementRotation(const FRotator& CurrentRotation, float DeltaSeconds, FRotator& DeltaRotation) const
{
	return Super::ComputeOrientToMovementRotation(CurrentRotation, DeltaSeconds, DeltaRotation);
}

FRotator UExtCharacterMovementComponent::ComputeOrientToMovementRotation(const FRotator& CurrentRotation, float DeltaSeconds)
{
	if (Velocity.SizeSquared() < KINDA_SMALL_NUMBER)
		return CurrentRotation;

	if (bUseVelocityAsMovementVector)
		return Velocity.GetSafeNormal().Rotation();
	else
	{
		if (Acceleration.SizeSquared() < KINDA_SMALL_NUMBER)
			// AI path following request can orient us in that direction (it's effectively an acceleration)
			return (bHasRequestedVelocity && RequestedVelocity.SizeSquared() > KINDA_SMALL_NUMBER) ? RequestedVelocity.GetSafeNormal().Rotation() : CurrentRotation;
		else 
			// Rotate toward direction of acceleration.
			return Acceleration.GetSafeNormal().Rotation();
	}
}

FRotator UExtCharacterMovementComponent::ComputeOrientToLookRotation(const FRotator& ControlRotation, const float NorthSegmentHalfWidth, float Buffer, float DeltaSeconds)
{
	float LookYawDelta = FMath::FindDeltaAngleDegrees(ControlRotation.Yaw, (Acceleration.SizeSquared2D() > KINDA_SMALL_NUMBER ? Acceleration : Velocity).Rotation().Yaw);
	LookCardinalDirection = FMathEx::FindCardinalDirection(LookYawDelta, LookCardinalDirection, NorthSegmentHalfWidth, Buffer);
	switch (LookCardinalDirection)
	{
	case ECardinalDirection::East:
		LookYawDelta -= 90.f;
		break;
	case ECardinalDirection::West:
		LookYawDelta += 90.f;
		break;
	case ECardinalDirection::South:
		if (LookYawDelta > 0.f)
			LookYawDelta -= 180.f;
		else
			LookYawDelta += 180.f;
		break;
	}

	RotationOffset = FMathEx::FSafeInterpTo(RotationOffset, LookYawDelta, DeltaSeconds, 5.0f);

	return FRotator(ControlRotation.Pitch, ControlRotation.Yaw + RotationOffset, ControlRotation.Roll);
}

bool UExtCharacterMovementComponent::CanTurnInPlaceInCurrentState() const
{
	checkComponentRoleAtLeast(ROLE_AutonomousProxy);

	// No need to test for ragdoll here since by definition the capsule does not rotate for ragdolls.
	// The root bone is adjusted instead.
	return bEnableTurnInPlace
		&& ExtCharacterOwner
		&& !ExtCharacterOwner->IsGettingUp();
}

void UExtCharacterMovementComponent::ResetTurnInPlaceState()
{
	// Reset TurnInPlace control variables.
	// We'd need benchmarks to support this but it's prob better to always assign instead of
	// conditionally assign to avoid stalls despite how good the CPU jump prediction could be.
	check(ExtCharacterOwner)
	bCanEnforceTurnInPlaceRotationMaxDistance = false;
	TurnInPlaceTargetYaw = -INFINITY;
	TurnInPlaceTimeCounter = 0.0f;
}

void UExtCharacterMovementComponent::ResetControllerDesireRotationState()
{
	// Reset Controller Desired Rotation control variables
	// We'd need benchmarks to support this but it's prob better to always assign instead of
	// conditionally assign to avoid stalls despite how good the CPU jump prediction could be.
	bCanEnforceControlRotationMaxDistance = false;
}

void UExtCharacterMovementComponent::PhysicsRotation(float DeltaSeconds)
{
	// Full override to support Rotation Modes, Adaptive Rotation rate and Turn In Place
	FULL_OVERRIDE();

	checkComponentRoleAtLeast(ROLE_AutonomousProxy);

	if (!HasValidData() || (!CharacterOwner->Controller && !bRunPhysicsWithNoController))
	{
		return;
	}

	FRotator CurrentRotation = UpdatedComponent->GetComponentRotation(); // Normalized
	CurrentRotation.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): CurrentRotation"));

	if (ExtCharacterOwner->IsRagdoll() || ExtCharacterOwner->IsGettingUp())
		return;

	FRotator DeltaRot(EForceInit::ForceInitToZero);

	if (bOrientRotationToMovement)
	{
		ResetTurnInPlaceState();
		ResetControllerDesireRotationState();

		// Sanity check
		check(RotationRateFactor >= 0.f && RotationRateFactor <= 1.f);

		const float AdjustedDeltaSeconds = RotationRateFactor * DeltaSeconds;

		// If falling use FallRotation which was set when the character started to fall which includes jumping, otherwise compute the movement rotation
		FRotator TargetRotation = (MovementMode != MOVE_Falling || (bCanRotateWhileJumping && ExtCharacterOwner->bIsJumping)) ? ComputeOrientToMovementRotation(CurrentRotation, AdjustedDeltaSeconds) : FallRotation;
		if (ShouldRemainVertical())
		{
			TargetRotation.Pitch = 0.f;
			TargetRotation.Roll = 0.f;
		}

		if (CurrentRotation.Equals(TargetRotation, AngleTolerance))
			return;

		// Calculate shortest direction delta rotation with no overshoot.
		DeltaRot = GetDeltaRotation(CurrentRotation, TargetRotation, AdjustedDeltaSeconds);
	}
	else if (CharacterOwner->Controller && bUseControllerDesiredRotation)
	{
		const FRotator ControlRotation = CharacterOwner->Controller->GetDesiredRotation();

		if (MovementMode == MOVE_Falling && !(bCanRotateWhileJumping && ExtCharacterOwner->bIsJumping))
		{
			ResetTurnInPlaceState();
			ResetControllerDesireRotationState();

			FRotator TargetRotation = FallRotation;
			if (ShouldRemainVertical())
			{
				TargetRotation.Pitch = 0.f;
				TargetRotation.Roll = 0.f;
			}

			if (CurrentRotation.Equals(TargetRotation, AngleTolerance))
				return;

			// Calculate shortest direction delta rotation with no overshoot.
			DeltaRot = GetDeltaRotation(CurrentRotation, TargetRotation, DeltaSeconds);
		}
		else // if (!IsFalling())
		{
			if (Velocity.SizeSquared2D() < KINDA_SMALL_NUMBER)
			{
				ResetControllerDesireRotationState();

				if (CanTurnInPlaceInCurrentState())
				{
					// Restore TurnInPlace from suspension.
					if (!FMath::IsFinite(TurnInPlaceTargetYaw) && TurnInPlaceTargetYaw < 0.f)
					{
						TurnInPlaceTargetYaw = INFINITY;
					}
					
					if (bUseTurnInPlaceDelay && TurnInPlaceDelay > 0.01f)
					{
						if (!FMath::IsFinite(TurnInPlaceTargetYaw)) // if not turning in place
						{
							const float MaxLookYawAngle = FMath::Clamp(LookAngleThreshold, 45.f, 90.f);
							const float LookYawDelta = FMath::FindDeltaAngleDegrees(CurrentRotation.Yaw, ControlRotation.Yaw);
							const float LookYawAngle = FMath::Abs(LookYawDelta);

							if (LookYawAngle > MaxLookYawAngle)
							{
								TurnInPlaceTimeCounter += DeltaSeconds;
								if (TurnInPlaceTimeCounter > TurnInPlaceDelay)
								{
									const bool bIsLookingRight = LookYawDelta >= 0.0f;
									const int32 TurnInPlaceSteps = ((FMath::FloorToInt(LookYawAngle - MaxLookYawAngle) / 90) + 1);
									const float TurnInPlaceAngle = TurnInPlaceSteps * (bIsLookingRight ? 90.0f : -90.f);

									TurnInPlaceTargetYaw = CurrentRotation.Yaw + TurnInPlaceAngle;
									TurnInPlaceTimeCounter = 0.0f;
								}
							}
							else
							{
								TurnInPlaceTimeCounter = 0.0f;
							}
						}
					}
					else
					{
						// Reset timer from delayed turn in place for when switched off in the middle of a countdown.
						TurnInPlaceTimeCounter = 0.0f;

						// Enforce max angular distance if needed.
						if (TurnInPlaceMaxDistance > 0.0f)
						{
							const float LookYawDelta = FMath::FindDeltaAngleDegrees(CurrentRotation.Yaw, ControlRotation.Yaw);
							if (LookYawDelta < -TurnInPlaceMaxDistance)
							{
								if (bCanEnforceTurnInPlaceRotationMaxDistance)
									CurrentRotation.Yaw = FRotator::NormalizeAxis(ControlRotation.Yaw + TurnInPlaceMaxDistance);
							}
							else if (LookYawDelta > TurnInPlaceMaxDistance)
							{
								if (bCanEnforceTurnInPlaceRotationMaxDistance)
									CurrentRotation.Yaw = FRotator::NormalizeAxis(ControlRotation.Yaw - TurnInPlaceMaxDistance);
							}
							else
							{
								bCanEnforceTurnInPlaceRotationMaxDistance = true;
							}
						}

						// Follow the current character rotation.
						const float CurrentTargetYaw = FMath::IsFinite(TurnInPlaceTargetYaw) ? TurnInPlaceTargetYaw : CurrentRotation.Yaw;
						const float LookYawDelta = FMath::FindDeltaAngleDegrees(CurrentTargetYaw, ControlRotation.Yaw);

						const float MaxLookYawAngle = FMath::Clamp(LookAngleThreshold, 45.f, 90.f);
						const float LookYawAngle = FMath::Abs(LookYawDelta);
						
						if (LookYawAngle > MaxLookYawAngle)
						{
							const bool bIsLookingRight = LookYawDelta >= 0.0f;
							const int32 TurnInPlaceSteps = ((FMath::FloorToInt(LookYawAngle - MaxLookYawAngle) / 90) + 1);
							const float TurnInPlaceAngle = TurnInPlaceSteps * (bIsLookingRight ? 90.0f : -90.f);

							TurnInPlaceTargetYaw = FMath::UnwindDegrees(CurrentTargetYaw + TurnInPlaceAngle);
						}
					}
				}
				else // if (!CanTurnInPlaceInCurrentState())
				{
					ResetTurnInPlaceState();
					return;
				}

				FRotator TargetRotation(
					ControlRotation.Pitch,
					FMath::IsFinite(TurnInPlaceTargetYaw) ? TurnInPlaceTargetYaw : CurrentRotation.Yaw,
					ControlRotation.Roll
				);

				if (ShouldRemainVertical())
				{
					TargetRotation.Pitch = 0.f;
					TargetRotation.Roll = 0.f;
				}

				if (CurrentRotation.Equals(TargetRotation, AngleTolerance))
				{
					TurnInPlaceTargetYaw = INFINITY;
					return;
				}

				DeltaRot.Pitch = CalculateConstantDeltaRotationAxis(CurrentRotation.Pitch, TargetRotation.Pitch, DeltaSeconds, TurnInPlaceRotationRate.Pitch);
				DeltaRot.Yaw = CalculateConstantDeltaRotationAxis(CurrentRotation.Yaw, TargetRotation.Yaw, DeltaSeconds, TurnInPlaceRotationRate.Yaw);
				DeltaRot.Roll = CalculateConstantDeltaRotationAxis(CurrentRotation.Roll, TargetRotation.Roll, DeltaSeconds, TurnInPlaceRotationRate.Roll);
			}
			else // if (IsMoving())
			{
				ResetTurnInPlaceState();

				// Enforce max angular distance if defined.
				if (ControlRotationMaxDistance > 0.0f)
				{
					float LookYawDelta = FMath::FindDeltaAngleDegrees(CurrentRotation.Yaw, ControlRotation.Yaw);
					if (LookYawDelta < -ControlRotationMaxDistance)
					{
						if (bCanEnforceControlRotationMaxDistance)
						{
							CurrentRotation.Yaw = FRotator::NormalizeAxis(ControlRotation.Yaw + ControlRotationMaxDistance);
							LookYawDelta = -ControlRotationMaxDistance;
						}
					}
					else  if (LookYawDelta > ControlRotationMaxDistance)
					{
						if (bCanEnforceControlRotationMaxDistance)
						{
							CurrentRotation.Yaw = FRotator::NormalizeAxis(ControlRotation.Yaw - ControlRotationMaxDistance);
							LookYawDelta = ControlRotationMaxDistance;
						}
					}
					else
					{
						bCanEnforceControlRotationMaxDistance = true;
					}
				}

				FRotator TargetRotation = ComputeOrientToLookRotation(ControlRotation, LookAngleThreshold, 5.0f, DeltaSeconds);
				if (ShouldRemainVertical())
				{
					TargetRotation.Pitch = 0.f;
					TargetRotation.Roll = 0.f;
				}

				if (CurrentRotation.Equals(TargetRotation, AngleTolerance))
					return;

				const float AdjustedDeltaSeconds = RotationRateFactor * DeltaSeconds;

				// Calculate shortest direction delta rotation with no overshoot.
				DeltaRot = GetDeltaRotation(CurrentRotation, TargetRotation, AdjustedDeltaSeconds);
			}
		}
	}
	else
	{
		ResetTurnInPlaceState();
		ResetControllerDesireRotationState();

		return;
	}
	
	DeltaRot.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): DeltaRotation"));

	// Set the new rotation.
	const FRotator DesiredRotation = CurrentRotation + DeltaRot;
	
	DesiredRotation.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): DesiredRotation"));
	MoveUpdatedComponent(FVector::ZeroVector, DesiredRotation, true);
}

void UExtCharacterMovementComponent::ResetRotationRateFactor()
{
	RotationRateFactor = 0.0f;
}



/// Crouch

bool UExtCharacterMovementComponent::CanCrouchInCurrentState() const
{
	// Full override to modify original conditions.
	// Character should not be allowed to remain crouched while falling/jumping.
	FULL_OVERRIDE();

	// Character can only crouch if moving on ground 
	return CanEverCrouch()
		&& IsMovingOnGround()
		&& ExtCharacterOwner
		&& !ExtCharacterOwner->IsRagdoll()
		&& !ExtCharacterOwner->IsGettingUp()
		&& !ExtCharacterOwner->bIsSprinting;
}



/// Walk

bool UExtCharacterMovementComponent::IsWalkingInsteadOfRunning() const
{
	if (ExtCharacterOwner)
	{
		return ExtCharacterOwner->bIsWalkingInsteadOfRunning;
	}

	return false;
}

void UExtCharacterMovementComponent::Walk(bool bClientSimulation)
{
	if (!HasValidData())
	{
		return;
	}

	// Sanity check
	check(bClientSimulation || CanWalkInCurrentState());

	if (ExtCharacterOwner)
	{
		if (!bClientSimulation)
		{
			ExtCharacterOwner->bIsWalkingInsteadOfRunning = true;
		}

		ExtCharacterOwner->OnStartWalk();
	}
}

void UExtCharacterMovementComponent::UnWalk(bool bClientSimulation)
{
	if (!HasValidData())
	{
		return;
	}

	if (CharacterOwner)
	{
		if (!bClientSimulation)
		{
			ExtCharacterOwner->bIsWalkingInsteadOfRunning = false;
		}

		ExtCharacterOwner->OnEndWalk();
	}
}

bool UExtCharacterMovementComponent::CanWalkInCurrentState() const
{
	// Character can walk in any situation.
	return CanEverWalkInsteadOfRun();
}



/// Sprint

bool UExtCharacterMovementComponent::IsSprinting() const
{
	if (ExtCharacterOwner)
	{
		return ExtCharacterOwner->bIsSprinting;
	}

	return false;
}

void UExtCharacterMovementComponent::Sprint(bool bClientSimulation)
{
	if (!HasValidData())
	{
		return;
	}

	// Sanity check
	check(bClientSimulation || CanSprintInCurrentState());

	if (ExtCharacterOwner)
	{ 
		if (!bClientSimulation)
		{
			ExtCharacterOwner->bIsSprinting = true;
		}

		ExtCharacterOwner->OnStartSprint();
	}
}

void UExtCharacterMovementComponent::UnSprint(bool bClientSimulation)
{
	if (!HasValidData())
	{
		return;
	}

	if (ExtCharacterOwner)
	{
		if (!bClientSimulation)
		{
			ExtCharacterOwner->bIsSprinting = false;
		}

		ExtCharacterOwner->OnEndSprint();
	}
}

bool UExtCharacterMovementComponent::CanSprintInCurrentState() const
{
	// Character can only sprint if not falling which includes jumping and not crouching.
	return CanEverSprint()
		&& IsMovingOnGround()
		&& ExtCharacterOwner
		&& !ExtCharacterOwner->IsRagdoll()
		&& !ExtCharacterOwner->IsGettingUp()
		&& !ExtCharacterOwner->bIsCrouched
		&& Velocity.SizeSquared2D() > KINDA_SMALL_NUMBER
		&& FVector::DotProduct(Acceleration.GetSafeNormal2D(), UpdatedComponent->GetForwardVector()) > 0.642788f;
}



/// Perform Action

bool UExtCharacterMovementComponent::IsPerformingGenericAction() const
{
	if (ExtCharacterOwner)
	{
		return ExtCharacterOwner->bIsPerformingGenericAction;
	}

	return false;
}

void UExtCharacterMovementComponent::PerformGenericAction(bool bClientSimulation)
{
	if (!HasValidData())
	{
		return;
	}

	// Sanity check
	check(bClientSimulation || CanPerformGenericActionInCurrentState());

	if (ExtCharacterOwner)
	{
		if (!bClientSimulation)
		{
			ExtCharacterOwner->bIsPerformingGenericAction = true;
		}

		ExtCharacterOwner->OnStartGenericAction();
	}
}

void UExtCharacterMovementComponent::UnPerformGenericAction(bool bClientSimulation)
{
	if (!HasValidData())
	{
		return;
	}

	if (ExtCharacterOwner)
	{
		if (!bClientSimulation)
		{
			ExtCharacterOwner->bIsPerformingGenericAction = false;
		}

		ExtCharacterOwner->OnEndGenericAction();
	}
}

bool UExtCharacterMovementComponent::CanPerformGenericActionInCurrentState() const
{
	// Character can only perform action if not falling, sprinting ragdoll or getting up.
	return CanEverPerformGenericAction()
		&& MovementMode != MOVE_None && MovementMode != MOVE_Falling
		&& ExtCharacterOwner
		&& !ExtCharacterOwner->bIsSprinting
		&& !ExtCharacterOwner->IsRagdoll()
		&& !ExtCharacterOwner->IsGettingUp();
}



/// Jump

bool UExtCharacterMovementComponent::IsJumping() const
{
	return ExtCharacterOwner && ExtCharacterOwner->bIsJumping;
}

bool UExtCharacterMovementComponent::IsLanding() const
{
	return ExtCharacterOwner && ExtCharacterOwner->IsLanding();
}



/// Walk Off Ledges

bool UExtCharacterMovementComponent::CanWalkOffLedges() const
{
	FULL_OVERRIDE();

	if (!bCanWalkOffLedges)
		return false;

	if (ExtCharacterOwner)
	{
		if (!bCanWalkOffLedgesWhenSprinting && ExtCharacterOwner->bIsSprinting)
			return false;

		if (!bCanWalkOffLedgesWhenPerformingGenericAction && ExtCharacterOwner->bIsPerformingGenericAction)
			return false;

		if (!bCanWalkOffLedgesWhenCrouching && ExtCharacterOwner->bIsCrouched)
			return false;

		if (!bCanWalkOffLedgesWhenWalking && ExtCharacterOwner->bIsWalkingInsteadOfRunning)
			return false;

		if (!bCanWalkOffLedgesWhenRunning && !ExtCharacterOwner->bIsWalkingInsteadOfRunning)
			return false;
	}

	return true;
}



/// Stop Prediction

bool UExtCharacterMovementComponent::PredictStopLocation(FVector& OutStopLocation, const float TimeLimit, const float TimeStep)
{
	// Cannot predict a stop with invalid data
	if (!HasValidData())
		return false;

	// Cannot predict a stop in anything below autonomous
	if (GetIsReplicated() && CharacterOwner->GetLocalRole() < ROLE_Authority)
		return false;

	// Cannot predict a stop if TimeStep or TimeLit are too small
	if (TimeStep < MIN_TICK_TIME || TimeLimit < TimeStep)
		return false;

	float Friction;
	bool bFluid;
	switch (MovementMode)
	{
		case MOVE_Walking:
		case MOVE_NavWalking:
			Friction = GroundFriction;
			bFluid = false;;
			break;
		case MOVE_Flying:
			Friction = 0.5f * GetPhysicsVolume()->FluidFriction;
			bFluid = true;
			break;
		default:
			// It doesn't make much sense to try to predict a stop when falling or swimming because of gravity and buoyancy. There are better ways to do that using traces.
			return false;
	}

	const float FrictionFactor = FMath::Max(0.f, GetBrakingFrictionFactor());
	Friction = FMath::Max(0.f, Friction * FrictionFactor);

	const bool bZeroAcceleration = Acceleration.IsZero();
	const float BrakingDeceleration = FMath::Max(GetMaxBrakingDeceleration(), 0.f);
	const bool bZeroBraking = (BrakingDeceleration == 0.f);

	const float ActualBrakingFriction = FMath::Max((bUseSeparateBrakingFriction ? BrakingFriction : Friction), 0.f);
	const bool bZeroBrakingFriction = (ActualBrakingFriction == 0.f);
	const bool bZeroFluidFriction = !bFluid || (Friction == 0.f);

	// Early out if we have zero acceleration and there is no breaking or friction
	if (bZeroAcceleration && bZeroBraking && bZeroBrakingFriction && bZeroFluidFriction)
		return false;

	const FVector AccelDir = bZeroAcceleration ? FVector::ZeroVector : Acceleration.GetSafeNormal();

	FVector LastVelocity = bZeroAcceleration ? Velocity : Velocity.ProjectOnToNormal(AccelDir);

	OutStopLocation = UpdatedComponent->GetComponentLocation();

	const int32 MaxPredictionIterations = TimeLimit / TimeStep;
	for (int32 Iterations = 0; Iterations < MaxPredictionIterations; ++Iterations)
	{
		// Only apply braking if there is no acceleration
		if (bZeroAcceleration)
		{
			const FVector OldVelocity = LastVelocity;

			// subdivide braking to get reasonably consistent results at lower frame rates
			// (important for packet loss situations w/ networking)
			float RemainingTime = TimeStep;
			const float MaxTimeStep = (1.0f / 33.0f);

			// Decelerate to brake to a stop
			const FVector RevAccel = (bZeroBraking ? FVector::ZeroVector : (BrakingDeceleration * LastVelocity.GetSafeNormal()));
			while (RemainingTime >= MIN_TICK_TIME)
			{
				// Zero friction uses constant deceleration, so no need for iteration.
				const float dt = ((RemainingTime > MaxTimeStep && !bZeroBrakingFriction) ? FMath::Min(MaxTimeStep, RemainingTime * 0.5f) : RemainingTime);
				RemainingTime -= dt;

				// apply friction and braking (reverse acceleration).
				LastVelocity -= (LastVelocity * ActualBrakingFriction + RevAccel) * dt;

				// Don't reverse direction
				if ((LastVelocity | OldVelocity) <= 0.f)
					return true;
			}
		}
		else
		{
			// Friction affects our ability to change direction. This is only done for input acceleration, not path following.
			const float VelSize = LastVelocity.Size();
			LastVelocity = LastVelocity - (LastVelocity - AccelDir * VelSize) * FMath::Min(TimeStep * Friction, 1.f);
		}

		// Apply fluid friction
		if (bFluid)
		{
			LastVelocity = LastVelocity * (1.f - FMath::Min(Friction * TimeStep, 1.f));
		}

		// Clamp to zero if nearly zero, or if below min threshold and braking.
		const float LastVelocitySquared = LastVelocity.SizeSquared();
		if (LastVelocitySquared <= FMath::Square(BrakingSpeedTolerance) || (!bZeroBraking && LastVelocitySquared < (BRAKE_TO_STOP_VELOCITY * BRAKE_TO_STOP_VELOCITY)))
			return true;

		OutStopLocation += LastVelocity * TimeStep;
	}

	return false;
}



/// Movement Prediction and Replication

void UExtCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	bWantsToWalkInsteadOfRun = ((Flags & FSavedMove_ExtCharacter::FLAG_WantsToWalkInsteadOfRun) != 0);
	bWantsToSprint = ((Flags & FSavedMove_ExtCharacter::FLAG_WantsToSprint) != 0);
	bWantsToPerformGenericAction = ((Flags & FSavedMove_ExtCharacter::FLAG_WantsToPerformGenericAction) != 0);
}

FNetworkPredictionData_Client* UExtCharacterMovementComponent::GetPredictionData_Client() const
{
	// Full override to use our own client prediction data class
	FULL_OVERRIDE();

	if (ClientPredictionData == nullptr)
	{
		UExtCharacterMovementComponent* MutableThis = const_cast<UExtCharacterMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_ExtCharacter(*this);

		// No need to assign MaxSmoothNetUpdateDist or NoSmoothNetUpdateDist here.
		// They are initialized from ClientMovement.NetworkMaxSmoothUpdateDistance and
		// ClientMovement.NetworkNoSmoothUpdateDistance respectively in the 
		// FNetworkPredictionData_Client ctor
	}

	return ClientPredictionData;
}

void FSavedMove_ExtCharacter::Clear()
{
	Super::Clear();

	bWantsToWalkInsteadOfRun = false;
	bWantsToSprint = false;
	bWantsToPerformGenericAction = false;
}

void FSavedMove_ExtCharacter::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	const UExtCharacterMovementComponent* const ExtCharacterMovement = CastChecked<UExtCharacterMovementComponent>(Character->GetCharacterMovement());

	bWantsToWalkInsteadOfRun = ExtCharacterMovement->bWantsToWalkInsteadOfRun;
	bWantsToSprint = ExtCharacterMovement->bWantsToSprint;
	bWantsToPerformGenericAction = ExtCharacterMovement->bWantsToPerformGenericAction;
}

void FSavedMove_ExtCharacter::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	// This is just the exact opposite of SetMoveFor. It copies the state from the saved move to the movement
	// component before a correction is made to a client.
	// Don't update flags here. They're automatically setup before corrections using the compressed flag methods.
}

uint8 FSavedMove_ExtCharacter::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (bWantsToWalkInsteadOfRun)
		Result |= FLAG_WantsToWalkInsteadOfRun;

	if (bWantsToSprint)
		Result |= FLAG_WantsToSprint;

	if (bWantsToPerformGenericAction)
		Result |= FLAG_WantsToPerformGenericAction;

	return Result;
}

FNetworkPredictionData_Client_ExtCharacter::FNetworkPredictionData_Client_ExtCharacter(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{

}

FSavedMovePtr FNetworkPredictionData_Client_ExtCharacter::AllocateNewMove()
{
	// Full override to instatiate our own saved move class
	FULL_OVERRIDE();

	return FSavedMovePtr(new FSavedMove_ExtCharacter());
}

