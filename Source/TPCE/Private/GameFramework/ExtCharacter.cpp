// Fill out your copyright notice in the Description page of Project Settings.

#include "GameFramework/ExtCharacter.h"
#include "GameFramework/ExtCharacterMovementComponent.h"

#include "GameFramework/PlayerController.h"
#include "Components/SceneComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "Curves/CurveFloat.h"
#include "Animation/AnimInstance.h"
#include "Animation/ExtCharacterAnimInstance.h"

#include "Net/UnrealNetwork.h"
#include "Logging/LogMacros.h"
#include "Kismet/Kismet.h"

#include "DisplayDebugHelpers.h"
#include "DrawDebugHelpers.h"

#include "EngineUtils.h"
#include "Engine/CollisionProfile.h"
#include "Engine/World.h"
#include "Engine/Canvas.h"

#include "PhysicsEngine/BodySetup.h"

#include "ExtraMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogExtCharacter, Log, All);

#define LOCTEXT_NAMESPACE "ExtCharacter"

AExtCharacter::AExtCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UExtCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Structure to hold one-time initialization
	static const struct FConstructorStatics
	{
		FName ID_Characters;
		FText NAME_Characters;

#if WITH_EDITOR
#if WITH_EDITORONLY_DATA
		FName NAME_LookRotationArrow;
		FName NAME_LastVelocityArrow;
		FName NAME_VelocityArrow;
		FName NAME_LookRotationYawArrow;
		FName NAME_LastAccelerationArrow;
		FName NAME_AccelerationArrow;
#endif
#endif
		FName NAME_MoveForwardInput;
		FName NAME_MoveRightInput;
		FName NAME_LookUpInput;
		FName NAME_LookRightInput;
		FName NAME_CrouchInput;
		FName NAME_JumpInput;
		FName NAME_WalkInput;
		FName NAME_SprintInput;
		FName NAME_GenericActionInput;
		FName NAME_FireInput;
		FName NAME_RagdollMeshCollisionProfile;
		FName NAME_RagdollCapsuleCollisionProfile;
		FName NAME_RagdollMeshConstraintProfile;
		FName NAME_PelvisBone;
		FName NAME_LeftFootBone;
		FName NAME_RightFootBone;

		FConstructorStatics() :
			ID_Characters(TEXT("Characters")),
			NAME_Characters(NSLOCTEXT("SpriteCategory", "Characters", "Characters")),

#if WITH_EDITOR
#if WITH_EDITORONLY_DATA
			NAME_LookRotationArrow(TEXT("LookRotationArrow")),
			NAME_LastVelocityArrow(TEXT("LastVelocityArrow")),
			NAME_VelocityArrow(TEXT("VelocityArrow")),
			NAME_LookRotationYawArrow(TEXT("LookRotationYawArrow")),
			NAME_LastAccelerationArrow(TEXT("LastAccelerationArrow")),
			NAME_AccelerationArrow(TEXT("AccelerationArrow")),
#endif
#endif

			NAME_MoveForwardInput(TEXT("MoveForward")),
			NAME_MoveRightInput(TEXT("MoveRight")),
			NAME_LookUpInput(TEXT("LookUp")),
			NAME_LookRightInput(TEXT("LookRight")),
			NAME_CrouchInput(TEXT("Crouch")),
			NAME_JumpInput(TEXT("Jump")),
			NAME_WalkInput(TEXT("Walk")),
			NAME_SprintInput(TEXT("Sprint")),
			NAME_GenericActionInput(TEXT("GenericAction")),
			NAME_FireInput(TEXT("Fire")),
			NAME_RagdollMeshCollisionProfile(NAME_Ragdoll),
			NAME_RagdollCapsuleCollisionProfile(NAME_Spectator),
			NAME_RagdollMeshConstraintProfile(NAME_Ragdoll),
			NAME_PelvisBone(NAME_Pelvis),
			NAME_LeftFootBone(NAME_Foot_L),
			NAME_RightFootBone(NAME_Foot_R)
		{
		}
	} ConstructorStatics;

	// Input settings
	MoveForwardInputName = ConstructorStatics.NAME_MoveForwardInput;
	MoveRightInputName = ConstructorStatics.NAME_MoveRightInput;
	LookUpInputName = ConstructorStatics.NAME_LookUpInput;
	LookRightInputName = ConstructorStatics.NAME_LookRightInput;
	CrouchInputName = ConstructorStatics.NAME_CrouchInput;
	JumpInputName = ConstructorStatics.NAME_JumpInput;
	WalkInputName = ConstructorStatics.NAME_WalkInput;
	SprintInputName = ConstructorStatics.NAME_SprintInput;
	GenericActionInputName = ConstructorStatics.NAME_GenericActionInput;
	FireInputName = ConstructorStatics.NAME_FireInput;

	RagdollMeshCollisionProfileName = ConstructorStatics.NAME_RagdollMeshCollisionProfile;
	RagdollCapsuleCollisionProfileName = ConstructorStatics.NAME_RagdollCapsuleCollisionProfile;
	RagdollMeshConstraintProfileName = ConstructorStatics.NAME_RagdollMeshConstraintProfile;

	PelvisBoneName = ConstructorStatics.NAME_PelvisBone;
	LeftFootBoneName = ConstructorStatics.NAME_LeftFootBone;
	RightFootBoneName = ConstructorStatics.NAME_RightFootBone;

	// Animation
	bEnableFootIK = true;

	// Look rotation settings
	LookUpInputSpeed = 0.0f;
	LookRightInputSpeed = 0.0f;

	// Camera control settings
	bFindCameraComponentWhenViewTarget = true;
	bRelayCameraFunctionsToController = false;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 92.0f);

	// GetUp Settings
	GetUpDelay = 1.0f;

	// Jump Settings
	JumpMaxHoldTime = 0.2f;
	LandingDelay = 0.5f;

	// Educated guesses based on capsule size.
	CrouchedEyeHeight = 48;
	BaseEyeHeight = 80;

	// Don't rotate when the controller rotates let the movement component do the work.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	RotationMode = ECharacterRotationMode::OrientToMovement;

	// Stop moving when unpossessed
	bStopWhenUnpossessed = true;

	// State
	bIsRagdoll = false;
	bIsFemale = false;
	bIsWalkingInsteadOfRunning = false;
	bIsSprinting = false;
	bIsPerformingGenericAction = false;
	bIsJumping = false;
	bHasLandedSafely = true;

	// Default gait
	Gait = ECharacterGait::Run;

	bIgnoreLookInputWhenRagdoll = false;
	bIgnoreMoveInputWhenRagdoll = true;

	MovementSettings.Primary.Standing.Walk.MaxSpeed = 165.f;
	MovementSettings.Primary.Standing.Walk.MaxAcceleration = 800.f;
	MovementSettings.Primary.Standing.Walk.BrakingDeceleration = 400.f;
	MovementSettings.Primary.Standing.Walk.Friction = 8.f;
	MovementSettings.Primary.Standing.Walk.BrakingFrictionFactor = 0.f;

	MovementSettings.Primary.Standing.Run.MaxSpeed = 375.f;
	MovementSettings.Primary.Standing.Run.MaxAcceleration = 1000.f;
	MovementSettings.Primary.Standing.Run.BrakingDeceleration = 600.f;
	MovementSettings.Primary.Standing.Run.Friction = 6.f;
	MovementSettings.Primary.Standing.Run.BrakingFrictionFactor = 0.f;

	MovementSettings.Primary.Crouched.Walk.MaxSpeed = 150.f;
	MovementSettings.Primary.Crouched.Walk.MaxAcceleration = 600.f;
	MovementSettings.Primary.Crouched.Walk.BrakingDeceleration = 600.f;
	MovementSettings.Primary.Crouched.Walk.Friction = 8.f;
	MovementSettings.Primary.Crouched.Walk.BrakingFrictionFactor = 0.f;

	MovementSettings.Primary.Crouched.Run.MaxSpeed = 200.f;
	MovementSettings.Primary.Crouched.Run.MaxAcceleration = 800.f;
	MovementSettings.Primary.Crouched.Run.BrakingDeceleration = 600.f;
	MovementSettings.Primary.Crouched.Run.Friction = 8.f;
	MovementSettings.Primary.Crouched.Run.BrakingFrictionFactor = 0.f;

	MovementSettings.Secondary.Standing.Walk.MaxSpeed = 165.f;
	MovementSettings.Secondary.Standing.Walk.MaxAcceleration = 800.f;
	MovementSettings.Secondary.Standing.Walk.BrakingDeceleration = 400.f;
	MovementSettings.Secondary.Standing.Walk.Friction = 8.f;
	MovementSettings.Secondary.Standing.Walk.BrakingFrictionFactor = 0.f;

	MovementSettings.Secondary.Standing.Run.MaxSpeed = 375.f;
	MovementSettings.Secondary.Standing.Run.MaxAcceleration = 1000.f;
	MovementSettings.Secondary.Standing.Run.BrakingDeceleration = 600.f;
	MovementSettings.Secondary.Standing.Run.Friction = 6.f;
	MovementSettings.Secondary.Standing.Run.BrakingFrictionFactor = 0.f;

	MovementSettings.Secondary.Crouched.Walk.MaxSpeed = 150.f;
	MovementSettings.Secondary.Crouched.Walk.MaxAcceleration = 600.f;
	MovementSettings.Secondary.Crouched.Walk.BrakingDeceleration = 600.f;
	MovementSettings.Secondary.Crouched.Walk.Friction = 8.f;
	MovementSettings.Secondary.Crouched.Walk.BrakingFrictionFactor = 0.f;

	MovementSettings.Secondary.Crouched.Run.MaxSpeed = 200.f;
	MovementSettings.Secondary.Crouched.Run.MaxAcceleration = 800.f;
	MovementSettings.Secondary.Crouched.Run.BrakingDeceleration = 600.f;
	MovementSettings.Secondary.Crouched.Run.Friction = 8.f;
	MovementSettings.Secondary.Crouched.Run.BrakingFrictionFactor = 0.f;

	MovementSettings.Sprint.MaxSpeed = 600.f;
	MovementSettings.Sprint.MaxAcceleration = 1000.f;
	MovementSettings.Sprint.BrakingDeceleration = 800.f;
	MovementSettings.Sprint.Friction = 0.5f;
	MovementSettings.Sprint.BrakingFrictionFactor = 0.f;

	UExtCharacterMovementComponent* ExtCharacterMovement = CastChecked<UExtCharacterMovementComponent>(GetCharacterMovement());

	ExtCharacterMovement->NavAgentProps.bCanCrouch = true;
	ExtCharacterMovement->NavAgentProps.bCanJump = true;
	ExtCharacterMovement->NavAgentProps.bCanWalk = true;
	ExtCharacterMovement->NavAgentProps.bCanSwim = false;
	ExtCharacterMovement->NavAgentProps.bCanFly = false;
	ExtCharacterMovement->ResetMoveState();

	ExtCharacterMovement->ExtraMovementProps.bCanWalkInsteadOfRun = true;
	ExtCharacterMovement->ExtraMovementProps.bCanSprint = true;
	ExtCharacterMovement->ExtraMovementProps.bCanPerformGenericAction = true;
	ExtCharacterMovement->ResetExtraMoveState();

	ExtCharacterMovement->bUseVelocityAsMovementVector = true;
	
	ExtCharacterMovement->CrouchedHalfHeight = 60.0f;
	ExtCharacterMovement->SetWalkableFloorAngle(50.0f);

	ExtCharacterMovement->MaxWalkSpeed = MovementSettings.Primary.Standing.Walk.MaxSpeed;
	ExtCharacterMovement->MaxWalkAcceleration = MovementSettings.Primary.Standing.Walk.MaxAcceleration;
	ExtCharacterMovement->WalkFriction = MovementSettings.Primary.Standing.Walk.Friction;
	ExtCharacterMovement->BrakingDecelerationWalking = MovementSettings.Primary.Standing.Walk.BrakingDeceleration;
	ExtCharacterMovement->BrakingFrictionFactor = MovementSettings.Primary.Standing.Walk.BrakingFrictionFactor;


