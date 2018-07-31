// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UObject/ObjectMacros.h"
#include "GameFramework/PlayerController.h"

#include "ExtPlayerController.generated.h"

/** A set of parameters to describe how to transition between view targets. */
USTRUCT(BlueprintType)
struct TPCE_API FAutoManagedCameraTransitionParams
{
	GENERATED_USTRUCT_BODY()

	FAutoManagedCameraTransitionParams()
	: BlendTime(0.f),
  	  BlendFunction(VTBlend_Cubic),
	  BlendExp(2.f)
	{}


	/** Total duration of blend to pending view target. 0 means no blending. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ViewTargetTransitionParams)
	float BlendTime;

	/** Function to apply to the blend parameter. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ViewTargetTransitionParams)
	TEnumAsByte<enum EViewTargetBlendFunction> BlendFunction;

	/** Exponent, used by certain blend functions to control the shape of the curve. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ViewTargetTransitionParams)
	float BlendExp;

	operator FViewTargetTransitionParams() const
	{
		FViewTargetTransitionParams params;
		params.BlendTime = BlendTime;
		params.BlendFunction = BlendFunction;
		params.BlendExp = BlendExp;
		params.bLockOutgoing = true;

		return params;
	}
};


/**
 * Base class for player controllers with extended functionality.
 * This class exists basically to modify (or fix) the base APlayerController implementation 
 * without having to patch the engine. It also supports IPawnControlInterface to provide
 * extra features for Pawns.
 *
 */
UCLASS(Abstract)
class TPCE_API AExtPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	AExtPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:

	UPROPERTY(EditAnywhere, Category = Camera)
	FAutoManagedCameraTransitionParams AutoManagedCameraTransitionParams;

	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;
	virtual void AutoManageActiveCameraTarget(AActor* SuggestedTarget) override;

	/** Convert current mouse 2D position to World Space 3D position projected on to a plane. Returns false if unable to determine value. **/
	UFUNCTION(BlueprintCallable, Category = "Game|Player", meta = (DisplayName = "ProjectMouseLocationOnToPlane", Keywords = "deproject"))
	bool DeprojectMousePositionToPlane(FVector& WorldLocation, const FPlane& Plane) const;

	/** Convert 2D screen position to World Space 3D position projected on to a plane. Returns false if unable to determine value. **/
	UFUNCTION(BlueprintCallable, Category = "Game|Player", meta = (DisplayName = "ProjectScreenLocationOnToPlane", Keywords = "deproject"))
	bool DeprojectScreenPositionToPlane(float ScreenX, float ScreenY, FVector& WorldLocation, const FPlane& Plane) const;
};
