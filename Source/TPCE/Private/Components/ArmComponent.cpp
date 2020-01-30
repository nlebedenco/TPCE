// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "Components/ArmComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Math/MathExtensions.h"

const FName UArmComponent::SocketName(TEXT("Endpoint"));

UArmComponent::UArmComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	bAutoActivate = true;
	bTickInEditor = true;
	bWantsInitializeComponent = true;

	TargetArmLength = 300.0f;

	bUsePawnControlRotation = false;
	bInheritPitch = true;
	bInheritYaw = true;
	bInheritRoll = true;

	bEnableTargetLag = false;
	bEnableTargetRotationLag = false;
	bDrawDebugMarkers = false;
	bUseTargetLagSubstepping = false;
	TargetLagSpeed = 10.f;
	TargetRotationLagSpeed = 10.f;
	TargetLagMaxTimeStep = 1.f / 60.f;
	TargetLagMaxDistance = 0.f;

	RelativeSocketRotation = FQuat::Identity;
}

void UArmComponent::OnRegister()
{
	Super::OnRegister();

	// Enforce reasonable limits to avoid potential div-by-zero
	TargetLagMaxTimeStep = FMath::Max(TargetLagMaxTimeStep, 1.f / 200.f);
	TargetLagSpeed = FMath::Max(TargetLagSpeed, 0.f);

	// Set initial location (without lag).
	FRotator DesiredRotation = GetTargetRotation();
	PreviousSmoothedRotation = DesiredRotation;

	FVector DesiredLocation = GetTargetLocation();
	PreviousTargetLocation = DesiredLocation;
	PreviousSmoothedLocation = DesiredLocation;

	// Now offset Target position back along our rotation
	DesiredLocation -= CalcTargetArm(DesiredLocation, DesiredRotation, 0.0f);

	UpdateRelativeSocketCoordinates(DesiredRotation, DesiredLocation);
}

void UArmComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const float InverseTargetLagMaxTimeStep = 1.f / TargetLagMaxTimeStep;

	const FVector TargetLocation = GetTargetLocation();
	// We lag the desired location, not the actual Target position, so rotating the Target around does not have lag
	FVector DesiredLocation = TargetLocation;
	if (bEnableTargetLag)
	{
		FVector Previous = PreviousSmoothedLocation;
		if (bUseTargetLagSubstepping && DeltaTime > TargetLagMaxTimeStep && TargetLagSpeed > 0.f)
		{
			const FVector TargetMovementStep = (TargetLocation - PreviousTargetLocation) * (TargetLagMaxTimeStep / DeltaTime);

			FVector LerpTarget = PreviousTargetLocation;
			float RemainingTime = DeltaTime;
			while (RemainingTime > KINDA_SMALL_NUMBER)
			{
				const float LerpAmount = FMath::Min(TargetLagMaxTimeStep, RemainingTime);
				LerpTarget += TargetMovementStep * (LerpAmount * InverseTargetLagMaxTimeStep);
				RemainingTime -= LerpAmount;

				DesiredLocation = VInterpTo(Previous, LerpTarget, LerpAmount, TargetLagSpeed);
				Previous = DesiredLocation;
			}
		}
		else
		{
			DesiredLocation = VInterpTo(Previous, DesiredLocation, DeltaTime, TargetLagSpeed);
		}

		// Clamp distance if requested
		bool bClampedDist = false;
		if (TargetLagMaxDistance > 0.f)
		{
			const FVector FromOrigin = DesiredLocation - TargetLocation;
			if (FromOrigin.SizeSquared() > FMath::Square(TargetLagMaxDistance))
			{
				DesiredLocation = TargetLocation + FromOrigin.GetClampedToMaxSize(TargetLagMaxDistance);
				bClampedDist = true;
			}
		}

#if ENABLE_DRAW_DEBUG
		if (bDrawDebugMarkers)
		{
			DrawDebugSphere(GetWorld(), TargetLocation, 5.f, 8, FColor::Green);
			DrawDebugSphere(GetWorld(), DesiredLocation, 5.f, 8, FColor::Yellow);

			const FVector MiddlePoint = (DesiredLocation + TargetLocation) * 0.5f;
			DrawDebugDirectionalArrow(GetWorld(), DesiredLocation, MiddlePoint, 7.5f, bClampedDist ? FColor::Red : FColor::Green);
			DrawDebugDirectionalArrow(GetWorld(), MiddlePoint, TargetLocation, 7.5f, bClampedDist ? FColor::Red : FColor::Green);
		}
	}
	else
	{
		if (bDrawDebugMarkers)
		{
			DrawDebugSphere(GetWorld(), DesiredLocation, 5.f, 8, FColor::Yellow);
		}
#endif
	}

	PreviousSmoothedLocation = DesiredLocation;

	FRotator DesiredRotation = GetTargetRotation();
	// Apply 'lag' to rotation if desired
	if (bEnableTargetRotationLag)
	{
		if (bUseTargetLagSubstepping && DeltaTime > TargetLagMaxTimeStep && TargetRotationLagSpeed > 0.f)
		{
			const FRotator ArmRotStep = (DesiredRotation - PreviousSmoothedRotation).GetNormalized() * (TargetLagMaxTimeStep / DeltaTime);
			FRotator LerpTarget = PreviousSmoothedRotation;
			float RemainingTime = DeltaTime;
			while (RemainingTime > KINDA_SMALL_NUMBER)
			{
				const float LerpAmount = FMath::Min(TargetLagMaxTimeStep, RemainingTime);
				LerpTarget += ArmRotStep * (LerpAmount * InverseTargetLagMaxTimeStep);
				RemainingTime -= LerpAmount;

				DesiredRotation = RInterpTo(PreviousSmoothedRotation, LerpTarget, LerpAmount, TargetRotationLagSpeed);
				PreviousSmoothedRotation = DesiredRotation;
			}
		}
		else
		{
			DesiredRotation = RInterpTo(PreviousSmoothedRotation, DesiredRotation, DeltaTime, TargetRotationLagSpeed);
		}
	}

	PreviousSmoothedRotation = DesiredRotation;

	// Now calculate the socket location in world space
	DesiredLocation -= CalcTargetArm(TargetLocation, DesiredRotation, DeltaTime);

	PreviousTargetLocation = TargetLocation;

	UpdateRelativeSocketCoordinates(DesiredRotation, DesiredLocation);
}