#if WITH_EDITOR
#if WITH_EDITORONLY_DATA
	float const HalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();

	LookRotationArrow = CreateEditorOnlyDefaultSubobject<UArrowComponent>(ConstructorStatics.NAME_LookRotationArrow);
	if (LookRotationArrow)
	{
		LookRotationArrow->ArrowColor = FColor(0, 214, 255);
		LookRotationArrow->ArrowSize = 0.74f;
		LookRotationArrow->bIsScreenSizeScaled = false;
		LookRotationArrow->bUseInEditorScaling = false;
		LookRotationArrow->bTreatAsASprite = false;
		LookRotationArrow->SpriteInfo.Category = ConstructorStatics.ID_Characters;
		LookRotationArrow->SpriteInfo.DisplayName = ConstructorStatics.NAME_Characters;
		LookRotationArrow->SetupAttachment(RootComponent);
		LookRotationArrow->SetRelativeLocation(FVector(0.f, 0.f, BaseEyeHeight));
		LookRotationArrow->SetVisibility(false);
	}

	LastVelocityArrow = CreateEditorOnlyDefaultSubobject<UArrowComponent>(ConstructorStatics.NAME_LastVelocityArrow);
	if (LastVelocityArrow)
	{
		LastVelocityArrow->ArrowColor = FColor(137, 0, 137);
		LastVelocityArrow->ArrowSize = 1.f;
		LastVelocityArrow->bIsScreenSizeScaled = false;
		LastVelocityArrow->bUseInEditorScaling = false;
		LastVelocityArrow->ScreenSize = 0.0025f;
		LastVelocityArrow->bTreatAsASprite = false;
		LastVelocityArrow->SpriteInfo.Category = ConstructorStatics.ID_Characters;
		LastVelocityArrow->SpriteInfo.DisplayName = ConstructorStatics.NAME_Characters;
		LastVelocityArrow->SetupAttachment(RootComponent);
		LastVelocityArrow->SetRelativeLocation(FVector(0.f, 0.f, -HalfHeight));
		LastVelocityArrow->SetRelativeScale3D(FVector(1.0f, 4.0f, 0.0f));
		LastVelocityArrow->SetVisibility(false);
	}

	VelocityArrow = CreateEditorOnlyDefaultSubobject<UArrowComponent>(ConstructorStatics.NAME_VelocityArrow);
	if (VelocityArrow)
	{
		VelocityArrow->ArrowColor = FColor::Purple;
		VelocityArrow->ArrowSize = 1.f;
		VelocityArrow->bIsScreenSizeScaled = false;
		VelocityArrow->bUseInEditorScaling = false;
		VelocityArrow->ScreenSize = 0.0025f;
		VelocityArrow->bTreatAsASprite = false;
		VelocityArrow->SpriteInfo.Category = ConstructorStatics.ID_Characters;
		VelocityArrow->SpriteInfo.DisplayName = ConstructorStatics.NAME_Characters;
		VelocityArrow->SetupAttachment(RootComponent);
		VelocityArrow->SetRelativeLocation(FVector(0.f, 0.f, 0.2f - HalfHeight));
		VelocityArrow->SetRelativeScale3D(FVector(1.0f, 4.0f, 0.0f));
		VelocityArrow->SetVisibility(false);
	}

	LookRotationYawArrow = CreateEditorOnlyDefaultSubobject<UArrowComponent>(ConstructorStatics.NAME_LookRotationYawArrow);
	if (LookRotationYawArrow)
	{
		LookRotationYawArrow->ArrowColor = FColor::Orange;
		LookRotationYawArrow->ArrowSize = 1.f;
		LookRotationYawArrow->bIsScreenSizeScaled = false;
		LookRotationYawArrow->bUseInEditorScaling = false;
		LookRotationYawArrow->ScreenSize = 0.0025f;
		LookRotationYawArrow->bTreatAsASprite = false;
		LookRotationYawArrow->SpriteInfo.Category = ConstructorStatics.ID_Characters;
		LookRotationYawArrow->SpriteInfo.DisplayName = ConstructorStatics.NAME_Characters;
		LookRotationYawArrow->SetupAttachment(RootComponent);
		LookRotationYawArrow->SetRelativeLocation(FVector(0.f, 0.f, 0.4f - HalfHeight));
		LookRotationYawArrow->SetRelativeScale3D(FVector(1.0f, 1.75f, 0.0f));
		LookRotationYawArrow->SetVisibility(false);
	}

	LastAccelerationArrow = CreateEditorOnlyDefaultSubobject<UArrowComponent>(ConstructorStatics.NAME_LastAccelerationArrow);
	if (LastAccelerationArrow)
	{
		LastAccelerationArrow->ArrowColor = FColor(137, 137, 0);
		LastAccelerationArrow->ArrowSize = 1.f;
		LastAccelerationArrow->bIsScreenSizeScaled = false;
		LastAccelerationArrow->bUseInEditorScaling = false;
		LastAccelerationArrow->ScreenSize = 0.0025f;
		LastAccelerationArrow->bTreatAsASprite = false;
		LastAccelerationArrow->SpriteInfo.Category = ConstructorStatics.ID_Characters;
		LastAccelerationArrow->SpriteInfo.DisplayName = ConstructorStatics.NAME_Characters;
		LastAccelerationArrow->SetupAttachment(RootComponent);
		LastAccelerationArrow->SetRelativeLocation(FVector(0.f, 0.f, 0.6f - HalfHeight));
		LastAccelerationArrow->SetRelativeScale3D(FVector(1.0f, 1.75f, 0.0f));
		LastAccelerationArrow->SetVisibility(false);
	}

	AccelerationArrow = CreateEditorOnlyDefaultSubobject<UArrowComponent>(ConstructorStatics.NAME_AccelerationArrow);
	if (AccelerationArrow)
	{
		AccelerationArrow->ArrowColor = FColor::Yellow;
		AccelerationArrow->ArrowSize = 1.f;
		AccelerationArrow->bIsScreenSizeScaled = false;
		AccelerationArrow->bUseInEditorScaling = false;
		AccelerationArrow->ScreenSize = 0.0025f;
		AccelerationArrow->bTreatAsASprite = false;
		AccelerationArrow->SpriteInfo.Category = ConstructorStatics.ID_Characters;
		AccelerationArrow->SpriteInfo.DisplayName = ConstructorStatics.NAME_Characters;
		AccelerationArrow->SetupAttachment(RootComponent);
		AccelerationArrow->SetRelativeLocation(FVector(0.f, 0.f, 0.8f - HalfHeight));
		AccelerationArrow->SetRelativeScale3D(FVector(1.0f, 1.75f, 0.0f));
		AccelerationArrow->SetVisibility(false);
	}

