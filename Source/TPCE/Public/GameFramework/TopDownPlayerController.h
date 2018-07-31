// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ExtPlayerController.h"

#include "TopDownPlayerController.generated.h"

class UTopDownPushToTargetComponent;
class USphereComponent;
class UArmComponent;
class UCameraComponent;

/**
 * A player controller implementation for top-down games intended to possess a TopDownCharacter.
 *
 * @see TopDownCharacter
 */
UCLASS(Abstract, ShowCategories=(Collision))
class TPCE_API ATopDownPlayerController : public AExtPlayerController
{
	GENERATED_BODY()
	
public:

	ATopDownPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

private:

	/** Mounting point of the camera boom. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UTopDownPushToTargetComponent* CameraTractor;

	/** Camera mount used to support the camera boom. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USphereComponent* CameraMount;

	/** Camera boom to position the camera at a fixed distance above the character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UArmComponent* CameraBoom;

	/** Camera settings */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* CameraComponent;

protected:

	/** Force scene component to use absolute coordinates */
	void ForceAbsolute(USceneComponent* SceneComponent);

public:

	// /**
	//  * If true, camera lags behind target position to smooth its movement.
	//  * @see CameraLagSpeed
	//  */
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Lag")
	// uint32 bEnableCameraLag : 1;
	// 
	// /**
	//  * If bUseCameraLagSubstepping is true, sub-step camera damping so that it handles fluctuating frame rates well (though this comes at a cost).
	//  * @see CameraLagMaxTimeStep
	//  */
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Lag", AdvancedDisplay)
	// uint32 bUseCameraLagSubstepping : 1;
	// 
	// /** If bEnableCameraLag is true, controls how quickly camera reaches target position. Low values are slower (more lag), high values are faster (less lag), while zero is instant (no lag). */
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Lag", meta = (editcondition = "bEnableCameraLag", ClampMin = "0.0", ClampMax = "1000.0", UIMin = "0.0", UIMax = "1000.0"))
	// float CameraLagSpeed;
	// 
	// /** Max time step used when sub-stepping camera lag. */
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Lag", AdvancedDisplay, meta = (editcondition = "bUseCameraLagSubstepping", ClampMin = "0.005", ClampMax = "0.5", UIMin = "0.005", UIMax = "0.5"))
	// float CameraLagMaxTimeStep;

	virtual void PostInitializeComponents() override; 
	virtual void BeginPlay() override;
	virtual void CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult) override;
	virtual bool HasActiveCameraComponent() const override;
	virtual bool HasActivePawnControlCameraComponent() const override;
	virtual void PlayerTick(float DeltaTime) override;

	void SetCameraTargetComponent(USceneComponent* NewTargetComponent, const FName& SocketName = NAME_None);

	/** Camera mount used to support the camera boom and be moved by the camera trator. */
	FORCEINLINE USphereComponent* GetCameraMount() { return CameraMount; }

	/** Camera boom to position the camera at a fixed distance above the character. */
	FORCEINLINE UArmComponent* GetCameraBoom() { return CameraBoom; }

	/** Camera settings. */
	FORCEINLINE UCameraComponent* GetCamera() { return CameraComponent; }
	
	/** Camera tractor responsible for moving the camera mount towards the view target. */
	FORCEINLINE UTopDownPushToTargetComponent* GetCameraTractor() { return CameraTractor; }

};
