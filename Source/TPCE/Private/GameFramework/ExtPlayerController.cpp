// Fill out your copyright notice in the Description page of Project Settings.

#include "GameFramework/ExtPlayerController.h"

#include "GameFramework/Pawn.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Interfaces/PawnControlInterface.h"

#include "Logging/LogMacros.h"
#include "Kismet/Kismet.h"

#include "ExtraMacros.h" 

DEFINE_LOG_CATEGORY_STATIC(LogExtPlayerController, Log, All);

#define LOCTEXT_NAMESPACE "ExtPlayerController"

AExtPlayerController::AExtPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{	
	bAutoManageActiveCameraTarget = true;	// Auto set view target when a pawn is possessed/unpossessed

	AutoManagedCameraTransitionParams.BlendTime = 1.0f;
	AutoManagedCameraTransitionParams.BlendFunction = EViewTargetBlendFunction::VTBlend_Cubic;
	AutoManagedCameraTransitionParams.BlendExp = 2.0f;
}

void AExtPlayerController::AutoManageActiveCameraTarget(AActor* SuggestedTarget)
{
	// Full override to support smooth transitions when changing view target
	FULL_OVERRIDE();

	if (bAutoManageActiveCameraTarget)
	{
	// See if there is a CameraActor with an auto-activate index that matches us.
		if (GetNetMode() == NM_Client)
		{
			// Clients don't know their own index on the server, so they have to trust that if they use a camera with an auto-activate index, that's their own index.
			ACameraActor* CurrentCameraActor = Cast<ACameraActor>(GetViewTarget());
			if (CurrentCameraActor)
			{
				const int32 CameraAutoIndex = CurrentCameraActor->GetAutoActivatePlayerIndex();
				if (CameraAutoIndex != INDEX_NONE)
				{
					return;
				}
			}
		}
		else
		{
			// See if there is a CameraActor in the level that auto-activates for this PC.
			ACameraActor* AutoCameraTarget = GetAutoActivateCameraForPlayer();
			if (AutoCameraTarget)
			{
				SetViewTarget(AutoCameraTarget, AutoManagedCameraTransitionParams);
				return;
			}
		}

		// No auto-activate CameraActor, so use the suggested target.
		SetViewTarget(SuggestedTarget, AutoManagedCameraTransitionParams);
	}
}

bool AExtPlayerController::DeprojectMousePositionToPlane(FVector& WorldLocation, const FPlane& Plane = FPlane(FVector::UpVector, 0.0f)) const
{
	FVector Location, Direction;

	if (DeprojectMousePositionToWorld(Location, Direction))
	{
		float T; // ignored
		return Kismet::Math::RayPlaneIntersection(Location, Direction, Plane, T, WorldLocation);
	}

	return false;
}

bool AExtPlayerController::DeprojectScreenPositionToPlane(float ScreenX, float ScreenY, FVector& WorldLocation, const FPlane& Plane = FPlane(FVector::UpVector, 0.0f)) const
{
	FVector Location, Direction;

	if (DeprojectScreenPositionToWorld(ScreenX, ScreenY, Location, Direction))
	{
		float T; // ignored
		return Kismet::Math::RayPlaneIntersection(Location, Direction, Plane, T, WorldLocation);
	}

	return false;
}

void AExtPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (IPawnControlInterface* PawnControl = Cast<IPawnControlInterface>(GetPawnOrSpectator()))
	{
		PawnControl->ProcessInput(DeltaTime, bGamePaused);
	}

	Super::PostProcessInput(DeltaTime, bGamePaused);
}

#undef LOCTEXT_NAMESPACE