#endif // WITH_EDITORONLY_DATA
#endif // WITH_EDITOR

	// Enable Replication
	bReplicates = true;
	SetReplicatingMovement(true);
	bNetUseOwnerRelevancy = true;

	// Replication Settings
	ExtCharacterMovement->NetworkSimulatedSmoothRotationTime = 0.05f;
	ExtCharacterMovement->ListenServerNetworkSimulatedSmoothRotationTime = 0.05f;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// must be set in the derived blueprint assets to avoid direct content references in C++.
}


/// Replication

void AExtCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Change the condition of the replicated movement property to not replicate in replays since its handled specifically via saving external replay data
	// TODO: check if saved replay data should include fields from ReplicatedExtMovement
	DOREPLIFETIME_CONDITION(AExtCharacter, ReplicatedExtMovement, COND_SimulatedOrPhysicsNoReplay);

	DOREPLIFETIME_CONDITION(AExtCharacter, ReplicatedExtMovementMode, COND_SimulatedOnly);

	DOREPLIFETIME_CONDITION(AExtCharacter, ReplicatedLook, COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(AExtCharacter, ReplicatedLookAtActor, COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(AExtCharacter, RotationMode, COND_SimulatedOnly);

	DOREPLIFETIME_CONDITION(AExtCharacter, bIsWalkingInsteadOfRunning, COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(AExtCharacter, bIsSprinting, COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(AExtCharacter, bIsPerformingGenericAction, COND_SimulatedOnly);
}

void AExtCharacter::PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker)
{
	// Needed full override to replace the original ReplicatedMovement for our ReplicatedExtMovement
	// and to avoid having RemoteViewPitch replicated unecessarily.
	FULL_OVERRIDE();

	// Workaround:: Skip original ReplicatedMovement if the custom tailored one can be used.
	if (IsReplicatingMovement() || GetAttachmentReplication().AttachParent)
	{
		if (GatherExtMovement())
		{
			DOREPLIFETIME_ACTIVE_OVERRIDE(AExtCharacter, ReplicatedExtMovement, IsReplicatingMovement());
			PRAGMA_DISABLE_DEPRECATION_WARNINGS
				DOREPLIFETIME_ACTIVE_OVERRIDE(AActor, ReplicatedMovement, false);
			PRAGMA_ENABLE_DEPRECATION_WARNINGS
		}
		else
		{
			GatherCurrentMovement();
			DOREPLIFETIME_ACTIVE_OVERRIDE(AExtCharacter, ReplicatedExtMovement, false);
		}
	}
	else
	{
		DOREPLIFETIME_ACTIVE_OVERRIDE(AExtCharacter, ReplicatedExtMovement, false);
		PRAGMA_DISABLE_DEPRECATION_WARNINGS
			DOREPLIFETIME_ACTIVE_OVERRIDE(AActor, ReplicatedMovement, false);
		PRAGMA_ENABLE_DEPRECATION_WARNINGS
	}

	// Workaround: Disable replication of ReplicatedMovementMode since we have a custom movement mode replication that includes jump state info.
	// Cannot use DOREPLIFETIME_ACTIVE_OVERRIDE(ACharacter, ReplicatedMovementMode, false) here because GET_MEMBER_NAME_CHECKED does not support 
	// protected/private members.
	{
		static const FName ReplicatedMovementModePropertyName(TEXT("ReplicatedMovementMode"));
		static UProperty* ReplicatedMovementModeProperty = GetReplicatedProperty(StaticClass(), ACharacter::StaticClass(), ReplicatedMovementModePropertyName);
		ChangedPropertyTracker.SetCustomIsActiveOverride(ReplicatedMovementModeProperty->RepIndex, false);
	}

	// Workaround: RemoteViewPitch is taken from ReplicatedLook.Rotation.Pitch
	DOREPLIFETIME_ACTIVE_OVERRIDE(ACharacter, RemoteViewPitch, false);

	// Workaround: Jump state is replicated with movement mode and use of IsJumpForceApplied() is not even reliable as it only covers half of the jump.
	// The server must dictate if a fall is a jump or not in any stage of the fall.
	DOREPLIFETIME_ACTIVE_OVERRIDE(ACharacter, bProxyIsJumpForceApplied, false);

	UCharacterMovementComponent* MyCharacterMovement = GetCharacterMovement();
	check(MyCharacterMovement);
	if (MyCharacterMovement->CurrentRootMotion.HasActiveRootMotionSources() || IsPlayingNetworkedRootMotionMontage())
	{
		const FAnimMontageInstance* RootMotionMontageInstance = GetRootMotionAnimMontageInstance();

		RepRootMotion.bIsActive = true;
		// Is position stored in local space?
		RepRootMotion.bRelativePosition = BasedMovement.HasRelativeLocation();
		RepRootMotion.bRelativeRotation = BasedMovement.HasRelativeRotation();
		RepRootMotion.Location = RepRootMotion.bRelativePosition ? BasedMovement.Location : FRepMovement::RebaseOntoZeroOrigin(GetActorLocation(), GetWorld()->OriginLocation);
		RepRootMotion.Rotation = RepRootMotion.bRelativeRotation ? BasedMovement.Rotation : GetActorRotation();
		RepRootMotion.MovementBase = BasedMovement.MovementBase;
		RepRootMotion.MovementBaseBoneName = BasedMovement.BoneName;
		if (RootMotionMontageInstance)
		{
			RepRootMotion.AnimMontage = RootMotionMontageInstance->Montage;
			RepRootMotion.Position = RootMotionMontageInstance->GetPosition();
		}
		else
		{
			RepRootMotion.AnimMontage = nullptr;
		}

		RepRootMotion.AuthoritativeRootMotion = MyCharacterMovement->CurrentRootMotion;
		RepRootMotion.Acceleration = MyCharacterMovement->GetCurrentAcceleration();
		RepRootMotion.LinearVelocity = MyCharacterMovement->Velocity;

		DOREPLIFETIME_ACTIVE_OVERRIDE(ACharacter, RepRootMotion, true);
	}
	else
	{
		RepRootMotion.Clear();

		DOREPLIFETIME_ACTIVE_OVERRIDE(ACharacter, RepRootMotion, false);
	}

	// Workaround: Jump state is replicated with movement mode and use of IsJumpForceApplied() is not even reliable as it only covers half of the jump.
	// The server must dictate if a fall is a jump or not in any stage of the fall.
	// bProxyIsJumpForceApplied = (JumpForceTimeRemaining > 0.0f);
	ReplicatedServerLastTransformUpdateTimeStamp = MyCharacterMovement->GetServerLastTransformUpdateTimeStamp();
	ReplicatedExtMovementMode = MyCharacterMovement->PackNetworkMovementMode();
	// Workaround: Most significant bit carries jump state info
	checkf(FMath::CeilLogTwo(EMovementMode::MOVE_MAX) < 7, TEXT("Most significant bit of ReplicatedMovementMode cannot be used to replicate jump state."));
	if (bIsJumping)
		ReplicatedExtMovementMode |= uint8(0x80);

	ReplicatedBasedMovement = BasedMovement;

	// Optimization: only update and replicate these values if they are actually going to be used.
	if (BasedMovement.HasRelativeLocation())
	{
		// When velocity becomes zero, force replication so the position is updated to match the server (it may have moved due to simulation on the client).
		ReplicatedBasedMovement.bServerHasVelocity = !MyCharacterMovement->Velocity.IsZero();

		// Make sure absolute rotations are updated in case rotation occurred after the base info was saved.
		if (!BasedMovement.HasRelativeRotation())
		{
			ReplicatedBasedMovement.Rotation = GetActorRotation();
		}
	}

	// Save bandwidth by not replicating this value unless it is necessary, since it changes every update.
	if ((MyCharacterMovement->NetworkSmoothingMode != ENetworkSmoothingMode::Linear) && !MyCharacterMovement->bNetworkAlwaysReplicateTransformUpdateTimestamp)
	{
		ReplicatedServerLastTransformUpdateTimeStamp = 0.f;
	}
}

bool AExtCharacter::GatherExtMovement()
{
	if (RootComponent && !RootComponent->IsSimulatingPhysics())
	{
		if (GetLocalRole() == ROLE_SimulatedProxy)
		{
			return RootComponent->GetAttachParent() == nullptr;
		}

		if (!RootComponent->GetAttachParent())
		{
			UExtCharacterMovementComponent* ExtCharacterMovement = GetExtCharacterMovement();
			check(ExtCharacterMovement);

			ReplicatedExtMovement.Location = RootComponent->GetComponentLocation();
			ReplicatedExtMovement.Rotation = RootComponent->GetComponentRotation();
			ReplicatedExtMovement.Velocity = ExtCharacterMovement->Velocity;
			ReplicatedExtMovement.Acceleration = ExtCharacterMovement->GetCurrentAcceleration().GetSafeNormal();			
			ReplicatedExtMovement.bIsPivotTurning = ExtCharacterMovement->IsPivotTurning();
			ReplicatedExtMovement.TurnInPlaceTargetYaw = ExtCharacterMovement->GetTurnInPlaceTargetYaw();

			return true;
		}
	}

	return false;
}

void AExtCharacter::PreNetReceive()
{
	// Full override because parent class implementation became obsolete with this class having a custom replicated movement mode.
	FULL_OVERRIDE();

	// This is not a typo. Skip ACharacter::PreNetReceive()
	Super::Super::PreNetReceive();
}

void AExtCharacter::PostNetReceive()
{
	// Full override because parent class implementation became obsolete with this class having a custom replicated movement mode.
	FULL_OVERRIDE();

	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		GetCharacterMovement()->bNetworkUpdateReceived = true;
	}

	// This is not a typo. Skip ACharacter::PostNetReceive()
	Super::Super::PostNetReceive();
}

void AExtCharacter::OnRep_ReplicatedExtMovement()
{
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		FRepMovement& MutableRepMovement = GetReplicatedMovement_Mutable();

		MutableRepMovement.Location = ReplicatedExtMovement.Location;
		MutableRepMovement.Rotation = ReplicatedExtMovement.Rotation;
		MutableRepMovement.LinearVelocity = ReplicatedExtMovement.Velocity;
		MutableRepMovement.AngularVelocity = FVector::ZeroVector;
		MutableRepMovement.bSimulatedPhysicSleep = false;
		MutableRepMovement.bRepPhysics = false;

		OnRep_ReplicatedMovement();

		UExtCharacterMovementComponent* ExtCharacterMovement = GetExtCharacterMovement();
		check(ExtCharacterMovement);
		ExtCharacterMovement->SetReplicatedAcceleration(ReplicatedExtMovement.Acceleration);
		ExtCharacterMovement->SetReplicatedPivotTurn(ReplicatedExtMovement.bIsPivotTurning);
		ExtCharacterMovement->SetReplicatedTurnInPlace(ReplicatedExtMovement.TurnInPlaceTargetYaw);
	}
}

