// Fill out your copyright notice in the Description page of Project Settings.

#include "GameFramework/TopDownCharacter.h"
#include "GameFramework/TopDownPlayerController.h"
#include "GameFramework/ExtCharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/ArmComponent.h"
#include "Components/InputComponent.h"
#include "Components/PushToTargetComponent.h"
#include "Kismet/Kismet.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogTopDownCharacter, Log, All);

ATopDownCharacter::ATopDownCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

	// Structure to hold one-time initialization
	static const struct FConstructorStatics
	{
		FName NAME_CameraPivot;
		FName NAME_InteractInput;
		FName NAME_TauntInput;

		FConstructorStatics():
			NAME_CameraPivot(TEXT("CameraPivot")),
			NAME_InteractInput(TEXT("Interact")),
			NAME_TauntInput(TEXT("Taunt"))
		{
		}
	} ConstructorStatics;

	InteractInputName = ConstructorStatics.NAME_InteractInput;
	TauntInputName = ConstructorStatics.NAME_TauntInput;

	bStopWhenUnpossessed = true;

	UExtCharacterMovementComponent* ExtCharacterMovement = CastChecked<UExtCharacterMovementComponent>(GetCharacterMovement());
	ExtCharacterMovement->bCanWalkOffLedges = true;
	ExtCharacterMovement->bCanWalkOffLedgesWhenCrouching = true;
	ExtCharacterMovement->bCanWalkOffLedgesWhenSprinting = true;
	ExtCharacterMovement->bCanWalkOffLedgesWhenPerformingGenericAction = true;
	ExtCharacterMovement->bUseControllerDesiredRotation = true;
	ExtCharacterMovement->bOrientRotationToMovement = true;
	ExtCharacterMovement->bRequestedMoveUseAcceleration = true;

	MaxCameraDistanceNeutral = 150.0f;
	MaxCameraDistanceAiming = 400.0f;

	// Create camera pivot
	CameraBracket = CreateDefaultSubobject<UArmComponent>(ConstructorStatics.NAME_CameraPivot);
	CameraBracket->SetupAttachment(RootComponent);
	CameraBracket->TargetArmLength = 0.0f;
	CameraBracket->TargetOffset = FVector(-150.0f, 150.0f, 0.0f);
	CameraBracket->bUsePawnControlRotation = false;
	CameraBracket->bInheritPitch = false;
	CameraBracket->bInheritYaw = true;
	CameraBracket->bInheritRoll = false;
	CameraBracket->SetIsReplicated(false);

	bDrawDebugMarkers = false;

	// Mouse look
	bUseMouseToLook = true;
}

void ATopDownCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// PlayerInputComponent->BindAxis(LookRightInputName);
	// PlayerInputComponent->BindAxis(LookUpInputName);
	
	PlayerInputComponent->BindAction(InteractInputName, IE_Pressed, this, &ThisClass::PlayerInputInteract);
	PlayerInputComponent->BindAction(TauntInputName, IE_Pressed, this, &ThisClass::PlayerInputTaunt);
}

void ATopDownCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// If in a passive state update the TargetArmLength according to movement
//	if (!bIsAiming && !bIsFiring)
//		CameraBracket->TargetArmLength = (GetCharacterMovement()->Velocity.SizeSquared2D() > KINDA_SMALL_NUMBER) ? -MaxCameraDistanceNeutral : 0.0f;

#if ENABLE_DRAW_DEBUG
	if (bDrawDebugMarkers)
	{
		if (IsLocallyControlled())
		{
			const FVector TargetLocation = CameraBracket->GetTargetLocation();
			const float TargetLength = FMath::Abs(CameraBracket->TargetArmLength);

			DrawDebugSphere(GetWorld(), TargetLocation, 5.0f, 8, FColor::Silver);
			DrawDebugCircle(GetWorld(), TargetLocation, TargetLength, 128, FColor::Silver, false, -1.0f, 0, 0.0f, FVector::ForwardVector, FVector::RightVector, false);

			const FVector ActorLocation = GetActorLocation();
			const FVector ActorForward = GetActorForwardVector();

			DrawDebugDirectionalArrow(GetWorld(), TargetLocation, TargetLocation + ActorForward * FMath::Max(0.0f, TargetLength - 10.0f), 400.0f, FColor::Cyan);
		}
	}
#endif
}