void UArmComponent::ApplyWorldOffset(const FVector & InOffset, bool bWorldShift)
{
	Super::ApplyWorldOffset(InOffset, bWorldShift);
	PreviousSmoothedLocation += InOffset;
	PreviousTargetLocation += InOffset;
}

bool UArmComponent::HasAnySockets() const
{
	return true;
}

FTransform UArmComponent::GetSocketTransform(FName InSocketName, ERelativeTransformSpace TransformSpace) const
{
	if (InSocketName != ThisClass::SocketName)
		return Super::GetSocketTransform(InSocketName, TransformSpace);

	FTransform RelativeTransform(RelativeSocketRotation, RelativeSocketLocation);

	switch (TransformSpace)
	{
	case RTS_World:
		return RelativeTransform * GetComponentTransform();
	case RTS_Actor:
		if (const AActor* Actor = GetOwner())
		{
			FTransform SocketTransform = RelativeTransform * GetComponentTransform();
			return SocketTransform.GetRelativeTransform(Actor->GetTransform());
		}
		break;
	case RTS_Component:
		return RelativeTransform;
	}

	return RelativeTransform;
}

void UArmComponent::QuerySupportedSockets(TArray<FComponentSocketDescription>& OutSockets) const
{
	new (OutSockets) FComponentSocketDescription(ThisClass::SocketName, EComponentSocketType::Socket);
}

void UArmComponent::UpdateRelativeSocketCoordinates(const FRotator& DesiredRot, const FVector& DesiredLoc)
{
	// Form a transform for new world transform for Target
	FTransform WorldCamTM(DesiredRot, DesiredLoc);
	// Convert to relative to component
	FTransform RelCamTM = WorldCamTM.GetRelativeTransform(GetComponentTransform());

	// Update socket location/rotation
	RelativeSocketLocation = RelCamTM.GetLocation();
	RelativeSocketRotation = RelCamTM.GetRotation();

	UpdateChildTransforms();
}

FVector UArmComponent::CalcTargetArm(const FVector& Origin, const FRotator& Rotation, float DeltaTime)
{
	return FMath::IsNearlyZero(TargetArmLength) ? FVector::ZeroVector : Rotation.Vector() * TargetArmLength;
}

FVector UArmComponent::GetTargetLocation() const
{
	return GetComponentLocation() + TargetOffset;
}

FRotator UArmComponent::GetTargetRotation() const
{
	FRotator DesiredRotation = GetComponentRotation();

	if (bUsePawnControlRotation)
	{
		if (APawn* OwningPawn = Cast<APawn>(GetAttachmentRootActor()))
		{
			DesiredRotation = OwningPawn->GetViewRotation() + GetRelativeRotation();
		}
	}

	// If inheriting rotation, check options for which components to inherit
	if (!IsUsingAbsoluteRotation())
	{
		if (!bInheritPitch)
		{
			DesiredRotation.Pitch = GetRelativeRotation().Pitch;
		}

		if (!bInheritYaw)
		{
			DesiredRotation.Yaw = GetRelativeRotation().Yaw;
		}

		if (!bInheritRoll)
		{
			DesiredRotation.Roll = GetRelativeRotation().Roll;
		}
	}

	return DesiredRotation;
}

FVector UArmComponent::VInterpTo(const FVector& Current, const FVector& Target, float DeltaTime, float InterpSpeed)
{
	return FMathEx::VSafeInterpTo(Current, Target, DeltaTime, InterpSpeed);
}

FRotator UArmComponent::RInterpTo(const FRotator& Current, const FRotator& Target, float DeltaTime, float InterpSpeed)
{
	return FMathEx::RSafeInterpTo(Current, Target, DeltaTime, InterpSpeed);
}