void AExtCharacter::OnRep_ReplicatedExtMovementMode()
{
	bIsJumping = (ReplicatedExtMovementMode & uint8(0x80)) == uint8(0x80);
	ReplicatedMovementMode = (ReplicatedExtMovementMode & uint8(0x7F));

	GetCharacterMovement()->bNetworkMovementModeChanged = true;
}

void AExtCharacter::OnRep_ReplicatedLook()
{
	RemoteViewPitch = (uint8)(ReplicatedLook.Rotation.Pitch * 255.f / 360.f);
}

void AExtCharacter::OnRep_IsWalkingInsteadOfRunning()
{
	UExtCharacterMovementComponent* ExtCharacterMovement = GetExtCharacterMovement();
	check(ExtCharacterMovement);

	if (bIsWalkingInsteadOfRunning)
	{
		ExtCharacterMovement->Walk(true);
	}
	else
	{
		ExtCharacterMovement->UnWalk(true);
	}
}

void AExtCharacter::OnRep_IsSprinting()
{
	UExtCharacterMovementComponent* ExtCharacterMovement = GetExtCharacterMovement();
	check(ExtCharacterMovement);

	if (bIsSprinting)
	{
		ExtCharacterMovement->Sprint(true);
	}
	else
	{
		ExtCharacterMovement->UnSprint(true);
	}
}

void AExtCharacter::OnRep_IsPerformingGenericAction()
{
	UExtCharacterMovementComponent* ExtCharacterMovement = GetExtCharacterMovement();
	check(ExtCharacterMovement);

	if (bIsPerformingGenericAction)
	{
		ExtCharacterMovement->PerformGenericAction(true);
	}
	else
	{
		ExtCharacterMovement->UnPerformGenericAction(true);
	}
}


/// General

#if WITH_EDITOR

bool AExtCharacter::CanEditChange(const UProperty* InProperty) const
{
	bool bCanChange = Super::CanEditChange(InProperty);

	if (bCanChange)
	{
		const FName PropertyName = (InProperty != NULL) ? InProperty->GetFName() : NAME_None;
		if (PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, bUseControllerRotationPitch)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, bUseControllerRotationYaw)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, bUseControllerRotationRoll))
		{
			bCanChange = false;
		}
	}

	return bCanChange;
}

void AExtCharacter::PostEditChangeProperty(struct FPropertyChangedEvent& e)
{
	const FName PropertyName = (e.Property != NULL) ? e.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, bIsRagdoll))
	{
		if (UWorld* World = GetWorld())
			if (World->IsGameWorld())
			{
				if (bIsRagdoll)
				{
					OnStartRagdoll();
				}
				else
				{
					OnEndRagdoll();
				}
			}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, RotationMode))
	{
		if (UWorld* World = GetWorld())
			if (World->IsGameWorld())
				OnRotationModeChangedInternal();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, bEnableDebugDraw))
	{
		if (UWorld* World = GetWorld())
			if (World->IsGameWorld())
				UpdateDebugComponentsVisibility();
	}

	// Note: Any changes must be made before AActor::PostEditChangeChainProperty is called because components will be reset when UActorComponent reruns construction scripts 
	Super::PostEditChangeProperty(e);
}

#endif

void AExtCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

#if WITH_EDITOR
	UpdateDebugComponentsVisibility();
#endif

	if (bIsRagdoll)
	{
		OnStartRagdoll();
	}

	if (RotationMode != ECharacterRotationMode::None)
		OnRotationModeChangedInternal();
}

void AExtCharacter::BeginPlay()
{
	Super::BeginPlay();

#if WITH_EDITOR
#if WITH_EDITORONLY_DATA
	// All debug components start invisible
	LookRotationArrow->SetVisibility(true);
	LookRotationYawArrow->SetVisibility(true);
	VelocityArrow->SetVisibility(true);
	LastVelocityArrow->SetVisibility(true);
	AccelerationArrow->SetVisibility(true);
	LastAccelerationArrow->SetVisibility(true);
#endif
#endif
}

void AExtCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Make sure all timers are cleared
	GetWorldTimerManager().ClearAllTimersForObject(this);

	Super::EndPlay(EndPlayReason);
}

void AExtCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

#if WITH_EDITOR

	UpdateDebugComponentsVisibility();

	if (bEnableDebugDraw)
	{
		UExtCharacterMovementComponent* ExtCharacterMovement = GetExtCharacterMovement();
		check(ExtCharacterMovement);

#if WITH_EDITORONLY_DATA

		const FVector ActorFeetLocation = ExtCharacterMovement->GetActorFeetLocation();

		const FRotator LookRotation = GetLookRotation();
		LookRotationArrow->SetRelativeLocation(FVector(0.f, 0.f, BaseEyeHeight));
		LookRotationArrow->SetWorldRotation(FRotator(LookRotation.Pitch, LookRotation.Yaw, 0.0f));

		const FVector Velocity = ExtCharacterMovement->Velocity;
		const FVector Acceleration = ExtCharacterMovement->GetCurrentAcceleration();

		const FRotator LastVelocityRotation = ExtCharacterMovement->LastMovementVelocity.Rotation();
		LastVelocityArrow->SetWorldLocation(ActorFeetLocation);
		LastVelocityArrow->SetWorldRotation(LastVelocityRotation);

		VelocityArrow->SetWorldLocation(ActorFeetLocation + FVector(0.f, 0.f, 0.2f));
		VelocityArrow->SetWorldRotation(Velocity.Rotation());
		VelocityArrow->SetRelativeScale3D(FVector(FMath::GetMappedRangeValueClamped(FVector2D(0.f, ExtCharacterMovement->GetMaxSpeed()), FVector2D(0.f, 1.f), Velocity.Size()), 4.0f, 0.0f));

		LookRotationYawArrow->SetWorldLocation(ActorFeetLocation + FVector(0.f, 0.f, 0.4f));
		LookRotationYawArrow->SetWorldRotation(FRotator(0.0f, LookRotation.Yaw, 0.0f));

		const FRotator LastAccelerationRotation = ExtCharacterMovement->LastMovementAcceleration.Rotation();
		LastAccelerationArrow->SetWorldLocation(ActorFeetLocation + FVector(0.f, 0.f, 0.6f));
		LastAccelerationArrow->SetWorldRotation(LastAccelerationRotation);

		AccelerationArrow->ArrowColor = ExtCharacterMovement->IsPivotTurning() ? FColor::Red : FColor::Yellow;
		AccelerationArrow->MarkRenderStateDirty();
		AccelerationArrow->SetWorldLocation(ActorFeetLocation + FVector(0.f, 0.f, 0.8f));
		AccelerationArrow->SetWorldRotation(Acceleration.Rotation());
		AccelerationArrow->SetRelativeScale3D(FVector(IsLocallyControlled() ? FMath::GetMappedRangeValueClamped(FVector2D(0.f, ExtCharacterMovement->GetMaxAcceleration()), FVector2D(0.f, 1.f), Acceleration.Size()) : Acceleration.IsNearlyZero() ? 0.f : 1.f, 1.75f, 0.0f));

#endif // WITH_EDITORONLY_DATA

		if (USkeletalMeshComponent* MyMesh = GetMesh())
		{
			const UWorld* World = GetWorld();	
			const FTransform& RootBoneTransform = GetMesh()->GetBoneTransform(0);
			const FVector RootBoneLocation = RootBoneTransform.GetLocation();
			const FRotator RootBoneRotation = RootBoneTransform.Rotator();

			const FVector Location = RootBoneLocation + FVector(0.0f, 0.0f, 0.2f); // lift the location a bit to reduce overlapping with the ground mesh
			const FRotator Rotation = RootBoneRotation;

			DrawDebugSphere(World, RootBoneLocation, 1.0f, 16, FColor::Red, false, -1.0f, SDPG_Foreground);
			DrawDebugCoordinateSystem(World, Location, Rotation, 100.f, false, -1.0f, SDPG_Foreground);

			FRotator TargetRotation = GetActorRotation();
			const float TurnInPlaceTargetYaw = ExtCharacterMovement->GetTurnInPlaceTargetYaw();
			if (FMath::IsFinite(TurnInPlaceTargetYaw))
				TargetRotation.Yaw = TurnInPlaceTargetYaw;

			DrawDebugDirectionalArrow(World, Location, Location + TargetRotation.Vector() * 100.0f, 100.0f, FColor::Orange, false, -1.0f, SDPG_Foreground);

			const FVector Direction = GetActorForwardVector();
			const float Aperture = ExtCharacterMovement->LookAngleThreshold;
			DrawDebugCone(World, Location, Direction, 100.0f, FMath::DegreesToRadians(Aperture), 0.0f, Aperture / 5.f, FColor::Cyan, false, -1.0f, SDPG_Foreground);			
		}
	}