void ATopDownCharacter::ProcessInput(float DeltaTime, bool bGamePaused)
{
	// Handle aiming state and look direction
	if (!bGamePaused && Controller && Controller->IsLocalPlayerController() && InputComponent && InputEnabled())
	{
		// If we're using the mouse to look we should be either aiming and/or firing.
		// Otherwise, we must be using an input controller (keyboard or gamepad) in which case the 
		// input itself is what triggers the aim.
		if (bUseMouseToLook)
		{
			if (bIsPerformingGenericAction)
			{
				FVector CursorLocation, CursorDirection;
				APlayerController* const PC = CastChecked<APlayerController>(Controller);
				if (PC->DeprojectMousePositionToWorld(CursorLocation, CursorDirection))
				{
					const FVector ActorLocation = GetActorLocation();
					float OutDistance;
					FVector Intersection;
					if (Kismet::Math::RayPlaneIntersection(CursorLocation, CursorDirection, FPlane(FVector::UpVector, ActorLocation.Z), OutDistance, Intersection))
					{
						const FVector DesiredDirection = Intersection - ActorLocation;

						PlayerInputLook(DesiredDirection);
					}
				}
			}
		}
		else
		{
			// const float LookRightValue = InputComponent->GetAxisValue(LookRightInputName);
			// const float LookUpValue = InputComponent->GetAxisValue(LookUpInputName);

			// if (LookRightValue == 0.0f && LookUpValue == 0.0f)
			// {
			// 	if (IsAiming())
			// 		UnAim();
			// }
			// else
			// {
			// 	if (IsAiming())
			// 	{
			// 		// X axis is forward, Y is right
			// 		const FVector InputDirection(LookUpValue, LookRightValue, 0.0f);
			// 
			// 		FVector ViewPointLocation;
			// 		FRotator ViewPointRotation;
			// 
			// 		APlayerController* const PC = CastChecked<APlayerController>(Controller);
			// 		PC->GetPlayerViewPoint(ViewPointLocation, ViewPointRotation);
			// 
			// 		const FRotationMatrix RotationMatrix(FRotator(0.0f, -ViewPointRotation.Yaw, 0.0f));
			// 		const FVector DesiredDirection = RotationMatrix.InverseTransformVector(InputDirection);
			// 
			// 		PlayerInputLook(DesiredDirection);
			// 		
			// 	}
			// 	else
			// 	{
			// 		Aim();
			// 	}
			// }
		}
	}
}

void ATopDownCharacter::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult)
{
	return (Controller) ? Controller->CalcCamera(DeltaTime, OutResult) : Super::CalcCamera(DeltaTime, OutResult);
}

bool ATopDownCharacter::HasActiveCameraComponent() const
{
	return (Controller) ? Controller->HasActiveCameraComponent() : Super::HasActiveCameraComponent();
}

bool ATopDownCharacter::HasActivePawnControlCameraComponent() const
{
	return (Controller) ? Controller->HasActiveCameraComponent() : Super::HasActiveCameraComponent();
}

void ATopDownCharacter::BecomeViewTarget(APlayerController* PC)
{
	Super::BecomeViewTarget(PC);

	// Setup player controller to track the camera bracket endpoint
	if (ATopDownPlayerController* TDPC = Cast<ATopDownPlayerController>(PC))
		TDPC->SetCameraTargetComponent(CameraBracket, UArmComponent::SocketName);
}

void ATopDownCharacter::EndViewTarget(APlayerController* PC)
{
	Super::EndViewTarget(PC);

	// Clean after ourselves
	if (ATopDownPlayerController* TDPC = Cast<ATopDownPlayerController>(PC))
		TDPC->SetCameraTargetComponent(nullptr);
}

void ATopDownCharacter::UnPossessed()
{
	Super::UnPossessed();

	CameraBracket->TargetArmLength = 0.0f;
}

void ATopDownCharacter::OnStartGenericAction()
{
	CameraBracket->TargetArmLength = -MaxCameraDistanceAiming;
	Super::OnStartGenericAction();
}


// void ATopDownCharacter::OnStartFire()
// {
// 	CameraBracket->TargetArmLength = -MaxCameraDistanceAiming
// 	Super::OnStartFire();
// }

void ATopDownCharacter::PlayerInputLook(const FVector& Direction)
{
	if (!Direction.IsNearlyZero() && Controller  && Controller->IsLocalPlayerController())
	{
		const FRotator DesiredRotation = Direction.ToOrientationRotator();
		const FRotator ControlRotation = GetControlRotation();

		// As we want to strictly look in the given direction, a common approach would be to call 
		// SetControlRotation(Direction.ToOrientationRotator()) BUT we also want the least ammount 
		// of impact in the normal update process of the control rotation. For example, we still 
		// want PlayerCameraManager->ProcessViewRotation to be called so that camera modifiers can 
		// be applied; we want Pawn->FaceDirection to be called and do its job; and we still want 
		// to honor PlayerController->IsLookInputIgnored. Our solution is to assign the rotation 
		// input directly since it's public but it would be way better if there was a virtual method 
		// in the PlayerController for this. It's stupid to have to call AddPitchInput, AddYawInput 
		// and AddRollInput separately. Unfotunately the consequence is that derived classes can't 
		// safely control how RotationInput is modified. For example, we had to be careful to check 
		// IsLookInputIgnored here but we could have easily missed it. In reality, this 
		// should be a concern of the PlayerController, not the caller.
		const FRotator DesiredRotationInput = DesiredRotation - ControlRotation;
		if (!DesiredRotationInput.IsNearlyZero())
		{
			APlayerController* const PC = CastChecked<APlayerController>(Controller);
			if (!PC->IsLookInputIgnored())
				PC->RotationInput = DesiredRotationInput;
		}
	}
}

void ATopDownCharacter::PlayerInputInteract()
{
	if (Controller  && Controller->IsLocalPlayerController())
	{
		Interact();
	}
}

void ATopDownCharacter::PlayerInputTaunt()
{
	if (Controller && Controller->IsLocalPlayerController())
	{
		Taunt();
	}
}

void ATopDownCharacter::Interact()
{
	if (Controller && Controller->IsLocalPlayerController())
	{
		SetRagdoll(true);
	}

	// if (Role == ROLE_AutonomousProxy)
	// 	Ragdoll();
	// DropDead();
}

void ATopDownCharacter::Taunt()
{
	if (Controller && Controller->IsLocalPlayerController())
	{
		SetRagdoll(false);
	}

	// if (Role == ROLE_AutonomousProxy)
	// 	UnRagdoll();
	// Revive();
}