#endif
}

void AExtCharacter::Restart()
{
	// Full override to have default movement mode set as the last step in the restart process.
	FULL_OVERRIDE();

	// Skip ACharacter::Restart() on purpose
	Super::Super::Restart();

	JumpCurrentCount = 0;

	StopJumping();
	UnCrouch(true);
	UnWalk(true);
	UnSprint(true);
	UnPerformGenericAction(true);

	// Don't call UnRagdoll() here. Some games may need to preserve the ragdoll state after possession.
	// The user can always override this method and call UnRagdoll() if desired.

	OnRestart();

	// Lock the new controller inputs if in ragdoll
	if (bIsRagdoll && Controller)
	{
		if (bIgnoreMoveInputWhenRagdoll)
			Controller->SetIgnoreMoveInput(true);

		if (bIgnoreLookInputWhenRagdoll)
			Controller->SetIgnoreLookInput(true);
	}

	// Set reset move state and movement mode
	UExtCharacterMovementComponent* ExtCharacterMovement = GetExtCharacterMovement();
	check(ExtCharacterMovement);
	ExtCharacterMovement->ResetMoveState();
	ExtCharacterMovement->ResetExtraMoveState();
	ExtCharacterMovement->SetDefaultMovementMode();
}

void AExtCharacter::UnPossessed()
{
	// Unlock the leaving Controller inputs if necessary
	if (bIsRagdoll && Controller)
	{
		if (bIgnoreMoveInputWhenRagdoll)
			Controller->SetIgnoreMoveInput(false);

		if (bIgnoreLookInputWhenRagdoll)
			Controller->SetIgnoreLookInput(false);
	}

	Super::UnPossessed();

	// Stop moving if required but only if not a ragdoll
	if (!bIsRagdoll && bStopWhenUnpossessed)
		GetCharacterMovement()->StopMovementImmediately();
}

void AExtCharacter::PawnStartFire(uint8 FireModeNum)
{
	Super::PawnStartFire(FireModeNum);

	UE_LOG(LogExtCharacter, Error, TEXT(__FUNCSIG__ " is not supported."));
}

void AExtCharacter::OnRestart()
{

}


/// Camera

void AExtCharacter::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult)
{
	if (bRelayCameraFunctionsToController && Controller)
		return Controller->CalcCamera(DeltaTime, OutResult);

	Super::CalcCamera(DeltaTime, OutResult);
}

bool AExtCharacter::HasActiveCameraComponent() const
{
	return (bRelayCameraFunctionsToController && Controller) ? Controller->HasActiveCameraComponent() : Super::HasActiveCameraComponent();
}

bool AExtCharacter::HasActivePawnControlCameraComponent() const
{
	return (bRelayCameraFunctionsToController && Controller) ? Controller->HasActivePawnControlCameraComponent() : Super::HasActivePawnControlCameraComponent();
}


/// Input

void AExtCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(MoveForwardInputName, this, &ThisClass::PlayerInputMoveForward);
	PlayerInputComponent->BindAxis(MoveRightInputName, this, &ThisClass::PlayerInputMoveRight);

	PlayerInputComponent->BindAxis(LookUpInputName, this, &ThisClass::PlayerInputLookUp);
	PlayerInputComponent->BindAxis(LookRightInputName, this, &ThisClass::PlayerInputLookRight);

	PlayerInputComponent->BindAction(CrouchInputName, IE_Pressed, this, &ThisClass::PlayerInputStartCrouch);
	PlayerInputComponent->BindAction(CrouchInputName, IE_Released, this, &ThisClass::PlayerInputStopCrouch);

	PlayerInputComponent->BindAction(JumpInputName, IE_Pressed, this, &ThisClass::PlayerInputStartJump);
	PlayerInputComponent->BindAction(JumpInputName, IE_Released, this, &ThisClass::PlayerInputStopJump);

	PlayerInputComponent->BindAction(WalkInputName, IE_Pressed, this, &ThisClass::PlayerInputStartWalk);
	PlayerInputComponent->BindAction(WalkInputName, IE_Released, this, &ThisClass::PlayerInputStopWalk);

	PlayerInputComponent->BindAction(SprintInputName, IE_Pressed, this, &ThisClass::PlayerInputStartSprint);
	PlayerInputComponent->BindAction(SprintInputName, IE_Released, this, &ThisClass::PlayerInputStopSprint);

	PlayerInputComponent->BindAction(GenericActionInputName, IE_Pressed, this, &ThisClass::PlayerInputStartGenericAction);
	PlayerInputComponent->BindAction(GenericActionInputName, IE_Released, this, &ThisClass::PlayerInputStopGenericAction);

	PlayerInputComponent->BindAction(FireInputName, IE_Pressed, this, &ThisClass::PlayerInputStartFire);
	PlayerInputComponent->BindAction(FireInputName, IE_Released, this, &ThisClass::PlayerInputStopFire);
}

void AExtCharacter::PlayerInputMoveForward(float Value)
{
	if (Value != 0.0f &&  Controller  && Controller->IsLocalPlayerController())
	{
		FVector Location;
		FRotator Rotation;
		Controller->GetPlayerViewPoint(Location, Rotation);
		AddMovementInput(FRotationMatrix(FRotator(0.0f, Rotation.Yaw, 0.0f)).GetUnitAxis(EAxis::X), Value);
	}
}

void AExtCharacter::PlayerInputMoveRight(float Value)
{
	if (Value != 0.0f && Controller  && Controller->IsLocalPlayerController())
	{
		FVector Location;
		FRotator Rotation;
		Controller->GetPlayerViewPoint(Location, Rotation);
		AddMovementInput(FRotationMatrix(FRotator(0.0f, Rotation.Yaw, 0.0f)).GetUnitAxis(EAxis::Y), Value);
	}
}

void AExtCharacter::PlayerInputLookUp(float Value)
{
	if (Value != 0.0f &&  Controller  && Controller->IsLocalPlayerController())
	{
		AddControllerPitchInput(LookUpInputSpeed > 0.0f ? LookUpInputSpeed * GetWorld()->GetDeltaSeconds() * Value : Value);
	}
}

void AExtCharacter::PlayerInputLookRight(float Value)
{
	if (Value != 0.0f && Controller  && Controller->IsLocalPlayerController())
	{
		AddControllerYawInput(LookRightInputSpeed  > 0.0f ? LookRightInputSpeed * GetWorld()->GetDeltaSeconds() * Value : Value);
	}
}

void AExtCharacter::PlayerInputStartCrouch()
{
	if (Controller && Controller->IsLocalPlayerController())
	{
		Crouch();
	}
}

void AExtCharacter::PlayerInputStopCrouch()
{
	if (Controller && Controller->IsLocalPlayerController())
	{
		UnCrouch();
	}
}

void AExtCharacter::PlayerInputStartJump()
{
	if (Controller && Controller->IsLocalPlayerController())
	{
		if (!bIsRagdoll || !bIgnoreMoveInputWhenRagdoll)
			Jump();
	}
}

void AExtCharacter::PlayerInputStopJump()
{
	if (Controller && Controller->IsLocalPlayerController())
	{
		if (!bIsRagdoll || !bIgnoreMoveInputWhenRagdoll)
			StopJumping();
	}
}

void AExtCharacter::PlayerInputStartWalk()
{
	if (Controller && Controller->IsLocalPlayerController())
	{
		Walk();
	}
}

void AExtCharacter::PlayerInputStopWalk()
{
	if (Controller && Controller->IsLocalPlayerController())
	{
		UnWalk();
	}
}

void AExtCharacter::PlayerInputStartSprint()
{
	if (Controller && Controller->IsLocalPlayerController())
	{
		Sprint();
	}
}

void AExtCharacter::PlayerInputStopSprint()
{
	if (Controller && Controller->IsLocalPlayerController())
	{
		UnSprint();
	}
}

void AExtCharacter::PlayerInputStartGenericAction()
{
	if (Controller && Controller->IsLocalPlayerController())
	{
		PerformGenericAction();
	}
}

void AExtCharacter::PlayerInputStopGenericAction()
{
	if (Controller && Controller->IsLocalPlayerController())
	{
		UnPerformGenericAction();
	}
}

void AExtCharacter::PlayerInputStartFire()
{
	if (Controller && Controller->IsLocalPlayerController())
	{
		// 	Fire();

		// Temporary ragdoll test
		ToggleRagdoll();
	}
}

void AExtCharacter::PlayerInputStopFire()
{
	if (Controller && Controller->IsLocalPlayerController())
	{
		// UnFire();
	}
}


/// Temporary For Ragdoll Tests

void AExtCharacter::ToggleRagdoll()
{
	SetRagdoll(!bIsRagdoll);

	if (GetLocalRole() < ROLE_Authority)
		ServerToggleRagdoll();
	else
	{
		MulticastSetRagdoll(bIsRagdoll);
	}
}

bool AExtCharacter::ServerToggleRagdoll_Validate()
{
	return true;
}

void AExtCharacter::ServerToggleRagdoll_Implementation()
{
	ToggleRagdoll();
}

void AExtCharacter::MulticastSetRagdoll_Implementation(bool Value)
{
	if (!IsLocallyControlled())
		SetRagdoll(Value);
}


/// Movement Handlers

void AExtCharacter::OnUpdateBeforeMovement(float DeltaSeconds)
{
	checkActorRoleAtLeast(ROLE_AutonomousProxy);
}

void AExtCharacter::OnUpdateAfterMovement(float DeltaSeconds)
{
	checkActorRoleAtLeast(ROLE_AutonomousProxy);

	ReplicatedLook.Rotation = GetControlRotation();

	UExtCharacterMovementComponent* ExtCharacterMovement = GetExtCharacterMovement();
	check(ExtCharacterMovement);

	const FVector Acceleration = ExtCharacterMovement->GetCurrentAcceleration();
	const bool bIsAccelerating = Acceleration.SizeSquared() > KINDA_SMALL_NUMBER;

	const FVector& Velocity = ExtCharacterMovement->Velocity;
	const bool bIsMoving2D = Velocity.SizeSquared() > KINDA_SMALL_NUMBER;
	if ((bIsAccelerating || !bIsMoving2D) && IsLanding())
	{
		CancelLanding();
	}
}

void AExtCharacter::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	UExtCharacterMovementComponent* ExtCharacterMovement = GetExtCharacterMovement();
	check(ExtCharacterMovement);

	if (bIsRagdoll)
	{
		if (USkeletalMeshComponent* MyMesh = GetMesh())
		{
			const FVector& Velocity = ExtCharacterMovement->Velocity;
			// Make the ragdoll follow the vertical component of the character's velocity for a better look and feel.
			FVector RagdollPivotVelocity = MyMesh->GetPhysicsLinearVelocity(PelvisBoneName);
			switch (ExtCharacterMovement->MovementMode)
			{
			case MOVE_Falling:
			case MOVE_Swimming:
			case MOVE_Flying:
				RagdollPivotVelocity.Z = Velocity.Z;
				if (Velocity.Z > 0.0f)
					RagdollPivotVelocity.Z *= 1.4f;

				MyMesh->SetPhysicsLinearVelocity(RagdollPivotVelocity, false, PelvisBoneName);
				break;
			}

			// Set the "Stiffness" of the ragdoll joints based on the ground speed. The faster the ragdoll moves, the stiffer the joints become.
			float InSpring = FMath::GetMappedRangeValueClamped(FVector2D(0.f, 1000.f), FVector2D(0.f, 25000.f), RagdollPivotVelocity.Size());
			MyMesh->SetAllMotorsAngularDriveParams(InSpring, 1.f, 0.f, false);
		}
	}
}


/// State Change Handlers

void AExtCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PrevCustomMode)
{
	// Full override necessary to ensure the Blueprint event and the delegates are called as the last step in the process.
	// User must override OnMovementModeChanged(PrevMovementMode, NewMovementMode, PrevCustomMode, NewCustomMode) now.
	FULL_OVERRIDE();

	if (!bPressedJump)
	{
		ResetJumpState();
	}

	UCharacterMovementComponent* MyCharacterMovement = GetCharacterMovement();
	check(MyCharacterMovement);

	// Record jump force start time for proxies. Allows us to expire the jump even if not continually ticking down a timer.
	if (bProxyIsJumpForceApplied && MyCharacterMovement->IsFalling())
	{
		ProxyJumpForceStartedTime = GetWorld()->GetTimeSeconds();
	}

	const EMovementMode CurrentMovementMode = MyCharacterMovement->MovementMode;
	const uint8 CurrentCustomMode = MyCharacterMovement->CustomMovementMode;

	if (GetLocalRole() >= ROLE_AutonomousProxy)
	{
		if (PrevMovementMode == MOVE_Falling)
		{
			// Confirm that we have landed safely since bHasLandedSafely can be assigned in OnLanded().
			// If safe take the current MovementMode into account. Landing on ground is never safe if we were falling unintentionally (not jumping).
			bHasLandedSafely = bHasLandedSafely && ((CurrentMovementMode != MOVE_Walking && CurrentMovementMode == MOVE_NavWalking) || bIsJumping);

			// Not falling anymore so reset the jumping flag
			bIsJumping = false;
		}
		else
		{
			// If we're coming from a mode other than falling and the jump land timer is still on then clear it as it has no purpose anymore.
			// This includes cases such as when we started jump landing, changed to moving on ground but before the timer expired started falling again.
			if (IsLanding())
				CancelLanding();

			// The safety of our future lading is still to be determined, by default we assume it's going to be safe.
			if (CurrentMovementMode == MOVE_Falling)
			{
				bHasLandedSafely = true;
			}
		}
	}

	if (!bIsRagdoll)
	{
		// Abort getting up process if movement mode changed in the middle of it.
		if (IsGettingUp())
			CancelGettingUp();
	}

	OnMovementModeChanged(PrevMovementMode, CurrentMovementMode, PrevCustomMode, CurrentCustomMode);

	K2_OnMovementModeChanged(PrevMovementMode, CurrentMovementMode, PrevCustomMode, CurrentCustomMode);
	MovementModeChangedDelegate.Broadcast(this, PrevMovementMode, PrevCustomMode);
}

void AExtCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, EMovementMode CurrentMovementMode, uint8 PrevCustomMovementMode, uint8 CurrentCustomMovementMode)
{
	
}

void AExtCharacter::OnCrouchedChanged()
{
	UpdateMovementComponentSettings();
}

void AExtCharacter::OnGaitChanged()
{
	UpdateMovementComponentSettings();
}

void AExtCharacter::OnPerformingGenericActionChanged()
{
	UpdateMovementComponentSettings();
}

void AExtCharacter::OnRotationModeChangedInternal()
{
	UExtCharacterMovementComponent* ExtCharacterMovement = GetExtCharacterMovement();
	check(ExtCharacterMovement);

	// Set RotationRateFactor to 0. This slows drastic changes in rotation to make rotation smoother.
	if (ExtCharacterMovement->Velocity.SizeSquared2D() > KINDA_SMALL_NUMBER)
		ExtCharacterMovement->ResetRotationRateFactor();

	switch (RotationMode)
	{
	case ECharacterRotationMode::OrientToMovement:
		bUseControllerRotationYaw = false;
		ExtCharacterMovement->bOrientRotationToMovement = true;
		break;
	case ECharacterRotationMode::OrientToController:
		bUseControllerRotationYaw = false;
		ExtCharacterMovement->bOrientRotationToMovement = false;
		ExtCharacterMovement->bUseControllerDesiredRotation = true;
		break;
	default:
		ExtCharacterMovement->bOrientRotationToMovement = false;
		ExtCharacterMovement->bUseControllerDesiredRotation = false;
		break;
	}

	OnRotationModeChanged();
	RotationModeChangedDelegate.Broadcast(this);
}

void AExtCharacter::OnRotationModeChanged()
{

}

void AExtCharacter::OnRagdollChanged()
{

}

void AExtCharacter::UpdateMovementComponentSettings()
{
	if (GetLocalRole() >= ROLE_AutonomousProxy)
	{
		switch (Gait)
		{
		case ECharacterGait::Walk:
			UpdateMovementComponentSettings(
				bIsPerformingGenericAction
				? bIsCrouched ? MovementSettings.Secondary.Crouched.Walk : MovementSettings.Secondary.Standing.Walk
				: bIsCrouched ? MovementSettings.Primary.Crouched.Walk : MovementSettings.Primary.Standing.Walk
			);
			break;
		case ECharacterGait::Run:
			UpdateMovementComponentSettings(
				bIsPerformingGenericAction
				? bIsCrouched ? MovementSettings.Secondary.Crouched.Run : MovementSettings.Secondary.Standing.Run
				: bIsCrouched ? MovementSettings.Primary.Crouched.Run : MovementSettings.Primary.Standing.Run
			);
			break;
		case ECharacterGait::Sprint:
			UpdateMovementComponentSettings(MovementSettings.Sprint);
			break;
		}
	}
}

void AExtCharacter::UpdateMovementComponentSettings(const FCharacterGaitSettings& Settings)
{
	UExtCharacterMovementComponent* ExtCharacterMovement = GetExtCharacterMovement();
	check(ExtCharacterMovement);

	ExtCharacterMovement->MaxWalkSpeed = Settings.MaxSpeed;
	ExtCharacterMovement->MaxWalkAcceleration = Settings.MaxAcceleration;
	ExtCharacterMovement->WalkFriction = Settings.Friction;
	ExtCharacterMovement->BrakingDecelerationWalking = Settings.BrakingDeceleration;
	ExtCharacterMovement->BrakingFrictionFactor = Settings.BrakingFrictionFactor;
}


/// Landing

void AExtCharacter::Landed(const FHitResult& Hit)
{
	checkActorRoleAtLeast(ROLE_AutonomousProxy);

	Super::Landed(Hit);

	UExtCharacterMovementComponent* ExtCharacterMovement = GetExtCharacterMovement();
	check(ExtCharacterMovement);
	ExtCharacterMovement->ResetRotationRateFactor();

	if (!bIsRagdoll && bIsJumping)
	{
		// Either cancel velocity or start the landing timer depending if we want to preserve movement on landing and if the player is trying to accelerate.
		// Must be LastControlInputVector and not Acceleration because Acceleration can be zero when Falling despite the input control vector.
		const bool bIsAccelerating = LastControlInputVector.SizeSquared2D() > KINDA_SMALL_NUMBER;
		if (!bIsAccelerating)
		{
			if (!ExtCharacterMovement->bPreserveMovementOnLanding || LandingDelay < 0.1f)
			{
				ExtCharacterMovement->Velocity = FVector::ZeroVector;
			}
			else
			{
				GetWorldTimerManager().SetTimer(LandingTimerHandle, this, &ThisClass::LandingTimer_OnTime, LandingDelay, false);
			}
		}
	}
}

void AExtCharacter::LandingTimer_OnTime()
{
	checkActorRoleAtLeast(ROLE_AutonomousProxy);

	// This callback is called only once so we don't have to clear the timer but the timer manager
	// does not invalidate the handle automatically so we have to do it manually here.
	LandingTimerHandle.Invalidate();
	OnLandingComplete();
}

void AExtCharacter::CancelLanding()
{
	checkActorRoleAtLeast(ROLE_AutonomousProxy);

	check(LandingTimerHandle.IsValid());
	GetWorldTimerManager().ClearTimer(LandingTimerHandle);
	OnLandingCanceled();
}

void AExtCharacter::OnLandingComplete()
{
	checkActorRoleAtLeast(ROLE_AutonomousProxy);

	UExtCharacterMovementComponent* ExtCharacterMovement = GetExtCharacterMovement();
	check(ExtCharacterMovement);
}

void AExtCharacter::OnLandingCanceled()
{
	checkActorRoleAtLeast(ROLE_AutonomousProxy);

	UExtCharacterMovementComponent* ExtCharacterMovement = GetExtCharacterMovement();
	check(ExtCharacterMovement);
}


/// Jumping

bool AExtCharacter::CanJumpInternal_Implementation() const
{
	UExtCharacterMovementComponent* ExtCharacterMovement = GetExtCharacterMovement();
	check(ExtCharacterMovement);

	return Super::CanJumpInternal_Implementation()
		// && IsJumpReady() // TODO: add method to check if jump is out of cooldown
		&& !bIsRagdoll
		&& !IsGettingUp()
		&& ExtCharacterMovement
		&& !ExtCharacterMovement->bWantsToCrouch
		&& !ExtCharacterMovement->bWantsToSprint;
}

void AExtCharacter::OnJumped_Implementation()
{
	checkActorRoleAtLeast(ROLE_AutonomousProxy);
	bIsJumping = true;
}


// Crouching

bool AExtCharacter::CanCrouch() const
{
	// Full override to allow blueprints to override the conditions to crouch
	FULL_OVERRIDE();

	return CanCrouchInternal();
}

bool AExtCharacter::CanCrouchInternal_Implementation() const
{
	UExtCharacterMovementComponent* ExtCharacterMovement = GetExtCharacterMovement();
	check(ExtCharacterMovement);

	return !bIsCrouched
		&& !bPressedJump
		&& ExtCharacterMovement
		&& !ExtCharacterMovement->bWantsToSprint
		&& ExtCharacterMovement->IsMovingOnGround()
		&& GetRootComponent() && !GetRootComponent()->IsSimulatingPhysics();
}

void AExtCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	OnCrouchedChanged();
	CrouchChangedDelegate.Broadcast(this);
}

void AExtCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	OnCrouchedChanged();
	CrouchChangedDelegate.Broadcast(this);
}


/// Walking

bool AExtCharacter::CanWalk() const
{
	return CanWalkInternal();
}

bool AExtCharacter::CanWalkInternal_Implementation() const
{
	return !bIsWalkingInsteadOfRunning;
}

void AExtCharacter::Walk(bool bClientSimulation)
{
	UExtCharacterMovementComponent* ExtCharacterMovement = GetExtCharacterMovement();
	check(ExtCharacterMovement);

	if (ExtCharacterMovement)
	{
		if (CanWalk())
		{
			ExtCharacterMovement->bWantsToWalkInsteadOfRun = true;
		}
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		else if (!ExtCharacterMovement->CanEverWalkInsteadOfRun())
		{
			UE_LOG(LogExtCharacter, Warning, TEXT("%s is trying to walk, but walking is disabled for this character! (check Extra Movement Capabilities)"), *GetName());
		}
#endif
	}
}

void AExtCharacter::UnWalk(bool bClientSimulation)
{
	UExtCharacterMovementComponent* ExtCharacterMovement = GetExtCharacterMovement();
	check(ExtCharacterMovement);

	if (ExtCharacterMovement)
	{
		ExtCharacterMovement->bWantsToWalkInsteadOfRun = false;
	}
}

void AExtCharacter::OnEndWalk()
{
	K2_OnEndWalk();

	// Sprint has priority over walk
	if (!bIsSprinting)
		SetGait(ECharacterGait::Run);
}

void AExtCharacter::OnStartWalk()
{
	K2_OnStartWalk();

	// Sprint has priority over walk
	if (!bIsSprinting)
		SetGait(ECharacterGait::Walk);
}


/// Sprinting

bool AExtCharacter::CanSprint() const
{
	return CanSprintInternal();
}

bool AExtCharacter::CanSprintInternal_Implementation() const
{
	UExtCharacterMovementComponent* ExtCharacterMovement = GetExtCharacterMovement();
	check(ExtCharacterMovement);

	return !bIsSprinting
		&& !bPressedJump
		&& !bIsRagdoll
		&& !IsGettingUp()
		&& ExtCharacterMovement
		&& !ExtCharacterMovement->bWantsToCrouch
		&& ExtCharacterMovement->IsMovingOnGround();
}

void AExtCharacter::Sprint(bool bClientSimulation)
{
	UExtCharacterMovementComponent* ExtCharacterMovement = GetExtCharacterMovement();
	check(ExtCharacterMovement);

	if (ExtCharacterMovement)
	{
		if (CanSprint())
		{
			ExtCharacterMovement->bWantsToSprint = true;
		}
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		else if (!ExtCharacterMovement->CanEverSprint())
		{
			UE_LOG(LogExtCharacter, Warning, TEXT("%s is trying to sprint, but sprinting is disabled for this character! (check its Extra Movement Capabilities)"), *GetName());
		}
#endif
	}
}

void AExtCharacter::UnSprint(bool bClientSimulation)
{
	UExtCharacterMovementComponent* ExtCharacterMovement = GetExtCharacterMovement();
	check(ExtCharacterMovement);

	if (ExtCharacterMovement)
	{
		ExtCharacterMovement->bWantsToSprint = false;
	}
}

void AExtCharacter::OnEndSprint()
{
	K2_OnEndSprint();

	SetGait(bIsWalkingInsteadOfRunning ? ECharacterGait::Walk : ECharacterGait::Run);
}

void AExtCharacter::OnStartSprint()
{
	K2_OnStartSprint();

	SetGait(ECharacterGait::Sprint);
}


/// Performing Action

bool AExtCharacter::CanPerformGenericAction() const
{
	return CanPerformGenericActionInternal();
}

bool AExtCharacter::CanPerformGenericActionInternal_Implementation() const
{
	UExtCharacterMovementComponent* ExtCharacterMovement = GetExtCharacterMovement();
	check(ExtCharacterMovement);

	return !bIsPerformingGenericAction;
}

void AExtCharacter::PerformGenericAction(bool bClientSimulation)
{
	UExtCharacterMovementComponent* ExtCharacterMovement = GetExtCharacterMovement();
	check(ExtCharacterMovement);

	if (ExtCharacterMovement)
	{
		if (CanPerformGenericAction())
		{
			ExtCharacterMovement->bWantsToPerformGenericAction = true;
		}
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		else if (!ExtCharacterMovement->CanEverPerformGenericAction())
		{
			UE_LOG(LogExtCharacter, Warning, TEXT("%s is trying to perform the generic action, but performing generic action is disabled for this character! (check its Extra Movement Capabilities)"), *GetName());
		}
#endif
	}
}

void AExtCharacter::UnPerformGenericAction(bool bClientSimulation)
{
	UExtCharacterMovementComponent* ExtCharacterMovement = GetExtCharacterMovement();
	check(ExtCharacterMovement);

	if (ExtCharacterMovement)
	{
		ExtCharacterMovement->bWantsToPerformGenericAction = false;
	}
}

void AExtCharacter::OnEndGenericAction()
{
	K2_OnEndGenericAction();

	OnPerformingGenericActionChanged();
	GenericActionChangedDelegate.Broadcast(this);
}

void AExtCharacter::OnStartGenericAction()
{
	K2_OnStartGenericAction();

	OnPerformingGenericActionChanged();
	GenericActionChangedDelegate.Broadcast(this);
}


/// Ragdoll

void AExtCharacter::SetRagdoll(bool Value)
{
	if (bIsRagdoll != Value)
	{
		bIsRagdoll = Value;

		if (Value)
		{
			OnStartRagdoll();
		}
		else
		{
			OnEndRagdoll();
		}
	}
}

void AExtCharacter::OnEndRagdoll()
{
	UExtCharacterMovementComponent* ExtCharacterMovement = GetExtCharacterMovement();
	check(ExtCharacterMovement);

	const EMovementMode MovementMode = ExtCharacterMovement->MovementMode;
	if (MovementMode != MOVE_None && MovementMode != MOVE_Falling && GetUpDelay > 0.1f)
	{
		GetWorldTimerManager().SetTimer(GettingUpTimerHandle, this, &ThisClass::GettingUpTimer_OnTime, GetUpDelay, false);
	}
	else
	{
		if (Controller)
		{
			if (bIgnoreMoveInputWhenRagdoll)
				Controller->SetIgnoreMoveInput(false);

			if (bIgnoreLookInputWhenRagdoll)
				Controller->SetIgnoreLookInput(false);
		}
	}

	ThisClass* DefaultCharacter = GetClass()->GetDefaultObject<ThisClass>();
	GetCapsuleComponent()->SetCollisionProfileName(DefaultCharacter->GetCapsuleComponent()->GetCollisionProfileName());

	if (USkeletalMeshComponent* MyMesh = GetMesh())
	{
		// Disable mesh collision and stop simulating physics
		MyMesh->SetAllBodiesSimulatePhysics(false);
		MyMesh->bUpdateJointsFromAnimation = false;
		if (USkeletalMeshComponent* DefaultMesh = DefaultCharacter->GetMesh())
		{
			MyMesh->SetCollisionProfileName(DefaultMesh->GetCollisionProfileName());
			MyMesh->CanCharacterStepUpOn = DefaultMesh->CanCharacterStepUpOn;
		}

		if (RagdollMeshConstraintProfileName != NAME_None)
			MyMesh->SetConstraintProfileForAll(NAME_None);

#ifdef UE_BUILD_DEBUG
		if (FBodyInstance* BodyInstance = MyMesh->GetBodyInstance(MyMesh->GetBoneName(0)))
		{
			TWeakObjectPtr<UBodySetup> BodySetup = BodyInstance->BodySetup;
			if (MyMesh->PhysicsTransformUpdateMode == EPhysicsTransformUpdateMode::SimulationUpatesComponentTransform && !(BodySetup.IsValid() && BodySetup->PhysicsType == EPhysicsType::PhysType_Kinematic))
			{
				UE_LOG(LogExtCharacter, Warning, TEXT("Use of a non-kinematic root bone may prevent the ragdoll from working properly. Either the mesh component Physics Transform Update Mode should be 'Component Transform Is Kinematic' or the root bone Physics Type should be Kinematic."));
			}
		}
#endif
	}

	K2_OnEndRagdoll();
	OnRagdollChanged();
	RagdollChangedDelegate.Broadcast(this);
}

void AExtCharacter::OnStartRagdoll()
{
	GetCapsuleComponent()->SetCollisionProfileName(RagdollCapsuleCollisionProfileName);

	// Enable mesh collision and start simulating physics
	if (USkeletalMeshComponent* MyMesh = GetMesh())
	{
		if (RagdollMeshCollisionProfileName != NAME_None)
			MyMesh->SetCollisionProfileName(RagdollMeshCollisionProfileName);

		FName BoneName = PelvisBoneName;

#ifdef UE_BUILD_DEBUG
		if (PelvisBoneName == NAME_None || MyMesh->GetBoneIndex(PelvisBoneName) <= 0)
		{
			BoneName = MyMesh->GetBoneName(1);
			UE_LOG(LogExtCharacter, Warning, TEXT("Invalid bone name '%s' for ragdoll pelvis. Using '%s' instead but the ragdoll mode will not work properly if the right constraints are not setup."), *PelvisBoneName.ToString(), *BoneName.ToString());
		}
#endif

		if (RagdollMeshConstraintProfileName != NAME_None)
			MyMesh->SetConstraintProfileForAll(RagdollMeshConstraintProfileName, true);

#ifdef UE_BUILD_DEBUG
		if (FBodyInstance* BodyInstance = MyMesh->GetBodyInstance(MyMesh->GetBoneName(0)))
		{
			TWeakObjectPtr<UBodySetup> BodySetup = BodyInstance->BodySetup;
			if (MyMesh->PhysicsTransformUpdateMode == EPhysicsTransformUpdateMode::SimulationUpatesComponentTransform && !(BodySetup.IsValid() && BodySetup->PhysicsType == EPhysicsType::PhysType_Kinematic))
			{
				UE_LOG(LogExtCharacter, Warning, TEXT("Use of a non-kinematic root bone may prevent the ragdoll from working properly. Either the mesh component Physics Transform Update Mode should be 'Component Transform Is Kinematic' or the root bone Physics Type should be Kinematic."));
			}
		}
#endif

		MyMesh->bUpdateJointsFromAnimation = true;
		MyMesh->SetAllBodiesBelowSimulatePhysics(BoneName, true, true);
		MyMesh->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_Yes;
	}

	// Cancel getting up if in progress
	if (IsGettingUp())
		CancelGettingUp();

	// Reset actions that cannot be requested while in ragdoll
	if (!CanJump())
		StopJumping();

	if (!CanCrouch())
		UnCrouch();

	if (!CanWalk())
		UnWalk();

	if (!CanSprint())
		UnSprint();

	if (!CanPerformGenericAction())
		UnPerformGenericAction();

	UExtCharacterMovementComponent* ExtCharacterMovement = GetExtCharacterMovement();
	check(ExtCharacterMovement);

	if (GetLocalRole() >= ROLE_AutonomousProxy)
	{
		// Clear the land timer if it is still on since it has no purpose anymore.
		if (IsLanding())
			CancelLanding();
	}
	
	// Ragdolls don't rotate
	ExtCharacterMovement->ResetTurnInPlaceState();
	ExtCharacterMovement->ResetControllerDesireRotationState();

	// Optionally ignore input
	if (Controller)
	{
		if (bIgnoreMoveInputWhenRagdoll)
			Controller->SetIgnoreMoveInput(true);

		if (bIgnoreLookInputWhenRagdoll)
			Controller->SetIgnoreLookInput(true);
	}

	K2_OnStartRagdoll();
	OnRagdollChanged();
	RagdollChangedDelegate.Broadcast(this);
}


/// Getting Up

void AExtCharacter::CancelGettingUp()
{
	check(GettingUpTimerHandle.IsValid());
	GetWorldTimerManager().ClearTimer(GettingUpTimerHandle);
	OnGettingUpCanceled();
}

void AExtCharacter::GettingUpTimer_OnTime()
{
	// This callback is called only once so we don't have to clear the timer but the timer manager
	// does not invalidate the handle automatically so we have to do it manually here.
	GettingUpTimerHandle.Invalidate();
	OnGettingUpComplete();
}

void AExtCharacter::OnGettingUpComplete()
{
	if (Controller)
	{
		if (bIgnoreMoveInputWhenRagdoll)
			Controller->SetIgnoreMoveInput(false);

		if (bIgnoreLookInputWhenRagdoll)
			Controller->SetIgnoreLookInput(false);
	}
}

void AExtCharacter::OnGettingUpCanceled()
{
	if (Controller)
	{
		if (bIgnoreMoveInputWhenRagdoll)
			Controller->SetIgnoreMoveInput(false);

		if (bIgnoreLookInputWhenRagdoll)
			Controller->SetIgnoreLookInput(false);
	}
}


/// General Getters & Setters

void AExtCharacter::SetGait(ECharacterGait NewGait)
{
	if (Gait != NewGait)
	{
		Gait = NewGait;
		OnGaitChanged();
		GaitChangedDelegate.Broadcast(this);
	}
}

// FVector AExtCharacter::GetAcceleration() const
// {
// 	if (GetRootComponent() && GetRootComponent()->IsSimulatingPhysics())
// 	{
// 		return GetRootComponent()->GetComponentVelocity().GetSafeNormal();
// 	}
// 
// 	const UCharacterMovementComponent* CharacterMovement = GetCharacterMovement();
// 	return CharacterMovement ? CharacterMovement->GetCurrentAcceleration() : FVector::ZeroVector;
// }

bool AExtCharacter::ServerSetLookAtActor_Validate(AActor* InActor)
{
	return true;
}

void AExtCharacter::ServerSetLookAtActor_Implementation(AActor* InActor)
{
	ReplicatedLookAtActor = InActor;
}

void AExtCharacter::SetLookAtActor(AActor* InActor)
{
	checkActorRoleAtLeast(ROLE_AutonomousProxy);

	ReplicatedLookAtActor = InActor;

	if (GetLocalRole() < ROLE_Authority)
		SetLookAtActor(InActor);
}

void AExtCharacter::SetRotationMode(ECharacterRotationMode Value)
{
	checkActorRoleAtLeast(ROLE_AutonomousProxy);

	if (RotationMode != Value)
	{
		RotationMode = Value;
		OnRotationModeChangedInternal();
	}
}

UPawnMovementComponent* AExtCharacter::GetMovementComponent() const
{
	// Full override to avoid unecessary component lookups
	FULL_OVERRIDE();

	return GetCharacterMovement();
}

#if WITH_EDITOR

void AExtCharacter::UpdateDebugComponentsVisibility()
{
	const bool bHiddenInGame = !bEnableDebugDraw;
	GetCapsuleComponent()->SetHiddenInGame(bHiddenInGame, false);

#if	WITH_EDITORONLY_DATA
	LookRotationArrow->SetHiddenInGame(bHiddenInGame);
	LookRotationYawArrow->SetHiddenInGame(bHiddenInGame);
	VelocityArrow->SetHiddenInGame(bHiddenInGame);
	AccelerationArrow->SetHiddenInGame(bHiddenInGame);
	LastVelocityArrow->SetHiddenInGame(bHiddenInGame);
	LastAccelerationArrow->SetHiddenInGame(bHiddenInGame);
#endif
}

#endif

#undef LOCTEXT_NAMESPACE
